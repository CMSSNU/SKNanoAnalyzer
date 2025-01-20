# SKNanoAnalyzer
---

## THIS IS DEVELOPMENT VERSION!!


## Notes
- ROOT v6.32.xx have memory leak issue while reading root files with <= v6.30.xx -> Avoid using ROOT >= v6.32.xx temporariliy
## How to
### Preliminary Setup
Here is an example to setup the environment using conda.
```bash
# create conda environment
conda create -n nano python=3.11 root=6.30.04 -c conda-forge
conda activate nano

# Install onnxruntime-cpp
conda install onnxruntime-cpp

# Install correctionlib
pip install correctionlib
```
I recommend to use micromamba, which is a faster alternative to anaconda that is infamous for its slow speed to solving the environment.
```bash
"${SHELL}" <(curl -L micro.mamba.pm/install.sh)
```
Then, you can create the environment using micromamba, just replace `conda` with `micromamba`.

For both `micromamba` and `conda`, Do not install the packages in home directory. It is because home directory cannot be accessed by the worker nodes. Use `/data6/Users/foo` instead.

If you are choose to use `micromamba` set your `[PACKAGE]` as `mamba` in the config file.
you can copy my environment by use `Nano.yml` under templates directory. Just Change the `prefix` to your own directory.
```bash
# @ $SKNANO_HOME/templates
# After modifying the prefix in Nano.yml
micromamba env create -f Nano.yml
```
### Installation
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

### Singularity Support
If you want to use Singularity image for the batch job, first compile the project within singularity image.
```bash
singularity exec $SINGULARITY_IMAGE bash -c "source setup.sh && ./scripts/build.sh"
```
`$SINGULARITY_IMAGE` variable will be automatically parsed from `config/config.$USER` file. Use SKNano.py to submit
batch jobs:
```bash
SKNano.py -a ExampleRun -i DYJets -e 2022 -n 10 --reduction 10 --no_exec ...
```


### Testing PRs
Here is a recommended way to test PRs.
```bash
# Always start from the clean state
git clone --recurse-submodules git@github.com:$GITACCOUNT/SKNanoAnalyzer.git
cd SKNanoAnalyzer
git remote add upstream git@github.com:CMSSNU/SKNanoAnalyzer.git

# fetch the PR
git fetch upstream pull/$PRNUMBER/head:pr$PRNUMBER
git checkout pr$PRNUMBER

# for example, fetching PR 16
git fetch upstream pull/16/head:pr16
git checkout pr16
```

### Compilation
We are using cmake for the default compiling management.
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
conda env create -n nano python=3.11 root=6.30.04 -c conda-forge
conda activate nano
pip install correctionlib
```
## How to Submit the job

Jobs can be submitted to htcondor using SKFlat.py
```bash
SKFlat.py -a AnalyzerName -i SamplePD -n number of jobs -e era
```

Basic usage is as aboves. There are some additional options for the submission:
- -i: You can pass the sample PD using this option. This option supports the basic regex, thus,
  ```bash
  SKFlat.py -a Vcb_FH -i 'ST*' -n 100 -e 2022EE
    ```
    will submit the jobs for all the samples starting with 'ST' in the sample list. Thus, please be careful when you adding the new samples to the sample list.
- -n: Number of jobs to submit. If you want to submit 100 jobs, you can use -n 100.
  You can also choose to set the ***number of files for each job***. To do this, pass this argument as negative value. For example, -n -10 will submit 10 files per job. For example, if *TTLJ_powheg* sample has 1700 files, 
  ```bash
    SKFlat.py -a Vcb_FH -i TTLJ_powheg -n -10 -e 2022EE
    ```
    will submit 170 jobs with 10 files each to cluster.
- -e: Era of the sample. You can also pass the multiple eras using comma. For example, -e 2022EE,2023 will submit the jobs for the samples in 2022EE and 2023 era.
- -r: This argument set the run. choose Run2 or Run3. This option overrids the -e option.
- --reduction: perform reduction by factor of input number. 
- --memory: set the memory for the job. Default is 2GB.
- --ncpu: set the number of cpus for the job. Default is 1.
- --userflags: set the user flags for the job. Default is empty. to set multiple flags, use comma. e.g. --userflags flag1,flag2
- --batchname: set the batch name for the job. Default is the analyzer name_userflags.
- --skimming_mode: by passing this flag, SKFlat.py will submit the jobs for the skimming mode. Detailed information as follows.

### Skimming mode
By passing --skimming_mode, SKFlat.py will submit the jobs for the skimming mode. In this mode, the jobs will create the output in `$SKNANO_RUN[2,3]_NANOAODPATH/Era/[Data,MC]/Skim/$USERNAME` directory, Instead of submit hadd layer in DAG, *PostProc* layer will add in the DAG. 

If your analyzer has name that starts with "Skim_", you will be asked to be enable the skimming mode. If you choose to enable the skimming mode, then skimming mode will be activated. Of course you can manually activate the skimming mode by passing --skimming_mode flag.

PostProc layer will create the Skimmed sample folder and will creat the skimTreeInfo.json file and dedicated json that saves the information of the skimmed samples under $SKNANO_DATA/era/Sample/Skim directory.
Each postproc job modify the skimTreeInfo.json sequentially, so ***DO NOT SUBMIT THE MULTIPLE DAG CLUSTERS THAT DO SKIMMING.*** After all the postproc jobs are done, you can submit the new jobs that using skimmed sample. A prefix of Skim_AnalyzerName will be added to the output file name.
```bash
SKFlat.py -a AnalyzerName -i DYJets -n -1 -e era --skimming_mode
```
or 
```bash
SKFlat.py -a Skim_AnalyzerName -i DYJets -n -1 -e era
```
Will create the Skim_AnalyzerName_DYJets (if you choose to answer "y" in latter one).
Then you can submit the jobs by
```bash
SKFlat.py -a AnalyzerName -i Skim_AnalyzerName_DYJets -n -1 -e era
```

## Note

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
