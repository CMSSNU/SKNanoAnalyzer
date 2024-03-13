#include "TestAnalyzer.h"

TestAnalyzer::TestAnalyzer() {}
TestAnalyzer::~TestAnalyzer() {}

void TestAnalyzer::executeEvent() {
    auto muons = GetAllMuons();
    if (! (muons.size() == 2)) return;
    auto mu1 = muons[0];
    auto mu2 = muons[1];
    Particle ZCand = mu1 + mu2;
    FillHist("ZCand/pt", ZCand.Pt(), 1., 100, 50., 150.);
    FillHist("ZCand/eta", ZCand.Eta(), 1., 100, -5., 5.);
    FillHist("ZCand/phi", ZCand.Phi(), 1., 100, -3.14, 3.14);
    FillHist("ZCand/mass", ZCand.M(), 1., 100, 70., 110.);
    FillHist("muons/1/pt", mu1.Pt(), 1., 200, 0., 200.);
    FillHist("muons/2/pt", mu2.Pt(), 1., 200, 0., 200.);
    FillHist("muons/1/eta", mu1.Eta(), 1., 100, -5., 5.);
    FillHist("muons/2/eta", mu2.Eta(), 1., 100, -5., 5.);
    FillHist("muons/1/phi", mu1.Phi(), 1., 100, -3.14, 3.14);
    FillHist("muons/2/phi", mu2.Phi(), 1., 100, -3.14, 3.14);
    FillHist("muons/1/mass", mu1.M(), 1., 100, 0., 0.2);
    FillHist("muons/2/mass", mu2.M(), 1., 100, 0., 0.2);
}

