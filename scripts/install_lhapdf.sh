#!/bin/bash
# clear the lhapdf directory
rm -rf $SKNANO_HOME/external/lhapdf

# download and install lhapdf
cd $SKNANO_HOME/external
wget --no-verbose https://lhapdf.hepforge.org/downloads/\?f\=LHAPDF-6.5.5.tar.gz -O LHAPDF-6.5.5.tar.gz
tar xf LHAPDF-6.5.5.tar.gz
cd LHAPDF-6.5.5/config
wget --no-verbose https://git.savannah.gnu.org/gitweb/\?p\=config.git\;a\=blob_plain\;f\=config.sub\;hb\=HEAD -O config.sub
wget --no-verbose https://git.savannah.gnu.org/gitweb/\?p\=config.git\;a\=blob_plain\;f\=config.guess\;hb\=HEAD -O config.guess
cd ..
ARCH=`arch`
if [ $ARCH == "arm64" ]; then
    ./configure --prefix=$SKNANO_HOME/external/lhapdf CXX=clang++
else
    ./configure --prefix=$SKNANO_HOME/external/lhapdf
fi
make -j4 && make install
cd $SKNANO_HOME/external
rm -rf LHAPDF*

# download CMS central PDF sets
mkdir -p $SKNANO_HOME/external/lhapdf/data
cd $SKNANO_HOME/external/lhapdf/data
wget --no-verbose https://lhapdfsets.web.cern.ch/current/NNPDF31_nnlo_hessian_pdfas.tar.gz
wget --no-verbose https://lhapdfsets.web.cern.ch/current/NNPDF31_nlo_hessian_pdfas.tar.gz
tar xf NNPDF31_nnlo_hessian_pdfas.tar.gz
tar xf NNPDF31_nlo_hessian_pdfas.tar.gz
cd $SKNANO_HOME
