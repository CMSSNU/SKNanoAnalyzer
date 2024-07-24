# SKNanoAnalyzer
---

NOTE: THIS IS DEVELOPMENT VERSION!!

VERSION 0.1.0

## Project Updates
[HedgeDoc](https://demo.hedgedoc.org/VrWRIlceTjO9SPOVLKUNVA?view)

## How to
Recommend to fork the repo to your account.
```bash
# Clone the repository
# jsonpog-integration should be cloned as a submodule if you don't use cvmfs
git clone --recurse-submodules git@github.com:$GITACCOUNT/SKNanoAnalyzer.git

# Add to your remote repo
git remote add upstream git@github.com:CMSSNU/SKNanoAnalyzer.git
git checkout $DEVBRANCH

# create config file
cp config/config.default config/config.$USER
# edit the configuration!

# first time setup
source setup.sh    # you have to do this every new session
```

### Compilation
Now using cmake for the default compiling management.
Use scripts/build.sh for clean compilation.
```bash
./scripts/build.sh
```

Or you can do it manually
```bash
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=$SKNANO_HOME ..
make
make install
cd $SKNANO_HOME
```

## Check modules
Every module(or class) can be imported both in ROOT and python
```cpp
root -l
Particle *p = new Particle;
p->SetPtEtaPhiM(30, 2.1, 1.3, 0.1);
p->SetCharge(1);
p->Print()
```

```python
python
from ROOT import Particle
p = Particle()
p.SetPtEtaPhiM(30, 2.1, 1.3, 0.1)
p.SetCharge(1)
p.Print()
```

For testing other modules and analyzers, check scripts/test.py

## About LHAPDFs
For using LHAPDFHandler and PDFReweight classes, two possible options
1. install lhapdf manually.
```bash
./scripts/install_lhapdf.sh
```
It would be run automatically for the first time setup.
2. use lhapdf from cvmfs

## About correctionlibs
In the config/config.$USER file, there is an option to choose bewteen conda and cvmfs. When configuring your environment with conda, at least ROOT and correctionlibs should be installed:
```bash
# example
conda env create -n nano python=3.11
conda activate nano
conda install -c conda-forge root
pip install correctionlib
```

## To do
- Make ExampleRun runable -> done
- DataFormats -> Electron, Muon, LHE(Jin), Jet/GenJet (Yeonjun), Tau/FatJet(Youngwan), Gen(Taehee)
- TODO: start validation task, check installation of correctionlib, private NanoAOD generation
- external dependencies: LHAPDF / GEScaleSyst(?) / CorrectionLib

## Note

# SKFlat - Run3 Update

## General
- Scope of the update: Run3 only
- Baseline: Central NanoAOD → postproc (Private Skim + add branch)
- Any missing variables in NanoAOD? Need processing from MiniAOD?
- Private NANO: MiniAOD $\to$ NanoAOD (JME Private) 
- Jihoon will follow (SKFlatMaker): need time

## Object updates
- Event(Trigger, MET): trigger list 조사 / json? 영완
- DataStream 이름 조사 - 영완
- Muon / Electron: 진 / 영완 (MCCorrection updated)
- Tau / FatJet: 영완 믿습니다(brave student) (updated)
- JetTaggingParameter (updating): 영완
- Jet / GenJet: 연준 / 지훈 (updated)
- GEN: 태희 / 진 (updating)
- Photon: 지훈(neeeww) (updating)
    - isolation value만
SNUCMSSW? 지훈(neeeww)
- systematic variation update

## SKFlat.py refactoring
- Run3용으로 update
- Done: Dagman framwork, Telegram notification, No Data-period support, auto iterating submission

### Validation task
- Drell Yan Peak 보기
- Scale Variation
- LHAPDF reweight validation
- GEN + GENJET validation: flag가 맞는지 확인하는 정도


## To do
- [ ]  Make a list of Data / MC samples and check the replicas [google docs](https://docs.google.com/spreadsheets/d/1fyk_y6L2OoVgDvDmSCGPMKjQEKwaNSo0Ji2NXtNKRf8/edit?usp=sharing)
- [ ]  Postproc development → survey for objects / branches / corrections / scale factors -> after sample list finalized
- [ ]  Make Baseline 
- support for local run
- survey for /gv0 storage (not using skim files)
- 오브젝트 가져올때 Skim된 Nano에서 branch가 drop되어있는지 확인(e.g. flags) -> Youngwan will follow
- SNUCMS에 github repo 파기


RDataframe / uproot (memory issue) / SKFlat
SKFlat(base) -> std::vector<Muon>
             -> ROOT::RVec<Muon>
Correctionlib -> only python binding -> Youngwan will follow
    
- history fill -> hash map in C++?

## Update Notes
    
- Modify Muon WPID because only tightest wp was saved, i. e.) isMediumPfISo() return false for TightPfIso
- I update the commonsampleinfo.json(not txt). cross section can be stored as formula under xsec_formula. if one runs python sampleManger --updateXsec, it will evaluate xsec expression.
- But due to the ambiguos description on SM xsec @ 13.6 TeV twiki pages, some of them need updates.
- I update the GetEffLumi.cc, python sampleManager --updateMCinfo reads the root files from $SKNANO_OUTPUT directory and automatically updates the sumW, sumsign, etc.
- python sampleManager --fillSamplePath reads the sample path and updates the json file under forSNU
- L1 Prefire will be added @ NanoAOD v14
- Met Filters will be added @ NanoAOD v13 (but it seems it already stored as flag)
- GetPUWeights() added. NOTE!: there are several vertex-related branch is stored in NanoAOD. PV_ means reconstructed vertices. nPileUp means the generator level vertices. thus this branch is not available in data. there is another branch, nTrueInt, which Out-of-time pu is not considered. PU Correction should be done by this value. useful link: https://opendata.cern.ch/docs/cms-guide-pileup-simulation
- I modify the SKNanoLoader.cc to it respects the MC-only branches. 
- for now, I made the json file of HLT branch and set there branch address using loop(famous nlohman-json included to our repo). my suggestion is keep this track and also managing trigger luminosity with same file. other ideas or suggestions will be appreciated.
- and MCCorrection updates.
-JetVetoMaps and GetCutValues are in MCCorrection class despite they are also needed in data. It is because if there are scattered parts of code that use jsonpog-integrated path, code maintanence will make headache. how about change mccorrection to just 'correction'?
- B-tagging SF method are updated. c-tagging sf's not yet available.
- add MeasureJetTaggingEff.cc, and automated json generating code
- add MET Filters: But have to follow this issue: https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETOptionalFiltersRun2#ECal_BadCalibration_Filter_Flag
- modify kMax** -> https://github.com/cms-sw/cmssw/blob/CMSSW_13_0_X/PhysicsTools/NanoAOD/python/nanoDQM_cfi.py ?
- add Gen, LHE
- add puppimet
- Compression? for  TTLJ LZMA:9-579G/Uncompressed-3.2T/ZLIB:4-1.2T
LZMA time:  58.3945574760437
ZLIB time:  47.72870230674744
Uncomp time:  46.95170283317566 (my dy sample analyzer, total 70000 Event)
- add sorting code
- tree filling: NewTree, SetBranch, FillTrees. check implementation is correct.
- Add JER. 
- GenJet-Jet Matching
- Skimming mode (both input and output.)
- hashmap comparison: https://martin.ankerl.com/2022/08/27/hashmap-bench-01/#absl__flat_hash_map
absl(from google), folly(from facebook) shows 3~5 times higher performance, it is worth to implement these if the writing hists is true bottleneck.

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