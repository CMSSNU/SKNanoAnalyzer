echo -e "\033[31m##################### WARNING ########################\033[0m"
echo -e "\033[31m####         THIS IS DEVELOPMENT VERSION          ####\033[0m"
echo -e "\033[31m######################################################\033[0m"
echo ""

# Set up environment
echo "@@@@ Working Directory: `pwd`"
export SKNANO_HOME=`pwd`
export SKNANO_RUNLOG="/home/$USER/workspace/SKNanoRunlog"
export SKNANO_OUTPUT="/home/$USER/workspace/SKNanoOutput"

# root configuration
RELEASE="`cat /etc/redhat-release`"
echo "@@@@ Configuring ROOT for $RELEASE"
if [[ $RELEASE == *"7."* ]]; then
    source /cvmfs/sft.cern.ch/lcg/views/LCG_105/x86_64-centos7-gcc12-opt/setup.sh
elif [[ $RELEASE == *"8."* ]]; then
    source /cvmfs/sft.cern.ch/lcg/views/LCG_104/x86_64-centos8-gcc12-opt/setup.sh
elif [[ $RELEASE == *"9."* ]]; then
    source /cvmfs/sft.cern.ch/lcg/views/LCG_105/x86_64-el9-gcc13-opt/setup.sh
else
    echo "@@@@ Not running on redhat 7, 8, or 9"
    echo "@@@@ Assuming root is installed in conda environment"
    source ~/.conda-activate
    conda activate nano
fi
echo "@@@@ ROOT path: $ROOTSYS"


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
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:$SKNANO_LIB
