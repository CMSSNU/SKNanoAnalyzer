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
    fChain->SetBranchAddress("Muon_mvaMuID_WP", Muon_mvaMuId);
    // fChain->SetBranchAddress("Muon_mvaLowPtId", Muon_mvaLowPtId);
    fChain->SetBranchAddress("Muon_pfIsoId", Muon_pfIsoId);
    fChain->SetBranchAddress("Muon_puppiIsoId", Muon_puppiIsoId);
    fChain->SetBranchAddress("Muon_tkIsoId", Muon_tkIsoId);
    fChain->SetBranchAddress("Muon_softMva", Muon_softMva);
    fChain->SetBranchAddress("Muon_mvaLowPt", Muon_mvaLowPt);
    fChain->SetBranchAddress("Muon_mvaTTH", Muon_mvaTTH);
    fChain->SetBranchAddress("nElectron", &nElectron);
    fChain->SetBranchAddress("Electron_pt", Electron_pt);
    fChain->SetBranchAddress("Electron_eta", Electron_eta);
    fChain->SetBranchAddress("Electron_phi", Electron_phi);
    fChain->SetBranchAddress("Electron_mass", Electron_mass);
    fChain->SetBranchAddress("Electron_charge", Electron_charge);
    fChain->SetBranchAddress("Electron_dxy", Electron_dxy);
    fChain->SetBranchAddress("Electron_dxyErr", Electron_dxyErr);
    fChain->SetBranchAddress("Electron_dz", Electron_dz);
    fChain->SetBranchAddress("Electron_dzErr", Electron_dzErr);
    fChain->SetBranchAddress("Electron_ip3d", Electron_ip3d);
    fChain->SetBranchAddress("Electron_sip3d", Electron_sip3d);
    fChain->SetBranchAddress("Electron_pfRelIso03_all", Electron_pfRelIso03_all);
    fChain->SetBranchAddress("Electron_miniPFRelIso_all", Electron_miniPFRelIso_all);
    fChain->SetBranchAddress("Electron_energyErr", Electron_energyErr);
    fChain->SetBranchAddress("Electron_convVeto", Electron_convVeto);
    fChain->SetBranchAddress("Electron_lostHits", Electron_lostHits);
    fChain->SetBranchAddress("Electron_seedGain", Electron_seedGain);
    fChain->SetBranchAddress("Electron_tightCharge", Electron_tightCharge);
    fChain->SetBranchAddress("Electron_sieie", Electron_sieie);
    fChain->SetBranchAddress("Electron_hoe", Electron_hoe);
    fChain->SetBranchAddress("Electron_eInvMinusPInv", Electron_eInvMinusPInv);
    fChain->SetBranchAddress("Electron_dr03EcalRecHitSumEt", Electron_dr03EcalRecHitSumEt);
    fChain->SetBranchAddress("Electron_dr03HcalDepth1TowerSumEt", Electron_dr03HcalDepth1TowerSumEt);
    fChain->SetBranchAddress("Electron_dr03TkSumPt", Electron_dr03TkSumPt);
    fChain->SetBranchAddress("Electron_dr03TkSumPtHEEP", Electron_dr03TkSumPtHEEP);
    fChain->SetBranchAddress("Electron_deltaEtaSC", Electron_deltaEtaSC);
    fChain->SetBranchAddress("Electron_mvaIso_WP80", Electron_mvaIso_WP80);
    fChain->SetBranchAddress("Electron_mvaIso_WP90", Electron_mvaIso_WP90);
    fChain->SetBranchAddress("Electron_mvaNoIso_WP80", Electron_mvaNoIso_WP80);
    fChain->SetBranchAddress("Electron_mvaNoIso_WP90", Electron_mvaNoIso_WP90);
    fChain->SetBranchAddress("Electron_cutBased_HEEP", Electron_cutBased_HEEP);
    fChain->SetBranchAddress("Electron_cutBased", Electron_cutBased);
    fChain->SetBranchAddress("Electron_mvaIso", Electron_mvaIso);
    fChain->SetBranchAddress("Electron_mvaNoIso", Electron_mvaNoIso);
    fChain->SetBranchAddress("Electron_mvaTTH", Electron_mvaTTH);
    fChain->SetBranchAddress("Electron_r9", Electron_r9);
}


