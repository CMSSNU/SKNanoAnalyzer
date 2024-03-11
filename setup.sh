RELEASE="`cat /etc/redhat-release`"
echo "@@@@@@@@@@@@@@@@ WARNING @@@@@@@@@@@@@@@@@@@"
echo "@@@@ THIS IS DEVELOPMENT VERSION        @@@@"
echo "@@@@ Setup in SNU Server might not work @@@@"
echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"

# Set up environment
echo "@@@@ Working Directory: `pwd`"
export SKNANO_HOME=`pwd`
export SKNANO_RUNLOG="/home/$USER/workspace/SKNanoRunlog"
export SKNANO_OUTPUT="/home/$USER/workspace/SKNanoOutput"

# root configuration
source ~/.conda-activate
conda activate nano


export SKNANO_LIB=$SKNANO_HOME/lib
export SKNANO_VERSION="Run3UltraLegacy_v1"
export SKNANO_DATA=$SKNANO_HOME/data/$SKNANO_VERSION
mkdir -p $SKNANO_DATA

export SKNANO_BIN=$SKNANO_HOME/bin
export SKNANO_PYTHON=$SKNANO_HOME/python
export PATH=$SKNANO_PYTHON:$PATH
export PYTHONPATH=$PYTHONPATH:$SKNANO_PYTHON

export ROOT_INCLUDE_PATH=$ROOT_INCLUDE_PATH:$SKNANO_HOME/DataFormats/include
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SKNANO_LIB
