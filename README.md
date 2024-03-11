# SKNanoAnalyzer
---

NOTE: THIS IS DEVELOPMENT VERSION!!
VERSION 0.1.0

## Project Updates
[HedgeDoc](https://demo.hedgedoc.org/VrWRIlceTjO9SPOVLKUNVA?view)

## How to
```bash
# Clone the repository
git clone git@github.com:CMSSNU/SKNanoAnalyzer.git

# Add to your remote repo
git remote add upstream git@github.com:$GITACCOUNT/SKNanoAnalyzer.git
git checkout $DEVBRANCH

# No first time setup yet
# NOTE: Assuming conda environment, no CMSSW or SFT dependencies!
source setup.sh
```

### Compilation
Now using cmake for the default compiling management.
```bash
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=$SKNANO_HOME ..
make
make install
cd $SKNANO_HOME
```

If you want to compile with updated source codes, remove the build directory follow the steps again.
