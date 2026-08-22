#!/usr/bin/env python3
"""Read an ``index``-mode SKNano sample as a chain of RNTuple shards.

``SKNano.py --merge-mode index`` skips the bulk concatenation: histograms are
merged into ``<sample>.root`` and the RNTuple shards stay addressable, described
by ``<sample>.root.chain.json``.  Downstream code that used to open one merged
file passes the shard list instead -- both uproot and RDataFrame already accept
one.
"""

from __future__ import annotations

import json
from pathlib import Path

CHAIN_SUFFIX = ".chain.json"


def index_path(sample_root: str | Path) -> Path:
    """Return the chain index that describes ``sample_root``."""
    path = Path(sample_root)
    if path.name.endswith(CHAIN_SUFFIX):
        return path
    return path.with_name(path.name + CHAIN_SUFFIX)


def load(sample_root: str | Path) -> dict:
    path = index_path(sample_root)
    if not path.is_file():
        raise FileNotFoundError(
            f"no chain index at {path}. A sample merged with --merge-mode single "
            f"is a complete file: open {Path(sample_root)} directly"
        )
    index = json.loads(path.read_text())
    if index.get("version") != 1:
        raise RuntimeError(f"unsupported chain index version in {path}")
    return index


def ntuples(sample_root: str | Path) -> list[str]:
    return sorted(load(sample_root)["ntuples"])


def _resolve(index_file: Path, recorded: str) -> str:
    """Paths are stored relative to the index so a sample can be relocated."""
    path = Path(recorded)
    return str(path if path.is_absolute() else index_file.parent / path)


def files(sample_root: str | Path, ntuple: str) -> list[str]:
    """Shard paths holding ``ntuple``, in the order they were produced."""
    index_file = index_path(sample_root)
    index = load(sample_root)
    if ntuple not in index["ntuples"]:
        raise KeyError(
            f"{ntuple} is not in {index_file}; "
            f"available: {sorted(index['ntuples'])}"
        )
    return [_resolve(index_file, entry["path"])
            for entry in index["ntuples"][ntuple]["files"]]


def total_entries(sample_root: str | Path, ntuple: str) -> int:
    return load(sample_root)["ntuples"][ntuple]["total_entries"]


def rdataframe(sample_root: str | Path, ntuple: str):
    """``ROOT.RDataFrame`` over every shard of ``ntuple``."""
    import ROOT

    paths = ROOT.std.vector("string")()
    for path in files(sample_root, ntuple):
        paths.push_back(path)
    return ROOT.RDataFrame(ntuple, paths)


def uproot_iterate(sample_root: str | Path, ntuple: str, expressions=None, **kwargs):
    """``uproot.iterate`` over every shard of ``ntuple``."""
    import uproot

    return uproot.iterate(
        [f"{path}:{ntuple}" for path in files(sample_root, ntuple)],
        expressions,
        **kwargs,
    )


def histograms(sample_root: str | Path) -> str:
    """The merged histogram file for this sample."""
    return _resolve(index_path(sample_root), load(sample_root)["histograms"])
