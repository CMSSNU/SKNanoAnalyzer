#!/usr/bin/env python3
"""Build and send the final SKNano DAG report."""

from __future__ import annotations

import argparse
import datetime as dt
import json
import os
import re
from collections import Counter
from pathlib import Path
from typing import Iterable, Optional

from telegram_reporter import send_telegram_message


STATUS_RE = re.compile(r"DAG status:\s*(\d+)\s*\((DAG_STATUS_[A-Z_]+)\)")
TOTAL_RE = re.compile(r"Total number of Nodes:\s*(\d+)", re.IGNORECASE)
FAILED_RE = re.compile(r"Nodes that failed:\s*(\d+)", re.IGNORECASE)
MEMORY_RE = re.compile(r"\bMemoryUsage\s*=\s*([0-9.]+)", re.IGNORECASE)

ERROR_PATTERNS = {
    "Memory limit / OOM": re.compile(
        r"bad_alloc|out of memory|memory limit|oom[- ]kill|killed process|exceeded.*memory",
        re.IGNORECASE,
    ),
    "Input or storage access": re.compile(
        r"xrootd|tfile.*zombie|failed to open|cannot open|no such file|input/output error|operation expired",
        re.IGNORECASE,
    ),
    "Analyzer exception or crash": re.compile(
        r"traceback|terminate called|uncaught exception|fatal error|segmentation|segfault|abort(?:ed)?",
        re.IGNORECASE,
    ),
    "Merge validation failure": re.compile(
        r"sknano-merge:|repeated object keys|failed to unzip|r__unzip_header|"
        r"merged .*(?:entry counts|schema) differ|staged partials lost entries",
        re.IGNORECASE,
    ),
}


def _latest(paths: Iterable[Path]) -> Optional[Path]:
    paths = list(paths)
    return max(paths, key=lambda path: path.stat().st_mtime) if paths else None


def parse_rescue(rescue_file: Path) -> dict:
    text = rescue_file.read_text(encoding="utf-8", errors="replace")
    total_match = TOTAL_RE.search(text)
    failed_match = FAILED_RE.search(text)
    failed_nodes = []

    lines = text.splitlines()
    for index, line in enumerate(lines):
        if FAILED_RE.search(line) and index + 1 < len(lines):
            node_lines = []
            for candidate in lines[index + 1 :]:
                candidate = candidate.lstrip("# ")
                if "ENDLIST" in candidate:
                    node_lines.append(candidate.split("ENDLIST", 1)[0])
                    break
                if candidate and not candidate.startswith("Rescue DAG"):
                    node_lines.append(candidate)
            node_line = ",".join(node_lines)
            failed_nodes = [
                node.strip()
                for node in node_line.split(",")
                if node.strip() and "ENDLIST" not in node
            ]
            break

    return {
        "path": rescue_file,
        "total": int(total_match.group(1)) if total_match else 0,
        "failed": int(failed_match.group(1)) if failed_match else len(failed_nodes),
        "failed_nodes": failed_nodes,
    }


def read_dag_status(dag_dir: Path) -> tuple[Optional[int], Optional[str]]:
    preferred = dag_dir / "dagfile.dag.dagman.out"
    preferred_match = None
    fallback_match = None
    logs = list(dag_dir.glob("*.dagman.out"))
    for path in logs:
        try:
            matches = list(
                STATUS_RE.finditer(path.read_text(encoding="utf-8", errors="replace"))
            )
        except OSError:
            continue
        for match in matches:
            if int(match.group(1)) == 4 or match.group(2) == "DAG_STATUS_RM":
                return 4, "DAG_STATUS_RM"
        if matches:
            fallback_match = matches[-1]
            if path == preferred:
                preferred_match = matches[-1]
    selected = preferred_match or fallback_match
    if not selected:
        return None, None
    return int(selected.group(1)), selected.group(2)


def scheduler_metrics(dag_dir: Path) -> dict:
    held = 0
    evicted = 0
    peak_memory_mb = 0.0
    hold_reasons = Counter()
    log_paths = list(dag_dir.glob("*.nodes.log")) + list(dag_dir.glob("*.dagman.out"))
    for path in log_paths:
        try:
            text = path.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        held += len(re.findall(r"Job was held", text, re.IGNORECASE))
        evicted += len(re.findall(r"Job was evicted", text, re.IGNORECASE))
        for match in MEMORY_RE.finditer(text):
            peak_memory_mb = max(peak_memory_mb, float(match.group(1)))
        for match in re.finditer(r"HoldReason\s*=\s*\"([^\"]+)\"", text):
            hold_reasons[match.group(1)] += 1
    return {
        "held": held,
        "evicted": evicted,
        "peak_memory_mb": peak_memory_mb,
        "hold_reasons": hold_reasons,
    }


def _tail(path: Path, size: int = 32768) -> str:
    try:
        with path.open("rb") as handle:
            handle.seek(0, os.SEEK_END)
            length = handle.tell()
            handle.seek(max(0, length - size))
            return handle.read().decode("utf-8", errors="replace")
    except OSError:
        return ""


def classify_errors(manifest: dict, max_files: int = 2000) -> tuple[Counter, list[str]]:
    categories = Counter()
    examples = []
    seen = set()
    inspected = 0
    for sample in manifest.get("samples", []):
        workdir = Path(sample.get("working_dir", ""))
        if not workdir.is_dir():
            continue
        error_files = list(workdir.glob("job_*.err"))
        error_files.extend(workdir.glob("merge_*.err"))
        error_files.extend([
            workdir / "hadd.err",
            workdir / "move.err",
            workdir / "postproc.err",
        ])
        for path in error_files:
            if inspected >= max_files:
                return categories, examples
            inspected += 1
            if not path.is_file() or path.stat().st_size == 0:
                continue
            text = _tail(path)
            matched = False
            for label, pattern in ERROR_PATTERNS.items():
                if pattern.search(text):
                    categories[label] += 1
                    matched = True
            if not matched:
                continue
            if len(examples) >= 3:
                continue
            lines = [line.strip() for line in text.splitlines() if line.strip()]
            if lines:
                matching_lines = [
                    line
                    for line in lines
                    if any(pattern.search(line) for pattern in ERROR_PATTERNS.values())
                ]
                excerpt = matching_lines[-1] if matching_lines else lines[-1]
                excerpt = re.sub(r"https://api\.telegram\.org/bot[^/\s]+", "[REDACTED]", excerpt)
                excerpt = excerpt[:240]
                key = (path.name, excerpt)
                if key not in seen:
                    examples.append(f"{path.parent.name}/{path.name}: {excerpt}")
                    seen.add(key)
    return categories, examples


def _duration(created_at: Optional[str]) -> str:
    if not created_at:
        return "unknown"
    try:
        start = dt.datetime.fromisoformat(created_at)
        elapsed = max(dt.timedelta(), dt.datetime.now(start.tzinfo) - start)
    except (TypeError, ValueError):
        return "unknown"
    seconds = int(elapsed.total_seconds())
    hours, remainder = divmod(seconds, 3600)
    minutes, seconds = divmod(remainder, 60)
    if hours:
        return f"{hours}h {minutes}m"
    if minutes:
        return f"{minutes}m {seconds}s"
    return f"{seconds}s"


def build_report(master_dir: Path) -> str:
    manifest_path = master_dir / "run_manifest.json"
    with manifest_path.open(encoding="utf-8") as handle:
        manifest = json.load(handle)

    dag_dir = master_dir / "dags"
    rescue_path = _latest(dag_dir.glob("dagfile.dag.rescue*"))
    rescue = parse_rescue(rescue_path) if rescue_path else None
    status_code, status_name = read_dag_status(dag_dir)
    metrics = scheduler_metrics(dag_dir)

    samples = manifest.get("samples", [])
    expected_jobs = sum(int(sample.get("jobs", 0)) for sample in samples)
    outputs = [Path(sample["output"]) for sample in samples if sample.get("output")]
    existing_outputs = sum(path.exists() for path in outputs)

    failed = rescue["failed"] if rescue else 0
    if status_code == 4 or status_name == "DAG_STATUS_RM":
        state = "CANCELLED"
    elif status_code == 0:
        state = "COMPLETED"
    elif status_code is not None and status_code != 0:
        state = "FAILED"
    elif existing_outputs == len(outputs):
        state = "COMPLETED"
    elif failed:
        state = "FAILED"
    else:
        state = "FINISHED WITH UNKNOWN STATUS"
    active_rescue = rescue if state in {"FAILED", "CANCELLED"} else None
    categories, examples = (
        classify_errors(manifest) if active_rescue or state == "FAILED" else (Counter(), [])
    )

    options = manifest.get("options", {})
    eras = sorted({str(sample.get("era")) for sample in samples if sample.get("era")})
    lines = [
        f"SKNano job {state.lower()}",
        "",
        f"Analyzer: {options.get('Analyzer') or 'unknown'}",
        f"Era: {', '.join(eras) or 'unknown'}",
        f"Samples: {len(samples)}",
        f"Analyzer jobs requested: {expected_jobs}",
        f"Final outputs found: {existing_outputs}/{len(outputs)}",
        f"Elapsed time: {_duration(manifest.get('created_at'))}",
    ]

    if active_rescue:
        succeeded = max(0, active_rescue["total"] - active_rescue["failed"])
        success_rate = 100.0 * succeeded / active_rescue["total"] if active_rescue["total"] else 0.0
        lines.extend(
            [
                f"DAG nodes: {succeeded}/{active_rescue['total']} succeeded ({success_rate:.1f}%)",
                f"Failed DAG nodes: {active_rescue['failed']}",
            ]
        )

    anomaly_lines = []
    if metrics["held"]:
        anomaly_lines.append(f"Held events: {metrics['held']}")
    if metrics["evicted"]:
        anomaly_lines.append(f"Evictions/restarts: {metrics['evicted']}")
    if metrics["peak_memory_mb"]:
        anomaly_lines.append(f"Peak reported memory: {metrics['peak_memory_mb']:.0f} MB")
    requested_memory = float(options.get("Memory") or 0)
    if requested_memory and metrics["peak_memory_mb"] >= requested_memory * 0.9:
        anomaly_lines.append(
            f"Memory approached/exceeded the initial {requested_memory:.0f} MB request"
        )
    if metrics["hold_reasons"]:
        reason, count = metrics["hold_reasons"].most_common(1)[0]
        anomaly_lines.append(f"Most common hold reason ({count}): {reason[:180]}")
    if anomaly_lines:
        lines.extend(["", "Scheduler observations:", *[f"- {item}" for item in anomaly_lines]])

    if categories:
        lines.extend(["", "Detected failure signatures:"])
        lines.extend(f"- {name}: {count} file(s)" for name, count in categories.most_common())
    if examples:
        lines.extend(["", "Representative error lines:"])
        lines.extend(f"- {example}" for example in examples)

    if active_rescue and active_rescue["failed_nodes"]:
        lines.extend(["", "Failed nodes:"])
        lines.extend(f"- {node}" for node in active_rescue["failed_nodes"][:10])
        if len(active_rescue["failed_nodes"]) > 10:
            lines.append(f"- ... and {len(active_rescue['failed_nodes']) - 10} more")

    lines.extend(["", f"Master directory: {master_dir}"])
    if outputs:
        try:
            output_dir = os.path.commonpath([str(path.parent) for path in outputs])
        except ValueError:
            output_dir = str(outputs[0].parent)
        lines.append(f"Output directory: {output_dir}")
    if active_rescue and active_rescue["failed"]:
        dag_file = manifest.get("submit", {}).get("dag_file") or str(dag_dir / "finaldag.dag")
        lines.extend(
            [
                f"Latest rescue file: {active_rescue['path']}",
                "Retry from the DAG directory:",
                f"condor_submit_dag -force {dag_file}",
            ]
        )
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description="Report final SKNano DAG status to Telegram")
    parser.add_argument("--master-dir", "--master_dir", dest="master_dir", required=True)
    parser.add_argument("--print-only", action="store_true", help="Print the report without sending it")
    args = parser.parse_args()

    master_dir = Path(args.master_dir).resolve()
    try:
        report = build_report(master_dir)
    except (OSError, ValueError, json.JSONDecodeError) as exc:
        report = "\n".join(
            [
                "SKNano job finished, but its report could not be generated",
                "",
                f"Master directory: {master_dir}",
                f"Reporter error: {exc}",
            ]
        )

    if args.print_only:
        print(report)
        return 0

    config_path = master_dir / "source_snapshot" / "config" / f"config.{os.environ.get('USER', '')}"
    if not config_path.exists():
        config_path = Path(os.environ.get("SKNANO_HOME", "")) / "config" / f"config.{os.environ.get('USER', '')}"
    send_telegram_message(report, config_path=str(config_path))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
