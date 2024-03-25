#include "LHAPDFHandler.h"

LHAPDFHandler::LHAPDFHandler() {
    CentralPDFName = "";
    ErrorSetMember_Start = -1;
    ErrorSetMember_End = -1;
    AlphaSMember_Up = -1;
    AlphaSMember_Down = -1;
}

LHAPDFHandler::~LHAPDFHandler() {}

void LHAPDFHandler::init() {
    cout << "[LHAPDFHandler::init] Central PDF = " << CentralPDFName << endl;
    PDFCentral = LHAPDF::mkPDF(CentralPDFName.Data(), 0);
    if (ErrorSetMember_Start >= 0 && ErrorSetMember_End >= 0 && ErrorSetMember_Start <= ErrorSetMember_End) {
        cout << "[LHAPDFHandler::init] Initializing PDF Error Sets" << endl;
        for (int i = ErrorSetMember_Start; i <= ErrorSetMember_End; i++) {
            cout << "[LHAPDFHandler::init] Error Set Member = " << i << endl;
            LHAPDF::PDF* this_pdf = LHAPDF::mkPDF(CentralPDFName.Data(), i);
            PDFErrorSet.push_back(this_pdf);
        }
        cout << "[LHAPDFHandler::init] End of PDF Error Sets initialization" << endl;
    }

    if (AlphaSMember_Down >= 0 && AlphaSMember_Up >= 0) {
        cout << "[LHAPDFHandler::init] Initializing PDF AlphaS Up and Down" << endl;
        cout << "[LHAPDFHandler::init] AlphaS Down Member = " << AlphaSMember_Down << endl;
        PDFAlphaS_Down = LHAPDF::mkPDF(CentralPDFName.Data(), AlphaSMember_Down);
        cout << "[LHAPDFHandler::init] AlphaS Up Member = " << AlphaSMember_Up << endl;
        PDFAlphaS_Up = LHAPDF::mkPDF(CentralPDFName.Data(), AlphaSMember_Up);
        cout << "[LHAPDFHandler::init] End of PDF AlphaS Up and Down initialization" << endl;
    }
}
