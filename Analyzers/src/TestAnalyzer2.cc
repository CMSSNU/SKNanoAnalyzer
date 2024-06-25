#include "TestAnalyzer2.h"

TestAnalyzer2::TestAnalyzer2() {}
TestAnalyzer2::~TestAnalyzer2() {}

void TestAnalyzer2::executeEvent() {
    auto muons = GetAllMuons();
    auto electrons = GetAllElectrons();
    auto jets = GetAllJets();
    auto genjets = GetAllGenJets();

    for(auto genjet: genjets){
        if(int(genjet.GetPartonFlavour()) != 0 || int(genjet.GetHadronFlavour() != 0)) 
        cout << "Parton Flavour " << genjet.GetPartonFlavour() << " Hadron Flavour " << int(genjet.GetHadronFlavour()) << endl; 
    }


    string channel = "";
    if (muons.size() == 2 && electrons.size() == 0) channel = "dimuon";
    if (muons.size() == 0 && electrons.size() == 2) channel = "dielectron";
    if (channel == "") return;

    Lepton lep1, lep2;
    if (channel == "dimuon") {
        lep1 = muons[0];
        lep2 = muons[1];
    }
    if (channel == "dielectron") {
        lep1 = electrons[0];
        lep2 = electrons[1];
    }
    Jet jet1 = jets[0];
    //if jet multiplicity is larger than 1 veto event
    if(jets.size() > 1) return;
    if(jet1.Pt() < 30) return;
    Particle dilepton = lep1 + lep2;
    FillHist(channel+"/dilepton/pt", dilepton.Pt(), 1., 200, 0., 200.);
    FillHist(channel+"/dilepton/eta", dilepton.Eta(), 1., 100, -5., 5.);
    FillHist(channel+"/dilepton/phi", dilepton.Phi(), 1., 100, -3.14, 3.14);
    FillHist(channel+"/dilepton/mass", dilepton.M(), 1., 200, 0., 200.);
    FillHist(channel+"/leptons/1/pt", lep1.Pt(), 1., 200, 0., 200.);
    FillHist(channel+"/leptons/2/pt", lep2.Pt(), 1., 200, 0., 200.);
    FillHist(channel+"/leptons/1/eta", lep1.Eta(), 1., 100, -5., 5.);
    FillHist(channel+"/leptons/2/eta", lep2.Eta(), 1., 100, -5., 5.);
    FillHist(channel+"/leptons/1/phi", lep1.Phi(), 1., 100, -3.14, 3.14);
    FillHist(channel+"/leptons/2/phi", lep2.Phi(), 1., 100, -3.14, 3.14);
    FillHist(channel+"/leptons/1/mass", lep1.M(), 1., 100, 0., 0.2);
    FillHist(channel+"/leptons/2/mass", lep2.M(), 1., 100, 0., 0.2);
    FillHist(channel+"/leptons/1/tkRelIso", lep1.TkRelIso(), 1., 100, 0., 0.2);
    FillHist(channel+"/leptons/2/tkRelIso", lep2.TkRelIso(), 1., 100, 0., 0.2);
    FillHist(channel+"/leptons/1/pfRelIso04", lep1.PfRelIso04(), 1., 100, 0., 0.2);
    FillHist(channel+"/leptons/2/pfRelIso04", lep2.PfRelIso04(), 1., 100, 0., 0.2);
    FillHist(channel+"/leptons/1/pfRelIso03", lep1.PfRelIso03(), 1., 100, 0., 0.2);
    FillHist(channel+"/leptons/2/pfRelIso03", lep2.PfRelIso03(), 1., 100, 0., 0.2);
    FillHist(channel+"/leptons/1/miniPFRelIso", lep1.MiniPFRelIso(), 1., 100, 0., 0.2);
    FillHist(channel+"/leptons/2/miniPFRelIso", lep2.MiniPFRelIso(), 1., 100, 0., 0.2);
    FillHist(channel+"/leptons/1/dxy", lep1.dXY(), 1., 100, -0.1, 0.1);
    FillHist(channel+"/leptons/2/dxy", lep2.dXY(), 1., 100, -0.1, 0.1);
    FillHist(channel+"/leptons/1/dz", lep1.dZ(), 1., 100, -0.1, 0.1);
    FillHist(channel+"/leptons/2/dz", lep2.dZ(), 1., 100, -0.1, 0.1);
    FillHist(channel+"/leptons/1/IP3D", lep1.IP3D(), 1., 100, -0.1, 0.1);
    FillHist(channel+"/leptons/2/IP3D", lep2.IP3D(), 1., 100, -0.1, 0.1);
    FillHist(channel+"/leptons/1/SIP3D", lep1.SIP3D(), 1., 100, 0., 0.1);
    FillHist(channel+"/leptons/2/SIP3D", lep2.SIP3D(), 1., 100, 0., 0.1);

    FillHist(channel+"/leptons/1/pt_vs_eta", lep1.Pt(), lep1.Eta(), 1., 100, 0., 200., 48, -2.4, 2.4);
    FillHist(channel+"/leptons/2/pt_vs_eta", lep2.Pt(), lep2.Eta(), 1., 100, 0., 200., 48, -2.4, 2.4);
    FillHist(channel+"/dilepton/pt_vs_mass", dilepton.Pt(), dilepton.M(), 1., 200, 0., 200., 40, 70., 110.);
    FillHist(channel+"/dilepton/pt_vs_eta", dilepton.Pt(), dilepton.Eta(), 1., 200, 0., 200., 100, -5., 5.);
    FillHist(channel+"/dilepton/pt_vs_phi", dilepton.Pt(), dilepton.Phi(), 1., 200, 0., 200., 100, -3.14, 3.14);
    FillHist(channel+"/dilepton/eta_vs_phi", dilepton.Eta(), dilepton.Phi(), 1., 100, -5., 5., 100, -3.14, 3.14);
    FillHist(channel+"/leptons/1/pt_vs_phi", lep1.Pt(), lep1.Phi(), 1., 100, 0., 200., 100, -3.14, 3.14);
    FillHist(channel+"/leptons/2/pt_vs_phi", lep2.Pt(), lep2.Phi(), 1., 100, 0., 200., 100, -3.14, 3.14);
    FillHist(channel+"/leptons/1/pt_vs_mass", lep1.Pt(), lep1.M(), 1., 100, 0., 200., 100, 0., 0.2);
    FillHist(channel+"/leptons/2/pt_vs_mass", lep2.Pt(), lep2.M(), 1., 100, 0., 200., 100, 0., 0.2);

    FillHist(channel+"/jets/1/pt", jet1.Pt(), 1., 200, 0., 200.);
    FillHist(channel+"/jets/1/eta", jet1.Eta(), 1., 100, -5., 5.);
    FillHist(channel+"/jets/1/phi", jet1.Phi(), 1., 100, -3.14, 3.14);
    FillHist(channel+"/jets/1/mass", jet1.M(), 1., 200, 0., 200.);
    FillHist(channel+"/jets/1/ptBalance", (jet1.Pt()-dilepton.Pt())/dilepton.Pt(), 1., 100, -2., 2.);
}

