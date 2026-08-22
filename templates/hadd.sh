#!/bin/bash
set -eo pipefail

export PATH="/opt/conda/bin:${PATH}"
export MAMBA_ROOT_PREFIX="/opt/conda"
eval "$(micromamba shell hook -s bash)"
micromamba activate Nano

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
