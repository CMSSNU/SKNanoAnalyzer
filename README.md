# SKNanoAnalyzer
---

## THIS IS DEVELOPMENT VERSION!!

## Introduction

- Setting up for the first time: run `./bootstrap.sh` and follow the
  [Setup Guide](docs/SetupGuide.md).
- Start with the [documentation index](docs/README.md).
- For installation and analysis submission, see [Getting Started](docs/GettingStarted.md).
- Before contributing code, read the [Development Guide](docs/DevelopmentGuide.md).

## Analyzer development and ownership

SKNano keeps the shared event-processing backend separate from runnable physics
analyses. Before adding or moving an analyzer, read the full
[Analyzer development and ownership guide](docs/AnalyzerDevelopment.md).

- `AnalyzerFramework` owns `AnalyzerCore`, loading, branch management, standard
  NanoAOD views, and output services. Do not add concrete physics analyzers to
  this directory.
- `CommonAnalyzers` is only for analyzers that are useful to all SKNano users
  and have no analysis-group-specific policy. Changes require a normal backend
  review.
- Personal and analysis-group analyzers belong in an external module
  repository. The module owns its CMake target, ROOT dictionary, tests, runtime
  data, documentation, and custom input schema. Start from
  `examples/AnalysisModule`.
- Official NanoAOD schemas and broadly reusable persistent objects belong in
  the backend. A custom schema stays with its consuming module until multiple
  independent modules need it; only then should it be promoted to shared
  input infrastructure.
- Backend branches pin reviewed external modules as Git submodules. Analyzer
  names and the usual `SKNano.py -a AnalyzerName ...` submission interface do
  not change; the installed analyzer manifest validates availability before a
  job is created.

The old flat `Analyzers/` layout and flat includes such as
`#include "AnalyzerCore.h"` are unsupported. Use namespaced includes such as
`#include <AnalyzerFramework/AnalyzerCore.h>`.

## Changelog

### Unreleased

- `./bootstrap.sh` is a one-shot interactive setup wizard. It installs
  micromamba if needed, creates the `Nano` environment, writes
  `config/config.$USER`, creates the output and log directories, builds the
  batch image from the new `templates/Nano.def`, initialises the submodules,
  and builds. Finished steps are detected and skipped, so it is safe to
  re-run; `--yes` takes every default. Every question is explained in
  [docs/SetupGuide.md](docs/SetupGuide.md).
- `config/config.$USER` gained `[MAMBA_EXE]` and `[MAMBA_ROOT_PREFIX]`;
  `setup.sh` reads them instead of assuming `$HOME/micromamba`. A key that is
  present but empty now keeps the built-in default rather than exporting the
  literal `[KEY]` string, and `[PACKAGE]` defaults to `mamba`.
- `source setup.sh` works under bash as well as zsh: the micromamba shell hook
  follows the sourcing shell, and the `conda` compatibility shim is a function
  rather than an alias.
- `scripts/install_libtorch.sh` pins the LibTorch build instead of following
  the nightly `latest` archive; override with `LIBTORCH_VERSION`.

### [2.0.0] - 2026-07-22

SKNano 2.0.0 is the NanoAODv15 and RNTuple release. This section summarizes
the user-visible differences from the `main` branch rather than listing every
internal commit.

#### Breaking changes

- RNTuple is the only supported input format.
- Analyzer datasets are written as typed RNTuples through `Output().Book()`,
  typed fields, and stable analyzer-owned buffers. Histograms remain ordinary
  ROOT objects.
- NanoAOD collections now use event-scoped `*ViewCollection` APIs such as
  `GetAllMuonViews()` and `GetAllJetViews()`. The former owning input-object
  classes and materialization APIs have been removed. Views and selected
  ranges must not be retained after advancing to the next event.
- The supported Linux environment is the repository's ROOT 6.40.02 toolchain.
  Create the shared `Nano` environment from `docs/Nano-linux-64.lock`, or use
  `docs/Nano.yml` as the solver-based fallback. ROOT, correctionlib, Abseil,
  ONNX Runtime, and the compiler toolchain must be kept ABI-compatible.

#### Analysis and I/O

- Added schema-driven lazy branch binding for NanoAODv15, including generated
  scalar/vector accessors and event-scoped physics-object views.
- Added native multi-file RNTuple input and schema-preserving RNTuple skimming.
- Added atomic output publication and `scripts/sknano_merge.py`, which validates
  RNTuple and histogram schemas and verifies the merged output before removing
  input shards.
- Added typed RNTuple output handles, histogram groups, sparse output profiles,
  and collision/schema checks.
- Added a shared analyzer task API. Flag-selectable studies can register
  validation, booking, and event callbacks while reusing one baseline event
  selection.
- Added configurable event failure handling through `--failure-policy` and
  `--max-event-errors`.
- Added deterministic counter-based random-number handling, batched correction
  and ONNX helpers, execution planning, cache tuning, and optional performance
  telemetry.

#### Submission and user tools

- `SKNano.py` now accepts data-period filtering with `-p`, sample exclusion
  with `--exclude`, and explicit failure policies.
- Every submission records a source snapshot, compressed source archive,
  sample-metadata snapshot, and `run_manifest.json` in the run directory.
- User flags are included in skim and analysis output paths, preventing outputs
  from different task selections from colliding.
- Telegram submission/completion reporting is optional. `setup.sh` reports only
  whether it is configured and no longer prints credentials.
- Merge jobs use scheduler defaults unless resources are explicitly requested,
  and batch setup activates the package environment before enabling strict
  shell checks.
- Added the schema-driven `sknano_plotter` workflow with inspect, validate,
  local plot, and Condor submission modes.
- `scripts/build.sh` supports incremental builds by default plus `--clean`,
  Ninja, build-type, and AddressSanitizer options.

#### Physics content

- Added 2024 NanoAODv15 sample metadata, trigger paths, golden JSON, muon
  corrections, tagging inputs, and modelling/reweighting resources.
- Updated jet corrections and JES/JER-to-MET propagation, lepton IDs, trigger
  object access, systematic handling, and Vcb calibration workflows.
#### Migration quick start

```bash
./bootstrap.sh          # first time only
source setup.sh
./scripts/build.sh --clean

# RNTuple input
SKNano.py -a AnalyzerName -i 'Sample*' -e 2024 -n 10
```

See `docs/README.md` for the maintained API and workflow documentation.

## To do
### Assigned
- [ ] FatJet and Tau: **Youngwan**
- [ ] SKNano.py skimming test **Yeonjoon**
- [ ] Auto-Validation: Check the consistency of the output for a specific NanoAOD file: **Jin**

For DY, check the before/after of correction
- [ ] Leptons **Jihoon**
- [ ] Jets(B-tagging) **Taehee**

## Make Tutorial
- [ ] From make Training tree, Export ONNX, Use this model in the Analyzer : **Yeonjoon(Review: Youngwan)**
- [ ] Simple DY tutorial : **Eunsu(Review: Jin)**
- [ ] tt reconstruction using Kinematic Fit : **Minseok(Review: Byunghun)**
- [ ] tt reconstruction using KinFit Library : **Yeonjoon**
- [ ] NLO vs LO comparison in MadGraph Tutorial: **Jin**
- [ ] Tau+Fatjet Tutorial : **Youngwan**

- [ ] At least try to install until next meeting : ***All***


## Useful links
- [MiniAOD](https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookMiniAOD)
- [NanoAOD](https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookNanoAOD)
- [NanoAOD-Tools (for postproc)](https://github.com/cms-sw/cmssw/tree/master/PhysicsTools/NanoAODTools)
- [SNU_SKFlatProduction](https://docs.google.com/spreadsheets/d/12SfHxUR_1_-13Tzt6mI0slnenQeeCT40zDdggx_KPR8/edit?pli=1#gid=1739893496)
- [NanoAOD wiki@gitlab](https://gitlab.cern.ch/cms-nanoAOD/nanoaod-doc/-/wikis/home)
- [Run2 Summer20 UL](https://docs.google.com/spreadsheets/d/1ABl2p2uwr2EfEbolBEVNcKb_fIXigYY9sqCRT8XIi1Q/edit#gid=1318927481)
- [Run3 Common Bkg Sample Preparation](https://docs.google.com/spreadsheets/d/1xEbHtxzJVaWJpxDrJ7tSpxcVQMRIFbZeoeO7davNFMA/edit#gid=1396344675)
- [SKFlatMaker](https://github.com/CMSSNU/SKFlatMaker)
- [SKFlatAnalyzer](https://github.com/CMSSNU/SKFlatAnalyzer)
- [SKFlat Manual](https://jskim.web.cern.ch/jskim/SKFlat/Manual/Manual_SKFlat.pdf)
- [NanoAOD Branches](https://cms-nanoaod-integration.web.cern.ch/autoDoc/)
- [PdmV Run3 DATA full table](https://pdmv-pages.web.cern.ch/run_3_data/full_table.html)
- [CMS 13.6TeV SM cross sections](https://twiki.cern.ch/twiki/bin/viewauth/CMS/MATRIXCrossSectionsat13p6TeV)
- [LHC ttbar NNLO+NNLL cross sections](https://twiki.cern.ch/twiki/bin/view/LHCPhysics/TtbarNNLO)
- [LHC Single Top NNLO+NNLL cross sections](https://twiki.cern.ch/twiki/bin/view/LHCPhysics/SingleTopRefXsec)
- [LHC Higgs Cross Section](https://twiki.cern.ch/twiki/bin/view/LHCPhysics/HiggsXSBR)
- [Rules for Run3 2024 dataset names](https://cms-pdmv.gitbook.io/project/mccontact/rules-for-run3-2024-dataset-names)
- [Rules for Run3 dataset names](https://cms-pdmv.gitbook.io/project/mccontact/rules-for-run3-dataset-names)
