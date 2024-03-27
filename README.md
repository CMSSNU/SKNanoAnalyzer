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
git clone git@github.com:$GITACCOUNT/SKNanoAnalyzer.git

# Add to your remote repo
git remote add upstream git@github.com:CMSSNU/SKNanoAnalyzer.git
git checkout $DEVBRANCH

# first time setup
# NOTE: Assuming conda environment, no CMSSW or SFT dependencies!
source setup.sh    # you have to do this every new session
# install lhapdf
./script/install_lhapdf.sh
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
For using LHAPDFHandler and PDFReweight classes, you should download the pdfsets from [here](http://lhapdfsets.web.cern.ch/lhapdfsets/current/).
Or you can link the path as
```bash
export LHAPDF_DATA_PATH="/cvmfs/sft.cern.ch/lcg/external/lhapdfsets/current/"
```
but it slows down in the initialization step

## To do
- Make ExampleRun runable -> done
- DataFormats -> Electron, Muon, LHE(Jin), Jet/GenJet (Yeonjun), Tau/FatJet(Youngwan), Gen(Taehee)
- TODO: start validation task, check installation of correctionlib, private NanoAOD generation
- external dependencies: LHAPDF / GEScaleSyst(?) / CorrectionLib


