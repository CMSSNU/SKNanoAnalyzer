#include "TestAnalyzer.h"

TestAnalyzer::TestAnalyzer() {}
TestAnalyzer::~TestAnalyzer() {}

void TestAnalyzer::executeEvent() {
    auto muons = GetAllMuons();
    if (! (muons.size() == 2)) return;
    auto mu1 = muons[0];
    auto mu2 = muons[1];
    Particle ZCand = mu1 + mu2;
    FillHist("ZMass", ZCand.M(), 1., 100, 50., 150.);
}

