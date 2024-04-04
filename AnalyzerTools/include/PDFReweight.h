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

    LHAPDF::PDF* ProdPDF;
    LHAPDF::PDF* NewPDF;
    RVec<LHAPDF::PDF*> PDFErrorSet;
    unsigned int NErrorSet;
    LHAPDF::PDF* NewPDFAlphaSDown;
    LHAPDF::PDF* NewPDFAlphaSUp;

    inline void SetProdPDF(LHAPDF::PDF *pdf_) { 
        if (!pdf_) {
            cerr << "[PDFReweight::SetProdPDF] NULL pointer to PDF object!" << endl;
            exit(EXIT_FAILURE);
        }
        ProdPDF = pdf_; 
    }
    inline void SetNewPDF(LHAPDF::PDF *pdf_) { 
        if (!pdf_) {
            cerr << "[PDFReweight::SetNewPDF] NULL pointer to PDF object!" << endl;
            exit(EXIT_FAILURE);
        }
        NewPDF = pdf_; 
    }
    inline void SetNewPDFErrorSet(RVec<LHAPDF::PDF*> newPDFErrorSet) {
        PDFErrorSet.clear();
        for (const auto &pdf : newPDFErrorSet) {
            if (!pdf) {
                cerr << "[PDFReweight::SetNewPDFErrorSet] NULL pointer to PDF object!" << endl;
                exit(EXIT_FAILURE);
            }
            PDFErrorSet.push_back(pdf);
        }
        NErrorSet = PDFErrorSet.size();
    }
    inline void SetNewPDFAlphaS(LHAPDF::PDF *newPDFAlphaSDown, LHAPDF::PDF *newPDFAlphaSUp) {
        if (!newPDFAlphaSDown || !newPDFAlphaSUp) {
            cerr << "[PDFReweight::SetNewPDFAlphaS] NULL pointer to PDF object!" << endl;
            exit(EXIT_FAILURE);
        }
        NewPDFAlphaSDown = newPDFAlphaSDown;
        NewPDFAlphaSUp = newPDFAlphaSUp;
    }
};

#endif
