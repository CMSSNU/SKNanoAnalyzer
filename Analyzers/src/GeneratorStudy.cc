#include "GeneratorStudy.h"

GeneratorStudy::GeneratorStudy() {}
GeneratorStudy::~GeneratorStudy() {}

void GeneratorStudy::initializeAnalyzer() {
}
void GeneratorStudy::executeEvent() {
    AllGens = GetAllGens();
    AllLHEs = GetAllLHEs();

    //cout << "AllGens: " << AllGens.size() << endl;
    //cout << "AllLHEs: " << AllLHEs.size() << endl;

    //for (const auto &gen: AllGens) {
    //    gen.Print();
    //}
}
