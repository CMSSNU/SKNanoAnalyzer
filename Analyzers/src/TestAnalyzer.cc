#include "TestAnalyzer.h"
//#include "correction.h"
//using correction::CorrectionSet;

TestAnalyzer::TestAnalyzer() {}
TestAnalyzer::~TestAnalyzer() {}

void TestAnalyzer::executeEvent() {
    auto allMuons = GetAllMuons();
    auto allElectrons = GetAllElectrons();
   
    FillHist("AllMuon/muon/size", allMuons.size(), 1., 10, 0., 10.);
    FillHist("AllElectron/electron/size", allElectrons.size(), 1., 10, 0., 10.);

    auto tightMuons = SelectMuons(allMuons, "POGTight", 20., 2.4);
    auto looseMuons = SelectMuons(allMuons, "POGLoose", 20., 2.4);
    auto tightElectrons = SelectElectrons(allElectrons, "POGTight", 20., 2.5);
    auto looseElectrons = SelectElectrons(allElectrons, "POGLoose", 20., 2.5);
    //cout << tightMuons.size() << "\t" << looseMuons.size() << "\t" << tightElectrons.size() << "\t" << looseElectrons.size() << endl;
    string channel = "";
    if (! (tightMuons.size() == 1 && tightElectrons.size() == 1)) return;
    channel = "emu";

    auto muon = tightMuons.at(0);
    auto electron = tightElectrons.at(0);

    FillHist(channel+"/muon/pt", muon.Pt(), 1., 200, 0., 200.);
    FillHist(channel+"/electron/pt", electron.Pt(), 1., 200, 0., 200.);
    FillHist(channel+"/muon/eta", muon.Eta(), 1., 100, -5., 5.);
    FillHist(channel+"/electron/eta", electron.Eta(), 1., 100, -5., 5.);
    FillHist(channel+"/muon/phi", muon.Phi(), 1., 100, -3.14, 3.14);
    FillHist(channel+"/electron/phi", electron.Phi(), 1., 100, -3.14, 3.14);
    FillHist(channel+"/muon/mass", muon.M(), 1., 100, 0., 0.2);
    FillHist(channel+"/electron/mass", electron.M(), 1., 100, 0., 0.2);
    FillHist(channel+"/muon/tkRelIso", muon.TkRelIso(), 1., 100, 0., 0.2);
    FillHist(channel+"/electron/tkRelIso", electron.TkRelIso(), 1., 100, 0., 0.2);
    FillHist(channel+"/muon/pfRelIso04", muon.PfRelIso04(), 1., 100, 0., 0.2);
    FillHist(channel+"/electron/pfRelIso04", electron.PfRelIso04(), 1., 100, 0., 0.2);
    FillHist(channel+"/muon/pfRelIso03", muon.PfRelIso03(), 1., 100, 0., 0.2);
    FillHist(channel+"/electron/pfRelIso03", electron.PfRelIso03(), 1., 100, 0., 0.2);
    FillHist(channel+"/muon/miniPFRelIso", muon.MiniPFRelIso(), 1., 100, 0., 0.2);
    FillHist(channel+"/electron/miniPFRelIso", electron.MiniPFRelIso(), 1., 100, 0., 0.2);
    FillHist(channel+"/muon/dxy", muon.dXY(), 1., 100, -0.1, 0.1);
    FillHist(channel+"/electron/dxy", electron.dXY(), 1., 100, -0.1, 0.1);
    FillHist(channel+"/muon/dz", muon.dZ(), 1., 100, -0.1, 0.1);
    FillHist(channel+"/electron/dz", electron.dZ(), 1., 100, -0.1, 0.1);
    FillHist(channel+"/muon/IP3D", muon.IP3D(), 1., 100, -0.1, 0.1);
    FillHist(channel+"/electron/IP3D", electron.IP3D(), 1., 100, -0.1, 0.1);
    FillHist(channel+"/muon/SIP3D", muon.SIP3D(), 1., 100, 0., 0.1);
    FillHist(channel+"/electron/SIP3D", electron.SIP3D(), 1., 100, 0., 0.1);
}

