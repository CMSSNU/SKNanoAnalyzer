#!/bin/bash
# Install the CPU LibTorch distribution used by the analyzers.
#
# The version is pinned: "latest" moves every night, and a LibTorch that no
# longer matches the ABI the framework was built against fails at load time
# rather than at build time. Bump LIBTORCH_VERSION together with
# external/libtorch/build-version when a newer build is needed.
set -euo pipefail

LIBTORCH_VERSION="${LIBTORCH_VERSION:-2.9.0.dev20250630+cpu}"
LIBTORCH_CHANNEL="${LIBTORCH_CHANNEL:-nightly}"

case "$LIBTORCH_VERSION" in
    *dev*) url_dir="nightly/cpu" ;;
    *)     url_dir="cpu" ;;
esac
[[ "$LIBTORCH_CHANNEL" == "nightly" ]] || url_dir="cpu"

# '+' has to reach the server percent-encoded.
url_version="${LIBTORCH_VERSION/+/%2B}"
LIBTORCH_URL="https://download.pytorch.org/libtorch/${url_dir}/libtorch-shared-with-deps-${url_version}.zip"

cd "$SKNANO_HOME/external"
echo -e "\033[33m@@@@ Downloading LibTorch ${LIBTORCH_VERSION}\033[0m"
if ! wget --no-verbose "$LIBTORCH_URL" -O libtorch.zip; then
    echo -e "\033[31m@@@@ Download failed: $LIBTORCH_URL\033[0m"
    echo -e "\033[31m@@@@ Nightly builds are eventually removed. Pick another build from\033[0m"
    echo -e "\033[31m@@@@ https://download.pytorch.org/libtorch/ and re-run with\033[0m"
    echo -e "\033[31m@@@@   LIBTORCH_VERSION=<version> ./scripts/install_libtorch.sh\033[0m"
    rm -f libtorch.zip
    exit 1
fi
unzip -q libtorch.zip -d libtorch
cd libtorch
mv libtorch/* ./
rm -rf libtorch
cd ..
rm libtorch.zip
