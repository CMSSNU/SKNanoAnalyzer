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

# No first time setup yet
# NOTE: Assuming conda environment, no CMSSW or SFT dependencies!
source setup.sh
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

## To do
- Make ExampleRun runable
- Event / Jet / AnalyzerParams update
- external dependencies: LHAPDF / GEScaleSyst(?) / CorrectionLib


