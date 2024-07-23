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
conda env create -n nano python=3.11 root=6.30.04 -c conda-forge
conda activate nano
pip install correctionlib
```

## To do
- Make ExampleRun runable -> done
- DataFormats -> Electron, Muon, LHE(Jin), Jet/GenJet (Yeonjun), Tau/FatJet(Youngwan), Gen(Taehee)
- TODO: start validation task, check installation of correctionlib, private NanoAOD generation
- external dependencies: LHAPDF / GEScaleSyst(?) / CorrectionLib


