#!/usr/bin/env python3
"""Unit tests for the SKNano merge engine and its DAG grouping.

Run directly or through ctest.  Exits 77 (ctest's skip code) when PyROOT or
hadd are unavailable, following the convention in docs/DevelopmentGuide.md.
"""

from __future__ import annotations

import json
from pathlib import Path
import shutil
import sys
import tempfile
import unittest

REPO_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO_ROOT / "python"))
# Import by name, not by path: the merge engine's worker pool pickles function
# references, which requires exactly one module instance.
sys.path.insert(0, str(REPO_ROOT / "scripts"))

import sknano_merge as merge

NTUPLES = ("Events", "Metadata")
HISTOGRAMS = ("CutFlow", "Systematics/Central/SelectedEvents",
              "Systematics/JESUp/SelectedEvents")


def build_shard(root, path: Path, index: int, entries: int) -> None:
    """Write one shard shaped like analyzer output: RNTuples plus histograms."""
    options = root.RDF.RSnapshotOptions()
    options.fOutputFormat = root.RDF.ESnapshotOutputFormat.kRNTuple
    for position, name in enumerate(NTUPLES):
        if position:
            options.fMode = "UPDATE"
        count = entries if position == 0 else 3
        frame = (
            root.RDataFrame(count)
            .Define("x", f"(int)(rdfentry_ + {index * 1000})")
            .Define("w", "1.5 * rdfentry_ + 0.25")
        )
        frame.Snapshot(name, str(path), ["x", "w"], options)

    out = root.TFile.Open(str(path), "UPDATE")
    for name in HISTOGRAMS:
        parts = name.split("/")
        directory = out
        for part in parts[:-1]:
            child = directory.GetDirectory(part)
            directory = child if child else directory.mkdir(part)
        histogram = root.TH1D(parts[-1], parts[-1], 4, 0.0, 4.0)
        for bin_index in range(entries):
            histogram.Fill(bin_index % 4, 1.0 + index)
        directory.cd()
        histogram.Write(parts[-1], root.TObject.kOverwrite)
    out.Close()


class MergeEngineTest(unittest.TestCase):
    shards: list[Path]

    @classmethod
    def setUpClass(cls) -> None:
        cls.root = merge._root_module()
        cls.tmp = Path(tempfile.mkdtemp(prefix="sknano_merge_test."))
        cls.shards = []
        for index in range(7):
            shard = cls.tmp / f"hists_{index}.root"
            build_shard(cls.root, shard, index, 20 + index)
            cls.shards.append(shard)
        cls.summaries = [merge.inspect(cls.root, shard) for shard in cls.shards]
        cls.schemas, cls.entries, cls.hist_schemas, cls.hist_entries = merge.combine(
            cls.summaries, cls.shards)

    @classmethod
    def tearDownClass(cls) -> None:
        shutil.rmtree(cls.tmp, ignore_errors=True)

    def _merge(self, name: str, *extra: str) -> Path:
        output = self.tmp / name
        argv = [
            "sknano_merge", "--output", str(output),
            "--temp-dir", str(self.tmp), *extra,
            *[str(path) for path in self.shards],
        ]
        saved, sys.argv = sys.argv, argv
        try:
            self.assertEqual(merge.main(), 0)
        finally:
            sys.argv = saved
        return output

    def test_shards_are_shaped_like_analyzer_output(self) -> None:
        self.assertEqual(sorted(self.schemas), sorted(NTUPLES))
        self.assertEqual(sorted(self.hist_schemas), sorted(HISTOGRAMS))

    def test_staged_merge_matches_input_sums(self) -> None:
        output = self._merge("staged.root", "--jobs", "2", "--batch-size", "3")
        merged = merge.inspect(self.root, output)
        self.assertEqual(
            {name: data.entries for name, data in merged.datasets.items()},
            self.entries)
        for name, total in self.hist_entries.items():
            self.assertAlmostEqual(merged.histograms[name].entries, total)

    def test_merged_output_carries_no_repeated_keys(self) -> None:
        output = self._merge("keys.root", "--jobs", "2", "--batch-size", "3")
        merged = merge.inspect(self.root, output)
        self.assertTrue(all(key.cycle == 1 for key in merged.keys))
        located = [(key.directory, key.name) for key in merged.keys]
        self.assertEqual(len(located), len(set(located)))

    def test_staged_and_single_pass_agree_bin_by_bin(self) -> None:
        staged = self._merge("a.root", "--jobs", "3", "--batch-size", "2")
        direct = self._merge("b.root", "--jobs", "1", "--batch-size", "100")
        left = self.root.TFile.Open(str(staged))
        right = self.root.TFile.Open(str(direct))
        for name in HISTOGRAMS:
            one, two = left.Get(name), right.Get(name)
            self.assertEqual(one.GetNcells(), two.GetNcells(), name)
            for cell in range(one.GetNcells()):
                self.assertEqual(one.GetBinContent(cell), two.GetBinContent(cell),
                                 f"{name} cell {cell}")
        left.Close()
        right.Close()
        self.assertEqual(
            {n: d.entries for n, d in merge.inspect(self.root, staged).datasets.items()},
            {n: d.entries for n, d in merge.inspect(self.root, direct).datasets.items()})

    def test_parallel_histogram_pass_matches_serial(self) -> None:
        """The split histogram pass must sum to the same thing as one process."""
        source = self.tmp / "hist_only"
        source.mkdir(exist_ok=True)
        shards = []
        for index in range(20):
            path = source / f"h_{index}.root"
            out = self.root.TFile.Open(str(path), "RECREATE", "", 404)
            for name in HISTOGRAMS:
                parts = name.split("/")
                directory = out
                for part in parts[:-1]:
                    child = directory.GetDirectory(part)
                    directory = child if child else directory.mkdir(part)
                histogram = self.root.TH1D(parts[-1], parts[-1], 4, 0.0, 4.0)
                for bin_index in range(8):
                    histogram.Fill(bin_index % 4, 1.0 + index)
                directory.cd()
                histogram.Write(parts[-1], self.root.TObject.kOverwrite)
            out.Close()
            shards.append(path)

        serial = self.tmp / "hist_serial.root"
        parallel = self.tmp / "hist_parallel.root"
        merge.merge_histograms(self.root, shards, serial)
        merge.merge_histograms(self.root, shards, parallel, 4, source)

        left = self.root.TFile.Open(str(serial))
        right = self.root.TFile.Open(str(parallel))
        for name in HISTOGRAMS:
            one, two = left.Get(name), right.Get(name)
            self.assertEqual(one.GetNcells(), two.GetNcells(), name)
            for cell in range(one.GetNcells()):
                self.assertEqual(one.GetBinContent(cell), two.GetBinContent(cell),
                                 f"{name} cell {cell}")
                self.assertEqual(one.GetBinError(cell), two.GetBinError(cell),
                                 f"{name} cell {cell} error")
        left.Close()
        right.Close()
        summary = merge.inspect(self.root, parallel)
        self.assertTrue(all(key.cycle == 1 for key in summary.keys))

    def test_key_integrity_rejects_repeated_cycles(self) -> None:
        """The signature hadd -n leaves behind: one key cycle per merge chunk."""
        path = self.tmp / "cycled.root"
        out = self.root.TFile.Open(str(path), "RECREATE", "", 404)
        for _ in range(3):
            histogram = self.root.TH1D("CutFlow", "CutFlow", 4, 0.0, 4.0)
            histogram.Fill(1.0)
            histogram.Write()  # no kOverwrite: appends a new cycle each time
        out.Close()
        with self.assertRaises(RuntimeError) as caught:
            merge.inspect(self.root, path)
        self.assertIn("repeated object keys", str(caught.exception))

    def _build_index(self) -> Path:
        """Publish a chain index once; index mode consumes its inputs by moving them."""
        output = self.tmp / "index.root"
        if Path(str(output) + ".chain.json").is_file():
            return output
        staging = self.tmp / "index_inputs"
        shutil.rmtree(staging, ignore_errors=True)
        staging.mkdir()
        copies = []
        for shard in self.shards:
            target = staging / shard.name
            shutil.copy2(shard, target)
            copies.append(target)
        argv = [
            "sknano_merge", "--mode", "index", "--output", str(output),
            "--shard-dir", str(self.tmp / "index_shards"), "--jobs", "1",
            *[str(path) for path in copies],
        ]
        saved, sys.argv = sys.argv, argv
        try:
            self.assertEqual(merge.main(), 0)
        finally:
            sys.argv = saved
        return output

    def test_index_mode_agrees_with_single_mode(self) -> None:
        single = self._merge("single.root", "--jobs", "1", "--batch-size", "100")
        single_entries = {
            name: data.entries
            for name, data in merge.inspect(self.root, single).datasets.items()
        }
        output = self._build_index()

        histograms_only = merge.inspect(self.root, output)
        self.assertEqual(histograms_only.datasets, {})
        for name, total in self.hist_entries.items():
            self.assertAlmostEqual(histograms_only.histograms[name].entries, total)

        import sknano_chain

        index = json.loads(Path(str(output) + ".chain.json").read_text())
        self.assertEqual(
            {name: value["total_entries"] for name, value in index["ntuples"].items()},
            single_entries)
        for name in index["ntuples"]:
            for path in sknano_chain.files(output, name):
                self.assertTrue(Path(path).is_file(), path)

    def test_chain_index_survives_relocation(self) -> None:
        """Shard paths are recorded relative to the index, not absolutely."""
        import sknano_chain

        output = self._build_index()
        moved = self.tmp / "relocated"
        moved.mkdir(exist_ok=True)
        for name in ("index.root", "index.root.chain.json"):
            shutil.copy2(self.tmp / name, moved / name)
        if not (moved / "index_shards").exists():
            shutil.copytree(self.tmp / "index_shards", moved / "index_shards")

        for name in sknano_chain.ntuples(moved / "index.root"):
            for path in sknano_chain.files(moved / "index.root", name):
                self.assertTrue(Path(path).is_file(), path)
                self.assertTrue(str(path).startswith(str(moved)), path)
        self.assertTrue(Path(sknano_chain.histograms(moved / "index.root")).is_file())

    def test_chain_reader_reads_every_shard(self) -> None:
        import sknano_chain

        output = self._build_index()
        for name in sknano_chain.ntuples(output):
            frame = sknano_chain.rdataframe(output, name)
            self.assertEqual(
                int(frame.Count().GetValue()),
                sknano_chain.total_entries(output, name))


class GroupingTest(unittest.TestCase):
    def test_group_sizes_cover_every_job(self) -> None:
        from dag_edges import group_sizes

        for total, size in ((1216, 100), (7, 3), (5, 100), (100, 100), (101, 100)):
            sizes = group_sizes(total, size)
            self.assertEqual(sum(sizes), total, (total, size))
            self.assertTrue(all(0 < value <= max(size, total) for value in sizes))
        self.assertEqual(group_sizes(1216, 100), [100] * 12 + [16])
        self.assertEqual(group_sizes(7, 0), [7])

    def test_ragged_grouper_maps_chunks_to_children(self) -> None:
        from dag_edges import RaggedGrouper, group_sizes

        sizes = group_sizes(7, 3)
        edges = list(RaggedGrouper(sizes).get_edges(None, None, None))
        self.assertEqual(edges, [((0, 1, 2), (0,)), ((3, 4, 5), (1,)), ((6,), (2,))])
        # Every analyzer node index, and therefore every hists_<index>.root,
        # belongs to exactly one group.
        covered = [index for parents, _ in edges for index in parents]
        self.assertEqual(covered, list(range(7)))

    def test_ragged_grouper_rejects_empty_groups(self) -> None:
        from dag_edges import RaggedGrouper

        with self.assertRaises(ValueError):
            RaggedGrouper([3, 0, 2])


def main() -> int:
    try:
        merge._root_module()
    except RuntimeError:
        print("PyROOT unavailable; skipping merge tests", file=sys.stderr)
        return 77
    if shutil.which("hadd") is None:
        print("hadd unavailable; skipping merge tests", file=sys.stderr)
        return 77
    result = unittest.main(argv=[sys.argv[0]], exit=False, verbosity=2).result
    return 0 if result.wasSuccessful() else 1


if __name__ == "__main__":
    raise SystemExit(main())
