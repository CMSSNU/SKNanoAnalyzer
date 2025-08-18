# Electrons Documentation

## Overview
This document summarizes the electron corrections and algorithms used in SKNanoAnalyzer for both Run 2 (NanoAODv9) and Run 3 (NanoAODv13) data periods.

## Electron Reconstruction

### Run 2 (NanoAODv9)
- **Energy Corrections**: Pre-applied scale and smearing corrections in NanoAOD
- **ID Algorithms**: Fall17v2 recommendations
- **Scale Factors**: POG-provided via correctionlib framework

### Run 3 (NanoAODv13)
- **Energy Corrections**: Applied post-processing via correctionlib (not pre-applied)
- **ID Algorithms**: Updated MVA and cut-based IDs for 13.6 TeV (Winter22v1)
- **Scale Factors**: Era-specific (2022, 2022EE, 2023, 2023BPix) POG recommendations

## Energy Scale and Smearing Corrections

### NanoAODv9 (Run 2)
- **Pre-applied**: Energy corrections embedded in stored electron objects
- **Scale Corrections**: Applied to data to match MC energy scale
- **Smearing**: Applied to MC to match data energy resolution
- **Systematics**: Scale uncertainty stored in `data/Run3_v13_Run2_v9/*/EGM/EGM_ScaleUnc.json.gz`. Resolution uncertainty stored in NanoAOD branches.

### NanoAODv13 (Run 3)
- **Pre-appllied**: Energy corrections embedded in stored electron objects
- **Scale Uncertainties**: Systematic variations available via POG corrections
- **Smearing**: MC resolution smearing applied with seed-based randomization
- **Systematics**: Integrated into CMS correctionlib ecosystem

## SKNanoAnalyzer Implementation

### Electron Class Structure
**File**: `DataFormats/include/Electron.h`, `DataFormats/src/Electron.cc`

#### Key Properties Stored:
- **Energy Uncertainties**:
  - `j_energyErr`: Energy uncertainty for propagation
  - `j_dEsigmaUp/Down`: Resolution uncertainty variations
- **Identification Variables**:
  - SuperCluster: `j_scEta`, track-cluster matching parameters
  - Shower shape: `j_sieie`, `j_hoe`, `j_r9`
  - Isolation: PF cluster isolation, track isolation variants

#### Electron ID Implementation:
- **Cut-based IDs**: Veto, Loose, Medium, Tight, HEEP working points
- **MVA-based IDs**: ISO and NoISO variants with WP80, WP90 working points

### Energy Corrections in MyCorrection

**File**: `AnalyzerTools/include/MyCorrection.h`

#### Available Methods:
- `GetElectronScaleUnc(scEta, seedGain, runNumber, r9, pt, syst)`: Energy scale corrections
- `GetElectronSmearUnc(electron, syst, seed)`: Energy resolution smearing
- `GetElectronRECOSF(eta, pt, phi, syst)`: Reconstruction efficiency scale factors
- `GetElectronIDSF(ID_key, eta, pt, phi, syst)`: Identification efficiency scale factors
- `GetElectronTriggerSF(trigger_key, eta, pt, phi, syst)`: Trigger efficiency scale factors

#### Energy Scale Implementation:
- **Run 2**: Uses "UL-EGM_ScaleUnc" correction set with era-specific parameters
- **Run 3**: Uses "Scale"/"ScaleJSON" with run-dependent corrections
- **Systematic Variations**: Total uncertainty propagation for up/down variations

## Identification Algorithms

### Available ID Working Points:
1. **Cut-based IDs**: Veto, Loose, Medium, Tight (POG standard)
2. **HEEP ID**: High-energy electron identification for high-pT electrons
3. **MVA IDs**: Machine learning-based with isolation (WP80, WP90, WPL)
4. **MVA NoIso IDs**: MVA without isolation requirements
> In NanoV12, NoIso ID has a bug [MVAElectronIDRun3](https://twiki.cern.ch/twiki/bin/view/CMS/MultivariateElectronIdentificationRun3). This is the main reason for using NanoV13.

### Identification Variables:
- **Shower Shape**: σ_iηiη, H/E ratio, R9
- **Track-Cluster Matching**: Δη, Δφ in seed and supercluster
- **Isolation**: ECAL/HCAL PF cluster isolation, tracker isolation
- **Quality**: Conversion veto, missing hits, charge consistency

## Era-Specific Configurations

### Run 2 Eras:
- 2016preVFP, 2016postVFP, 2017, 2018
- UL (Ultra-Legacy) reprocessing recommendations
- Fall17v2 IDs recommended

### Run 3 Eras:
- 2022, 2022EE, 2023, 2023BPix
- Winter22v1 IDs recommended

## Usage Examples

### Basic Electron Selection:
```cpp
RVec<Electron *> selectedElectrons;
for (auto &electron : *electrons) {
    if (electron.Pt() > 25 && abs(electron.Eta()) < 2.5 && 
        electron.PassID("POGTight")) {
        selectedElectrons.push_back(&electron);
    }
}
```

### Energy Scale Corrections:
```cpp
float scaleSF = myCorrection.GetElectronScaleUnc(electron.scEta(), electron.SeedGain(),
                                                 runNumber, electron.r9(), electron.Pt(),
                                                 MyCorrection::variation::nom);
TLorentzVector corrected_p4 = electron.P4() * scaleSF;
```

### Scale Factor Application:
```cpp
float recoSF = myCorrection.GetElectronRECOSF(abs(electron.Eta()), electron.Pt(),
                                              electron.Phi(), MyCorrection::variation::nom);
float idSF = myCorrection.GetElectronIDSF("Tight", abs(electron.Eta()), electron.Pt(),
                                          electron.Phi(), MyCorrection::variation::nom);
```

### Custom Analysis IDs:
```cpp
// Run-dependent custom selection
if (run == 2) {
    if (electron.PassID("HcToWALooseRun2")) selectedElectrons.push_back(&electron);
} else {
    if (electron.PassID("HcToWALooseRun3")) selectedElectrons.push_back(&electron);
}
```

## Key Differences Run 2 vs Run 3

| Aspect | Run 2 (NanoAODv9) | Run 3 (NanoAODv13) |
|--------|-------------------|---------------------|
| ID Algorithms | Fall17v2 | Winter22v1 |
| Systematic Framework | POG JSON files | Integrated correctionlib |
| Phi Dependencies | Not used in scale factors | Some SFs depend on phi |

## References
- [CMS EGamma POG Run2](https://cms-egamma.github.io/Run2/)
- [Electron Performance 2022-2023](https://cds.cern.ch/record/2904365)
- [EGM Scale Factors JSON](https://cms-egamma.github.io/EgammaSFJSON/)
- [CMS EGamma Results](https://twiki.cern.ch/twiki/bin/view/CMSPublic/PhysicsResultsEGM)
- [correctionlib Framework](https://github.com/cms-nanoAOD/correctionlib)
