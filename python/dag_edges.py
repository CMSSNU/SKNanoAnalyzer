#!/usr/bin/env python3
"""DAG edge shapes SKNano needs that htcondor2 does not ship."""

from __future__ import annotations

from htcondor2 import dags


class RaggedGrouper(dags.BaseEdge):
    """Join consecutive chunks of parent nodes to single child nodes.

    ``dags.Grouper`` requires both layers to divide evenly into chunks, so it
    cannot express 1216 analyzer jobs grouped 100 at a time.  This edge takes
    the chunk sizes explicitly and therefore tolerates a short final group.

    Chunk ``i`` of the parent layer becomes the parent set of child node ``i``,
    which is what lets a group's merge start as soon as its own analyzer jobs
    finish, rather than waiting for the whole sample.
    """

    def __init__(self, sizes):
        self.sizes = [int(size) for size in sizes]
        if not self.sizes or any(size <= 0 for size in self.sizes):
            raise ValueError(f"chunk sizes must all be positive: {sizes}")

    def __repr__(self) -> str:
        return f"RaggedGrouper(n_groups={len(self.sizes)}, total={sum(self.sizes)})"

    def get_edges(self, parent, child, join_factory):
        start = 0
        for index, size in enumerate(self.sizes):
            yield (tuple(range(start, start + size)), (index,))
            start += size


def group_sizes(total: int, group_size: int) -> list[int]:
    """Split ``total`` nodes into groups of at most ``group_size``."""
    if group_size <= 0 or total <= group_size:
        return [total]
    sizes = [group_size] * (total // group_size)
    remainder = total % group_size
    if remainder:
        sizes.append(remainder)
    return sizes
