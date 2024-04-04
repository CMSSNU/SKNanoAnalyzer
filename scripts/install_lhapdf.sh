#!/bin/sh
mkdir -p $SKNANO_HOME/external && cd $SKNANO_HOME/external
wget https://lhapdf.hepforge.org/downloads/\?f\=LHAPDF-6.5.4.tar.gz -O LHAPDF-6.5.4.tar.gz
tar xf LHAPDF-6.5.4.tar.gz
cd LHAPDF-6.5.4/config
wget https://git.savannah.gnu.org/gitweb/\?p\=config.git\;a\=blob_plain\;f\=config.sub\;hb\=HEAD -O config.sub
wget https://git.savannah.gnu.org/gitweb/\?p\=config.git\;a\=blob_plain\;f\=config.guess\;hb\=HEAD -O config.guess
cd ..
#./configure --prefix=$SKNANO_HOME/external/lhapdf CXX=clang++
./configure --prefix=$SKNANO_HOME/external/lhapdf
make -j4 && make install
cd $SKNANO_HOME/external
rm -rf LHAPDF*
