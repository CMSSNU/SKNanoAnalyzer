#define SKNanoLoader_cxx
#include "SKNanoLoader.h"

SKNanoLoader::SKNanoLoader() {
    MaxEvent = -1;
    NSkipEvent = 0;
    LogEvery = 1000;
    IsDATA = false;
    DataStream = "";
    MCSample = "";
    SetEra("2018");
    xsec = 1.;
    sumW = 1.;
    sumSign = 1.;
    Userflags.clear();
}

SKNanoLoader::~SKNanoLoader() {
    if (!fChain) return;
    delete fChain->GetCurrentFile();
}

void SKNanoLoader::Loop() {
    long nentries = fChain->GetEntries();
    if (MaxEvent > 0)
        nentries = min(nentries, MaxEvent);

    cout << "[SKNanoLoader::Loop] Event Loop Started" << endl;
    for (long jentry=0; jentry<nentries; jentry++) {
        if (jentry < NSkipEvent) continue;
        if (jentry % 1000 == 0) cout << "[SKNanoLoader::Loop] Processing " << jentry << " / " << nentries << endl;
        if (fChain->GetEntry(jentry) < 0) exit(EIO);

        executeEvent();
    }
    cout << "[SKNanoLoader::Loop] Event Loop Finished" << endl;
}

void SKNanoLoader::Init() {
    // Set object pointer
    fChain->SetBranchAddress("nMuon", &nMuon);
    fChain->SetBranchAddress("Muon_pt", Muon_pt);
    fChain->SetBranchAddress("Muon_eta", Muon_eta);
    fChain->SetBranchAddress("Muon_phi", Muon_phi);
    fChain->SetBranchAddress("Muon_mass", Muon_mass);
    fChain->SetBranchAddress("Muon_charge", Muon_charge);
    fChain->SetBranchAddress("Muon_dxy", Muon_dxy);
    fChain->SetBranchAddress("Muon_dxyErr", Muon_dxyErr);
    fChain->SetBranchAddress("Muon_dz", Muon_dz);
    fChain->SetBranchAddress("Muon_dzErr", Muon_dzErr);
    fChain->SetBranchAddress("Muon_ip3d", Muon_ip3d);
    fChain->SetBranchAddress("Muon_sip3d", Muon_sip3d);
    fChain->SetBranchAddress("Muon_pfRelIso03_all", Muon_pfRelIso03_all);
    fChain->SetBranchAddress("Muon_pfRelIso04_all", Muon_pfRelIso04_all);
    fChain->SetBranchAddress("Muon_miniPFRelIso_all", Muon_miniPFRelIso_all);
    fChain->SetBranchAddress("Muon_tkRelIso", Muon_tkRelIso);
    fChain->SetBranchAddress("Muon_isTracker", Muon_isTracker);
    fChain->SetBranchAddress("Muon_isStandalone", Muon_isStandalone);
    fChain->SetBranchAddress("Muon_isGlobal", Muon_isGlobal);
    fChain->SetBranchAddress("Muon_looseId", Muon_looseId);
    fChain->SetBranchAddress("Muon_mediumId", Muon_mediumId);
    fChain->SetBranchAddress("Muon_mediumPromptId", Muon_mediumPromptId);
    fChain->SetBranchAddress("Muon_tightId", Muon_tightId);
    fChain->SetBranchAddress("Muon_softId", Muon_softId);
    fChain->SetBranchAddress("Muon_softMvaId", Muon_softMvaId);
    fChain->SetBranchAddress("Muon_triggerIdLoose", Muon_triggerIdLoose);
    fChain->SetBranchAddress("Muon_highPtId", Muon_highPtId);
    fChain->SetBranchAddress("Muon_miniIsoId", Muon_miniIsoId);
    fChain->SetBranchAddress("Muon_multiIsoId", Muon_multiIsoId);
    fChain->SetBranchAddress("Muon_mvaId", Muon_mvaId);
    fChain->SetBranchAddress("Muon_mvaLowPtId", Muon_mvaLowPtId);
    fChain->SetBranchAddress("Muon_pfIsoId", Muon_pfIsoId);
    fChain->SetBranchAddress("Muon_puppiIsoId", Muon_puppiIsoId);
    fChain->SetBranchAddress("Muon_tkIsoId", Muon_tkIsoId);
    fChain->SetBranchAddress("Muon_softMva", Muon_softMva);
    fChain->SetBranchAddress("Muon_mvaLowPt", Muon_mvaLowPt);
    fChain->SetBranchAddress("Muon_mvaTTH", Muon_mvaTTH);
}


