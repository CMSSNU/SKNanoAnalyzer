#!/bin/bash
echo @@@@ Prepare to build SKNanoAnalyzer in $SKNANO_BUILDDIR
rm -rf $SKNANO_BUILDDIR $SKNANO_LIB 
mkdir -p $SKNANO_BUILDDIR && cd $SKNANO_BUILDDIR

export CORRECTION_CMAKE_PREFIX=`correction config --cmake`
cmake -DCMAKE_INSTALL_PREFIX=$SKNANO_INSTALLDIR $CORRECTION_CMAKE_PREFIX \
          -DCMAKE_PREFIX_PATH=$LIBTORCH_INSTALL_DIR $SKNANO_HOME \
          -DCMAKE_BUILD_TYPE=Release
echo @@@@ make -j6
make -j6

echo @@@@ install to $SKNANO_LIB
make install


