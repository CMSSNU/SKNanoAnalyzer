#ifndef LHAPDFHandler_h
#define LHAPDFHandler_h

// NOTE: This class is a duplicate of LHAPDFHandler.h in SKFlatAnalyzer
// From NanoAOD, the PDF weights are stored as a branch in Event tree
// Validation needed to check if the PDF weights are stored in the same way

#include <iostream>
#include <string>
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
    string CentralPDFName;
    LHAPDF::PDF* PDFCentral;
    int ErrorSetMember_Start, ErrorSetMember_End;
    RVec<LHAPDF::PDF*> PDFErrorSet;
    // PDF AlphaS sets: Usually the last two member of central PDF sets
    int AlphaSMember_Down, AlphaSMember_Up;
    LHAPDF::PDF* PDFAlphaS_Down;
    LHAPDF::PDF* PDFAlphaS_Up;

    void init();
};

#endif
