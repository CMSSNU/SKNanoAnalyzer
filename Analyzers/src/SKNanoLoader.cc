#define SKNanoLoader_cxx
#include "SKNanoLoader.h"
#include "json.hpp"
using json = nlohmann::json;

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
    for (auto& [key, value] : TriggerMap) {
        delete value;
    }
    if (!fChain) return;
    if (fChain->GetCurrentFile()) fChain->GetCurrentFile()->Close();
}
void SKNanoLoader::Loop() {
    long nentries = fChain->GetEntries();
    if (MaxEvent > 0)
        nentries = min(nentries, MaxEvent);

    cout << "[SKNanoLoader::Loop] Event Loop Started" << endl;
    for (long jentry=0; jentry<nentries; jentry++) {
        if (jentry < NSkipEvent) continue;
        if (jentry % LogEvery == 0) cout << "[SKNanoLoader::Loop] Processing " << jentry << " / " << nentries << endl;
        if (fChain->GetEntry(jentry) < 0) exit(EIO);
        executeEvent();
    }
    cout << "[SKNanoLoader::Loop] Event Loop Finished" << endl;
}
void SKNanoLoader::SetMaxLeafSize(){
    //check how much time it takes to read the tree
    //and set the maximum leaf size accordingly
    auto start = std::chrono::high_resolution_clock::now();

    // Get Maximum length of arrays
    df = new ROOT::RDataFrame(*fChain);
    auto maxPhotonPtr = df->Max("nPhoton");
    auto maxJetPtr = df->Max("nJet");
    auto maxMuonPtr = df->Max("nMuon");
    auto maxElectronPtr = df->Max("nElectron");
    auto maxTauPtr = df->Max("nTau");
    auto maxFatJetPtr = df->Max("nFatJet");

    if(!IsDATA){
        auto maxGenPartPtr = df->Max("nGenPart");
        auto maxLHEPartPtr = df->Max("nLHEPart");
        auto maxGenJetPtr = df->Max("nGenJet");
        const int kMaxGen = !IsDATA ? *maxGenPartPtr : 0;
        const int kMaxLHE = !IsDATA ? *maxLHEPartPtr : 0;
        const int kMaxGenJet = !IsDATA ? *maxGenJetPtr : 0;
        GenJet_eta.resize(kMaxGenJet);
        GenJet_mass.resize(kMaxGenJet);
        GenJet_phi.resize(kMaxGenJet);
        GenJet_pt.resize(kMaxGenJet);
        GenJet_partonFlavour.resize(kMaxGenJet);
        GenJet_hadronFlavour.resize(kMaxGenJet);

        GenPart_pdgId.resize(kMaxGen);
        GenPart_status.resize(kMaxGen);
        GenPart_statusFlags.resize(kMaxGen);
        GenPart_genPartIdxMother.resize(kMaxGen);
        GenPart_pt.resize(kMaxGen);
        GenPart_eta.resize(kMaxGen);
        GenPart_phi.resize(kMaxGen);
        GenPart_mass.resize(kMaxGen);

        LHEPart_pt.resize(kMaxLHE);
        LHEPart_eta.resize(kMaxLHE);
        LHEPart_phi.resize(kMaxLHE);
        LHEPart_mass.resize(kMaxLHE);
        LHEPart_incomingpz.resize(kMaxLHE);
        LHEPart_pdgId.resize(kMaxLHE);
        LHEPart_status.resize(kMaxLHE);
        LHEPart_spin.resize(kMaxLHE);
    } 

    const int kMaxPhoton = *maxPhotonPtr;
    const int kMaxJet = *maxJetPtr;
    const int kMaxMuon = *maxMuonPtr;
    const int kMaxElectron = *maxElectronPtr;
    const int kMaxTau = *maxTauPtr;
    const int kMaxFatJet = *maxFatJetPtr;
    auto RDataFrameFinishTime = std::chrono::high_resolution_clock::now();

    Muon_pt.resize(kMaxMuon);
    Muon_eta.resize(kMaxMuon);
    Muon_phi.resize(kMaxMuon);
    Muon_mass.resize(kMaxMuon);
    Muon_charge.resize(kMaxMuon);
    Muon_dxy.resize(kMaxMuon);
    Muon_dxyErr.resize(kMaxMuon);
    Muon_dz.resize(kMaxMuon);
    Muon_dzErr.resize(kMaxMuon);
    Muon_ip3d.resize(kMaxMuon);
    Muon_sip3d.resize(kMaxMuon);
    Muon_pfRelIso03_all.resize(kMaxMuon);
    Muon_pfRelIso04_all.resize(kMaxMuon);
    Muon_miniPFRelIso_all.resize(kMaxMuon);
    Muon_tkRelIso.resize(kMaxMuon);
    Muon_isTracker.resize(kMaxMuon);
    Muon_isStandalone.resize(kMaxMuon);
    Muon_isGlobal.resize(kMaxMuon);
    Muon_looseId.resize(kMaxMuon);
    Muon_mediumId.resize(kMaxMuon);
    Muon_mediumPromptId.resize(kMaxMuon);
    Muon_tightId.resize(kMaxMuon);
    Muon_softId.resize(kMaxMuon);
    Muon_softMvaId.resize(kMaxMuon);
    Muon_triggerIdLoose.resize(kMaxMuon);
    Muon_highPtId.resize(kMaxMuon);
    Muon_miniIsoId.resize(kMaxMuon);
    Muon_multiIsoId.resize(kMaxMuon);
    Muon_mvaMuId.resize(kMaxMuon);
    // Muon_mvaLowPtId.resize(kMaxMuon);  // Uncomment if needed
    Muon_pfIsoId.resize(kMaxMuon);
    Muon_puppiIsoId.resize(kMaxMuon);
    Muon_tkIsoId.resize(kMaxMuon);
    Muon_softMva.resize(kMaxMuon);
    Muon_mvaLowPt.resize(kMaxMuon);
    Muon_mvaTTH.resize(kMaxMuon);

    Electron_genPartFlav.resize(kMaxElectron);
    Electron_genPartIdx.resize(kMaxElectron);
    Electron_pt.resize(kMaxElectron);
    Electron_eta.resize(kMaxElectron);
    Electron_phi.resize(kMaxElectron);
    Electron_mass.resize(kMaxElectron);
    Electron_charge.resize(kMaxElectron);
    Electron_dxy.resize(kMaxElectron);
    Electron_dxyErr.resize(kMaxElectron);
    Electron_dz.resize(kMaxElectron);
    Electron_dzErr.resize(kMaxElectron);
    Electron_ip3d.resize(kMaxElectron);
    Electron_sip3d.resize(kMaxElectron);
    Electron_pfRelIso03_all.resize(kMaxElectron);
    Electron_miniPFRelIso_all.resize(kMaxElectron);
    Electron_energyErr.resize(kMaxElectron);
    Electron_convVeto.resize(kMaxElectron);
    Electron_lostHits.resize(kMaxElectron);
    Electron_seedGain.resize(kMaxElectron);
    Electron_tightCharge.resize(kMaxElectron);
    Electron_sieie.resize(kMaxElectron);
    Electron_hoe.resize(kMaxElectron);
    Electron_eInvMinusPInv.resize(kMaxElectron);
    Electron_dr03EcalRecHitSumEt.resize(kMaxElectron);
    Electron_dr03HcalDepth1TowerSumEt.resize(kMaxElectron);
    Electron_dr03TkSumPt.resize(kMaxElectron);
    Electron_dr03TkSumPtHEEP.resize(kMaxElectron);
    Electron_deltaEtaSC.resize(kMaxElectron);
    Electron_mvaIso_WP80.resize(kMaxElectron);
    Electron_mvaIso_WP90.resize(kMaxElectron);
    Electron_mvaNoIso_WP80.resize(kMaxElectron);
    Electron_mvaNoIso_WP90.resize(kMaxElectron);
    Electron_cutBased_HEEP.resize(kMaxElectron);
    Electron_cutBased.resize(kMaxElectron);
    Electron_mvaIso.resize(kMaxElectron);
    Electron_mvaNoIso.resize(kMaxElectron);
    Electron_mvaTTH.resize(kMaxElectron);
    Electron_r9.resize(kMaxElectron);

    Photon_pt.resize(kMaxPhoton);
    Photon_eta.resize(kMaxPhoton);
    Photon_phi.resize(kMaxPhoton);
    Photon_energyRaw.resize(kMaxPhoton);
    Photon_hoe.resize(kMaxPhoton);
    Photon_cutBased.resize(kMaxPhoton);
    Photon_mvaID.resize(kMaxPhoton);
    Photon_mvaID_WP80.resize(kMaxPhoton);
    Photon_mvaID_WP90.resize(kMaxPhoton);
    Photon_pixelSeed.resize(kMaxPhoton);
    Photon_sieie.resize(kMaxPhoton);
    Photon_isScEtaEB.resize(kMaxPhoton);
    Photon_isScEtaEE.resize(kMaxPhoton);

    Jet_genJetIdx.resize(kMaxJet);
    Jet_hadronFlavour.resize(kMaxJet);
    Jet_partonFlavour.resize(kMaxJet);
    Jet_PNetRegPtRawCorr.resize(kMaxJet);
    Jet_PNetRegPtRawCorrNeutrino.resize(kMaxJet);
    Jet_PNetRegPtRawRes.resize(kMaxJet);
    Jet_area.resize(kMaxJet);
    Jet_btagDeepFlavB.resize(kMaxJet);
    Jet_btagDeepFlavCvB.resize(kMaxJet);
    Jet_btagDeepFlavCvL.resize(kMaxJet);
    Jet_btagDeepFlavQG.resize(kMaxJet);
    Jet_btagPNetB.resize(kMaxJet);
    Jet_btagPNetCvB.resize(kMaxJet);
    Jet_btagPNetCvL.resize(kMaxJet);
    Jet_btagPNetQvG.resize(kMaxJet);
    Jet_btagPNetTauVJet.resize(kMaxJet);
    Jet_btagRobustParTAK4B.resize(kMaxJet);
    Jet_btagRobustParTAK4CvB.resize(kMaxJet);
    Jet_btagRobustParTAK4CvL.resize(kMaxJet);
    Jet_btagRobustParTAK4QG.resize(kMaxJet);
    Jet_chEmEF.resize(kMaxJet);
    Jet_chHEF.resize(kMaxJet);
    Jet_electronIdx1.resize(kMaxJet);
    Jet_electronIdx2.resize(kMaxJet);
    Jet_eta.resize(kMaxJet);
    Jet_jetId.resize(kMaxJet);
    Jet_mass.resize(kMaxJet);
    Jet_muEF.resize(kMaxJet);
    Jet_muonIdx1.resize(kMaxJet);
    Jet_muonIdx2.resize(kMaxJet);
    Jet_nConstituents.resize(kMaxJet);
    Jet_nElectrons.resize(kMaxJet);
    Jet_nMuons.resize(kMaxJet);
    Jet_nSVs.resize(kMaxJet);
    Jet_neEmEF.resize(kMaxJet);
    Jet_neHEF.resize(kMaxJet);
    Jet_phi.resize(kMaxJet);
    Jet_pt.resize(kMaxJet);
    Jet_rawFactor.resize(kMaxJet);
    Jet_svIdx1.resize(kMaxJet);
    Jet_svIdx2.resize(kMaxJet);

    Tau_pt.resize(kMaxTau);
    Tau_eta.resize(kMaxTau);
    Tau_phi.resize(kMaxTau);
    Tau_mass.resize(kMaxTau);
    Tau_charge.resize(kMaxTau);
    Tau_dxy.resize(kMaxTau);
    Tau_dz.resize(kMaxTau);
    Tau_idDeepTau2018v2p5VSe.resize(kMaxTau);
    Tau_idDeepTau2018v2p5VSjet.resize(kMaxTau);
    Tau_idDeepTau2018v2p5VSmu.resize(kMaxTau);
    Tau_decayMode.resize(kMaxTau);
    Tau_idDecayModeNewDMs.resize(kMaxTau);
    Tau_genPartFlav.resize(kMaxTau);
    Tau_genPartIdx.resize(kMaxTau);

    FatJet_pt.resize(kMaxFatJet);
    FatJet_eta.resize(kMaxFatJet);
    FatJet_phi.resize(kMaxFatJet);
    FatJet_mass.resize(kMaxFatJet);
    FatJet_msoftdrop.resize(kMaxFatJet);
    FatJet_area.resize(kMaxFatJet);
    FatJet_genJetAK8Idx.resize(kMaxFatJet);
    FatJet_jetId.resize(kMaxFatJet);
    FatJet_lsf3.resize(kMaxFatJet);
    FatJet_nBHadrons.resize(kMaxFatJet);
    FatJet_nCHadrons.resize(kMaxFatJet);
    FatJet_nConstituents.resize(kMaxFatJet);
    FatJet_btagDDBvLV2.resize(kMaxFatJet);
    FatJet_btagDDCvBV2.resize(kMaxFatJet);
    FatJet_btagDDCvLV2.resize(kMaxFatJet);
    FatJet_btagDeepB.resize(kMaxFatJet);
    FatJet_btagHbb.resize(kMaxFatJet);
    FatJet_particleNetWithMass_H4qvsQCD.resize(kMaxFatJet);
    FatJet_particleNetWithMass_HccvsQCD.resize(kMaxFatJet);
    FatJet_particleNetWithMass_HbbvsQCD.resize(kMaxFatJet);
    FatJet_particleNetWithMass_QCD.resize(kMaxFatJet);
    FatJet_particleNetWithMass_TvsQCD.resize(kMaxFatJet);
    FatJet_particleNetWithMass_WvsQCD.resize(kMaxFatJet);
    FatJet_particleNetWithMass_ZvsQCD.resize(kMaxFatJet);
    FatJet_particleNet_QCD.resize(kMaxFatJet);
    FatJet_particleNet_QCD0HF.resize(kMaxFatJet);
    FatJet_particleNet_QCD1HF.resize(kMaxFatJet);
    FatJet_particleNet_QCD2HF.resize(kMaxFatJet);
    FatJet_particleNet_XbbVsQCD.resize(kMaxFatJet);
    FatJet_particleNet_XccVsQCD.resize(kMaxFatJet);
    FatJet_particleNet_XqqVsQCD.resize(kMaxFatJet);
    FatJet_particleNet_XggVsQCD.resize(kMaxFatJet);
    FatJet_particleNet_XteVsQCD.resize(kMaxFatJet);
    FatJet_particleNet_XtmVsQCD.resize(kMaxFatJet);
    FatJet_particleNet_XttVsQCD.resize(kMaxFatJet);
    FatJet_particleNet_massCorr.resize(kMaxFatJet);
    FatJet_tau1.resize(kMaxFatJet);
    FatJet_tau2.resize(kMaxFatJet);
    FatJet_tau3.resize(kMaxFatJet);
    FatJet_tau4.resize(kMaxFatJet);
    FatJet_subJetIdx1.resize(kMaxFatJet);
    FatJet_subJetIdx2.resize(kMaxFatJet);

    delete df;
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    auto RDataFrameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(RDataFrameFinishTime - start);
    auto resizingDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - RDataFrameFinishTime);
    cout << "[SKNanoLoader::SetMaxLeafSize] Resizing Time: " << resizingDuration.count() << " ms" << endl;
    cout << "[SKNanoLoader::SetMaxLeafSize] RDataFrame Finish Time: " << RDataFrameDuration.count() << " ms" << endl;
    cout << "[SKNanoLoader::SetMaxLeafSize] Maximum Leaf Size Set" << endl;
    cout << "[SKNanoLoader::SetMaxLeafSize] Time taken: " << duration.count() << " ms" << endl;
    cout << "[SKNanoLoader::SetMaxLeafSize] Maximum Leaf Size Set" << endl;
    cout << "[SKNanoLoader::SetMaxLeafSize] kMaxPhoton: " << kMaxPhoton << endl;
    cout << "[SKNanoLoader::SetMaxLeafSize] kMaxJet: " << kMaxJet << endl;
    cout << "[SKNanoLoader::SetMaxLeafSize] kMaxMuon: " << kMaxMuon << endl;
    cout << "[SKNanoLoader::SetMaxLeafSize] kMaxElectron: " << kMaxElectron << endl;
    cout << "[SKNanoLoader::SetMaxLeafSize] kMaxTau: " << kMaxTau << endl;
    cout << "[SKNanoLoader::SetMaxLeafSize] kMaxFatJet: " << kMaxFatJet << endl;


}
void SKNanoLoader::Init() {

    if(fChain->GetEntries() == 0){
        cout << "[SKNanoLoader::Init] No Entries in the Tree" << endl;
        cout << "[SKNanoLoader::Init] Exiting without make output..." << endl;
        exit(0);
    }
    
    SetMaxLeafSize();
    if(!IsDATA){
        fChain->SetBranchAddress("Generator_id1", &Generator_id1);
        fChain->SetBranchAddress("Generator_id2", &Generator_id2);
        fChain->SetBranchAddress("Generator_x1", &Generator_x1);
        fChain->SetBranchAddress("Generator_x2", &Generator_x2);
        fChain->SetBranchAddress("Generator_xpdf1", &Generator_xpdf1);
        fChain->SetBranchAddress("Generator_xpdf2", &Generator_xpdf2);
        fChain->SetBranchAddress("Generator_scalePDF", &Generator_scalePDF);
        fChain->SetBranchAddress("Generator_weight", &Generator_weight);
        fChain->SetBranchAddress("genWeight", &genWeight);
        fChain->SetBranchAddress("LHEWeight_originalXWGTUP", &LHEWeight_originalXWGTUP);
        fChain->SetBranchAddress("LHEPdfWeight", &LHEPdfWeight);
        fChain->SetBranchAddress("LHEScaleWeight", &LHEScaleWeight);
        fChain->SetBranchAddress("PSWeight", &PSWeight);
        fChain->SetBranchAddress("Pileup_nPU", &Pileup_nPU);
        fChain->SetBranchAddress("Pileup_nTrueInt", &Pileup_nTrueInt); 
        fChain->SetBranchAddress("Electron_genPartFlav", Electron_genPartFlav.data());
        fChain->SetBranchAddress("Electron_genPartIdx", Electron_genPartIdx.data());
        fChain->SetBranchAddress("Jet_genJetIdx", Jet_genJetIdx.data());
        fChain->SetBranchAddress("Jet_hadronFlavour", Jet_hadronFlavour.data());
        fChain->SetBranchAddress("Jet_partonFlavour", Jet_partonFlavour.data());
        fChain->SetBranchAddress("FatJet_genJetAK8Idx",FatJet_genJetAK8Idx.data());
        fChain->SetBranchAddress("FatJet_nBHadrons",FatJet_nBHadrons.data());
        fChain->SetBranchAddress("FatJet_nCHadrons",FatJet_nCHadrons.data());
        fChain->SetBranchAddress("Tau_genPartFlav", Tau_genPartFlav.data());
        fChain->SetBranchAddress("Tau_genPartIdx", Tau_genPartIdx.data());

        fChain->SetBranchAddress("nGenPart", &nGenPart);
        fChain->SetBranchAddress("GenPart_genPartIdxMother", GenPart_genPartIdxMother.data());
        fChain->SetBranchAddress("GenPart_pdgId", GenPart_pdgId.data());
        fChain->SetBranchAddress("GenPart_status", GenPart_status.data());
        fChain->SetBranchAddress("GenPart_statusFlags", GenPart_statusFlags.data());
        fChain->SetBranchAddress("GenPart_mass", GenPart_mass.data());
        fChain->SetBranchAddress("GenPart_pt", GenPart_pt.data());
        fChain->SetBranchAddress("GenPart_eta", GenPart_eta.data());
        fChain->SetBranchAddress("GenPart_phi", GenPart_phi.data());

        fChain->SetBranchAddress("nLHEPart", &nLHEPart);
        fChain->SetBranchAddress("LHEPart_pdgId", LHEPart_pdgId.data());
        fChain->SetBranchAddress("LHEPart_pt", LHEPart_pt.data());
        fChain->SetBranchAddress("LHEPart_eta", LHEPart_eta.data());
        fChain->SetBranchAddress("LHEPart_phi", LHEPart_phi.data());
        fChain->SetBranchAddress("LHEPart_mass", LHEPart_mass.data());
        fChain->SetBranchAddress("LHEPart_status", LHEPart_status.data());
        fChain->SetBranchAddress("LHEPart_spin", LHEPart_spin.data());

        fChain->SetBranchAddress("nGenJet", &nGenJet);
        fChain->SetBranchAddress("GenJet_pt", GenJet_pt.data());
        fChain->SetBranchAddress("GenJet_eta", GenJet_eta.data());
        fChain->SetBranchAddress("GenJet_phi", GenJet_phi.data());
        fChain->SetBranchAddress("GenJet_mass", GenJet_mass.data());
        fChain->SetBranchAddress("GenJet_partonFlavour", GenJet_partonFlavour.data());
        fChain->SetBranchAddress("GenJet_hadronFlavour", GenJet_hadronFlavour.data());
    }
    fChain->SetBranchAddress("nMuon", &nMuon);
    fChain->SetBranchAddress("Muon_pt", Muon_pt.data());
    fChain->SetBranchAddress("Muon_eta", Muon_eta.data());
    fChain->SetBranchAddress("Muon_phi", Muon_phi.data());
    fChain->SetBranchAddress("Muon_mass", Muon_mass.data());
    fChain->SetBranchAddress("Muon_charge", Muon_charge.data());
    fChain->SetBranchAddress("Muon_dxy", Muon_dxy.data());
    fChain->SetBranchAddress("Muon_dxyErr", Muon_dxyErr.data());
    fChain->SetBranchAddress("Muon_dz", Muon_dz.data());
    fChain->SetBranchAddress("Muon_dzErr", Muon_dzErr.data());
    fChain->SetBranchAddress("Muon_ip3d", Muon_ip3d.data());
    fChain->SetBranchAddress("Muon_sip3d", Muon_sip3d.data());
    fChain->SetBranchAddress("Muon_pfRelIso03_all", Muon_pfRelIso03_all.data());
    fChain->SetBranchAddress("Muon_pfRelIso04_all", Muon_pfRelIso04_all.data());
    fChain->SetBranchAddress("Muon_miniPFRelIso_all", Muon_miniPFRelIso_all.data());
    fChain->SetBranchAddress("Muon_tkRelIso", Muon_tkRelIso.data());
    fChain->SetBranchAddress("Muon_isTracker", Muon_isTracker.data());
    fChain->SetBranchAddress("Muon_isStandalone", Muon_isStandalone.data());
    fChain->SetBranchAddress("Muon_isGlobal", Muon_isGlobal.data());
    fChain->SetBranchAddress("Muon_looseId", Muon_looseId.data());
    fChain->SetBranchAddress("Muon_mediumId", Muon_mediumId.data());
    fChain->SetBranchAddress("Muon_mediumPromptId", Muon_mediumPromptId.data());
    fChain->SetBranchAddress("Muon_tightId", Muon_tightId.data());
    fChain->SetBranchAddress("Muon_softId", Muon_softId.data());
    fChain->SetBranchAddress("Muon_softMvaId", Muon_softMvaId.data());
    fChain->SetBranchAddress("Muon_triggerIdLoose", Muon_triggerIdLoose.data());
    fChain->SetBranchAddress("Muon_highPtId", Muon_highPtId.data());
    fChain->SetBranchAddress("Muon_miniIsoId", Muon_miniIsoId.data());
    fChain->SetBranchAddress("Muon_multiIsoId", Muon_multiIsoId.data());
    fChain->SetBranchAddress("Muon_mvaMuID_WP", Muon_mvaMuId.data());
    // fChain->SetBranchAddress("Muon_mvaLowPtId", Muon_mvaLowPtId.data());
    fChain->SetBranchAddress("Muon_pfIsoId", Muon_pfIsoId.data());
    fChain->SetBranchAddress("Muon_puppiIsoId", Muon_puppiIsoId.data());
    fChain->SetBranchAddress("Muon_tkIsoId", Muon_tkIsoId.data());
    fChain->SetBranchAddress("Muon_softMva", Muon_softMva.data());
    fChain->SetBranchAddress("Muon_mvaLowPt", Muon_mvaLowPt.data());
    fChain->SetBranchAddress("Muon_mvaTTH", Muon_mvaTTH.data());

    fChain->SetBranchAddress("nElectron", &nElectron);
    fChain->SetBranchAddress("Electron_pt", Electron_pt.data());
    fChain->SetBranchAddress("Electron_eta", Electron_eta.data());
    fChain->SetBranchAddress("Electron_phi", Electron_phi.data());
    fChain->SetBranchAddress("Electron_mass", Electron_mass.data());
    fChain->SetBranchAddress("Electron_charge", Electron_charge.data());
    fChain->SetBranchAddress("Electron_dxy", Electron_dxy.data());
    fChain->SetBranchAddress("Electron_dxyErr", Electron_dxyErr.data());
    fChain->SetBranchAddress("Electron_dz", Electron_dz.data());
    fChain->SetBranchAddress("Electron_dzErr", Electron_dzErr.data());
    fChain->SetBranchAddress("Electron_ip3d", Electron_ip3d.data());
    fChain->SetBranchAddress("Electron_sip3d", Electron_sip3d.data());
    fChain->SetBranchAddress("Electron_pfRelIso03_all", Electron_pfRelIso03_all.data());
    fChain->SetBranchAddress("Electron_miniPFRelIso_all", Electron_miniPFRelIso_all.data());
    fChain->SetBranchAddress("Electron_energyErr", Electron_energyErr.data());
    fChain->SetBranchAddress("Electron_convVeto", Electron_convVeto.data());
    fChain->SetBranchAddress("Electron_lostHits", Electron_lostHits.data());
    fChain->SetBranchAddress("Electron_seedGain", Electron_seedGain.data());
    fChain->SetBranchAddress("Electron_tightCharge", Electron_tightCharge.data());
    fChain->SetBranchAddress("Electron_sieie", Electron_sieie.data());
    fChain->SetBranchAddress("Electron_hoe", Electron_hoe.data());
    fChain->SetBranchAddress("Electron_eInvMinusPInv", Electron_eInvMinusPInv.data());
    fChain->SetBranchAddress("Electron_dr03EcalRecHitSumEt", Electron_dr03EcalRecHitSumEt.data());
    fChain->SetBranchAddress("Electron_dr03HcalDepth1TowerSumEt", Electron_dr03HcalDepth1TowerSumEt.data());
    fChain->SetBranchAddress("Electron_dr03TkSumPt", Electron_dr03TkSumPt.data());
    fChain->SetBranchAddress("Electron_dr03TkSumPtHEEP", Electron_dr03TkSumPtHEEP.data());
    fChain->SetBranchAddress("Electron_deltaEtaSC", Electron_deltaEtaSC.data());
    fChain->SetBranchAddress("Electron_mvaIso_WP80", Electron_mvaIso_WP80.data());
    fChain->SetBranchAddress("Electron_mvaIso_WP90", Electron_mvaIso_WP90.data());
    fChain->SetBranchAddress("Electron_mvaNoIso_WP80", Electron_mvaNoIso_WP80.data());
    fChain->SetBranchAddress("Electron_mvaNoIso_WP90", Electron_mvaNoIso_WP90.data());
    fChain->SetBranchAddress("Electron_cutBased_HEEP", Electron_cutBased_HEEP.data());
    fChain->SetBranchAddress("Electron_cutBased", Electron_cutBased.data());
    fChain->SetBranchAddress("Electron_mvaIso", Electron_mvaIso.data());
    fChain->SetBranchAddress("Electron_mvaNoIso", Electron_mvaNoIso.data());
    fChain->SetBranchAddress("Electron_mvaTTH", Electron_mvaTTH.data());
    fChain->SetBranchAddress("Electron_r9", Electron_r9.data());
    fChain->SetBranchAddress("nPhoton", &nPhoton);
    fChain->SetBranchAddress("Photon_pt", Photon_pt.data());
    fChain->SetBranchAddress("Photon_eta", Photon_eta.data());
    fChain->SetBranchAddress("Photon_phi", Photon_phi.data());
    fChain->SetBranchAddress("Photon_energyRaw", Photon_energyRaw.data());
    fChain->SetBranchAddress("Photon_hoe", Photon_hoe.data());
    fChain->SetBranchAddress("Photon_cutBased", Photon_cutBased.data());
    fChain->SetBranchAddress("Photon_mvaID", Photon_mvaID.data());
    fChain->SetBranchAddress("Photon_mvaID_WP80", Photon_mvaID_WP80.data());
    fChain->SetBranchAddress("Photon_mvaID_WP90", Photon_mvaID_WP90.data());
    fChain->SetBranchAddress("Photon_pixelSeed", Photon_pixelSeed.data());
    fChain->SetBranchAddress("Photon_sieie", Photon_sieie.data());
    fChain->SetBranchAddress("Photon_isScEtaEB", Photon_isScEtaEB.data());
    fChain->SetBranchAddress("Photon_isScEtaEE", Photon_isScEtaEE.data());

    fChain->SetBranchAddress("nJet", &nJet);
    fChain->SetBranchAddress("Jet_PNetRegPtRawCorr", Jet_PNetRegPtRawCorr.data());
    fChain->SetBranchAddress("Jet_PNetRegPtRawCorrNeutrino", Jet_PNetRegPtRawCorrNeutrino.data());
    fChain->SetBranchAddress("Jet_PNetRegPtRawRes", Jet_PNetRegPtRawRes.data());
    fChain->SetBranchAddress("Jet_area", Jet_area.data());
    fChain->SetBranchAddress("Jet_btagDeepFlavB", Jet_btagDeepFlavB.data());
    fChain->SetBranchAddress("Jet_btagDeepFlavCvB", Jet_btagDeepFlavCvB.data());
    fChain->SetBranchAddress("Jet_btagDeepFlavCvL", Jet_btagDeepFlavCvL.data());
    fChain->SetBranchAddress("Jet_btagDeepFlavQG", Jet_btagDeepFlavQG.data());
    fChain->SetBranchAddress("Jet_btagPNetB", Jet_btagPNetB.data());
    fChain->SetBranchAddress("Jet_btagPNetCvB", Jet_btagPNetCvB.data());
    fChain->SetBranchAddress("Jet_btagPNetCvL", Jet_btagPNetCvL.data());
    fChain->SetBranchAddress("Jet_btagPNetQvG", Jet_btagPNetQvG.data());
    fChain->SetBranchAddress("Jet_btagPNetTauVJet", Jet_btagPNetTauVJet.data());
    fChain->SetBranchAddress("Jet_btagRobustParTAK4B", Jet_btagRobustParTAK4B.data());
    fChain->SetBranchAddress("Jet_btagRobustParTAK4CvB", Jet_btagRobustParTAK4CvB.data());
    fChain->SetBranchAddress("Jet_btagRobustParTAK4CvL", Jet_btagRobustParTAK4CvL.data());
    fChain->SetBranchAddress("Jet_btagRobustParTAK4QG", Jet_btagRobustParTAK4QG.data());
    fChain->SetBranchAddress("Jet_chEmEF", Jet_chEmEF.data());
    fChain->SetBranchAddress("Jet_chHEF", Jet_chHEF.data());
    fChain->SetBranchAddress("Jet_electronIdx1", Jet_electronIdx1.data());
    fChain->SetBranchAddress("Jet_electronIdx2", Jet_electronIdx2.data());
    fChain->SetBranchAddress("Jet_eta", Jet_eta.data());

    //fChain->SetBranchAddress("Jet_hfadjacentEtaStripsSize", Jet_hfadjacentEtaStripsSize);
    //fChain->SetBranchAddress("Jet_hfcentralEtaStripSize", Jet_hfcentralEtaStripSize);
    //fChain->SetBranchAddress("Jet_hfsigmaEtaEta", Jet_hfsigmaEtaEta);
    //fChain->SetBranchAddress("Jet_hfsigmaPhiPhi", Jet_hfsigmaPhiPhi);
    fChain->SetBranchAddress("Jet_jetId", Jet_jetId.data());
    fChain->SetBranchAddress("Jet_mass", Jet_mass.data());
    fChain->SetBranchAddress("Jet_muEF", Jet_muEF.data());
    fChain->SetBranchAddress("Jet_muonIdx1", Jet_muonIdx1.data());
    fChain->SetBranchAddress("Jet_muonIdx2", Jet_muonIdx2.data());
    //fChain->SetBranchAddress("Jet_muonSubtrFactor", Jet_muonSubtrFactor);
    fChain->SetBranchAddress("Jet_nConstituents", Jet_nConstituents.data());
    fChain->SetBranchAddress("Jet_nElectrons", Jet_nElectrons.data());
    fChain->SetBranchAddress("Jet_nMuons", Jet_nMuons.data());
    fChain->SetBranchAddress("Jet_nSVs", Jet_nSVs.data());
    fChain->SetBranchAddress("Jet_neEmEF", Jet_neEmEF.data());
    fChain->SetBranchAddress("Jet_neHEF", Jet_neHEF.data());
    fChain->SetBranchAddress("Jet_phi", Jet_phi.data());
    fChain->SetBranchAddress("Jet_pt", Jet_pt.data());
    fChain->SetBranchAddress("Jet_rawFactor", Jet_rawFactor.data());
    fChain->SetBranchAddress("Jet_svIdx1", Jet_svIdx1.data());
    fChain->SetBranchAddress("Jet_svIdx2", Jet_svIdx2.data());
    fChain->SetBranchAddress("nTau", &nTau);
    fChain->SetBranchAddress("Tau_pt", Tau_pt.data());
    fChain->SetBranchAddress("Tau_eta", Tau_eta.data());
    fChain->SetBranchAddress("Tau_phi", Tau_phi.data());
    fChain->SetBranchAddress("Tau_mass", Tau_mass.data());
    fChain->SetBranchAddress("Tau_charge", Tau_charge.data());
    fChain->SetBranchAddress("Tau_dxy", Tau_dxy.data());
    fChain->SetBranchAddress("Tau_dz", Tau_dz.data());
    fChain->SetBranchAddress("Tau_idDeepTau2018v2p5VSe", Tau_idDeepTau2018v2p5VSe.data());
    fChain->SetBranchAddress("Tau_idDeepTau2018v2p5VSjet", Tau_idDeepTau2018v2p5VSjet.data());
    fChain->SetBranchAddress("Tau_idDeepTau2018v2p5VSmu", Tau_idDeepTau2018v2p5VSmu.data());
    fChain->SetBranchAddress("Tau_decayMode", Tau_decayMode.data());
    fChain->SetBranchAddress("Tau_idDecayModeNewDMs", Tau_idDecayModeNewDMs.data());
    fChain->SetBranchAddress("nFatJet",&nFatJet);
    fChain->SetBranchAddress("FatJet_pt",FatJet_pt.data());
    fChain->SetBranchAddress("FatJet_eta",FatJet_eta.data());
    fChain->SetBranchAddress("FatJet_phi",FatJet_phi.data());
    fChain->SetBranchAddress("FatJet_mass",FatJet_mass.data());
    fChain->SetBranchAddress("FatJet_area",FatJet_area.data());
    fChain->SetBranchAddress("FatJet_jetId",FatJet_jetId.data());
    fChain->SetBranchAddress("FatJet_lsf3",FatJet_lsf3.data());
    fChain->SetBranchAddress("FatJet_msoftdrop",FatJet_msoftdrop.data());
    fChain->SetBranchAddress("FatJet_nConstituents",FatJet_nConstituents.data());
    fChain->SetBranchAddress("FatJet_btagDDBvLV2",FatJet_btagDDBvLV2.data());
    fChain->SetBranchAddress("FatJet_btagDDCvBV2",FatJet_btagDDCvBV2.data());
    fChain->SetBranchAddress("FatJet_btagDDCvLV2",FatJet_btagDDCvLV2.data());
    fChain->SetBranchAddress("FatJet_btagDeepB",FatJet_btagDeepB.data());
    fChain->SetBranchAddress("FatJet_btagHbb",FatJet_btagHbb.data());
    fChain->SetBranchAddress("FatJet_particleNetWithMass_H4qvsQCD",FatJet_particleNetWithMass_H4qvsQCD.data());
    fChain->SetBranchAddress("FatJet_particleNetWithMass_HbbvsQCD",FatJet_particleNetWithMass_HbbvsQCD.data());
    fChain->SetBranchAddress("FatJet_particleNetWithMass_HccvsQCD",FatJet_particleNetWithMass_HccvsQCD.data());
    fChain->SetBranchAddress("FatJet_particleNetWithMass_QCD",FatJet_particleNetWithMass_QCD.data());
    fChain->SetBranchAddress("FatJet_particleNetWithMass_TvsQCD",FatJet_particleNetWithMass_TvsQCD.data());
    fChain->SetBranchAddress("FatJet_particleNetWithMass_WvsQCD",FatJet_particleNetWithMass_WvsQCD.data());
    fChain->SetBranchAddress("FatJet_particleNetWithMass_ZvsQCD",FatJet_particleNetWithMass_ZvsQCD.data());
    fChain->SetBranchAddress("FatJet_particleNet_QCD",FatJet_particleNet_QCD.data());
    fChain->SetBranchAddress("FatJet_particleNet_QCD0HF",FatJet_particleNet_QCD0HF.data());
    fChain->SetBranchAddress("FatJet_particleNet_QCD1HF",FatJet_particleNet_QCD1HF.data());
    fChain->SetBranchAddress("FatJet_particleNet_QCD2HF",FatJet_particleNet_QCD2HF.data());
    fChain->SetBranchAddress("FatJet_particleNet_XbbVsQCD",FatJet_particleNet_XbbVsQCD.data());
    fChain->SetBranchAddress("FatJet_particleNet_XccVsQCD",FatJet_particleNet_XccVsQCD.data());
    fChain->SetBranchAddress("FatJet_particleNet_XggVsQCD",FatJet_particleNet_XggVsQCD.data());
    fChain->SetBranchAddress("FatJet_particleNet_XqqVsQCD",FatJet_particleNet_XqqVsQCD.data());
    fChain->SetBranchAddress("FatJet_particleNet_XteVsQCD",FatJet_particleNet_XteVsQCD.data());
    fChain->SetBranchAddress("FatJet_particleNet_XtmVsQCD",FatJet_particleNet_XtmVsQCD.data());
    fChain->SetBranchAddress("FatJet_particleNet_XttVsQCD",FatJet_particleNet_XttVsQCD.data());
    fChain->SetBranchAddress("FatJet_particleNet_massCorr",FatJet_particleNet_massCorr.data());
    fChain->SetBranchAddress("FatJet_tau1",FatJet_tau1.data());
    fChain->SetBranchAddress("FatJet_tau2",FatJet_tau2.data());
    fChain->SetBranchAddress("FatJet_tau3",FatJet_tau3.data());
    fChain->SetBranchAddress("FatJet_tau4",FatJet_tau4.data());
    fChain->SetBranchAddress("FatJet_subJetIdx1",FatJet_subJetIdx1.data());
    fChain->SetBranchAddress("FatJet_subJetIdx2",FatJet_subJetIdx2.data());

    fChain->SetBranchAddress("PuppiMET_pt", &PuppiMET_pt);
    fChain->SetBranchAddress("PuppiMET_phi", &PuppiMET_phi);
    fChain->SetBranchAddress("PuppiMET_sumEt", &PuppiMET_sumEt);
    fChain->SetBranchAddress("PuppiMET_ptJERUp", &PuppiMET_ptJERUp);
    fChain->SetBranchAddress("PuppiMET_phiJERUp", &PuppiMET_phiJERUp);
    fChain->SetBranchAddress("PuppiMET_ptJERDown", &PuppiMET_ptJERDown);
    fChain->SetBranchAddress("PuppiMET_phiJERDown", &PuppiMET_phiJERDown);
    fChain->SetBranchAddress("PuppiMET_ptUnclusteredUp", &PuppiMET_ptUnclusteredUp);
    fChain->SetBranchAddress("PuppiMET_phiUnclusteredUp", &PuppiMET_phiUnclusteredUp);
    fChain->SetBranchAddress("PuppiMET_ptUnclusteredDown", &PuppiMET_ptUnclusteredDown);
    fChain->SetBranchAddress("PuppiMET_phiUnclusteredDown", &PuppiMET_phiUnclusteredDown);
    fChain->SetBranchAddress("PuppiMET_ptJESUp", &PuppiMET_ptJESUp);
    fChain->SetBranchAddress("PuppiMET_phiJESUp", &PuppiMET_phiJESUp);
    fChain->SetBranchAddress("PuppiMET_ptJESDown", &PuppiMET_ptJESDown);
    fChain->SetBranchAddress("PuppiMET_phiJESDown", &PuppiMET_phiJESDown);

    fChain->SetBranchAddress("Rho_fixedGridRhoFastjetAll", &fixedGridRhoFastjetAll);

    fChain->SetBranchAddress("PV_npvsGood", &PV_npvsGood);

    fChain->SetBranchAddress("Flag_METFilters", &Flag_METFilters);
    fChain->SetBranchAddress("Flag_goodVertices", &Flag_goodVertices);
    fChain->SetBranchAddress("Flag_globalSuperTightHalo2016Filter", &Flag_globalSuperTightHalo2016Filter);
    //fChain->SetBranchAddress("Flag_ECalDeadCellTriggerPrimitiveFilter", &Flag_ECalDeadCellTriggerPrimitiveFilter);
    //documented as this brach exist @ NanoAOD, but not in the file
    fChain->SetBranchAddress("Flag_BadPFMuonFilter", &Flag_BadPFMuonFilter);
    fChain->SetBranchAddress("Flag_BadPFMuonDzFilter", &Flag_BadPFMuonDzFilter);
    fChain->SetBranchAddress("Flag_hfNoisyHitsFilter", &Flag_hfNoisyHitsFilter);
    fChain->SetBranchAddress("Flag_ecalBadCalibFilter", &Flag_ecalBadCalibFilter);
    fChain->SetBranchAddress("Flag_eeBadScFilter", &Flag_eeBadScFilter);
    //fChain->SetBranchAddress("Flag_ecalBadCalibFilter", &Flag_ecalBadCalibFilter);
    fChain->SetBranchAddress("run", &Run);

   


    string json_path = string(getenv("SKNANO_DATA")) + "/" + DataEra.Data() + "/Trigger/NanoAODv12_HLT_Path.json";
    ifstream json_file(json_path);
    if (json_file.is_open()) {
        cout << "[SKNanoLoader::Init] Loading HLT Paths in" << json_path << endl;
        json j;
        json_file >> j;
        RVec<TString> not_in_tree;
        for (auto& [key, value] : j.items()) {
            cout << "[SKNanoLoader::Init] HLT Path: " << key << endl;
            Bool_t* passHLT = new Bool_t();
            TString key_str = key;
            TriggerMap[key_str] = passHLT;
            //if key_str is in tree, set branch address
            if(fChain->GetBranch(key_str)){
                fChain->SetBranchAddress(key_str, TriggerMap[key_str]);
            }
            else{
                not_in_tree.push_back(key_str);
                TriggerMap.erase(key_str);
            }   
        }
        if (not_in_tree.size() > 0)
        {
            //print in yellow color
            cout << "\033[1;33m[SKNanoLoader::Init] Following HLT Paths are not in the tree\033[0m" << endl;
            for (auto &path : not_in_tree)
            {
                cout << "\033[1;33m" << path << "\033[0m" << endl;
            }
        }
    }
    else cerr << "[SKNanoLoader::Init] Cannot open " << json_path << endl;
}


