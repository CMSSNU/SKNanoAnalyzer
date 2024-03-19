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
    FillHist("muons/1/tkRelIso", mu1.TkRelIso(), 1., 100, 0., 0.2);
    FillHist("muons/2/tkRelIso", mu2.TkRelIso(), 1., 100, 0., 0.2);
    FillHist("muons/1/pfRelIso04", mu1.PfRelIso04(), 1., 100, 0., 0.2);
    FillHist("muons/2/pfRelIso04", mu2.PfRelIso04(), 1., 100, 0., 0.2);
    FillHist("muons/1/pfRelIso03", mu1.PfRelIso03(), 1., 100, 0., 0.2);
    FillHist("muons/2/pfRelIso03", mu2.PfRelIso03(), 1., 100, 0., 0.2);
    FillHist("muons/1/miniPFRelIso", mu1.MiniPFRelIso(), 1., 100, 0., 0.2);
    FillHist("muons/2/miniPFRelIso", mu2.MiniPFRelIso(), 1., 100, 0., 0.2);
    FillHist("muons/1/dxy", mu1.dXY(), 1., 100, -0.1, 0.1);
    FillHist("muons/2/dxy", mu2.dXY(), 1., 100, -0.1, 0.1);
    FillHist("muons/1/dz", mu1.dZ(), 1., 100, -0.1, 0.1);
    FillHist("muons/2/dz", mu2.dZ(), 1., 100, -0.1, 0.1);
    FillHist("muons/1/IP3D", mu1.IP3D(), 1., 100, -0.1, 0.1);
    FillHist("muons/2/IP3D", mu2.IP3D(), 1., 100, -0.1, 0.1);
    FillHist("muons/1/SIP3D", mu1.SIP3D(), 1., 100, 0., 0.1);
    FillHist("muons/2/SIP3D", mu2.SIP3D(), 1., 100, 0., 0.1);

    FillHist("muons/1/pt_vs_eta", mu1.Pt(), mu1.Eta(), 1., 100, 0., 200., 48, -2.4, 2.4);
    FillHist("muons/2/pt_vs_eta", mu2.Pt(), mu2.Eta(), 1., 100, 0., 200., 48, -2.4, 2.4);
    FillHist("ZCand/pt_vs_mass", ZCand.Pt(), ZCand.M(), 1., 100, 50., 150., 40, 70., 110.);
    FillHist("ZCand/pt_vs_eta", ZCand.Pt(), ZCand.Eta(), 1., 100, 50., 150., 100, -5., 5.);
    FillHist("ZCand/pt_vs_phi", ZCand.Pt(), ZCand.Phi(), 1., 100, 50., 150., 100, -3.14, 3.14);
    FillHist("ZCand/eta_vs_phi", ZCand.Eta(), ZCand.Phi(), 1., 100, -5., 5., 100, -3.14, 3.14);
    FillHist("muons/1/pt_vs_phi", mu1.Pt(), mu1.Phi(), 1., 100, 0., 200., 100, -3.14, 3.14);
    FillHist("muons/2/pt_vs_phi", mu2.Pt(), mu2.Phi(), 1., 100, 0., 200., 100, -3.14, 3.14);
    FillHist("muons/1/pt_vs_mass", mu1.Pt(), mu1.M(), 1., 100, 0., 200., 100, 0., 0.2);
    FillHist("muons/2/pt_vs_mass", mu2.Pt(), mu2.M(), 1., 100, 0., 200., 100, 0., 0.2);

}

