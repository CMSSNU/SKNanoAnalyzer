#!/usr/bin/env python3
"""Phase 0 benchmark for the SKNano merge rewrite.

Each case consumes a disjoint window of input shards so that the 211 GB page
cache on this node cannot make a later case look faster than it is.
"""

from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import subprocess
import sys
import time

SHARDS = Path(
    "/gv0/Users/yeonjoon/SKNanoOutput/HadronAnalyzer/LambdaCTraining/2024"
    "/TTLJ_powheg_CustomBPH"
)
BENCH = Path("/gv0/Users/yeonjoon/SKNanoMergeBench")


def shard_window(index: int, count: int) -> list[Path]:
    files = sorted(SHARDS.glob("hists_*.root"), key=lambda p: int(p.stem.split("_")[1]))
    start = index * count
    window = files[start:start + count]
    if len(window) != count:
        raise SystemExit(f"not enough shards for window {index}")
    return window


def timed(command: list[str]) -> dict:
    """Run a command and return its wall time and child resource usage."""
    import resource

    before = resource.getrusage(resource.RUSAGE_CHILDREN)
    started = time.time()
    completed = subprocess.run(command, capture_output=True, text=True)
    wall = time.time() - started
    after = resource.getrusage(resource.RUSAGE_CHILDREN)
    usage = {
        "wall_s": round(wall, 2),
        "user_s": round(after.ru_utime - before.ru_utime, 2),
        "sys_s": round(after.ru_stime - before.ru_stime, 2),
        "max_rss_mb": round(after.ru_maxrss / 1024, 1),
        "returncode": completed.returncode,
    }
    usage["cpu_frac"] = round((usage["user_s"] + usage["sys_s"]) / max(wall, 1e-9), 3)
    if completed.returncode != 0:
        usage["stderr_tail"] = completed.stderr[-2000:]
    return usage


def key_report(path: Path) -> dict:
    """Describe the output the way the historical -n corruption would show up."""
    import ROOT

    ROOT.gROOT.SetBatch(True)
    root_file = ROOT.TFile.Open(str(path), "READ")
    if not root_file or root_file.IsZombie():
        return {"readable": False}
    zombie = bool(root_file.TestBit(ROOT.TFile.kRecovered))
    names: list[str] = []
    cycles: list[int] = []
    hist_integral: dict[str, float] = {}
    ntuple_entries: dict[str, int] = {}

    def visit(directory, prefix: str = "") -> None:
        for key in directory.GetListOfKeys():
            name, class_name = str(key.GetName()), str(key.GetClassName())
            full = f"{prefix}{name}"
            names.append(full)
            cycles.append(int(key.GetCycle()))
            klass = ROOT.TClass.GetClass(class_name)
            if "RNTuple" in class_name:
                reader = ROOT.RNTupleReader.Open(full, str(path))
                ntuple_entries[full] = int(reader.GetNEntries())
            elif klass and klass.InheritsFrom(ROOT.TDirectory.Class()):
                visit(directory.GetDirectory(name), full + "/")
            elif klass and klass.InheritsFrom(ROOT.TH1.Class()):
                obj = key.ReadObj()
                hist_integral[full] = float(obj.Integral()) if obj else None

    visit(root_file)
    root_file.Close()
    return {
        "readable": True,
        "recovered": zombie,
        "n_keys": len(names),
        "n_unique_names": len(set(names)),
        "duplicate_names": sorted({n for n in names if names.count(n) > 1})[:10],
        "max_cycle": max(cycles) if cycles else 0,
        "hist_integral": hist_integral,
        "ntuple_entries": ntuple_entries,
    }


def expected_totals(inputs: list[Path]) -> dict:
    import ROOT

    ROOT.gROOT.SetBatch(True)
    hist_integral: dict[str, float] = {}
    ntuple_entries: dict[str, int] = {}
    for path in inputs:
        root_file = ROOT.TFile.Open(str(path), "READ")

        def visit(directory, prefix: str = "") -> None:
            for key in directory.GetListOfKeys():
                name, class_name = str(key.GetName()), str(key.GetClassName())
                full = f"{prefix}{name}"
                klass = ROOT.TClass.GetClass(class_name)
                if "RNTuple" in class_name:
                    reader = ROOT.RNTupleReader.Open(full, str(path))
                    ntuple_entries[full] = ntuple_entries.get(full, 0) + int(reader.GetNEntries())
                elif klass and klass.InheritsFrom(ROOT.TDirectory.Class()):
                    visit(directory.GetDirectory(name), full + "/")
                elif klass and klass.InheritsFrom(ROOT.TH1.Class()):
                    obj = key.ReadObj()
                    hist_integral[full] = hist_integral.get(full, 0.0) + float(obj.Integral())

        visit(root_file)
        root_file.Close()
    return {"hist_integral": hist_integral, "ntuple_entries": ntuple_entries}


CASES = {
    "hadd_fk404": lambda out, ins: ["hadd", "-fk404", "-v", "0", str(out)] + [str(p) for p in ins],
    "hadd_ff": lambda out, ins: ["hadd", "-ff", "-v", "0", str(out)] + [str(p) for p in ins],
    "hadd_f404": lambda out, ins: ["hadd", "-f404", "-v", "0", str(out)] + [str(p) for p in ins],
    "hadd_n8": lambda out, ins: ["hadd", "-fk404", "-v", "0", "-n", "8", str(out)] + [str(p) for p in ins],
}


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--case", required=True, choices=sorted(CASES) + ["ioceiling", "histtrack"])
    parser.add_argument("--window", type=int, required=True, help="disjoint shard window index")
    parser.add_argument("--count", type=int, default=24)
    parser.add_argument("--keep", action="store_true")
    args = parser.parse_args()

    BENCH.mkdir(parents=True, exist_ok=True)
    inputs = shard_window(args.window, args.count)
    input_bytes = sum(p.stat().st_size for p in inputs)
    result = {
        "case": args.case,
        "window": args.window,
        "n_inputs": len(inputs),
        "input_gb": round(input_bytes / 1e9, 2),
    }

    if args.case == "ioceiling":
        for ways in (1, 4, 8):
            subset = inputs[:8]
            started = time.time()
            procs = []
            chunk = max(1, len(subset) // ways)
            for i in range(0, len(subset), chunk):
                group = subset[i:i + chunk]
                procs.append(subprocess.Popen(
                    ["bash", "-c", "cat " + " ".join(f"'{p}'" for p in group) + " > /dev/null"]))
            for proc in procs:
                proc.wait()
            elapsed = time.time() - started
            read = sum(p.stat().st_size for p in subset)
            result[f"read_{ways}way_mbps"] = round(read / elapsed / 1e6, 1)
        print(json.dumps(result, indent=2))
        return 0

    if args.case == "histtrack":
        started = time.time()
        totals = expected_totals(inputs)
        result["wall_s"] = round(time.time() - started, 2)
        result["per_shard_s"] = round((time.time() - started) / len(inputs), 3)
        result["totals"] = {k: len(v) for k, v in totals.items()}
        print(json.dumps(result, indent=2))
        return 0

    output = BENCH / f"{args.case}_w{args.window}.root"
    output.unlink(missing_ok=True)
    result["usage"] = timed(CASES[args.case](output, inputs))
    if output.exists():
        result["output_gb"] = round(output.stat().st_size / 1e9, 2)
        result["size_ratio"] = round(output.stat().st_size / input_bytes, 4)
        result["output"] = key_report(output)
        result["expected"] = expected_totals(inputs)
        got, want = result["output"], result["expected"]
        result["hist_match"] = got.get("hist_integral") == want["hist_integral"]
        result["ntuple_match"] = got.get("ntuple_entries") == want["ntuple_entries"]
        if not args.keep:
            output.unlink()
    print(json.dumps(result, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
