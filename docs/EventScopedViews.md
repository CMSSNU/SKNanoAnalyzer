# Event-scoped physics-object views

[Documentation index](README.md)

NanoAOD input rows are exposed only through event-scoped `*ViewCollection`
ranges. The former owning input classes and their materialization APIs have
been removed from `DataFormats`, `AnalyzerCore`, and `MyCorrection`.

## Contract

- `GetAll*Views()` binds a small SoA descriptor and does not read every object
  field.
- A field activates its NanoAOD column on first access. Derived correction
  lanes (muon momentum, AK4 jet and AK8 fat-jet JEC/JER/JES) are populated
  once per event on first corrected-field access. For both jet collections the
  nominal lanes never compute a variation: the JER and JES lanes each run their
  own stage on first access.
- Selection is represented by raw input indices. `SelectMuonViews()`,
  `SelectElectronViews()`, and `SelectJetViews()` return ranges over those
  indices without copying physics objects.
- `SelectJetViews()` and its AK8 analogue `SelectFatJetViews()` project:
  `Pt()`, `M()`, `P4()` (and `SDMass()` for fat jets) read the nominal, JES or
  JER lane the call asked for, the corrected momentum on data.
  `SelectFatJetViews()` takes positions in the collection it is given, so a
  `SelectFatJets()` result works with `AllIndices()`. `SelectFatJets()` cuts on
  the same lanes but returns raw `FatJetView`s.
- A view or selected range is valid only for the event in which it was
  created. Access after the loader advances to another event throws.
- `Lepton` and `Particle` remain explicit value types for output and tool
  boundaries that cannot consume a view, such as a kinematic fitter. They are
  not NanoAOD input-row containers.

## Analyzer API

Analyzers use:

```cpp
const auto allMuons = GetAllMuonViews();
auto indices = SelectMuonIndices(allMuons, MuonView::MuonID::POG_TIGHT,
                                 25.f, 2.4f);
const auto muons = SelectMuonViews(allMuons, std::move(indices));

const auto allJets = GetAllJetViews();
auto jetIndices = SelectJetIndices(allJets, JetView::JetID::TIGHT, 30.f, 2.5f,
                                   jesVariation, jerVariation);
const auto jets = SelectJetViews(allJets, std::move(jetIndices),
                                 jesVariation, jerVariation);
```

There is no manual branch-enable or snapshot-spec step. The first accessor that
needs a column or derived correction lane activates it automatically. A view
must never be retained after the loader advances to the next event.
