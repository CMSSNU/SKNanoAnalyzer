#!/bin/bash
set -eo pipefail

# Start from a clean environment, the same way run.sh does. The submitting
# shell is handed to the job verbatim (getenv=True), and a ROOTSYS or a
# LD_LIBRARY_PATH pointing at the host ROOT makes PyROOT segfault the moment
# it touches gROOT.
unset CONDA_PREFIX CONDA_DEFAULT_ENV CONDA_PROMPT_MODIFIER CONDA_SHLVL
unset MAMBA_EXE ROOTSYS PYTHONHOME PYTHONPATH LD_LIBRARY_PATH
export PATH="[MAMBA_BIN_PATH]:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
export MAMBA_ROOT_PREFIX="[MAMBA_ROOT_PREFIX]"
# [MAMBA_BIN_PATH] lives inside the singularity image the batch jobs run in.
# The same script is also run by hand on a login node, where that prefix does
# not exist, so fall back to the micromamba the submitting shell used.
if ! command -v micromamba > /dev/null 2>&1; then
    export PATH="[MAMBA_HOST_BIN_PATH]:$PATH"
    export MAMBA_ROOT_PREFIX="[MAMBA_HOST_ROOT_PREFIX]"
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

# Conda activation scripts may probe unset toolchain variables. Enable nounset
# only after the environment has finished activating.
set -u

cd [WORKDIR]
shopt -s nullglob
inputs=([INPUT_GLOB])
if (( ${#inputs[@]} == 0 )); then
  echo "No merge inputs matching [INPUT_GLOB] under [WORKDIR]" >&2
  exit 1
fi

python3 "[SKNANO_HOME]/scripts/sknano_merge.py" \
  --output [TARGET] --mode [MERGE_MODE] [MODE_ARGS] \
  --jobs [MERGE_JOBS] --cache-size [CACHE_SIZE] \
  --batch-cache-size [BATCH_CACHE_SIZE] [DELETE_FLAG] \
  "${inputs[@]}"
cp [PROVENANCE] [TARGET_PROVENANCE]
