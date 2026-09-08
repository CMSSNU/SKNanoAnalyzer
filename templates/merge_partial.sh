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
