#!/bin/bash

TARGETs=("Analyzers" 
         "AnalyzerTools" 
         "DataFormats" 
         "external/KinematicFitter" 
         "external/RoccoR")

echo "Cleaning $PWD"
rm -rf CMakeFiles Makefile cmake_install.cmake module.modulemap _deps CMakeCache.txt

for TARGET in ${TARGETs[@]}; do
    echo "Cleaning $TARGET"
    cd $TARGET
    rm -rf CMakeFiles Makefile cmake_install.cmake module.modulemap _deps
    cd -
done
