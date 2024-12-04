#!/bin/sh
export SKNANO_BUILDDIR=$SKNANO_HOME/build

#add use_ninja argument
use_ninja=0
if [ "$1" = "use_ninja" ]; then
    use_ninja=1
fi

echo @@@@ Prepare to build SKNanoAnalyzer in $SKNANO_BUILDDIR
rm -rf $SKNANO_BUILDDIR $SKNANO_LIB 
mkdir $SKNANO_BUILDDIR && cd $SKNANO_BUILDDIR

export CORRECTION_CMAKE_PREFIX=`correction config --cmake`
if [ $use_ninja -eq 1 ]; then
    echo @@@@ cmake -DCMAKE_INSTALL_PREFIX=$SKNANO_HOME $CORRECTION_CMAKE_PREFIX -DCMAKE_PREFIX_PATH=$LIBTORCH_LIB_DIR/../share/cmake/Torch $SKNANO_HOME -DCMAKE_BUILD_TYPE=Debug -GNinja
    cmake -DCMAKE_INSTALL_PREFIX=$SKNANO_HOME $CORRECTION_CMAKE_PREFIX -DCMAKE_PREFIX_PATH=$LIBTORCH_LIB_DIR/../share/cmake/Torch $SKNANO_HOME -DCMAKE_BUILD_TYPE=Debug -GNinja
    echo @@@@ ninja
    ninja -d stats
    echo @@@@ ninja install
    ninja install
else
    echo @@@@ cmake -DCMAKE_INSTALL_PREFIX=$SKNANO_HOME $CORRECTION_CMAKE_PREFIX -DCMAKE_PREFIX_PATH=$LIBTORCH_LIB_DIR/../share/cmake/Torch $SKNANO_HOME -DCMAKE_BUILD_TYPE=Debug
    cmake -DCMAKE_INSTALL_PREFIX=$SKNANO_HOME $CORRECTION_CMAKE_PREFIX -DCMAKE_PREFIX_PATH=$LIBTORCH_LIB_DIR/../share/cmake/Torch $SKNANO_HOME -DCMAKE_BUILD_TYPE=Debug
    echo @@@@ make -j6
    make -j6

    echo @@@@ install to $SKNANO_LIB
    make install
fi


