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
group=$1
group_size=[GROUP_SIZE]
njobs=[NJOBS]

# DAG node index j of the analyzer layer produced output/hists_j.root, so a
# group owns a contiguous shard range and needs no extra bookkeeping.
target="output/partial_$(printf '%05d' "$group").root"
# The merge publishes atomically and only after validation, so an existing
# target means this group already succeeded. Condor may restart an evicted
# node whose shards are gone; that must not fail the DAG.
if [[ -f "$target" ]]; then
  echo "group ${group} already merged into ${target}"
  exit 0
fi

start=$(( group * group_size ))
end=$(( start + group_size - 1 ))
if (( end >= njobs )); then
  end=$(( njobs - 1 ))
fi

inputs=()
for (( i = start; i <= end; i++ )); do
  shard="output/hists_${i}.root"
  if [[ ! -f "$shard" ]]; then
    echo "group ${group} is missing ${shard}" >&2
    exit 1
  fi
  inputs+=("$shard")
done

python3 "[SKNANO_HOME]/scripts/sknano_merge.py" \
  --output "$target" \
  --jobs 1 --cache-size [CACHE_SIZE] --temp-dir output \
  --delete-inputs "${inputs[@]}"
