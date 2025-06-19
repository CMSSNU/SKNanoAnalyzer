#!/bin/bash
#add use_ninja argument
use_ninja=0
if [ "$1" = "use_ninja" ]; then
    use_ninja=1
fi

echo @@@@ Prepare to build SKNanoAnalyzer in $SKNANO_BUILDDIR
rm -rf $SKNANO_BUILDDIR $SKNANO_LIB 
mkdir -p $SKNANO_BUILDDIR && cd $SKNANO_BUILDDIR

export CORRECTION_CMAKE_PREFIX=`correction config --cmake`
if [ $use_ninja -eq 1 ]; then
    echo @@@@ cmake -DCMAKE_INSTALL_PREFIX=$SKNANO_INSTALLDIR $CORRECTION_CMAKE_PREFIX $SKNANO_HOME -GNinja -DCMAKE_CXX_FLAGS=-fdiagnostics-color=always -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=$LIBTORCH_INSTALL_DIR
    cmake -DCMAKE_INSTALL_PREFIX=$SKNANO_INSTALLDIR $CORRECTION_CMAKE_PREFIX $SKNANO_HOME -GNinja -DCMAKE_CXX_FLAGS=-fdiagnostics-color=always -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=$LIBTORCH_INSTALL_DIR
    echo @@@@ ninja
    ninja -d stats
    echo @@@@ ninja install
    ninja install
    
else
    cmake -DCMAKE_INSTALL_PREFIX=$SKNANO_INSTALLDIR $CORRECTION_CMAKE_PREFIX \
          -DCMAKE_PREFIX_PATH=$LIBTORCH_INSTALL_DIR \
          -DCMAKE_BUILD_TYPE=Release \
          $SKNANO_HOME \
    echo @@@@ make -j6
    make -j6

    echo @@@@ install to $SKNANO_LIB
    make install
fi