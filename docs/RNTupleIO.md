# RNTuple I/O

[Documentation index](README.md)

SKNanoAnalyzer uses ROOT 6.40 RNTuple input exclusively. `SKNanoLoader` opens
the `Events` RNTuple in each input file, and the generated `BranchScalar`,
`BranchVector`, collection views, and lazy activation APIs bind directly to
RNTuple fields.

The framework code generator consumes only the canonical NanoAOD schema.
External modules bind custom scalar and vector fields through typed runtime
handles and provide their own event-scoped row views.

## Run an analyzer

```bash
SKNano.py -a ExampleRun -i 'MyRNTupleSample*' -e 2024
```

Generated jobs call `GetInputEntries()`, so reduction works across multi-file
RNTuple inputs. A direct C++ setup is equivalent:

```cpp
ExampleRun module;
module.SetRNTupleName("Events");
module.AddFile("input.rntuple.root");
```

All files in one job must use a compatible RNTuple schema. Multi-file jobs
switch readers at file boundaries while preserving lazy field state.

## I/O behavior

- Scalar and vector views are created lazily on first access.
- `RNTupleSource` snapshots the top-level field names and types when it opens a
  file. `hasField()`/`fieldType()` answer from that snapshot, because
  `RNTupleReader::GetDescriptor()` clones the descriptor under a lock and
  `FindFieldId()` scans every top-level field linearly -- roughly 20 us per call
  on a NanoAOD-sized schema, which is not viable on a per-event path.
- Typed `ColumnHandle`/`ScalarHandle` cache the column they resolve to. The
  cache is keyed on `BranchManager::generation()`, which is bumped whenever an
  RNTuple is attached or the manager is cleared, so a new file re-resolves every
  handle and optional fields stay honest when availability differs per file.
- Arithmetic `RVec<T>` fields expose page-backed contiguous views without an
  intermediate analyzer buffer copy.
- `RVec<bool>` is materialized as bytes because it has no stable `bool *`
  representation compatible with the existing view contract.
- ROOT's RNTuple cluster cache is enabled.
- Analyzer datasets booked through `Output().Book()` are written as RNTuples.
  Histograms remain ordinary ROOT histogram objects in the same file.
- Analyzer RNTuple output uses LZ4 level 4, 64 MiB target clusters, buffered
  writes, page checksums, and bounded compression parallelism from `--ncpu`.
  The sparse multi-dataset profile uses a 4 MiB buffer budget per dataset.
- Batch merging goes through `scripts/sknano_merge.py`. It checks every shard's
  RNTuple and histogram schemas, explicitly stages `hadd -fk404` in batches of
  at most 100 inputs, and fully deserializes every intermediate file before it
  can feed the next stage. It then verifies merged dataset entries, histogram
  entries, and compression, atomically publishes the target, and deletes
  shards only after all checks pass. Staged partials are placed under the
  output filesystem (or `--temp-dir`) so a large merge cannot silently exhaust
  the node-local `/tmp` filesystem.
- Merging is NFS-latency bound rather than CPU bound, so the batches of a stage
  run as independent `hadd` processes over disjoint inputs. `hadd -n` and
  `hadd -j` are never used: both corrupt output on ROOT 6.40.02, and every
  merge output is screened for the resulting repeated key cycles.
- Skimming records selected global entries during analysis and writes the
  original input schema through the RNTuple Snapshot backend.
Set `SKNANO_PERFORMANCE_REPORT=/path/report.json` to collect backend-tagged
event-loop telemetry. That variable also turns on ROOT's own RNTuple metrics,
which wrap every page read in an `RNTupleTimer` (two `clock()` calls per read)
and populate the `file_bytes_read`/`file_read_calls` counters; jobs that only
get a report path from `SetOutfilePath()` leave those counters at zero and do
not pay for the timers. The standalone `test_rntuple_source` target validates
scalar and vector access against an RNTuple file.
