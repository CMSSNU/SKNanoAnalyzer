#!/bin/bash
unset CONDA_PREFIX CONDA_DEFAULT_ENV CONDA_PROMPT_MODIFIER CONDA_SHLVL
unset MAMBA_EXE
export PATH=[MAMBA_BIN_PATH]:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
export MAMBA_ROOT_PREFIX=[MAMBA_ROOT_PREFIX]
# [MAMBA_BIN_PATH] lives inside the singularity image the batch jobs run in.
# The same script is also run by hand on a login node, where that prefix does
# not exist, so fall back to the micromamba the submitting shell used.
if ! command -v micromamba > /dev/null 2>&1; then
    export PATH=[MAMBA_HOST_BIN_PATH]:$PATH
    export MAMBA_ROOT_PREFIX=[MAMBA_HOST_ROOT_PREFIX]
fi
if ! command -v micromamba > /dev/null 2>&1; then
    echo "micromamba not found in [MAMBA_BIN_PATH] nor [MAMBA_HOST_BIN_PATH]" >&2
    exit 1
fi
eval "$(micromamba shell hook -s bash)"
if [ -d "${MAMBA_ROOT_PREFIX}/envs/[MAMBA_ENV]" ]; then
    micromamba activate [MAMBA_ENV] || exit 1
else
    micromamba activate [MAMBA_ENV_PREFIX] || exit 1
fi

export SKNANO_HOME=[SKNANO_HOME]
export SKNANO_DATA=[SKNANO_DATA]
export SKNANO_BIN=$SKNANO_HOME/bin
export PYTHONPATH=$PYTHONPATH:$SKNANO_HOME/python

export PATH=$PATH:[LHAPDF_BIN_DIR]
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:[LHAPDF_LIB_DIR]
export LHAPDF_INCLUDE_DIR=[LHAPDF_INCLUDE_DIR]
export LHAPDF_LIB_DIR=[LHAPDF_LIB_DIR]
export CORRECTION_INCLUDE_DIR=`correction config --incdir`
export CORRECTION_LIB_DIR=`correction config --libdir`
export JSONPOG_REPO_PATH=[JSONPOG_REPO_PATH]
export ROCCOR_PATH=[ROCCOR_PATH]
export ONNXRUNTIME_INCLUDE_DIR=${CONDA_PREFIX}/include/onnxruntime/core/session
export ONNXRUNTIME_LIB_DIR=${CONDA_PREFIX}/lib

export ROOT_HIST=0

# Run from a copy on the node's local disk; see templates/run.sh.
run_dir=[WORKDIR]
run_lib=[SKNANO_RUNLOG_LIB]
run_include=[SKNANO_RUNLOG_INCLUDE]
if [ -n "${_CONDOR_SCRATCH_DIR:-}" ]; then
    # A subdirectory: HTCondor transfers back only top-level sandbox files.
    stage=$_CONDOR_SCRATCH_DIR/sknano_stage
else
    stage=$(mktemp -d "${TMPDIR:-/tmp}/sknano_stage.XXXXXX") && trap 'rm -rf "$stage"' EXIT
fi
if [ -n "$stage" ] && mkdir -p "$stage/lib" "$stage/include" \
    && cp -rL "$run_lib/." "$stage/lib" \
    && cp -rL "$run_include/." "$stage/include" \
    && cp "$run_dir/job_$1.py" "$stage/"; then
    run_dir=$stage
    run_lib=$stage/lib
    run_include=$stage/include
else
    echo "run.python.sh: staging to local disk failed, running from $run_dir" >&2
fi

export LD_LIBRARY_PATH=$run_lib:${CONDA_PREFIX}/lib:[LHAPDF_LIB_DIR]:$CORRECTION_LIB_DIR
export ROOT_INCLUDE_PATH=$run_include[ROOT_INCLUDE_PATH_EXTRA]

cd "$run_dir"

python job_$1.py
exit $?
