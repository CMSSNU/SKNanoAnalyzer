# Jets Documentation

## Overview
This document summarizes the jet corrections and algorithms used in SKNanoAnalyzer for both Run 2 (NanoAODv9) and Run 3 (NanoAODv13) data periods.

## Jet Algorithms

### Run 2 (NanoAODv9)
- **Algorithm**: ak4 PFJets with Charged Hadron Subtraction (CHS)
- **Energy Corrections**: L1+L2+L3+residual corrections applied
- **Pileup Mitigation**: CHS algorithm used to reduce pileup contamination
- **pT Threshold**: Jets with pT > 15 GeV are stored

### Run 3 (NanoAODv13)
- **Algorithm**: ak4 PFJets with Pileup Per Particle Identification (PUPPI)
- **Energy Corrections**: L2+L3+residual corrections applied (L1 as dummy)
- **Pileup Mitigation**: PUPPI algorithm assigns weights to particles based on probability to originate from leading vertex vs. pileup
- **pT Threshold**: Jets with pT > 15 GeV are stored

## Jet Energy Corrections (JEC)

### NanoAODv9 (Run 2)
- **JEC Levels**: L1+L2+L3+residual corrections applied during NanoAOD production
- **Algorithm**: CHS jets with dedicated JEC versions from global tag
- **Storage**: Systematic uncertainties not stored persistently, but per-event information needed to compute variations is saved

### NanoAODv13 (Run 3)
- **JEC Levels**: L2+L3+residual corrections applied during NanoAOD production
- **Algorithm**: PUPPI jets with era-specific corrections
- **Storage**: No systematic variations stored, but quantities (rho, area, pt, eta) available for recomputation
- **Tools**: nanoAOD-tools modules available for JEC/JER systematic variations

## SKNanoAnalyzer Implementation

### Jet Class Structure
**File**: `DataFormats/include/Jet.h`, `DataFormats/src/Jet.cc`

#### Key Properties Stored:
- **Energy Corrections**:
  - `j_rawFactor`: Raw factor for uncorrected pT
  - `j_PNetRegPtRawCorr`: ParticleNet regression correction
  - `j_bRegCorr`, `j_cRegCorr`: b-jet and c-jet regression corrections
  - `jet_rawPt`, `jet_originalPt`: Raw and original pT values

#### Jet ID Implementation:
- **Run 2**: Uses NanoAOD jetID bits directly
- **Run 3**: Custom implementation due to NanoAODv12 bug
  - Tight Jet ID implemented with eta-dependent cuts
  - Tight Lepton Veto ID includes muon and charged EM energy fraction cuts

### JERC Corrections in MyCorrection

**File**: `AnalyzerTools/include/MyCorrection.h`

#### Available Methods:
- `GetJER(eta, pt, rho)`: Jet Energy Resolution
- `GetJERSF(eta, pt, syst, source)`: JER Scale Factors
- `GetJESSF(area, eta, pt, phi, rho, runNumber)`: JES corrections
- `GetJESUncertainty(eta, pt, syst, source)`: JES systematic uncertainties

#### Systematic Variations:
- Supports `up`, `down`, and `nom` variations
- Source-specific uncertainties available for detailed studies
- Uses correctionlib framework for consistent POG recommendations

## B-Tagging Algorithms

### Available Taggers:
1. **DeepJet (DeepFlavour)**: Legacy algorithm for Run 2/early Run 3
2. **ParticleNet**: Current recommended algorithm for Run 3
3. **ParT (RobustParT)**: Alternative algorithm for systematic studies

### Discriminators Stored:
- **B-tagging**: `btagDeepFlavB`, `btagPNetB`, `btagRobustParTAK4B`
- **C-tagging**: `btagDeepFlavCvB/CvL`, `btagPNetCvB/CvL`, `btagRobustParTAK4CvB/CvL`
- **QvG tagging**: `btagDeepFlavQG`, `btagPNetQvG`, `btagRobustParTAK4QG`

## Jet Quality Cuts

### Jet ID Levels:
- **LOOSE**: Basic quality requirements
- **TIGHT**: Standard analysis quality
- **TIGHTLEPVETO**: Tight + lepton overlap removal

### Pileup Jet ID:
- **LOOSE/MEDIUM/TIGHT**: Available for low-pT jet discrimination
- Applied for jets typically below 50 GeV pT

## Era-Specific Configurations

### Run 2 Eras:
- 2016preVFP, 2016postVFP, 2017, 2018
- CHS algorithm throughout
- Era-specific JEC versions from global tags

### Run 3 Eras:
- 2022, 2022EE, 2023, 2023BPix
- PUPPI algorithm standard
- Updated ParticleNet taggers
- Improved jet regression corrections

## Usage Examples

### Basic Jet Selection:
```cpp
RVec<Jet *> selectedJets;
for (auto &jet : *jets) {
    if (jet.Pt() > 30 && abs(jet.Eta()) < 2.4 && jet.PassID("tight")) {
        selectedJets.push_back(&jet);
    }
}
```

### B-Tagging:
```cpp
float btagWP = myCorrection.GetBTaggingWP(JetTagging::JetFlavTagger::ParticleNet, 
                                          JetTagging::JetFlavTaggerWP::Medium);
bool isBTagged = jet.GetBTaggerResult(JetTagging::JetFlavTagger::ParticleNet) > btagWP;
```

### Systematic Variations:
```cpp
float jesSF = myCorrection.GetJESSF(jet.GetArea(), jet.Eta(), jet.Pt(), 
                                    jet.Phi(), rho, runNumber);
float jesUnc = myCorrection.GetJESUncertainty(jet.Eta(), jet.Pt(), 
                                              MyCorrection::variation::up);
```

## Key Differences Run 2 vs Run 3

| Aspect | Run 2 (NanoAODv9) | Run 3 (NanoAODv13) |
|--------|-------------------|---------------------|
| Pileup Algorithm | CHS | PUPPI |
| JEC Application | L1+L2+L3+residual | L2+L3+residual |
| Jet ID | NanoAOD bits | Custom implementation |
| B-Tagger | DeepJet primary | DeepJet primary |
| SystematicTools | correctionlib | correctionlib |

## References
- [CMS NanoAOD Documentation](https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookNanoAOD)
- [Run 3 Jet Performance](http://cds.cern.ch/record/2911750)
- [CMS Jet ID 13.6 TeV](https://twiki.cern.ch/twiki/bin/viewauth/CMS/JetID13p6TeV)
- [Latest JEC/JER Recommendation](https://cms-jerc.web.cern.ch/Recommendations/)
