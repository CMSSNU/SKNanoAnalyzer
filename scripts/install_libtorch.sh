#!/bin/bash
cd $SKNANO_HOME/external
#print in yellow
echo -e "\e[93m@@@@ Downloading LibTorch\e[0m"
wget --no-verbose https://download.pytorch.org/libtorch/nightly/cpu/libtorch-shared-with-deps-latest.zip -O libtorch.zip
unzip -q libtorch.zip -d libtorch
cd libtorch
mv libtorch/* ./
rm -rf libtorch
cd ..
rm libtorch.zip
