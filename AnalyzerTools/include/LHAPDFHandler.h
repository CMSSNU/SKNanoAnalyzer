#ifndef LHAPDFHandler_h
#define LHAPDFHandler_h

#include <iostream>
#include "LHAPDF/LHAPDF.h"
#include "TString.h"
#include "ROOT/RVec.hxx"
using namespace std;
using namespace ROOT::VecOps;

class LHAPDFHandler {
public:
    LHAPDFHandler();
    ~LHAPDFHandler();

    // PDF error sets: Name of the Central PDF and members (0~100 etc..)
    TString CentralPDFName;
    LHAPDF::PDF* PDFCentral;
    int ErrorSetMember_Start, ErrorSetMember_End;
    RVec<LHAPDF::PDF*> PDFErrorSet;
    // PDF AlphaS sets: Usually the last two member o0f central PDF sets
    int AlphaSMember_Down, AlphaSMember_Up;
    LHAPDF::PDF* PDFAlphaS_Down;
    LHAPDF::PDF* PDFAlphaS_Up;

    void init();
};

#endif
