#include "GeneratorStudy.h"

GeneratorStudy::GeneratorStudy() {}
GeneratorStudy::~GeneratorStudy() {}

void GeneratorStudy::initializeAnalyzer() {
}
void GeneratorStudy::executeEvent() {
    LHEs = GetAllLHEs();
    GenJets = GetAllGenJets();

    // Split channels based on the number of outgoing jets in the LHE file
    RVec<LHE> leptons, muons, electrons, taus;
    RVec<LHE> outgoing_jets;
    for (const auto &lhe: LHEs) {
        if (lhe.Status() == -1) continue;

        if (fabs(lhe.PdgId()) == 11) {
            electrons.emplace_back(lhe);
            leptons.emplace_back(lhe);
        } else if (fabs(lhe.PdgId()) == 13) {
            muons.emplace_back(lhe);
            leptons.emplace_back(lhe);
        } else if (fabs(lhe.PdgId()) == 15) {
            taus.emplace_back(lhe);
            leptons.emplace_back(lhe);
        } else {
            outgoing_jets.emplace_back(lhe);
        }
    }
    
    TString n_outgoing_jets = "";
    if (outgoing_jets.size() == 0) {
        n_outgoing_jets = "0j";
    } else if (outgoing_jets.size() == 1) {
        n_outgoing_jets = "1j";
    } else if (outgoing_jets.size() == 2) {
        n_outgoing_jets = "2j";
    } else {
        cerr << "[GeneratorStudy::executeEvent] Unexpected number of outgoing jets" << endl;
    }

    TString channel = "DY"+n_outgoing_jets;
    if (electrons.size() == 2) {
        channel += "_ee";
    } else if (muons.size() == 2) {
        channel += "_mumu";
    } else if (taus.size() == 2) {
        channel += "_tautau";
    } else {
        cerr << "[GeneratorStudy::executeEvent] Unexpected lepton configuration" << endl;
    }

    const Particle ZCand = leptons[0] + leptons[1];
    const float weight = 1.0;
    FillHist("Inclusive/n_outgoing_jets", outgoing_jets.size(), weight, 5, 0., 5.);
    FillHist("Inclusive/Z_pt", ZCand.Pt(), weight, 300, 0., 300.);
    FillHist("Inclusive/Z_eta", ZCand.Eta(), weight, 100, -5., 5.);
    FillHist("Inclusive/Z_phi", ZCand.Phi(), weight, 64, -3.2, 3.2);
    FillHist("Inclusive/Z_mass", ZCand.M(), weight, 100, 40., 140.);

    FillHist(channel+"/Z_pt", ZCand.Pt(), weight, 300, 0., 300.);
    FillHist(channel+"/Z_eta", ZCand.Eta(), weight, 100, -5., 5.);
    FillHist(channel+"/Z_phi", ZCand.Phi(), weight, 64, -3.2, 3.2);
    FillHist(channel+"/Z_mass", ZCand.M(), weight, 100, 40., 140.);
}