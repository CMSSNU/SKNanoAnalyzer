#!/bin/bash
echo -e "\033[31m##################### WARNING ########################\033[0m"
echo -e "\033[31m####         THIS IS DEVELOPMENT VERSION          ####\033[0m"
echo -e "\033[31m######################################################\033[0m"
echo ""


# Set up environment
export SKNANO_HOME=`pwd`
#export SKNANO_HOME="/data9/Users/$USER/SKNanoAnalyzer"
export SKNANO_RUNLOG="/gv0/Users/$USER/SKNanoRunlog"
export SKNANO_OUTPUT="/gv0/Users/$USER/SKNanoOutput"
echo "@@@@ Working Directory: $SKNANO_HOME"

# check configuration
CONFIG_FILE=$SKNANO_HOME/config/config.$USER
if [ -f "${CONFIG_FILE}" ]; then
    echo "@@@@ Reading configuration from $CONFIG_FILE"
    SYSTEM=$(grep '\[SYSTEM\]' "${CONFIG_FILE}" | cut -d' ' -f2)
    PACKAGE=$(grep '\[PACKAGE\]' "${CONFIG_FILE}" | cut -d' ' -f2)
    export TOKEN_TELEGRAMBOT=$(grep '\[TOKEN_TELEGRAMBOT\]' "${CONFIG_FILE}" | cut -d' ' -f2)
    export USER_CHATID=$(grep '\[USER_CHATID\]' "${CONFIG_FILE}" | cut -d' ' -f2)
    export SINGULARITY_IMAGE=$(grep '\[SINGULARITY_IMAGE\]' "${CONFIG_FILE}" | cut -d' ' -f2)
else
    echo "@@@@ Configuration file $CONFIG_FILE not found"
    echo "@@@@ Please create a configuration file in config/ with your username"
fi
echo "@@@@ System:  $SYSTEM"
echo "@@@@ Package: $PACKAGE"
echo "@@@@ Telegram Bot Token: $TOKEN_TELEGRAMBOT"
echo "@@@@ Telegram Chat ID: $USER_CHATID"
echo "@@@@ Using singularity image: $SINGULARITY_IMAGE"

# ROOT Package Settings
if [ $PACKAGE = "conda" ]; then
    echo "@@@@ Primary environment using conda"
    IS_SINGULARITY=$(env | grep -i "SINGULARITY_ENVIRONMENT")
    if [[ -n "$IS_SINGULARITY" || -n "$GITHUB_ACTION" ]]; then
        # Building within Singularity image, will be used for batch jobs
        echo "@@@@ Detected Singularity environment"
        source /opt/conda/bin/activate
        conda activate torch
    else
        source ~/.conda-activate
        #source /data9/Users/$USER/miniconda3/bin/activate
        conda activate nano
    fi
elif [ $PACKAGE = "mamba" ]; then
    # set up mamba environment
    micromamba activate Nano
    # from this point on, we can follow conda version of setup
    PACKAGE="conda"
elif [ $PACKAGE = "cvmfs" ]; then
    echo -e "\033[31m@@@@ cvmfs is not supported anymore\033[0m"
    exit
else
    echo "@@@@ Package not recognized"
    echo "@@@@ Please check configuration file in config/config.$USER"
fi
echo "@@@@ ROOT path: $ROOTSYS"


export SKNANO_VERSION="Run3_v12_Run2_v9"
export SKNANO_DATA=$SKNANO_HOME/data/$SKNANO_VERSION
mkdir -p $SKNANO_DATA

export SKNANO_BIN=$SKNANO_HOME/bin
export SKNANO_PYTHON=$SKNANO_HOME/python
export SKNANO_BUILDDIR=$SKNANO_HOME/build
export SKNANO_INSTALLDIR=$SKNANO_HOME/install
export PATH=$SKNANO_PYTHON:$PATH
export PYTHONPATH=$PYTHONPATH:$SKNANO_PYTHON
export SKNANO_LIB=$SKNANO_INSTALLDIR/lib
export SKNANO_RUN3_NANOAODPATH="/gv0/DATA/SKNano/Run3NanoAODv12/"
export SKNANO_RUN2_NANOAODPATH="/gv0/DATA/SKNano/Run2NanoAODv9/"
export ROOT_INCLUDE_DIRS=$ROOT_INCLUDE_DIRS:$SKNANO_INSTALLDIR/include

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SKNANO_LIB
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:$SKNANO_LIB

# setting LHAPDFs
if [[ ! -d "external/lhapdf" ]]; then
    echo "@@@@ Installing LHAPDF for conda environment"
    ./scripts/install_lhapdf.sh
    if [ $? -ne 0 ]; then
        echo -e "\033[31m@@@@ LHAPDF installation failed\033[0m"
        exit 1
    fi
fi
export PATH=$PATH:$SKNANO_HOME/external/lhapdf/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SKNANO_HOME/external/lhapdf/lib
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:$SKNANO_HOME/external/lhapdf/lib
export LHAPDF_DATA_PATH=$SKNANO_HOME/external/lhapdf/data
export LHAPDF_INCLUDE_DIR=`lhapdf-config --incdir`
export LHAPDF_LIB_DIR=`lhapdf-config --libdir`

echo "@@@@ LHAPDF include: $LHAPDF_INCLUDE_DIR"
echo "@@@@ LHAPDF lib: $LHAPDF_LIB_DIR"
echo "@@@@ reading data from $LHAPDF_DATA_PATH"

# setting up libtorch
if [[ ! -d "external/libtorch" ]]; then
    echo "@@@@ Installing LibTorch"
    ./scripts/install_libtorch.sh
    if [ $? -ne 0 ]; then
        echo -e "\033[31m@@@@ LibTorch installation failed\033[0m"
        exit 1
    fi
fi
export LIBTORCH_INCLUDE_DIR=$SKNANO_HOME/external/libtorch/include
export LIBTORCH_LIB_DIR=$SKNANO_HOME/external/libtorch/lib
export LIBTORCH_INSTALL_DIR=$SKNANO_HOME/external/libtorch

# env for correctionlib
CORRECTIONLIBS=$(conda list | grep "correctionlib")
if [ -z "$CORRECTIONLIBS" ]; then
    echo -e "\033[31m@@@@ correctionlib not found in conda environment\033[0m"
    echo -e "\033[31m@@@@ Please install correctionlib in conda environment\033[0m"
    exit 1
fi
export CORRECTION_INCLUDE_DIR=`correction config --incdir`
export CORRECTION_LIB_DIR=`correction config --libdir`
export JSONPOG_INTEGRATION_PATH=$SKNANO_HOME/external/jsonpog-integration
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CORRECTION_LIB_DIR
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:$CORRECTION_LIB_DIR

echo "@@@@ Correction include: $CORRECTION_INCLUDE_DIR"
echo "@@@@ Correction lib: $CORRECTION_LIB_DIR"

# env for onnxruntime
ONNXRUNTIME=$(conda list | grep "onnxruntime")
if [ -z "$ONNXRUNTIME" ]; then
    echo -e "\033[31m@@@@ onnxruntime not found in conda environment\033[0m"
    echo -e "\033[31m@@@@ Please install onnxruntime in conda environment\033[0m"
    exit 1
fi
export ONNXRUNTIME_INCLUDE_DIR=${CONDA_PREFIX}/include/onnxruntime/core/session
export ONNXRUNTIME_LIB_DIR=${CONDA_PREFIX}/lib
