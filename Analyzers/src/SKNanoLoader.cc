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
    if (!fChain) return;
    for (auto& [key, value] : TriggerMap) {
        delete value;
    }
    delete fChain->GetCurrentFile();
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

void SKNanoLoader::Init() {
    // Set object pointer
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
        fChain->SetBranchAddress("Electron_genPartFlav", Electron_genPartFlav);
        fChain->SetBranchAddress("Electron_genPartIdx", Electron_genPartIdx);
        fChain->SetBranchAddress("Jet_genJetIdx", Jet_genJetIdx);
        fChain->SetBranchAddress("Jet_hadronFlavour", Jet_hadronFlavour);
        fChain->SetBranchAddress("Jet_partonFlavour", Jet_partonFlavour);
        fChain->SetBranchAddress("FatJet_genJetAK8Idx",FatJet_genJetAK8Idx);
        fChain->SetBranchAddress("FatJet_nBHadrons",FatJet_nBHadrons);
        fChain->SetBranchAddress("FatJet_nCHadrons",FatJet_nCHadrons);
        fChain->SetBranchAddress("Tau_genPartFlav", Tau_genPartFlav);
        fChain->SetBranchAddress("Tau_genPartIdx", Tau_genPartIdx);

        fChain->SetBranchAddress("nGenPart", &nGenPart);
        fChain->SetBranchAddress("GenPart_genPartIdxMother", GenPart_genPartIdxMother);
        fChain->SetBranchAddress("GenPart_pdgId", GenPart_pdgId);
        fChain->SetBranchAddress("GenPart_status", GenPart_status);
        fChain->SetBranchAddress("GenPart_statusFlags", GenPart_statusFlags);
        fChain->SetBranchAddress("GenPart_mass", GenPart_mass);
        fChain->SetBranchAddress("GenPart_pt", GenPart_pt);
        fChain->SetBranchAddress("GenPart_eta", GenPart_eta);
        fChain->SetBranchAddress("GenPart_phi", GenPart_phi);

        fChain->SetBranchAddress("nLHEPart", &nLHEPart);
        fChain->SetBranchAddress("LHEPart_pdgId", LHEPart_pdgId);
        fChain->SetBranchAddress("LHEPart_pt", LHEPart_pt);
        fChain->SetBranchAddress("LHEPart_eta", LHEPart_eta);
        fChain->SetBranchAddress("LHEPart_phi", LHEPart_phi);
        fChain->SetBranchAddress("LHEPart_mass", LHEPart_mass);
        fChain->SetBranchAddress("LHEPart_status", LHEPart_status);
        fChain->SetBranchAddress("LHEPart_spin", LHEPart_spin);

        fChain->SetBranchAddress("nGenJet", &nGenJet);
        fChain->SetBranchAddress("GenJet_pt", GenJet_pt);
        fChain->SetBranchAddress("GenJet_eta", GenJet_eta);
        fChain->SetBranchAddress("GenJet_phi", GenJet_phi);
        fChain->SetBranchAddress("GenJet_mass", GenJet_mass);
        fChain->SetBranchAddress("GenJet_partonFlavour", GenJet_partonFlavour);
        fChain->SetBranchAddress("GenJet_hadronFlavour", GenJet_hadronFlavour);
    }
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
    //fChain->SetBranchAddress("Muon_mvaTTH", Muon_mvaTTH);Due to the unknown error, I hard coded the value of Muon_mvaTTH as array of zeros.
    for (int i = 0; i < kMaxMuon; i++) Muon_mvaTTH[i] = 0.;


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
    
    fChain->SetBranchAddress("nJet", &nJet);
    fChain->SetBranchAddress("Jet_PNetRegPtRawCorr", Jet_PNetRegPtRawCorr);
    fChain->SetBranchAddress("Jet_PNetRegPtRawCorrNeutrino", Jet_PNetRegPtRawCorrNeutrino);
    fChain->SetBranchAddress("Jet_PNetRegPtRawRes", Jet_PNetRegPtRawRes);
    fChain->SetBranchAddress("Jet_area", Jet_area);
    fChain->SetBranchAddress("Jet_btagDeepFlavB", Jet_btagDeepFlavB);
    fChain->SetBranchAddress("Jet_btagDeepFlavCvB", Jet_btagDeepFlavCvB);
    fChain->SetBranchAddress("Jet_btagDeepFlavCvL", Jet_btagDeepFlavCvL);
    fChain->SetBranchAddress("Jet_btagDeepFlavQG", Jet_btagDeepFlavQG);
    fChain->SetBranchAddress("Jet_btagPNetB", Jet_btagPNetB);
    fChain->SetBranchAddress("Jet_btagPNetCvB", Jet_btagPNetCvB);
    fChain->SetBranchAddress("Jet_btagPNetCvL", Jet_btagPNetCvL);
    fChain->SetBranchAddress("Jet_btagPNetQvG", Jet_btagPNetQvG);
    fChain->SetBranchAddress("Jet_btagPNetTauVJet", Jet_btagPNetTauVJet);
    fChain->SetBranchAddress("Jet_btagRobustParTAK4B", Jet_btagRobustParTAK4B);
    fChain->SetBranchAddress("Jet_btagRobustParTAK4CvB", Jet_btagRobustParTAK4CvB);
    fChain->SetBranchAddress("Jet_btagRobustParTAK4CvL", Jet_btagRobustParTAK4CvL);
    fChain->SetBranchAddress("Jet_btagRobustParTAK4QG", Jet_btagRobustParTAK4QG);
    fChain->SetBranchAddress("Jet_chEmEF", Jet_chEmEF);
    fChain->SetBranchAddress("Jet_chHEF", Jet_chHEF);
    fChain->SetBranchAddress("Jet_electronIdx1", Jet_electronIdx1);
    fChain->SetBranchAddress("Jet_electronIdx2", Jet_electronIdx2);
    fChain->SetBranchAddress("Jet_eta", Jet_eta);

    //fChain->SetBranchAddress("Jet_hfadjacentEtaStripsSize", Jet_hfadjacentEtaStripsSize);
    //fChain->SetBranchAddress("Jet_hfcentralEtaStripSize", Jet_hfcentralEtaStripSize);
    //fChain->SetBranchAddress("Jet_hfsigmaEtaEta", Jet_hfsigmaEtaEta);
    //fChain->SetBranchAddress("Jet_hfsigmaPhiPhi", Jet_hfsigmaPhiPhi);
    fChain->SetBranchAddress("Jet_jetId", Jet_jetId);
    fChain->SetBranchAddress("Jet_mass", Jet_mass);
    fChain->SetBranchAddress("Jet_muEF", Jet_muEF);
    fChain->SetBranchAddress("Jet_muonIdx1", Jet_muonIdx1);
    fChain->SetBranchAddress("Jet_muonIdx2", Jet_muonIdx2);
    //fChain->SetBranchAddress("Jet_muonSubtrFactor", Jet_muonSubtrFactor);
    fChain->SetBranchAddress("Jet_nConstituents", Jet_nConstituents);
    fChain->SetBranchAddress("Jet_nElectrons", Jet_nElectrons);
    fChain->SetBranchAddress("Jet_nMuons", Jet_nMuons);
    fChain->SetBranchAddress("Jet_nSVs", Jet_nSVs);
    fChain->SetBranchAddress("Jet_neEmEF", Jet_neEmEF);
    fChain->SetBranchAddress("Jet_neHEF", Jet_neHEF);
    fChain->SetBranchAddress("Jet_phi", Jet_phi);
    fChain->SetBranchAddress("Jet_pt", Jet_pt);
    fChain->SetBranchAddress("Jet_rawFactor", Jet_rawFactor);
    fChain->SetBranchAddress("Jet_svIdx1", Jet_svIdx1);
    fChain->SetBranchAddress("Jet_svIdx2", Jet_svIdx2);
    fChain->SetBranchAddress("nTau", &nTau);
    fChain->SetBranchAddress("Tau_pt", Tau_pt);
    fChain->SetBranchAddress("Tau_eta", Tau_eta);
    fChain->SetBranchAddress("Tau_phi", Tau_phi);
    fChain->SetBranchAddress("Tau_mass", Tau_mass);
    fChain->SetBranchAddress("Tau_charge", Tau_charge);
    fChain->SetBranchAddress("Tau_dxy", Tau_dxy);
    fChain->SetBranchAddress("Tau_dz", Tau_dz);
    fChain->SetBranchAddress("Tau_idDeepTau2018v2p5VSe", Tau_idDeepTau2018v2p5VSe);
    fChain->SetBranchAddress("Tau_idDeepTau2018v2p5VSjet", Tau_idDeepTau2018v2p5VSjet);
    fChain->SetBranchAddress("Tau_idDeepTau2018v2p5VSmu", Tau_idDeepTau2018v2p5VSmu);
    fChain->SetBranchAddress("Tau_decayMode", Tau_decayMode);
    fChain->SetBranchAddress("Tau_idDecayModeNewDMs", Tau_idDecayModeNewDMs);
    fChain->SetBranchAddress("nFatJet",&nFatJet);
    fChain->SetBranchAddress("FatJet_pt",FatJet_pt);
    fChain->SetBranchAddress("FatJet_eta",FatJet_eta);
    fChain->SetBranchAddress("FatJet_phi",FatJet_phi);
    fChain->SetBranchAddress("FatJet_mass",FatJet_mass);
    fChain->SetBranchAddress("FatJet_area",FatJet_area);
    fChain->SetBranchAddress("FatJet_jetId",FatJet_jetId);
    fChain->SetBranchAddress("FatJet_lsf3",FatJet_lsf3);
    fChain->SetBranchAddress("FatJet_msoftdrop",FatJet_msoftdrop);
    fChain->SetBranchAddress("FatJet_nConstituents",FatJet_nConstituents);
    fChain->SetBranchAddress("FatJet_btagDDBvLV2",FatJet_btagDDBvLV2);
    fChain->SetBranchAddress("FatJet_btagDDCvBV2",FatJet_btagDDCvBV2);
    fChain->SetBranchAddress("FatJet_btagDDCvLV2",FatJet_btagDDCvLV2);
    fChain->SetBranchAddress("FatJet_btagDeepB",FatJet_btagDeepB);
    fChain->SetBranchAddress("FatJet_btagHbb",FatJet_btagHbb);
    fChain->SetBranchAddress("FatJet_particleNetWithMass_H4qvsQCD",FatJet_particleNetWithMass_H4qvsQCD);
    fChain->SetBranchAddress("FatJet_particleNetWithMass_HbbvsQCD",FatJet_particleNetWithMass_HbbvsQCD);
    fChain->SetBranchAddress("FatJet_particleNetWithMass_HccvsQCD",FatJet_particleNetWithMass_HccvsQCD);
    fChain->SetBranchAddress("FatJet_particleNetWithMass_QCD",FatJet_particleNetWithMass_QCD);
    fChain->SetBranchAddress("FatJet_particleNetWithMass_TvsQCD",FatJet_particleNetWithMass_TvsQCD);
    fChain->SetBranchAddress("FatJet_particleNetWithMass_WvsQCD",FatJet_particleNetWithMass_WvsQCD);
    fChain->SetBranchAddress("FatJet_particleNetWithMass_ZvsQCD",FatJet_particleNetWithMass_ZvsQCD);
    fChain->SetBranchAddress("FatJet_particleNet_QCD",FatJet_particleNet_QCD);
    fChain->SetBranchAddress("FatJet_particleNet_QCD0HF",FatJet_particleNet_QCD0HF);
    fChain->SetBranchAddress("FatJet_particleNet_QCD1HF",FatJet_particleNet_QCD1HF);
    fChain->SetBranchAddress("FatJet_particleNet_QCD2HF",FatJet_particleNet_QCD2HF);
    fChain->SetBranchAddress("FatJet_particleNet_XbbVsQCD",FatJet_particleNet_XbbVsQCD);
    fChain->SetBranchAddress("FatJet_particleNet_XccVsQCD",FatJet_particleNet_XccVsQCD);
    fChain->SetBranchAddress("FatJet_particleNet_XggVsQCD",FatJet_particleNet_XggVsQCD);
    fChain->SetBranchAddress("FatJet_particleNet_XqqVsQCD",FatJet_particleNet_XqqVsQCD);
    fChain->SetBranchAddress("FatJet_particleNet_XteVsQCD",FatJet_particleNet_XteVsQCD);
    fChain->SetBranchAddress("FatJet_particleNet_XtmVsQCD",FatJet_particleNet_XtmVsQCD);
    fChain->SetBranchAddress("FatJet_particleNet_XttVsQCD",FatJet_particleNet_XttVsQCD);
    fChain->SetBranchAddress("FatJet_particleNet_massCorr",FatJet_particleNet_massCorr);
    fChain->SetBranchAddress("FatJet_tau1",FatJet_tau1);
    fChain->SetBranchAddress("FatJet_tau2",FatJet_tau2);
    fChain->SetBranchAddress("FatJet_tau3",FatJet_tau3);
    fChain->SetBranchAddress("FatJet_tau4",FatJet_tau4);
    fChain->SetBranchAddress("FatJet_subJetIdx1",FatJet_subJetIdx1);
    fChain->SetBranchAddress("FatJet_subJetIdx2",FatJet_subJetIdx2);

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
        for (auto& [key, value] : j.items()) {
            cout << "[SKNanoLoader::Init] HLT Path: " << key << endl;
            Bool_t* passHLT = new Bool_t();
            TString key_str = key;
            TriggerMap[key_str] = passHLT;
            fChain->SetBranchAddress(key_str, TriggerMap[key_str]);
        }
    } 
    else cerr << "[SKNanoLoader::Init] Cannot open " << json_path << endl;
    

}


