#!/usr/bin/env python3
"""Validated, atomic merger for SKNano ROOT/RNTuple output shards.

Merging is I/O-latency bound, not CPU bound: a single ``hadd`` process digests
about 68 MB/s of input while /gv0 sustains ~890 MB/s.  Throughput therefore
comes from running several *independent* ``hadd`` processes, each with its own
disjoint inputs and its own output file.  See ``docs/MergePerformance.md``.

Two ROOT 6.40.02 merge paths are known to corrupt output and must never be
used here; see ``_run_hadd`` and ``check_key_integrity``.
"""

from __future__ import annotations

import argparse
import concurrent.futures
import hashlib
import json
import math
import multiprocessing
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import uuid
from typing import NamedTuple

MAX_BATCH_SIZE = 100


class Dataset(NamedTuple):
    entries: int
    schema: tuple[tuple[str, str], ...]


class Histogram(NamedTuple):
    class_name: str
    dimension: int
    cells: int
    entries: float


class KeyIdentity(NamedTuple):
    directory: str
    name: str
    cycle: int


class FileSummary(NamedTuple):
    datasets: dict[str, Dataset]
    histograms: dict[str, Histogram]
    keys: tuple[KeyIdentity, ...]
    others: tuple[str, ...]


def _root_module():
    try:
        import ROOT  # type: ignore
    except ImportError as error:
        raise RuntimeError("PyROOT is required to validate merge inputs") from error
    ROOT.gROOT.SetBatch(True)
    return ROOT


def _open_readable(root, file_path: Path):
    root_file = root.TFile.Open(str(file_path), "READ")
    if not root_file or root_file.IsZombie() or root_file.TestBit(root.TFile.kRecovered):
        raise RuntimeError(f"invalid or recovered ROOT file: {file_path}")
    return root_file


def _schema_of(root, ntuple) -> tuple[int, tuple[tuple[str, str], ...]]:
    reader = root.RNTupleReader.Open(ntuple)
    descriptor = reader.GetDescriptor()
    schema: list[tuple[str, str]] = []

    def add_field(field) -> None:
        schema.append(
            (str(descriptor.GetQualifiedFieldName(field.GetId())),
             str(field.GetTypeName()))
        )
        for child in descriptor.GetFieldIterable(field):
            add_field(child)

    for field in descriptor.GetTopLevelFields():
        add_field(field)
    return int(reader.GetNEntries()), tuple(sorted(schema))


def _walk_keys(root, root_file) -> list[tuple[str, str, str, object, int]]:
    """Cheap pass over the key tree: identity and class only, no ReadObj."""
    found: list[tuple[str, str, str, object, int]] = []

    def visit(directory, prefix: str = "") -> None:
        for key in directory.GetListOfKeys():
            name = str(key.GetName())
            class_name = str(key.GetClassName())
            found.append((prefix, name, class_name, directory, int(key.GetCycle())))
            if "RNTuple" in class_name:
                continue
            klass = root.TClass.GetClass(class_name)
            if klass and klass.InheritsFrom(root.TDirectory.Class()):
                child = directory.GetDirectory(name)
                if child:
                    visit(child, f"{prefix}/{name}" if prefix else name)

    visit(root_file)
    return found


def inspect(root, file_path: Path) -> FileSummary:
    """Describe one file in a single open: datasets, histograms, and key identity.

    The previous implementation opened every shard twice and then reopened it
    once more per RNTuple.  One open is enough and NFS round trips dominate.

    Key identity is collected and checked before anything is deserialized, so a
    chunked-merge corruption is reported as such instead of surfacing as an
    unzip failure on whichever object happens to be read first.
    """
    root_file = _open_readable(root, file_path)
    walk = _walk_keys(root, root_file)
    keys = tuple(
        KeyIdentity(prefix, name, cycle) for prefix, name, _, _, cycle in walk
    )
    check_key_integrity(keys, file_path)

    datasets: dict[str, Dataset] = {}
    histograms: dict[str, Histogram] = {}
    others: list[str] = []
    for prefix, name, class_name, directory, _ in walk:
        path = f"{prefix}/{name}" if prefix else name
        if "RNTuple" in class_name:
            # ROOT::RNTuple is not a TObject, so TKey::ReadObj() segfaults on
            # it.  TDirectory::Get returns the correctly typed object.
            entries, schema = _schema_of(root, directory.Get(name))
            datasets[path] = Dataset(entries, schema)
            continue
        klass = root.TClass.GetClass(class_name)
        if klass and klass.InheritsFrom(root.TDirectory.Class()):
            continue
        if klass and klass.InheritsFrom(root.TH1.Class()):
            histogram = directory.Get(name)
            if not histogram:
                raise RuntimeError(f"cannot read histogram {path} from {file_path}")
            histograms[path] = Histogram(
                class_name,
                int(histogram.GetDimension()),
                int(histogram.GetNcells()),
                float(histogram.GetEntries()),
            )
            continue
        others.append(path)

    root_file.Close()
    return FileSummary(datasets, histograms, keys, tuple(others))


def check_key_integrity(keys: tuple[KeyIdentity, ...], file_path: Path) -> None:
    """Reject the corruption signature that hadd's excess-file path produces.

    Reproduced on ROOT 6.40.02 (2026-08-22): ``hadd -n 8`` over 24 shards
    yields a file that opens cleanly and is *not* flagged kRecovered, but
    carries ``LambdaCTriplets;1..;3`` and ``CutFlow;1..;2`` -- one key cycle per
    internal chunk -- with the highest RNTuple cycle unreadable
    ("R__unzip_header: error in header").  Duplicate names and cycles above one
    are therefore the tripwire, and they cost nothing to check.
    """
    seen: dict[tuple[str, str], int] = {}
    duplicates: list[str] = []
    high_cycles: list[str] = []
    for key in keys:
        location = f"{key.directory}/{key.name}" if key.directory else key.name
        if key.cycle > 1:
            high_cycles.append(f"{location};{key.cycle}")
        if (key.directory, key.name) in seen:
            duplicates.append(location)
        seen[(key.directory, key.name)] = key.cycle
    if duplicates or high_cycles:
        raise RuntimeError(
            f"merge output carries repeated object keys in {file_path}: "
            f"duplicate_names={sorted(set(duplicates))[:10]}, "
            f"cycles_above_one={sorted(set(high_cycles))[:10]}; this is the "
            f"signature of ROOT's chunked-merge corruption"
        )


def _process_pool(workers: int):
    """A spawn pool whose children can import this module.

    ``spawn`` hands the child our ``sys.path``, so putting this file's directory
    on it keeps the workers importable even when the module was loaded by path
    rather than by name (as the tests do).
    """
    directory = str(Path(__file__).resolve().parent)
    if directory not in sys.path:
        sys.path.insert(0, directory)
    return concurrent.futures.ProcessPoolExecutor(
        max_workers=workers, mp_context=multiprocessing.get_context("spawn"))


def _inspect_worker(file_path: str) -> FileSummary:
    return inspect(_root_module(), Path(file_path))


def _inspect_many(root, paths: list[Path], jobs: int) -> list[FileSummary]:
    """Inspect files, spreading the NFS round trips over ``jobs`` processes."""
    # Each worker pays a fresh ROOT import, so stay serial for small sets.
    if jobs <= 1 or len(paths) < 16:
        return [inspect(root, path) for path in paths]
    with _process_pool(min(jobs, len(paths))) as pool:
        return list(pool.map(_inspect_worker, [str(path) for path in paths]))


def combine(summaries: list[FileSummary], labels: list[Path]) -> tuple[
    dict[str, tuple[tuple[str, str], ...]],
    dict[str, int],
    dict[str, tuple[str, int, int]],
    dict[str, float],
]:
    schemas: dict[str, tuple[tuple[str, str], ...]] = {}
    entries: dict[str, int] = {}
    histogram_schemas: dict[str, tuple[str, int, int]] = {}
    histogram_entries: dict[str, float] = {}
    for summary, label in zip(summaries, labels):
        check_key_integrity(summary.keys, label)
        changed = sorted(
            name for name, data in summary.datasets.items()
            if name in schemas and schemas[name] != data.schema
        )
        if changed:
            raise RuntimeError(f"RNTuple schema mismatch in {label}: changed={changed}")
        for name, data in summary.datasets.items():
            schemas[name] = data.schema
            entries[name] = entries.get(name, 0) + data.entries
        shard_hist_schemas = {
            name: (hist.class_name, hist.dimension, hist.cells)
            for name, hist in summary.histograms.items()
        }
        changed = sorted(
            name for name, value in shard_hist_schemas.items()
            if name in histogram_schemas and histogram_schemas[name] != value
        )
        if changed:
            raise RuntimeError(f"histogram schema mismatch in {label}: changed={changed}")
        histogram_schemas.update(shard_hist_schemas)
        for name, hist in summary.histograms.items():
            histogram_entries[name] = histogram_entries.get(name, 0.0) + hist.entries
    return schemas, entries, histogram_schemas, histogram_entries


def _run_hadd(
    output: Path,
    inputs: list[Path],
    temp_dir: Path,
    cache_size: str,
) -> None:
    """Run one bounded, strictly sequential hadd invocation.

    Two options are banned outright, both reproduced as data loss on this ROOT:

    ``-n`` drives hadd's excess-file path, which re-enters the output file and
    emits one key cycle per chunk; the resulting RNTuple fails to decompress
    (see ``check_key_integrity``).  The caller must instead keep ``inputs``
    below ``MAX_BATCH_SIZE`` and stage explicitly.

    ``-j`` selects hadd's multiprocess path even at ``-j 1``, and ROOT may
    remove the worker-produced target before this process can validate it.
    Concurrency belongs to the caller, which runs whole hadd processes over
    disjoint inputs.
    """
    if len(inputs) > MAX_BATCH_SIZE:
        raise RuntimeError(
            f"refusing to hadd {len(inputs)} inputs at once; the caller must "
            f"stage in batches of at most {MAX_BATCH_SIZE}"
        )
    command = ["hadd", "-fk404", "-v", "0"]
    if cache_size and cache_size != "0":
        command.extend(["-cachesize", cache_size])
    command.append(str(output))
    command.extend(str(path) for path in inputs)
    environment = os.environ.copy()
    environment["TMPDIR"] = str(temp_dir)
    subprocess.run(command, check=True, env=environment)


def _staged_hadd(
    root,
    inputs: list[Path],
    output: Path,
    jobs: int,
    batch_size: int,
    temp_dir: Path,
    cache_size: str,
    batch_cache_size: str,
    expected_entries: dict[str, int],
) -> None:
    """Merge through explicitly bounded and validated intermediate files.

    Each stage runs its batches as independent hadd processes.  They share no
    output file, so this is the one form of concurrency the ROOT bugs above do
    not touch, and it scales close to linearly: 4.0x on four processes and 5.3x
    on eight, measured on /gv0.
    """
    current = inputs
    stage = 0
    while len(current) > batch_size:
        # Staging is already unavoidable at this size, so make the batches small
        # enough to keep every worker busy rather than leaving most idle.
        width = min(batch_size, max(2, math.ceil(len(current) / jobs)))
        batches = [
            current[start:start + width]
            for start in range(0, len(current), width)
        ]
        workers = max(1, min(jobs, len(batches)))
        print(
            f"Merge stage {stage + 1}: {len(current)} inputs -> "
            f"{len(batches)} validated partials on {workers} processes",
            flush=True,
        )
        partials = [
            temp_dir / f"stage_{stage:02d}_{index:05d}.root"
            for index in range(len(batches))
        ]
        with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
            list(pool.map(
                lambda pair: _run_hadd(pair[0], pair[1], temp_dir, batch_cache_size),
                list(zip(partials, batches)),
            ))
        _validate_partials(root, partials, expected_entries, jobs)
        current = partials
        stage += 1

    print(f"Final merge stage: {len(current)} inputs -> {output}", flush=True)
    _run_hadd(output, current, temp_dir, cache_size)


def _validate_partials(
    root, partials: list[Path], expected: dict[str, int], jobs: int
) -> None:
    """Fully deserialize staged partials before they may feed the next stage."""
    summaries = _inspect_many(root, partials, jobs)
    for summary, path in zip(summaries, partials):
        check_key_integrity(summary.keys, path)
    produced: dict[str, int] = {}
    for summary in summaries:
        for name, data in summary.datasets.items():
            produced[name] = produced.get(name, 0) + data.entries
    if produced != expected:
        raise RuntimeError(
            f"staged partials lost entries: expected={expected}, actual={produced}"
        )
    for path in partials:
        compression = _compression_of(root, path)
        if compression != 404:
            raise RuntimeError(
                f"intermediate file compression is {compression}, "
                f"expected LZ4 level 4 (404): {path}"
            )


def _compression_of(root, path: Path) -> int:
    root_file = _open_readable(root, path)
    compression = int(root_file.GetCompressionSettings())
    root_file.Close()
    return compression


def _histogram_worker(work: tuple[list[str], str]) -> str:
    paths, output = work
    merge_histograms(_root_module(), [Path(path) for path in paths], Path(output))
    return output


def merge_histograms(
    root,
    inputs: list[Path],
    output: Path,
    jobs: int = 1,
    temp_dir: Path | None = None,
) -> None:
    """Accumulate every histogram in memory and write each one exactly once.

    Nothing here goes through TFileMerger, and no object is written to the
    output more than once, so the key-cycle corruption above cannot arise by
    construction.

    The pass is dominated by opening shards over NFS, so with ``jobs`` above one
    it splits into disjoint subsets whose partial sums are combined here --
    again disjoint inputs, disjoint outputs.
    """
    if jobs > 1 and len(inputs) >= 16 and temp_dir is not None:
        width = math.ceil(len(inputs) / jobs)
        chunks = [inputs[start:start + width]
                  for start in range(0, len(inputs), width)]
        work = [
            ([str(path) for path in chunk], str(temp_dir / f"hist_{index:04d}.root"))
            for index, chunk in enumerate(chunks)
        ]
        with _process_pool(len(work)) as pool:
            partials = [Path(path) for path in pool.map(_histogram_worker, work)]
        merge_histograms(root, partials, output)
        for path in partials:
            path.unlink(missing_ok=True)
        return

    totals: dict[str, object] = {}
    order: list[str] = []
    singletons: dict[str, object] = {}

    for index, path in enumerate(inputs):
        root_file = _open_readable(root, path)

        def visit(directory, prefix: str = "") -> None:
            for key in directory.GetListOfKeys():
                name = str(key.GetName())
                class_name = str(key.GetClassName())
                full = f"{prefix}/{name}" if prefix else name
                if "RNTuple" in class_name:
                    continue
                klass = root.TClass.GetClass(class_name)
                if klass and klass.InheritsFrom(root.TDirectory.Class()):
                    child = directory.GetDirectory(name)
                    if child:
                        visit(child, full)
                    continue
                if klass and klass.InheritsFrom(root.TH1.Class()):
                    histogram = key.ReadObj()
                    if full in totals:
                        if not totals[full].Add(histogram):
                            raise RuntimeError(f"cannot add histogram {full} from {path}")
                    else:
                        clone = histogram.Clone()
                        clone.SetDirectory(0)
                        totals[full] = clone
                        order.append(full)
                    continue
                if index == 0 and full not in singletons:
                    obj = key.ReadObj()
                    if obj:
                        singletons[full] = obj.Clone() if hasattr(obj, "Clone") else obj
                        order.append(full)

        visit(root_file)
        root_file.Close()

    out_file = root.TFile.Open(str(output), "RECREATE", "", 404)
    if not out_file or out_file.IsZombie():
        raise RuntimeError(f"cannot create histogram output {output}")
    for full in order:
        obj = totals.get(full, singletons.get(full))
        directory = out_file
        parts = full.split("/")
        for part in parts[:-1]:
            child = directory.GetDirectory(part)
            directory = child if child else directory.mkdir(part)
        directory.cd()
        obj.Write(parts[-1], root.TObject.kOverwrite)
    out_file.Close()


def _schema_hash(schema: tuple[tuple[str, str], ...]) -> str:
    digest = hashlib.sha256()
    for name, type_name in schema:
        digest.update(name.encode())
        digest.update(b"\0")
        digest.update(type_name.encode())
        digest.update(b"\0")
    return digest.hexdigest()[:16]


def _relative_to_index(path: Path, index_dir: Path) -> str:
    """Record shard locations relative to the index so a sample can be moved."""
    try:
        return str(path.relative_to(index_dir))
    except ValueError:
        return str(path)


def write_chain_index(
    summaries: list[FileSummary],
    shard_paths: list[Path],
    schemas: dict[str, tuple[tuple[str, str], ...]],
    totals: dict[str, int],
    output: Path,
    index_path: Path,
) -> None:
    index_dir = index_path.parent
    ntuples: dict[str, dict] = {}
    for name, schema in schemas.items():
        ntuples[name] = {
            "total_entries": totals[name],
            "schema_hash": _schema_hash(schema),
            "files": [
                {
                    "path": _relative_to_index(path, index_dir),
                    "entries": summary.datasets[name].entries,
                }
                for summary, path in zip(summaries, shard_paths)
                if name in summary.datasets
            ],
        }
    index_path.write_text(json.dumps({
        "version": 1,
        "mode": "index",
        "histograms": _relative_to_index(output, index_dir),
        "ntuples": ntuples,
    }, indent=2) + "\n")


def _validate_output(
    root,
    partial: Path,
    schemas: dict,
    totals: dict,
    histogram_schemas: dict,
    histogram_totals: dict,
) -> None:
    merged = inspect(root, partial)
    check_key_integrity(merged.keys, partial)
    if {name: data.schema for name, data in merged.datasets.items()} != schemas:
        raise RuntimeError("merged RNTuple schema differs from the inputs")
    merged_counts = {name: data.entries for name, data in merged.datasets.items()}
    if merged_counts != totals:
        raise RuntimeError(
            f"merged RNTuple entry counts differ: expected={totals}, "
            f"actual={merged_counts}"
        )
    merged_histogram_schemas = {
        name: (hist.class_name, hist.dimension, hist.cells)
        for name, hist in merged.histograms.items()
    }
    if merged_histogram_schemas != histogram_schemas:
        raise RuntimeError("merged histogram schema differs from the inputs")
    for name, expected_entries in histogram_totals.items():
        actual_entries = merged.histograms[name].entries
        if not math.isclose(
            actual_entries, expected_entries, rel_tol=1.0e-12, abs_tol=1.0e-9
        ):
            raise RuntimeError(
                f"merged histogram entry count differs for {name}: "
                f"expected={expected_entries}, actual={actual_entries}"
            )
    if _compression_of(root, partial) != 404:
        raise RuntimeError(
            f"merged file compression is {_compression_of(root, partial)}, "
            f"expected LZ4 level 4 (404)"
        )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument(
        "--mode",
        choices=("single", "index"),
        default="single",
        help=(
            "single: concatenate everything into --output. "
            "index: merge histograms only and publish a chain index next to it, "
            "leaving the RNTuple shards in --shard-dir"
        ),
    )
    parser.add_argument(
        "--jobs",
        type=int,
        default=1,
        help=(
            "number of independent hadd processes to run over disjoint batches, "
            "and the width of shard validation (default: 1). This never becomes "
            "hadd's own -j, which corrupts output"
        ),
    )
    parser.add_argument(
        "--batch-size",
        type=int,
        default=MAX_BATCH_SIZE,
        help=(
            f"maximum inputs per explicit hadd stage (default and safe maximum: "
            f"{MAX_BATCH_SIZE})"
        ),
    )
    parser.add_argument(
        "--cache-size",
        default="2g",
        help=(
            "hadd -cachesize for the final stage (default: 2g, which is also the "
            "largest value hadd accepts; 0 disables)"
        ),
    )
    parser.add_argument(
        "--batch-cache-size",
        default="512m",
        help=(
            "hadd -cachesize for each concurrent batch merge "
            "(default: 512m; 0 disables). Budget jobs x this value"
        ),
    )
    parser.add_argument(
        "--temp-dir",
        type=Path,
        help="directory for staged partials (default: output directory)",
    )
    parser.add_argument(
        "--shard-dir",
        type=Path,
        help="index mode: directory the RNTuple shards are published under",
    )
    parser.add_argument("--delete-inputs", action="store_true")
    parser.add_argument("inputs", nargs="+", type=Path)
    args = parser.parse_args()

    if args.jobs < 1:
        parser.error("--jobs must be positive")
    if not 2 <= args.batch_size <= MAX_BATCH_SIZE:
        parser.error(f"--batch-size must be between 2 and {MAX_BATCH_SIZE}")
    if args.mode == "index" and args.shard_dir is None:
        parser.error("--mode index requires --shard-dir")
    if args.mode == "index" and args.delete_inputs:
        parser.error(
            "--mode index republishes the shards under --shard-dir; "
            "--delete-inputs would discard the sample's data"
        )
    inputs = [path.resolve() for path in args.inputs]
    if len(set(inputs)) != len(inputs):
        parser.error("duplicate input paths are not allowed")
    output = args.output.resolve()
    if output in inputs:
        parser.error("output must not also be an input")
    for path in inputs:
        if not path.is_file():
            parser.error(f"input does not exist: {path}")

    root = _root_module()
    output.parent.mkdir(parents=True, exist_ok=True)
    temp_parent = (args.temp_dir or output.parent).resolve()
    temp_parent.mkdir(parents=True, exist_ok=True)
    if args.mode == "single" and len(inputs) > args.batch_size:
        input_bytes = sum(path.stat().st_size for path in inputs)
        # A staged merge keeps one intermediate generation while assembling the
        # next, so budget for both generations at once.
        required = 2 * input_bytes
        available = shutil.disk_usage(temp_parent).free
        if available < required:
            raise RuntimeError(
                f"merge temporary directory {temp_parent} has {available} bytes "
                f"free but needs about {required} bytes for {input_bytes} bytes "
                f"of input shards"
            )

    summaries = _inspect_many(root, inputs, args.jobs)
    schemas, totals, histogram_schemas, histogram_totals = combine(summaries, inputs)

    if args.mode == "index":
        return _publish_index(
            root, args, inputs, summaries, schemas, totals,
            histogram_schemas, histogram_totals, output,
        )

    partial = output.with_name(f".{output.name}.partial.{uuid.uuid4().hex}")
    try:
        merge_temp = Path(tempfile.mkdtemp(
            prefix=f".{output.name}.hadd.", dir=temp_parent))
        try:
            _staged_hadd(
                root, inputs, partial, args.jobs, args.batch_size, merge_temp,
                args.cache_size, args.batch_cache_size, totals,
            )
        finally:
            # NFS can briefly retain an entry after hadd has closed and unlinked
            # it.  Cleanup must not invalidate a complete merge; the output is
            # validated below before publication or input deletion.
            shutil.rmtree(merge_temp, ignore_errors=True)
        _validate_output(
            root, partial, schemas, totals, histogram_schemas, histogram_totals)
        os.replace(partial, output)
    except Exception:
        if partial.exists():
            partial.unlink()
        raise

    if args.delete_inputs:
        for path in inputs:
            path.unlink()
    print(f"Merged {len(inputs)} shards into {output}")
    return 0


def _publish_index(
    root, args, inputs, summaries, schemas, totals,
    histogram_schemas, histogram_totals, output: Path,
) -> int:
    """Merge histograms only and leave the bulk RNTuples as addressable shards."""
    shard_dir = args.shard_dir.resolve()
    shard_dir.mkdir(parents=True, exist_ok=True)
    partial = output.with_name(f".{output.name}.partial.{uuid.uuid4().hex}")
    hist_temp = Path(tempfile.mkdtemp(
        prefix=f".{output.name}.hist.",
        dir=(args.temp_dir or output.parent).resolve(),
    ))
    try:
        merge_histograms(root, inputs, partial, args.jobs, hist_temp)
        merged = inspect(root, partial)
        check_key_integrity(merged.keys, partial)
        if merged.datasets:
            raise RuntimeError(
                "index mode must not write RNTuples into the histogram file: "
                f"{sorted(merged.datasets)}"
            )
        carried = set(summaries[0].others)
        extra = sorted({name for s in summaries for name in s.others} - carried)
        if extra:
            raise RuntimeError(
                "index mode copies non-histogram objects from the first shard, "
                f"but later shards carry objects it lacks: {extra[:10]}"
            )
        merged_schemas = {
            name: (hist.class_name, hist.dimension, hist.cells)
            for name, hist in merged.histograms.items()
        }
        if merged_schemas != histogram_schemas:
            raise RuntimeError("merged histogram schema differs from the inputs")
        for name, expected_entries in histogram_totals.items():
            actual = merged.histograms[name].entries
            if not math.isclose(
                actual, expected_entries, rel_tol=1.0e-12, abs_tol=1.0e-9
            ):
                raise RuntimeError(
                    f"merged histogram entry count differs for {name}: "
                    f"expected={expected_entries}, actual={actual}"
                )
    except Exception:
        if partial.exists():
            partial.unlink()
        raise
    finally:
        shutil.rmtree(hist_temp, ignore_errors=True)

    published: list[Path] = []
    for path in inputs:
        target = shard_dir / path.name
        if target.resolve() != path:
            shutil.move(str(path), str(target))
        published.append(target)
    os.replace(partial, output)
    write_chain_index(
        summaries, published, schemas, totals, output,
        output.with_suffix(output.suffix + ".chain.json"),
    )
    print(
        f"Indexed {len(inputs)} shards under {shard_dir}; "
        f"histograms merged into {output}"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as error:
        print(f"sknano-merge: {error}", file=sys.stderr)
        raise SystemExit(1)
