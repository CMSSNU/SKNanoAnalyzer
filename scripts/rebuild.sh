#!/bin/bash
# Rebuild the project without clean
# check if the build directory exists
if [ -d "build/${SYSTEM}" ]; then
    cd build/${SYSTEM}
    make && make install
    cd -
else
    echo "Build directory does not exist. Run build.sh first."
fi
