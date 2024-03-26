#ifndef PDFReweight_h
#define PDFReweight_h

#include <iostream>
#include "TString.h"
#include "ROOT/RVec.hxx"
#include "LHAPDFHandler.h"

using namespace std;
using namespace ROOT::VecOps;

class PDFReweight {
public:
    PDFReweight();
    ~PDFReweight();

    inline void SetProdPDF(LHAPDF::PDF *pdf_) { ProdPDF = pdf_; }
    inline void SetNewPDF(LHAPDF::PDF *pdf_) { NewPDF = pdf_; }
    inline void SetNewPDFErrorSet(RVec<LHAPDF::PDF*> newPDFErrorSet) {
        PDFErrorSet = newPDFErrorSet;
        NErrorSet = PDFErrorSet.size();
    }
    inline void SetNewPDFAlphaS(LHAPDF::PDF *newPDFAlphaSDown, LHAPDF::PDF *newPDFAlphaSUp) {
        NewPDFAlphaSDown = newPDFAlphaSDown;
        NewPDFAlphaSUp = newPDFAlphaSUp;
    }

private:
    LHAPDF::PDF* ProdPDF;
    LHAPDF::PDF* NewPDF;
    RVec<LHAPDF::PDF*> PDFErrorSet;
    unsigned int NErrorSet;
    LHAPDF::PDF* NewPDFAlphaSDown;
    LHAPDF::PDF* NewPDFAlphaSUp;
};

#endif
