#!/bin/bash
HOSTNAME=`hostname`
echo -e "\033[32m########################################################################\033[0m"
echo -e "\033[32m                                                                        \033[0m"
echo -e "\033[32m        ███████╗██╗  ██╗███╗   ██╗ █████╗ ███╗   ██╗ ██████╗            \033[0m"
echo -e "\033[32m        ██╔════╝██║ ██╔╝████╗  ██║██╔══██╗████╗  ██║██╔═══██╗           \033[0m"
echo -e "\033[32m        ███████╗█████╔╝ ██╔██╗ ██║███████║██╔██╗ ██║██║   ██║           \033[0m"
echo -e "\033[32m        ╚════██║██╔═██╗ ██║╚██╗██║██╔══██║██║╚██╗██║██║   ██║           \033[0m"
echo -e "\033[32m        ███████║██║  ██╗██║ ╚████║██║  ██║██║ ╚████║╚██████╔╝           \033[0m"
echo -e "\033[32m        ╚══════╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝╚═╝  ╚═══╝ ╚═════╝            \033[0m"
echo -e "\033[32m                                                                        \033[0m"
echo -e "\033[32m                            Version 2.0.0                               \033[0m"
echo -e "\033[32m########################################################################\033[0m"
echo ""

# check os
if [[ "$(uname)" == "Darwin" ]]; then
    export SYSTEM="osx"
elif [[ -f "/etc/redhat-release" ]]; then
    export SYSTEM="redhat"
else
    echo -e "\033[31mUnsupported OS\33[0m"
    return 1
fi

# Set up environment
export SKNANO_HOME=`pwd`
export SKNANO_RUNLOG="/gv0/Users/$USER/SKNanoRunlog"
export SKNANO_OUTPUT="/gv0/Users/$USER/SKNanoOutput"
# Root of the input productions. Sample metadata stores only the part below
# this, so moving to another cluster is one variable rather than a rewrite of
# every sample json.
export SKNANO_INPUT_ROOT="/gv0/DATA/SKNano/NanoAODv15_RNTuple"
echo "@@@@ Working Directory: $SKNANO_HOME"

CONFIG_FILE="$SKNANO_HOME/config/config.$USER"
# micromamba is the supported package manager; the config file may override it.
PACKAGE="${PACKAGE:-mamba}"
# check configuration
if [ -f "${CONFIG_FILE}" ]; then
    echo -e "\033[32m@@@@ Reading configuration from $CONFIG_FILE\033[0m"
    # Read one "[KEY] value" line. awk rather than cut, because cut prints the
    # whole line when it contains no delimiter, which turned a valueless
    # "[SINGULARITY_IMAGE]" into the literal string "[SINGULARITY_IMAGE]".
    _sknano_config_value() {
        awk -v k="[$1]" '$1 == k { print $2; exit }' "${CONFIG_FILE}"
    }
    _sknano_value=$(_sknano_config_value PACKAGE)
    [[ -n "${_sknano_value}" ]] && PACKAGE="${_sknano_value}"
    export TOKEN_TELEGRAMBOT=$(_sknano_config_value TOKEN_TELEGRAMBOT)
    export USER_CHATID=$(_sknano_config_value USER_CHATID)
    export SINGULARITY_IMAGE=$(_sknano_config_value SINGULARITY_IMAGE)
    # Override the defaults above. A key that is present but empty --
    # config.default ships every key that way -- keeps the default.
    for _sknano_key in SKNANO_HOME SKNANO_RUNLOG SKNANO_OUTPUT SKNANO_INPUT_ROOT \
                       MAMBA_EXE MAMBA_ROOT_PREFIX; do
        _sknano_value=$(_sknano_config_value "${_sknano_key}")
        if [[ -n "${_sknano_value}" ]]; then
            export "${_sknano_key}=${_sknano_value}"
        fi
    done
    unset _sknano_key _sknano_value
    unset -f _sknano_config_value
else
    echo -e "\033[31m@@@@ Configuration file $CONFIG_FILE not found\033[0m"
    echo -e "\033[3m@@@@ Please create a configuration file in config/ with your username\033[0m"
fi
echo "@@@@ System:  $SYSTEM"
echo "@@@@ Package: $PACKAGE"
if [[ -n "$TOKEN_TELEGRAMBOT" && -n "$USER_CHATID" ]]; then
    echo "@@@@ Telegram reporting: configured"
else
    echo "@@@@ Telegram reporting: disabled"
fi
echo "@@@@ Using singularity image: $SINGULARITY_IMAGE"
echo "@@@@ SKNano Home: $SKNANO_HOME"
echo "@@@@ SKNano Runlog: $SKNANO_RUNLOG"
echo "@@@@ SKNano Output: $SKNANO_OUTPUT"

# ROOT Package Settings
if [ "$PACKAGE" = "conda" ]; then
    echo -e "\033[32m@@@@ Primary environment using conda\033[0m"
    IS_SINGULARITY=$(env | grep -i "SINGULARITY_ENVIRONMENT")
    if [[ -n "$IS_SINGULARITY" || -n "$GITHUB_ACTION" ]]; then
        # Building within Singularity image, will be used for batch jobs
        echo -e "\033[32m@@@@ Detected Singularity environment\033[0m"
        source /opt/conda/bin/activate
        conda activate torch
    else
        source ~/.conda-activate
        conda activate nano
    fi
elif [ "$PACKAGE" = "mamba" ]; then
    echo -e "\033[32m@@@@ Primary environment using mamba\033[0m"
    # Detect Singularity or GitHub Actions
    IS_SINGULARITY=$(env | grep -i "SINGULARITY_ENVIRONMENT")
    if [[ -n "$IS_SINGULARITY" || -n "$GITHUB_ACTION" ]]; then
        echo -e "\033[32m@@@@ Detected Singularity environment\033[0m"
    else
        # The configured micromamba wins over whatever happens to be on PATH,
        # so a shell profile pointing elsewhere cannot silently swap the
        # environment out from under a submission.
        if [[ -n "${MAMBA_EXE:-}" && -x "${MAMBA_EXE}" ]]; then
            export PATH="$(dirname "${MAMBA_EXE}"):${PATH}"
        elif ! command -v micromamba &> /dev/null; then
            export PATH="$HOME/micromamba/bin:${PATH}"
            : "${MAMBA_ROOT_PREFIX:=$HOME/micromamba}"
            export MAMBA_ROOT_PREFIX
        fi
    fi

    # micromamba shell hook should only be run once per session
    if [[ -z "$__MAMBA_SETUP_DONE" ]]; then
        unalias mamba 2>/dev/null
        _sknano_shell="${ZSH_VERSION:+zsh}${BASH_VERSION:+bash}"
        eval "$(micromamba shell hook -s "${_sknano_shell:-bash}")"
        unset _sknano_shell
        export __MAMBA_SETUP_DONE=1
    fi
    micromamba activate Nano
    # from this point on, we can follow conda version of setup
    PACKAGE="conda"
    # A function rather than an alias: bash does not expand aliases in
    # non-interactive shells, so the `conda list` checks below failed with
    # "conda: command not found" whenever setup.sh was sourced from bash.
    conda() { micromamba "$@"; }
elif [ "$PACKAGE" = "cvmfs" ]; then
    echo -e "\033[31m@@@@ cvmfs is not supported anymore\033[0m"
    return 1
else
    echo "@@@@ Package not recognized"
    echo "@@@@ Please check configuration file in config/config.$USER"
fi
echo "@@@@ ROOT path: $ROOTSYS"


export SKNANO_VERSION="Run3_v15_Run2_v15"
export SKNANO_DATA=$SKNANO_HOME/data/$SKNANO_VERSION
mkdir -p $SKNANO_DATA

export SKNANO_BIN=$SKNANO_HOME/bin
export SKNANO_PYTHON=$SKNANO_HOME/python
export SKNANO_BUILDDIR=$SKNANO_HOME/build/$SYSTEM
export SKNANO_INSTALLDIR=$SKNANO_HOME/install/$SYSTEM
export PATH=$SKNANO_PYTHON:$PATH
export PYTHONPATH=$PYTHONPATH:$SKNANO_PYTHON
export SKNANO_LIB=$SKNANO_INSTALLDIR/lib
if [[ -z "$SKNANO_ANALYSIS_MODULE_DIRS" ]]; then
    _sknano_module_dirs="$(find "$SKNANO_HOME" -maxdepth 1 -type d -name '*_Analyzers' \
        -exec test -f '{}/CMakeLists.txt' \; -print | paste -sd';' -)"
    [[ -n "$_sknano_module_dirs" ]] && export SKNANO_ANALYSIS_MODULE_DIRS="$_sknano_module_dirs"
    unset _sknano_module_dirs
fi
export SKNANO_RUN3_NANOAODPATH="/gv0/DATA/SKNano/NanoAODv15/"
export SKNANO_RUN2_NANOAODPATH="/gv0/DATA/SKNano/NanoAODv15/"

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SKNANO_LIB
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:$SKNANO_LIB

# setting LHAPDFs
if [[ ! -d "external/lhapdf/$SYSTEM" ]]; then
    echo -e "\033[32m@@@@ Installing LHAPDF for conda environment\033[0m"
    ./scripts/install_lhapdf.sh
    if [ $? -ne 0 ]; then
        echo -e "\033[31m@@@@ LHAPDF installation failed\033[0m"
        return 1
    fi
fi
export PATH=$PATH:$SKNANO_HOME/external/lhapdf/$SYSTEM/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SKNANO_HOME/external/lhapdf/$SYSTEM/lib
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:$SKNANO_HOME/external/lhapdf/$SYSTEM/lib
export LHAPDF_DATA_PATH=$SKNANO_HOME/external/lhapdf/data
export LHAPDF_INCLUDE_DIR=$SKNANO_HOME/external/lhapdf/$SYSTEM/include
export LHAPDF_LIB_DIR=$SKNANO_HOME/external/lhapdf/$SYSTEM/lib
#export LHAPDF_INCLUDE_DIR=`lhapdf-config --incdir`
#export LHAPDF_LIB_DIR=`lhapdf-config --libdir`

echo "@@@@ LHAPDF include: $LHAPDF_INCLUDE_DIR"
echo "@@@@ LHAPDF lib: $LHAPDF_LIB_DIR"
echo "@@@@ reading data from $LHAPDF_DATA_PATH"

# setting up libtorch
if [[ ! -d "external/libtorch" ]]; then
    echo -e "\033[32m@@@@ Installing LibTorch\033[0m"
    ./scripts/install_libtorch.sh
    if [ $? -ne 0 ]; then
        echo -e "\033[31m@@@@ LibTorch installation failed\033[0m"
        return 1
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
    return 1 
fi

export CORRECTION_INCLUDE_DIR=`correction config --incdir`
export CORRECTION_LIB_DIR=`correction config --libdir`
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CORRECTION_LIB_DIR
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:$CORRECTION_LIB_DIR
echo "@@@@ Correction include: $CORRECTION_INCLUDE_DIR"
echo "@@@@ Correction lib: $CORRECTION_LIB_DIR"

# ROCCOR
export ROCCOR_PATH=$SKNANO_HOME/external/RoccoR

# The RoccoR submodule carries no build files, so they come from templates/.
# They are generated, not edited, so refresh them whenever they differ: copying
# only when missing left an old copy in place after the template was fixed, and
# the stale one then failed the CMake configure step.
for _sknano_roccor_file in CMakeLists.txt RoccoR_LinkDef.hpp; do
    _sknano_template="$SKNANO_HOME/templates/RoccoR/$_sknano_roccor_file"
    if [[ ! -f "$_sknano_template" ]]; then
        echo -e "\033[31m@@@@ Template $_sknano_roccor_file not found in templates/RoccoR/\033[0m"
    elif ! cmp -s "$_sknano_template" "$ROCCOR_PATH/$_sknano_roccor_file"; then
        cp "$_sknano_template" "$ROCCOR_PATH/$_sknano_roccor_file"
        echo -e "\033[32m@@@@ Copied $_sknano_roccor_file to external/RoccoR/\033[0m"
    fi
done
unset _sknano_roccor_file _sknano_template

# JSONPOG integration auto-update
check_jsonpog_updates() {
    local auto_update=${1:-false}
    echo -e "\033[32m@@@@ Checking for updates in jsonpog-integration repository...\033[0m"
    export JSONPOG_REPO_PATH="$SKNANO_HOME/external/jsonpog-integration"

    if [ "$auto_update" = false ]; then
        echo -e "\033[32m@@@@ Auto-update is disabled. Skipping update check.\033[0m"
        return 0
    fi

    if [ ! -d "$JSONPOG_REPO_PATH" ]; then
        echo -e "\033[31m@@@@ JSONPOG Repository not found\033[0m"
        return 1
    fi
    
    cd "$JSONPOG_REPO_PATH"
    git fetch origin
    CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD)

    if [ -z "$(git symbolic-ref -q HEAD)" ]; then
        echo -e "\033[32m@@@@ HEAD is detached. Switching back to the previous branch: $CURRENT_BRANCH\033[0m"
        git checkout "$CURRENT_BRANCH"
    fi

    # Check the current version of local and jsonpog version
    LOCAL_COMMIT_HASH=$(git rev-parse HEAD)
    LOCAL_COMMIT_DATE=$(git log -1 --format=%ci)
    echo -e "\033[32m@@@@ Current Local commit: $LOCAL_COMMIT_HASH\033[0m"
    echo -e "\033[32m@@@@ Current Local commit date: $LOCAL_COMMIT_DATE\033[0m"
    UPSTREAM_COMMIT_HASH=$(git ls-remote origin -h refs/heads/master | awk '{print $1}')
    UPSTREAM_COMMIT_DATE=$(git log -1 --format=%ci origin/master)
    echo -e "\033[32m@@@@ Latest JSONPOG (origin/master) commit: $UPSTREAM_COMMIT_HASH\033[0m"
    echo -e "\033[32m@@@@ Latest JSONPOG commit date: $UPSTREAM_COMMIT_DATE\033[0m"
    
    # Check if the local repository is behind the remote repository
    BEHIND=$(git rev-list --count origin/master..HEAD)

    if [ "$BEHIND" -gt 0 ]; then
        echo -e "\033[32m@@@@ Repository is $BEHIND commits behind origin/master.\033[0m"

        if [ "$auto_update" = true ]; then
            echo -e "\033[32m@@@@ Auto-update is enabled. Updating jsonpog-integration repository...\033[0m"
            git merge origin/master
            echo -e "\033[32m@@@@ Update completed!\033[0m"
        else
            echo -e "\033[32m@@@@ Auto-update is disabled. Skipping update.\033[0m"
        fi
    else
        echo -e "\033[32m@@@@ jsonpog-integration repository is already up-to-date.\033[0m"
    fi

    cd "$SKNANO_HOME"
}

# Call the function with auto_update set to false by default
# To enable auto-update, call with: check_jsonpog_updates true
check_jsonpog_updates false

export JSONPOG_REPO_PATH="$SKNANO_HOME/CMS_corrections"

# env for onnxruntime
ONNXRUNTIME=$(conda list | grep "onnxruntime")
if [ -z "$ONNXRUNTIME" ]; then
    echo -e "\033[31m@@@@ onnxruntime not found in conda environment\033[0m"
    echo -e "\033[31m@@@@ Please install onnxruntime in conda environment\033[0m"
    return 1
fi
export ONNXRUNTIME_INCLUDE_DIR=${CONDA_PREFIX}/include/onnxruntime/core/session
export ONNXRUNTIME_LIB_DIR=${CONDA_PREFIX}/lib
echo "@@@@ onnxruntime include: $ONNXRUNTIME_INCLUDE_DIR"
echo "@@@@ onnxruntime lib: $ONNXRUNTIME_LIB_DIR"
