# Muons Documentation

## Overview
This document summarizes the muon corrections and algorithms used in SKNanoAnalyzer for both Run 2 (NanoAODv9) and Run 3 (NanoAODv13) data periods.

## Muon Reconstruction

- **Reconstruction**: Standalone, Tracker, and Global muons
- **Momentum Corrections**: Rochester corrections applied post-processing
- **Scale Factors**: POG-provided ID, isolation, and single lepton trigger efficiency provided by correctionlib. Custom efficiencies stored in `data/Run3_v13_Run2_v9/*/MUO`.
- **Tracking**: Various isolation algorithms (PF, Mini, PUPPI, TK)

## Momentum Scale Corrections (Rochester Corrections)
- **Application**: Applied post-processing using `external/RoccoR`.
- **Method**: Two-step correction process using Z-boson mass constraints
- **Coverage**: Available for 2016, 2017, and 2018 datasets
- **Implementation**: Via RoccoR package with era-specific text files
- **NOTE**: Run3 Rochester corrections are preliminary.

## SKNanoAnalyzer Implementation

### Muon Class Structure
**File**: `DataFormats/include/Muon.h`, `DataFormats/src/Muon.cc`

#### Key Properties Stored:
- **Momentum Scale Variations**:
  - `j_miniAODPt`: Original pT from MiniAOD
  - `j_momentumScaleUp/Down`: Scale uncertainty variations
  - Rochester corrections applied via MyCorrection class

#### Muon ID Implementation:
- **Boolean IDs**: Loose, Medium, MediumPrompt, Tight, Soft, SoftMVA, TriggerLoose
- **Working Point IDs**: High-pT, isolation variants (Mini, Multi, PF, PUPPI, TK)
- **MVA Scores**: SoftMVA, LowPt MVA, TTH MVA discriminators

### Momentum Corrections in MyCorrection

**File**: `AnalyzerTools/include/MyCorrection.h`

#### Available Methods:
- `GetMuonScaleSF(muon, syst, matched_pt)`: Rochester momentum scale corrections
- `GetMuonRECOSF(muon, syst)`: Reconstruction efficiency scale factors
- `GetMuonIDSF(ID_key, muon, syst)`: Identification efficiency scale factors
- `GetMuonTriggerSF(trigger_key, muons, syst)`: Trigger efficiency scale factors

#### Rochester Correction Implementation:
- Applied for muons with pT > 10 GeV (radiation considerations for low-pT)
- Data: Uses `kScaleDT()` with charge, pT, η, φ parameters
- MC: Uses `kSpreadMC()` (with gen matching) or `kSmearMC()` (without)
- Systematic uncertainties via error propagation

## Identification Algorithms

### Available ID Working Points:
1. **POG Standard IDs**: Loose, Medium, MediumPrompt, Tight, Soft
2. **High-pT IDs**: Tracker and Global variants for high-momentum muons
3. **MVA-based IDs**: SoftMVA, LowPt MVA for specialized selections

### Isolation Algorithms:
- **PF Isolation**: Particle Flow-based (6 working points: VLoose to VVTight)
- **Mini Isolation**: Variable cone size (4 working points: Loose to VTight)  
- **Multi Isolation**: Combined approach (2 working points: Loose, Medium)
- **PUPPI Isolation**: Pileup-mitigated (3 working points: Loose to Tight)
- **Tracker Isolation**: Track-only (2 working points: Loose, Tight)

## Usage Examples

### Basic Muon Selection:
```cpp
RVec<Muon *> selectedMuons;
for (auto &muon : *muons) {
    if (muon.Pt() > 25 && abs(muon.Eta()) < 2.4 && muon.PassID("POGTight")) {
        selectedMuons.push_back(&muon);
    }
}
```

### Rochester Corrections:
```cpp
float roccorSF = myCorrection.GetMuonScaleSF(muon, MyCorrection::variation::nom, genPt);
TLorentzVector corrected_p4 = muon.P4() * roccorSF;
```

### Scale Factor Application:
```cpp
float idSF = myCorrection.GetMuonIDSF("NUM_TightID_DEN_TrackerMuons", muon, 
                                      MyCorrection::variation::nom);
float isoSF = myCorrection.GetMuonIDSF("NUM_TightRelIso_DEN_TightIDandIPCut", muon,
                                       MyCorrection::variation::nom);
```

### Systematic Variations:
```cpp
float roccor_up = myCorrection.GetMuonScaleSF(muon, MyCorrection::variation::up);
float roccor_dn = myCorrection.GetMuonScaleSF(muon, MyCorrection::variation::down);
```

## Key Differences Run 2 vs Run 3

| Aspect | Run 2 (NanoAODv9) | Run 3 (NanoAODv13) |
|--------|-------------------|---------------------|
| Rochester Corrections | RoccoR package | correctionlib framework |
| RECO Scale Factors | Applied via POG SFs | No correction needed |
| ID Algorithms | Standard POG IDs | Enhanced MVA methods |
| Isolation | PF/Mini/PUPPI variants | Improved PUPPI-based |
| Systematic Framework | Manual implementation | correctionlib integration |

## References
- [CMS Muon POG](https://twiki.cern.ch/twiki/bin/view/CMSPublic/PhysicsResultsMUO)
- [Rochester Corrections Paper](https://arxiv.org/abs/1208.3710)
- [Run 3 Muon Performance](https://www.researchgate.net/publication/377547277_Performance_of_the_CMS_Muon_System_with_Early_Run_3_Data)
- [Muon POG Recommendations](https://muon-wiki.docs.cern.ch/guidelines/corrections/)
- [CMS NanoAOD Documentation](https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookNanoAOD)
- [RoccoR gitlab](https://gitlab.cern.ch/hyseo/RoccoR)
