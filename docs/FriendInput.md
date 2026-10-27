# Friend input: custom columns next to the central NanoAOD

[Documentation index](README.md)

A private NanoAOD production (extra tables, extended collections) used to be
consumed as a full copy of NanoAOD plus the custom tables, and every analysis
module kept its own sample jsons and hand-copied normalisation for that copy.
A *friend* keeps the central NanoAODv15 RNTuple as the input and stores only
the custom columns in a second file per central file, aligned entry for entry.
Analyzers read friend columns exactly as central ones: generated branches,
typed handles and module row views cannot tell the two files apart.

## File contract (format 1)

One friend file per central file, at

    $SKNANO_FRIEND_ROOT/<production>/<path of the central file below $SKNANO_INPUT_ROOT>

(a central file outside the input root keeps its absolute path minus the
leading slash). The pairing is by path, never by position.

`Events` has the central file's entry count and order and holds

- `run`, `luminosityBlock`, `event` copied from the central file. The loader
  compares them with the central values on every entry and aborts the job on
  the first mismatch. They are never served to analyzers from the friend.
- `SKFriend_status` (uint8): `0` present; `1` absent from a filtered
  production (its filter rejected the event); `2` absent from an unfiltered
  production (lost). Entries with a non-zero status never reach
  `executeEvent()`; their friend columns are empty.
- every top-level field of the production that the central file does not
  have, plus the complete collections listed as overrides. Cardinality fields
  (`nX`) are stored as plain `std::uint32_t`.

`SKFriendInfo` (one entry) records `formatVersion`, `production`,
`centralFile`, `centralEntries`, `overrides`, `fields`, `sourceFiles`,
`nStatus`, `missingRuns`/`missingLumis`, `missingSumW`, `missingSumSign`,
`missingCount` and `filterNote`. `RNTupleSource::addFriend` reads it to decide
which central collections the friend replaces and refuses a file built for a
different entry count or format version.

Rules enforced on attach: a friend field that also exists in the central file
is served from the friend only when its collection (`X` for `X_y` and `nX`) is
in `overrides`, otherwise the job fails with a configuration error; two
friends may not provide the same field.

## Filtered and unfiltered productions

What an event absent from the private production means, and therefore how
the sample is normalised, depends on whether the producer filters events:

- **Unfiltered** (`filtered: false`, the default). Every central event should
  be there, so an absent one is lost. MC drops lost events one by one and
  subtracts their genWeight from the registry `sumW`/`sumsign`, which is
  exact. Data can only subtract luminosity per lumi, so a lumi that lost any
  event is dropped whole and listed for brilcalc. A data production that
  lost part of more than 1 % of a file's lumis (and more than 5) is refused
  as looking filtered.
- **Filtered** (`filtered: true`). Assumed lossless: an absent event was
  rejected by the filter. MC is normalised to the sum of the aux files'
  `Runs.genEventSumw` (recorded before the filter), not to the registry, so
  the analysed events and the normalisation describe the same processed
  sample — the same as reading the private copy directly. `sumsign` is not
  stored in `Runs` and is taken as `sumw·√(N/sumw2)`, exact for generators
  whose weights have one magnitude. Data uses the usual golden-JSON
  luminosity. The analysis selection has to stay inside the producer's
  filter, as it had to for the private copy.

For MC the declaration is checked: the builder compares each aux file's
`Runs.genEventCount` (events read) with the events it wrote and refuses a
production whose filtering does not match `filtered`. Data has no such count,
so its declaration is trusted. On the Hadron av6 pools the check reads
87 703 / 87 703 per TTLJ file (unfiltered) and about 64 900 / 109 515 per
TTLL file (prefiltered).

## Declaring a production: `<module>/friends.yml`

The file lives at the module root or one directory below it, where the
sample jsons live (`Hadron_Analyzers/Hadron/friends.yml`).

```yaml
hadron_av6:                       # production name, the directory under the friend root
  analyzers: [HadronAnalyzer]     # analyzers that attach it (required)
  eras: ["2024"]                  # optional, every era when omitted
  overrides: [GenPart]            # central collections replaced by the friend
  exclude_prefixes: [LambdaCToPKPiLoose, D0ToKPiLoose]
  filtered: false                 # the producer applies no event filter
  sources:                        # builder only; {era} and {PD} are substituted
    - /gv0/DATA/SKNano/CustomNanoAODv15/hadron_av6_ttlj_rntuple640/{era}/{PD}/**/*.root
  samples: [TTLJ_powheg]          # optional allowlist of registry names
  on_missing: error               # or run_without when a sample has no friend

hadron_av6_prefilter:
  analyzers: [HadronAnalyzer]
  eras: ["2024"]
  overrides: [GenPart]
  exclude_prefixes: [LambdaCToPKPiLoose, D0ToKPiLoose]
  filtered: true                  # HLT AND b-tag prefilter
  filter_note: "HLT(IsoMu24|Ele30_WPTight_Gsf) AND >=2 b-tag candidates"
  sources:
    - /gv0/DATA/SKNano/CustomNanoAODv15/hadron_av6_rntuple640/{era}/{PD}/**/*.root
  samples: [TTLL_powheg, Muon0, EGamma0]
```

A pool produced partly with and partly without a filter is two productions.
The submission (`SKNano.py`) and the builder read the same file, so a
production is described once.

Friends are built into `SKNANO_FRIEND_ROOT`, `/gv0/Users/$USER/SKNanoFriend`
by default like the analysis output and run log, overridable in
`config/config.$USER`. A module whose friends other users read pins the
builder's directory with `root:` in its `friends.yml`
(e.g. `root: /gv0/Users/yeonjoon/SKNanoFriend`), so every submission finds
the same files.

## Building

`sknano_friend` (C++, `tools/sknano_friend/`) builds one friend:

```bash
sknano_friend build --central tree_1.root --aux NANO_1.root --aux NANO_2.root \
    --out $SKNANO_FRIEND_ROOT/hadron_av6/2024/.../tree_1.root --production hadron_av6 \
    --override GenPart --exclude-prefix D0ToKPiLoose --mc [--filtered] \
    --scratch /tmp --report tree_1.root.build.json
sknano_friend check <friend> <central>
```

It reads the central event keys, indexes the aux files' `Events` keys (a
duplicate key is fatal), checks the filter declaration against `Runs` for MC,
assigns the status of every central entry, and copies the selected fields.
The aux files are read in their own order and staged in LZ4 chunks of
`--chunk` (5000) entries sorted by central entry under `--scratch`, then
merged in central order, so all I/O is sequential: a multi-threaded `cmsRun`
scatters events (one TTLJ central file mapped onto 175 554 separate runs
across 6 private files), and reading the aux files in central order would
load a cluster per entry. For every extension column (a selected `X_y` whose
collection `X` the central file also has) the builder compares `nX` between
the two files on every matched event and refuses to build on a mismatch.

The driver runs it over a sample the way jobs resolve the sample:

```bash
scripts/sknano_friend.py build    -f hadron_av6                       # every sample it lists, on condor
scripts/sknano_friend.py build    -f hadron_av6_prefilter -i Muon0 -p C,D
scripts/sknano_friend.py validate -f hadron_av6 -e 2024 -i TTLJ_powheg
scripts/sknano_friend.py status   -f hadron_av6
```

`-f` names the production. `-e`, `-i`, `-p`, `--nmax`, `--memory` and
`--no_exec` mean what they mean for `SKNano.py` (comma lists or a `.txt`,
data samples expanded into their periods); without `-e`/`-i` the production's
own `eras` and `samples` are used. `--local` builds on this machine instead of
condor.

`build` caches a lumi → aux files index per (production, era, PD) under
`<production>/_index/`, picks every aux file holding part of a lumi of the
central file (a lumi may be spread over several files on either side; each
central file is built on its own and only its events are joined), and
runs the builder locally or as one condor job per central file
(`condor_submit` must be on `PATH`; on ai-tamsa1 use the ssh shim the Hadron
module ships). A central file no aux file covers is built with every entry
lost for an unfiltered production and refused for a filtered one. Existing
friends are skipped unless `--force`.

Condor jobs stage under `<production>/_scratch/` on the shared friend root
rather than on the worker: on the SNU workers `/tmp` and the condor scratch
are RAM-backed, and a stage written there is charged to the job's memory
(held at 7 GB for a 243k-entry file). Staged on disk, a TTLJ job took
9–20 min and at most 2.2 GB. `--nmax` (100) bounds how many jobs read
the input filesystem at once; `--memory` (6 GB) is the request per job. `validate`
requires every central file to have its friend and its `.build.json`, and
writes `<production>/<era>/<sample>/friend_summary.json` with the status
totals and, for an unfiltered production, the lumis that lost events and the
lost `sumW`/`sumsign`; for a filtered MC production it holds `norm`, the
`Runs` sums of every aux file the friends were built from, each counted once.
For data with lost lumis it also writes `missing_lumis.json` in the
run → lumi-range form `brilcalc -i` and the golden json use.

## What a submission does

When `-a <Analyzer>` matches a production's `analyzers`, `SKNano.py` pairs a
friend with every central file (`module.AddFile(central, {friend})` in the
generated job), fails the submission when a file or the sample summary is
missing (`on_missing: error`), and

- MC, unfiltered: subtracts the summary's `missing_sumw`/`missing_sumsign`
  from the registry `sumW`/`sumsign` before writing them into the job;
- MC, filtered: writes the summary's `norm` instead of the registry values;
- data: prints how many lumis lost events; those events are skipped in the
  loop, and the recorded luminosity of `missing_lumis.json` has to be
  subtracted from the trigger luminosity by hand (brilcalc).

The resolved friend paths go into
`metadata/data_snapshot/resolved_inputs/<era>/<sample>.json`, the production,
status totals and the before/after normalisation into
`metadata/data_snapshot/friends/<era>/<sample>.json`, both listed in the
`run_manifest.json`. `--no-friends` runs the analyzer on the central files
alone. The performance report counts `friend_missing_events` and
`friend_filtered_events`.

## Limitations and follow-ups

- Skims (`SnapshotSelectedInput`) copy the central columns only; a skim of a
  friend-attached sample needs its own friend build against the skim files.
- A filtered production is assumed lossless. A lost aux file drops its
  events from the analysis while its `Runs` sums also leave the
  normalisation, so MC stays consistent; data would silently lose luminosity.
  A per-lumi pre-filter event count from the producer would make losses
  visible there as well.
- Only `genWeight`, `sumW` and `sumsign` are corrected; sums of LHE scale or
  PDF weights from `Runs` are not.
- Only local paths are supported; central files, aux files and friends on
  another site (xrootd) are a follow-up.
- Dropping the standard tables in the producer itself would shrink the
  private production on disk; the builder only needs the custom fields.
  How much a friend saves depends on the custom content: for TTLJ the
  friend is 3.6 TB against 4.6 TB for the private copy and 512 GB for the
  central sample, dominated by `LambdaCToPKPi` (3.7 kB of 8.2 kB per event).
