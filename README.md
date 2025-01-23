# SKNanoAnalyzer
---

## THIS IS DEVELOPMENT VERSION!!


## Notes
- ROOT v6.32.xx have memory leak issue while reading root files with <= v6.30.xx -> Avoid using ROOT >= v6.32.xx temporariliy
## Introduction
- If you want to engage in the development and commit to the repository, please read [Development Guide](docs/DevelopmentGuide.md) first.
- For setting up the environment and starting the analysis, please refer [Getting Started](docs/GettingStarted.md).

## To do
### Assigned
- [ ] Add new features to ExampleRun.cc and make them manual: **Yeonjoon**
- [ ] Check the update in jsonpog-correction when sourcing setup.sh: **Chihwan**
- [ ] Clean up gv0: **Jin**
- [ ] Compare the sample list in SKFlat with the necessary samples, and bring them if necessary, check xsec, etc.: **Taehee+Eunsu**
- [ ] Update the CheckStorages.sh to respect the folder structure under gv0: **Jin**
- [ ] FatJet and Tau: **Youngwan**


### Not Assigned Yet
- [ ] Auto-Validation: Make validation files and check the consistency of the output for a specific NanoAOD file
- [ ] Update the network to 10G? (later)
- [ ] Check for CVMFS environment compatability
- [ ] Update MeasureTaggingEff.cc
- [ ] Rochester Correction Follow-up

For DY, check the before/after of
- [ ] Electrons
- [ ] Muons
- [ ] Jets(B-tagging)

## Make Tutorial
- [ ] From make Training tree, Export ONNX, Use this model in the Analyzer : **Yeonjoon**
- [ ] tt reconstruction using KinFitter
- [ ] NLO vs LO comparison


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
