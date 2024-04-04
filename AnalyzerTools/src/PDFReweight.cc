#include "PDFReweight.h"

PDFReweight::PDFReweight() {
    ProdPDF = nullptr;
    NewPDF = nullptr;
    NewPDFAlphaSDown = nullptr;
    NewPDFAlphaSUp = nullptr;
}
PDFReweight::~PDFReweight() {
    delete ProdPDF;
    delete NewPDF;
    delete NewPDFAlphaSDown;
    delete NewPDFAlphaSUp;
}
