# Documentation

SKNanoAnalyzer documentation is grouped by workflow. A first-time user should
start with the [Setup Guide](SetupGuide.md) and `./bootstrap.sh`; framework
contributors should read the core API documents before changing analyzer or I/O
contracts.

## Setup and daily use

- [Setup Guide](SetupGuide.md): the `./bootstrap.sh` wizard explained question
  by question, plus first run and common errors. Start here.
- [Getting Started](GettingStarted.md): installation, builds, job submission,
  sample metadata, skimming, and notifications.
- [Environment Setup](SettingEnv.md): manual micromamba and Singularity
  setup for the SNU cluster; what the wizard automates.
- [Development Guide](DevelopmentGuide.md): repository workflow, systematic
  APIs, coding conventions, and pull requests.
- [Analyzer Development](AnalyzerDevelopment.md): framework/common/module
  ownership, external analyzer builds, and custom input views.

## Core framework and I/O

- [Event-scoped Views](EventScopedViews.md): lazy NanoAOD collection access and
  object-lifetime rules.
- [Analyzer Task API](AnalyzerTaskAPI.md): shared selections with independently
  selectable analyzer tasks.
- [Typed Output](OutputHandles.md): RNTuple fields and histogram groups.

- [RNTuple I/O](RNTupleIO.md): input selection, conversion, merging, and
  performance telemetry.
- [ROOT Schema Compatibility](RootSchemaCompatibility.md): persistent-object
  ABI and schema-evolution policy.

These documents describe one connected data path:

```text
NanoAOD input -> event-scoped views -> analyzer tasks -> typed RNTuple/histogram output
```

The YAML files [noSyst.yaml](noSyst.yaml) and
[ExampleSystematic.yaml](ExampleSystematic.yaml) are small systematic
configuration examples. [Nano-linux-64.lock](Nano-linux-64.lock) is the pinned
Linux environment used by the setup guide, and
[`templates/Nano.def`](../templates/Nano.def) is the Apptainer definition it
builds the batch image from.
