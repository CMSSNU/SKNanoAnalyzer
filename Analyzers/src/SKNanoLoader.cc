#define SKNanoLoader_cxx
#include "SKNanoLoader.h"
using json = nlohmann::json;

SKNanoLoader::SKNanoLoader()
{
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

SKNanoLoader::~SKNanoLoader()
{
    for (auto &[key, value] : TriggerMap)
    {
        delete value.first;
    }
    if (!fChain)
        return;
    if (fChain->GetCurrentFile())
        fChain->GetCurrentFile()->Close();
}

void SKNanoLoader::Loop()
{
    long nentries = fReader->GetEntries();
    if (MaxEvent > 0)
        nentries = std::min(nentries, MaxEvent);
    auto startTime = std::chrono::steady_clock::now();
    cout << "[SKNanoLoader::Loop] Event Loop Started" << endl;

    int jentry = 0;
    while (fReader->Next())
    {
        if (jentry < NSkipEvent)
            continue;
        // Log progress for every LogEvery events
        if (jentry % LogEvery == 0)
        {
            auto currentTime = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsedTime = currentTime - startTime;
            double timePerEvent = elapsedTime.count() / (jentry + 1);
            double estimatedRemaining = (nentries - jentry) * timePerEvent;

            cout << "[SKNanoLoader::Loop] Processing " << jentry << " / " << nentries
                 << " | Elapsed: " << std::fixed << std::setprecision(2) << elapsedTime.count() << "s, Remaining: " << estimatedRemaining << "s" << endl;
        }
        executeEvent();
        jentry++;
    }
    cout << "[SKNanoLoader::Loop] Event Loop Finished" << endl;
}

void SKNanoLoader::Init()
{
    auto SafeSetBranchAddress = [this](const TString &branchName, void *address)
    {
        TBranch *branch = fChain->GetBranch(branchName);
        if (!branch)
        {
            cout << "[SKNanoGenLoader::Init] Warning:Branch " << branchName << " not found" << endl;
            return;
        }
        fChain->SetBranchStatus(branchName, 1);
        fChain->SetBranchAddress(branchName, address);
    };

    cout << "[SKNanoLoader::Init] Initializing. Era = " << DataEra << " Run =  " << Run << endl;
    if (fChain->GetEntries() == 0)
    {
        cout << "[SKNanoLoader::Init] No Entries in the Tree" << endl;
        cout << "[SKNanoLoader::Init] Exiting without make output..." << endl;
        exit(0);
    }

    fChain->SetBranchStatus("*", 0);
    fReader = new TTreeReader(fChain);

    string json_path = string(getenv("SKNANO_DATA")) + "/" + DataEra.Data() + "/Trigger/HLT_Path.json";
    ifstream json_file(json_path);
    if (json_file.is_open())
    {
        cout << "[SKNanoLoader::Init] Loading HLT Paths in" << json_path << endl;
        json j;
        json_file >> j;
        RVec<TString> not_in_tree;
        for (auto &[key, value] : j.items())
        {
            cout << "[SKNanoLoader::Init] HLT Path: " << key << endl;
            Bool_t *passHLT = new Bool_t();
            TString key_str = key;
            TriggerMap[key_str].first = passHLT;
            TriggerMap[key_str].second = value["lumi"];
            // if key_str is in tree, set branch address
            if (fChain->GetBranch(key_str))
            {
                SafeSetBranchAddress(key_str, TriggerMap[key_str].first);
            }
            else if (key_str == "Full")
            {
                *TriggerMap[key_str].first = true;
            }
            else
            {
                not_in_tree.push_back(key_str);
                TriggerMap.erase(key_str);
            }
        }
        if (not_in_tree.size() > 0)
        {
            // print in yellow color
            cout << "\033[1;33m[SKNanoLoader::Init] Following HLT Paths are not in the tree\033[0m" << endl;
            for (auto &path : not_in_tree)
            {
                cout << "\033[1;33m" << path << "\033[0m" << endl;
            }
        }
    }
    else
        cerr << "[SKNanoLoader::Init] Cannot open " << json_path << endl;
}

void SKNanoLoader::InitBranch()
{
    // ==============================================================
    //  _         _                         _       _    _         _     _
    // | |__  ___| |_____ __ __  __ ___  __| |___  | |__| |___  __| |__ (_)___
    // | '_ \/ -_) / _ \ V  V / / _/ _ \/ _` / -_) | '_ \ / _ \/ _| / / | (_-<
    // |_.__/\___|_\___/\_/\_/  \__\___/\__,_\___| |_.__/_\___/\__|_\_\ |_/__/

    //            _                                      _          _ _
    //  __ _ _  _| |_ ___ ___ __ _ ___ _ _  ___ _ _ __ _| |_ ___ __| | |
    // / _` | || |  _/ _ \___/ _` / -_) ' \/ -_) '_/ _` |  _/ -_) _` |_|
    // \__,_|\_,_|\__\___/   \__, \___|_||_\___|_| \__,_|\__\___\__,_(_)
    //                       |___/
    //==============================================================
    // Electron
    Electron_IPx.init(*fReader, "Electron_IPx");
    Electron_IPy.init(*fReader, "Electron_IPy");
    Electron_IPz.init(*fReader, "Electron_IPz");
    Electron_PreshowerEnergy.init(*fReader, "Electron_PreshowerEnergy");
    Electron_charge.init(*fReader, "Electron_charge");
    Electron_convVeto.init(*fReader, "Electron_convVeto");
    Electron_cutBased.init(*fReader, "Electron_cutBased");
    Electron_cutBased_HEEP.init(*fReader, "Electron_cutBased_HEEP");
    Electron_deltaEtaSC.init(*fReader, "Electron_deltaEtaSC");
    Electron_dr03EcalRecHitSumEt.init(*fReader, "Electron_dr03EcalRecHitSumEt");
    Electron_dr03HcalDepth1TowerSumEt.init(*fReader, "Electron_dr03HcalDepth1TowerSumEt");
    Electron_dr03TkSumPt.init(*fReader, "Electron_dr03TkSumPt");
    Electron_dr03TkSumPtHEEP.init(*fReader, "Electron_dr03TkSumPtHEEP");
    Electron_dxy.init(*fReader, "Electron_dxy");
    Electron_dxyErr.init(*fReader, "Electron_dxyErr");
    Electron_dz.init(*fReader, "Electron_dz");
    Electron_dzErr.init(*fReader, "Electron_dzErr");
    Electron_eInvMinusPInv.init(*fReader, "Electron_eInvMinusPInv");
    Electron_ecalEnergy.init(*fReader, "Electron_ecalEnergy");
    Electron_ecalEnergyError.init(*fReader, "Electron_ecalEnergyError");
    Electron_energyErr.init(*fReader, "Electron_energyErr");
    Electron_eta.init(*fReader, "Electron_eta");
    Electron_fbrem.init(*fReader, "Electron_fbrem");
    Electron_fsrPhotonIdx.init(*fReader, "Electron_fsrPhotonIdx");
    Electron_genPartFlav.init(*fReader, "Electron_genPartFlav");
    Electron_genPartIdx.init(*fReader, "Electron_genPartIdx");
    Electron_gsfTrketaMode.init(*fReader, "Electron_gsfTrketaMode");
    Electron_gsfTrkpMode.init(*fReader, "Electron_gsfTrkpMode");
    Electron_gsfTrkpModeErr.init(*fReader, "Electron_gsfTrkpModeErr");
    Electron_gsfTrkphiMode.init(*fReader, "Electron_gsfTrkphiMode");
    Electron_hoe.init(*fReader, "Electron_hoe");
    Electron_ip3d.init(*fReader, "Electron_ip3d");
    Electron_ipLengthSig.init(*fReader, "Electron_ipLengthSig");
    Electron_isEB.init(*fReader, "Electron_isEB");
    Electron_isEcalDriven.init(*fReader, "Electron_isEcalDriven");
    Electron_isPFcand.init(*fReader, "Electron_isPFcand");
    Electron_jetDF.init(*fReader, "Electron_jetDF");
    Electron_jetIdx.init(*fReader, "Electron_jetIdx");
    Electron_jetNDauCharged.init(*fReader, "Electron_jetNDauCharged");
    Electron_jetPtRelv2.init(*fReader, "Electron_jetPtRelv2");
    Electron_jetRelIso.init(*fReader, "Electron_jetRelIso");
    Electron_lostHits.init(*fReader, "Electron_lostHits");
    Electron_mass.init(*fReader, "Electron_mass");
    Electron_miniPFRelIso_all.init(*fReader, "Electron_miniPFRelIso_all");
    Electron_miniPFRelIso_chg.init(*fReader, "Electron_miniPFRelIso_chg");
    Electron_mvaHZZIso.init(*fReader, "Electron_mvaHZZIso");
    Electron_mvaIso.init(*fReader, "Electron_mvaIso");
    Electron_mvaIso_WP80.init(*fReader, "Electron_mvaIso_WP80");
    Electron_mvaIso_WP90.init(*fReader, "Electron_mvaIso_WP90");
    Electron_mvaIso_WPHZZ.init(*fReader, "Electron_mvaIso_WPHZZ");
    Electron_mvaNoIso.init(*fReader, "Electron_mvaNoIso");
    Electron_mvaNoIso_WP80.init(*fReader, "Electron_mvaNoIso_WP80");
    Electron_mvaNoIso_WP90.init(*fReader, "Electron_mvaNoIso_WP90");
    Electron_pdgId.init(*fReader, "Electron_pdgId");
    Electron_pfRelIso03_all.init(*fReader, "Electron_pfRelIso03_all");
    Electron_pfRelIso03_chg.init(*fReader, "Electron_pfRelIso03_chg");
    Electron_pfRelIso04_all.init(*fReader, "Electron_pfRelIso04_all");
    Electron_phi.init(*fReader, "Electron_phi");
    Electron_photonIdx.init(*fReader, "Electron_photonIdx");
    Electron_promptMVA.init(*fReader, "Electron_promptMVA");
    Electron_pt.init(*fReader, "Electron_pt");
    Electron_r9.init(*fReader, "Electron_r9");
    Electron_rawEnergy.init(*fReader, "Electron_rawEnergy");
    Electron_scEtOverPt.init(*fReader, "Electron_scEtOverPt");
    Electron_seedGain.init(*fReader, "Electron_seedGain");
    Electron_seediEtaOriX.init(*fReader, "Electron_seediEtaOriX");
    Electron_seediPhiOriY.init(*fReader, "Electron_seediPhiOriY");
    Electron_sieie.init(*fReader, "Electron_sieie");
    Electron_sip3d.init(*fReader, "Electron_sip3d");
    Electron_superclusterEta.init(*fReader, "Electron_superclusterEta");
    Electron_svIdx.init(*fReader, "Electron_svIdx");
    Electron_tightCharge.init(*fReader, "Electron_tightCharge");
    Electron_vidNestedWPBitmap.init(*fReader, "Electron_vidNestedWPBitmap");
    Electron_vidNestedWPBitmapHEEP.init(*fReader, "Electron_vidNestedWPBitmapHEEP");
    nElectron.init(*fReader, "nElectron");

    // FatJet
    FatJet_area.init(*fReader, "FatJet_area");
    FatJet_chEmEF.init(*fReader, "FatJet_chEmEF");
    FatJet_chHEF.init(*fReader, "FatJet_chHEF");
    FatJet_chMultiplicity.init(*fReader, "FatJet_chMultiplicity");
    FatJet_electronIdx3SJ.init(*fReader, "FatJet_electronIdx3SJ");
    FatJet_eta.init(*fReader, "FatJet_eta");
    FatJet_genJetAK8Idx.init(*fReader, "FatJet_genJetAK8Idx");
    FatJet_globalParT3_QCD.init(*fReader, "FatJet_globalParT3_QCD");
    FatJet_globalParT3_TopbWev.init(*fReader, "FatJet_globalParT3_TopbWev");
    FatJet_globalParT3_TopbWmv.init(*fReader, "FatJet_globalParT3_TopbWmv");
    FatJet_globalParT3_TopbWq.init(*fReader, "FatJet_globalParT3_TopbWq");
    FatJet_globalParT3_TopbWqq.init(*fReader, "FatJet_globalParT3_TopbWqq");
    FatJet_globalParT3_TopbWtauhv.init(*fReader, "FatJet_globalParT3_TopbWtauhv");
    FatJet_globalParT3_WvsQCD.init(*fReader, "FatJet_globalParT3_WvsQCD");
    FatJet_globalParT3_XWW3q.init(*fReader, "FatJet_globalParT3_XWW3q");
    FatJet_globalParT3_XWW4q.init(*fReader, "FatJet_globalParT3_XWW4q");
    FatJet_globalParT3_XWWqqev.init(*fReader, "FatJet_globalParT3_XWWqqev");
    FatJet_globalParT3_XWWqqmv.init(*fReader, "FatJet_globalParT3_XWWqqmv");
    FatJet_globalParT3_Xbb.init(*fReader, "FatJet_globalParT3_Xbb");
    FatJet_globalParT3_Xcc.init(*fReader, "FatJet_globalParT3_Xcc");
    FatJet_globalParT3_Xcs.init(*fReader, "FatJet_globalParT3_Xcs");
    FatJet_globalParT3_Xqq.init(*fReader, "FatJet_globalParT3_Xqq");
    FatJet_globalParT3_Xtauhtaue.init(*fReader, "FatJet_globalParT3_Xtauhtaue");
    FatJet_globalParT3_Xtauhtauh.init(*fReader, "FatJet_globalParT3_Xtauhtauh");
    FatJet_globalParT3_Xtauhtaum.init(*fReader, "FatJet_globalParT3_Xtauhtaum");
    FatJet_globalParT3_massCorrGeneric.init(*fReader, "FatJet_globalParT3_massCorrGeneric");
    FatJet_globalParT3_massCorrX2p.init(*fReader, "FatJet_globalParT3_massCorrX2p");
    FatJet_globalParT3_withMassTopvsQCD.init(*fReader, "FatJet_globalParT3_withMassTopvsQCD");
    FatJet_globalParT3_withMassWvsQCD.init(*fReader, "FatJet_globalParT3_withMassWvsQCD");
    FatJet_globalParT3_withMassZvsQCD.init(*fReader, "FatJet_globalParT3_withMassZvsQCD");
    FatJet_hadronFlavour.init(*fReader, "FatJet_hadronFlavour");
    FatJet_hfEmEF.init(*fReader, "FatJet_hfEmEF");
    FatJet_hfHEF.init(*fReader, "FatJet_hfHEF");
    FatJet_lsf3.init(*fReader, "FatJet_lsf3");
    FatJet_mass.init(*fReader, "FatJet_mass");
    FatJet_msoftdrop.init(*fReader, "FatJet_msoftdrop");
    FatJet_muEF.init(*fReader, "FatJet_muEF");
    FatJet_muonIdx3SJ.init(*fReader, "FatJet_muonIdx3SJ");
    FatJet_n2b1.init(*fReader, "FatJet_n2b1");
    FatJet_n3b1.init(*fReader, "FatJet_n3b1");
    FatJet_nConstituents.init(*fReader, "FatJet_nConstituents");
    FatJet_neEmEF.init(*fReader, "FatJet_neEmEF");
    FatJet_neHEF.init(*fReader, "FatJet_neHEF");
    FatJet_neMultiplicity.init(*fReader, "FatJet_neMultiplicity");
    FatJet_particleNetLegacy_QCD.init(*fReader, "FatJet_particleNetLegacy_QCD");
    FatJet_particleNetLegacy_Xbb.init(*fReader, "FatJet_particleNetLegacy_Xbb");
    FatJet_particleNetLegacy_Xcc.init(*fReader, "FatJet_particleNetLegacy_Xcc");
    FatJet_particleNetLegacy_Xqq.init(*fReader, "FatJet_particleNetLegacy_Xqq");
    FatJet_particleNetLegacy_mass.init(*fReader, "FatJet_particleNetLegacy_mass");
    FatJet_particleNetWithMass_H4qvsQCD.init(*fReader, "FatJet_particleNetWithMass_H4qvsQCD");
    FatJet_particleNetWithMass_HbbvsQCD.init(*fReader, "FatJet_particleNetWithMass_HbbvsQCD");
    FatJet_particleNetWithMass_HccvsQCD.init(*fReader, "FatJet_particleNetWithMass_HccvsQCD");
    FatJet_particleNetWithMass_QCD.init(*fReader, "FatJet_particleNetWithMass_QCD");
    FatJet_particleNetWithMass_TvsQCD.init(*fReader, "FatJet_particleNetWithMass_TvsQCD");
    FatJet_particleNetWithMass_WvsQCD.init(*fReader, "FatJet_particleNetWithMass_WvsQCD");
    FatJet_particleNetWithMass_ZvsQCD.init(*fReader, "FatJet_particleNetWithMass_ZvsQCD");
    FatJet_particleNet_QCD.init(*fReader, "FatJet_particleNet_QCD");
    FatJet_particleNet_QCD0HF.init(*fReader, "FatJet_particleNet_QCD0HF");
    FatJet_particleNet_QCD1HF.init(*fReader, "FatJet_particleNet_QCD1HF");
    FatJet_particleNet_QCD2HF.init(*fReader, "FatJet_particleNet_QCD2HF");
    FatJet_particleNet_WVsQCD.init(*fReader, "FatJet_particleNet_WVsQCD");
    FatJet_particleNet_XbbVsQCD.init(*fReader, "FatJet_particleNet_XbbVsQCD");
    FatJet_particleNet_XccVsQCD.init(*fReader, "FatJet_particleNet_XccVsQCD");
    FatJet_particleNet_XggVsQCD.init(*fReader, "FatJet_particleNet_XggVsQCD");
    FatJet_particleNet_XqqVsQCD.init(*fReader, "FatJet_particleNet_XqqVsQCD");
    FatJet_particleNet_XteVsQCD.init(*fReader, "FatJet_particleNet_XteVsQCD");
    FatJet_particleNet_XtmVsQCD.init(*fReader, "FatJet_particleNet_XtmVsQCD");
    FatJet_particleNet_XttVsQCD.init(*fReader, "FatJet_particleNet_XttVsQCD");
    FatJet_particleNet_massCorr.init(*fReader, "FatJet_particleNet_massCorr");
    FatJet_phi.init(*fReader, "FatJet_phi");
    FatJet_pt.init(*fReader, "FatJet_pt");
    FatJet_rawFactor.init(*fReader, "FatJet_rawFactor");
    FatJet_subJetIdx1.init(*fReader, "FatJet_subJetIdx1");
    FatJet_subJetIdx2.init(*fReader, "FatJet_subJetIdx2");
    FatJet_tau1.init(*fReader, "FatJet_tau1");
    FatJet_tau2.init(*fReader, "FatJet_tau2");
    FatJet_tau3.init(*fReader, "FatJet_tau3");
    FatJet_tau4.init(*fReader, "FatJet_tau4");
    nFatJet.init(*fReader, "nFatJet");

    // FatJetPFCand
    FatJetPFCand_jetIdx.init(*fReader, "FatJetPFCand_jetIdx");
    FatJetPFCand_pfCandIdx.init(*fReader, "FatJetPFCand_pfCandIdx");
    nFatJetPFCand.init(*fReader, "nFatJetPFCand");

    // Flag
    Flag_BadChargedCandidateFilter.init(*fReader, "Flag_BadChargedCandidateFilter");
    Flag_BadChargedCandidateSummer16Filter.init(*fReader, "Flag_BadChargedCandidateSummer16Filter");
    Flag_BadPFMuonDzFilter.init(*fReader, "Flag_BadPFMuonDzFilter");
    Flag_BadPFMuonFilter.init(*fReader, "Flag_BadPFMuonFilter");
    Flag_BadPFMuonSummer16Filter.init(*fReader, "Flag_BadPFMuonSummer16Filter");
    Flag_CSCTightHalo2015Filter.init(*fReader, "Flag_CSCTightHalo2015Filter");
    Flag_CSCTightHaloFilter.init(*fReader, "Flag_CSCTightHaloFilter");
    Flag_CSCTightHaloTrkMuUnvetoFilter.init(*fReader, "Flag_CSCTightHaloTrkMuUnvetoFilter");
    Flag_EcalDeadCellBoundaryEnergyFilter.init(*fReader, "Flag_EcalDeadCellBoundaryEnergyFilter");
    Flag_EcalDeadCellTriggerPrimitiveFilter.init(*fReader, "Flag_EcalDeadCellTriggerPrimitiveFilter");
    Flag_HBHENoiseFilter.init(*fReader, "Flag_HBHENoiseFilter");
    Flag_HBHENoiseIsoFilter.init(*fReader, "Flag_HBHENoiseIsoFilter");
    Flag_HcalStripHaloFilter.init(*fReader, "Flag_HcalStripHaloFilter");
    Flag_chargedHadronTrackResolutionFilter.init(*fReader, "Flag_chargedHadronTrackResolutionFilter");
    Flag_ecalBadCalibFilter.init(*fReader, "Flag_ecalBadCalibFilter");
    Flag_ecalLaserCorrFilter.init(*fReader, "Flag_ecalLaserCorrFilter");
    Flag_eeBadScFilter.init(*fReader, "Flag_eeBadScFilter");
    Flag_globalSuperTightHalo2016Filter.init(*fReader, "Flag_globalSuperTightHalo2016Filter");
    Flag_globalTightHalo2016Filter.init(*fReader, "Flag_globalTightHalo2016Filter");
    Flag_goodVertices.init(*fReader, "Flag_goodVertices");
    Flag_hcalLaserEventFilter.init(*fReader, "Flag_hcalLaserEventFilter");
    Flag_hfNoisyHitsFilter.init(*fReader, "Flag_hfNoisyHitsFilter");
    Flag_muonBadTrackFilter.init(*fReader, "Flag_muonBadTrackFilter");
    Flag_trkPOGFilters.init(*fReader, "Flag_trkPOGFilters");
    Flag_trkPOG_logErrorTooManyClusters.init(*fReader, "Flag_trkPOG_logErrorTooManyClusters");
    Flag_trkPOG_manystripclus53X.init(*fReader, "Flag_trkPOG_manystripclus53X");
    Flag_trkPOG_toomanystripclus53X.init(*fReader, "Flag_trkPOG_toomanystripclus53X");

    // FsrPhoton
    FsrPhoton_dROverEt2.init(*fReader, "FsrPhoton_dROverEt2");
    FsrPhoton_electronIdx.init(*fReader, "FsrPhoton_electronIdx");
    FsrPhoton_eta.init(*fReader, "FsrPhoton_eta");
    FsrPhoton_muonIdx.init(*fReader, "FsrPhoton_muonIdx");
    FsrPhoton_phi.init(*fReader, "FsrPhoton_phi");
    FsrPhoton_pt.init(*fReader, "FsrPhoton_pt");
    FsrPhoton_relIso03.init(*fReader, "FsrPhoton_relIso03");
    nFsrPhoton.init(*fReader, "nFsrPhoton");

    // GenDressedLepton
    GenDressedLepton_eta.init(*fReader, "GenDressedLepton_eta");
    GenDressedLepton_hasTauAnc.init(*fReader, "GenDressedLepton_hasTauAnc");
    GenDressedLepton_mass.init(*fReader, "GenDressedLepton_mass");
    GenDressedLepton_pdgId.init(*fReader, "GenDressedLepton_pdgId");
    GenDressedLepton_phi.init(*fReader, "GenDressedLepton_phi");
    GenDressedLepton_pt.init(*fReader, "GenDressedLepton_pt");
    nGenDressedLepton.init(*fReader, "nGenDressedLepton");

    // GenIsolatedPhoton
    GenIsolatedPhoton_eta.init(*fReader, "GenIsolatedPhoton_eta");
    GenIsolatedPhoton_mass.init(*fReader, "GenIsolatedPhoton_mass");
    GenIsolatedPhoton_phi.init(*fReader, "GenIsolatedPhoton_phi");
    GenIsolatedPhoton_pt.init(*fReader, "GenIsolatedPhoton_pt");
    nGenIsolatedPhoton.init(*fReader, "nGenIsolatedPhoton");

    // GenJet
    GenJet_eta.init(*fReader, "GenJet_eta");
    GenJet_hadronFlavour.init(*fReader, "GenJet_hadronFlavour");
    GenJet_mass.init(*fReader, "GenJet_mass");
    GenJet_nBHadrons.init(*fReader, "GenJet_nBHadrons");
    GenJet_nCHadrons.init(*fReader, "GenJet_nCHadrons");
    GenJet_partonFlavour.init(*fReader, "GenJet_partonFlavour");
    GenJet_phi.init(*fReader, "GenJet_phi");
    GenJet_pt.init(*fReader, "GenJet_pt");
    nGenJet.init(*fReader, "nGenJet");

    // GenJetAK8
    GenJetAK8_eta.init(*fReader, "GenJetAK8_eta");
    GenJetAK8_hadronFlavour.init(*fReader, "GenJetAK8_hadronFlavour");
    GenJetAK8_mass.init(*fReader, "GenJetAK8_mass");
    GenJetAK8_nBHadrons.init(*fReader, "GenJetAK8_nBHadrons");
    GenJetAK8_nCHadrons.init(*fReader, "GenJetAK8_nCHadrons");
    GenJetAK8_partonFlavour.init(*fReader, "GenJetAK8_partonFlavour");
    GenJetAK8_phi.init(*fReader, "GenJetAK8_phi");
    GenJetAK8_pt.init(*fReader, "GenJetAK8_pt");
    nGenJetAK8.init(*fReader, "nGenJetAK8");

    // GenMET
    GenMET_phi.init(*fReader, "GenMET_phi");
    GenMET_pt.init(*fReader, "GenMET_pt");

    // GenPart
    GenPart_eta.init(*fReader, "GenPart_eta");
    GenPart_genPartIdxMother.init(*fReader, "GenPart_genPartIdxMother");
    GenPart_iso.init(*fReader, "GenPart_iso");
    GenPart_mass.init(*fReader, "GenPart_mass");
    GenPart_pdgId.init(*fReader, "GenPart_pdgId");
    GenPart_phi.init(*fReader, "GenPart_phi");
    GenPart_pt.init(*fReader, "GenPart_pt");
    GenPart_status.init(*fReader, "GenPart_status");
    GenPart_statusFlags.init(*fReader, "GenPart_statusFlags");
    nGenPart.init(*fReader, "nGenPart");

    // GenVisTau
    GenVisTau_charge.init(*fReader, "GenVisTau_charge");
    GenVisTau_eta.init(*fReader, "GenVisTau_eta");
    GenVisTau_genPartIdxMother.init(*fReader, "GenVisTau_genPartIdxMother");
    GenVisTau_mass.init(*fReader, "GenVisTau_mass");
    GenVisTau_phi.init(*fReader, "GenVisTau_phi");
    GenVisTau_pt.init(*fReader, "GenVisTau_pt");
    GenVisTau_status.init(*fReader, "GenVisTau_status");
    nGenVisTau.init(*fReader, "nGenVisTau");

    // GenVtx
    GenVtx_t0.init(*fReader, "GenVtx_t0");
    GenVtx_x.init(*fReader, "GenVtx_x");
    GenVtx_y.init(*fReader, "GenVtx_y");
    GenVtx_z.init(*fReader, "GenVtx_z");

    // Generator
    Generator_binvar.init(*fReader, "Generator_binvar");
    Generator_id1.init(*fReader, "Generator_id1");
    Generator_id2.init(*fReader, "Generator_id2");
    Generator_scalePDF.init(*fReader, "Generator_scalePDF");
    Generator_weight.init(*fReader, "Generator_weight");
    Generator_x1.init(*fReader, "Generator_x1");
    Generator_x2.init(*fReader, "Generator_x2");
    Generator_xpdf1.init(*fReader, "Generator_xpdf1");
    Generator_xpdf2.init(*fReader, "Generator_xpdf2");

    // IsoTrack
    IsoTrack_charge.init(*fReader, "IsoTrack_charge");
    IsoTrack_dxy.init(*fReader, "IsoTrack_dxy");
    IsoTrack_dz.init(*fReader, "IsoTrack_dz");
    IsoTrack_eta.init(*fReader, "IsoTrack_eta");
    IsoTrack_fromPV.init(*fReader, "IsoTrack_fromPV");
    IsoTrack_isFromLostTrack.init(*fReader, "IsoTrack_isFromLostTrack");
    IsoTrack_isHighPurityTrack.init(*fReader, "IsoTrack_isHighPurityTrack");
    IsoTrack_isPFcand.init(*fReader, "IsoTrack_isPFcand");
    IsoTrack_miniPFRelIso_all.init(*fReader, "IsoTrack_miniPFRelIso_all");
    IsoTrack_miniPFRelIso_chg.init(*fReader, "IsoTrack_miniPFRelIso_chg");
    IsoTrack_pdgId.init(*fReader, "IsoTrack_pdgId");
    IsoTrack_pfRelIso03_all.init(*fReader, "IsoTrack_pfRelIso03_all");
    IsoTrack_pfRelIso03_chg.init(*fReader, "IsoTrack_pfRelIso03_chg");
    IsoTrack_phi.init(*fReader, "IsoTrack_phi");
    IsoTrack_pt.init(*fReader, "IsoTrack_pt");
    nIsoTrack.init(*fReader, "nIsoTrack");

    // Jet
    Jet_PNetRegPtRawCorr.init(*fReader, "Jet_PNetRegPtRawCorr");
    Jet_PNetRegPtRawCorrNeutrino.init(*fReader, "Jet_PNetRegPtRawCorrNeutrino");
    Jet_PNetRegPtRawRes.init(*fReader, "Jet_PNetRegPtRawRes");
    Jet_UParTAK4RegPtRawCorr.init(*fReader, "Jet_UParTAK4RegPtRawCorr");
    Jet_UParTAK4RegPtRawCorrNeutrino.init(*fReader, "Jet_UParTAK4RegPtRawCorrNeutrino");
    Jet_UParTAK4RegPtRawRes.init(*fReader, "Jet_UParTAK4RegPtRawRes");
    Jet_UParTAK4V1RegPtRawCorr.init(*fReader, "Jet_UParTAK4V1RegPtRawCorr");
    Jet_UParTAK4V1RegPtRawCorrNeutrino.init(*fReader, "Jet_UParTAK4V1RegPtRawCorrNeutrino");
    Jet_UParTAK4V1RegPtRawRes.init(*fReader, "Jet_UParTAK4V1RegPtRawRes");
    Jet_area.init(*fReader, "Jet_area");
    Jet_btagDeepFlavB.init(*fReader, "Jet_btagDeepFlavB");
    Jet_btagDeepFlavCvB.init(*fReader, "Jet_btagDeepFlavCvB");
    Jet_btagDeepFlavCvL.init(*fReader, "Jet_btagDeepFlavCvL");
    Jet_btagDeepFlavQG.init(*fReader, "Jet_btagDeepFlavQG");
    Jet_btagPNetB.init(*fReader, "Jet_btagPNetB");
    Jet_btagPNetCvB.init(*fReader, "Jet_btagPNetCvB");
    Jet_btagPNetCvL.init(*fReader, "Jet_btagPNetCvL");
    Jet_btagPNetCvNotB.init(*fReader, "Jet_btagPNetCvNotB");
    Jet_btagPNetQvG.init(*fReader, "Jet_btagPNetQvG");
    Jet_btagPNetTauVJet.init(*fReader, "Jet_btagPNetTauVJet");
    Jet_btagUParTAK4B.init(*fReader, "Jet_btagUParTAK4B");
    Jet_btagUParTAK4CvB.init(*fReader, "Jet_btagUParTAK4CvB");
    Jet_btagUParTAK4CvL.init(*fReader, "Jet_btagUParTAK4CvL");
    Jet_btagUParTAK4CvNotB.init(*fReader, "Jet_btagUParTAK4CvNotB");
    Jet_btagUParTAK4Ele.init(*fReader, "Jet_btagUParTAK4Ele");
    Jet_btagUParTAK4Mu.init(*fReader, "Jet_btagUParTAK4Mu");
    Jet_btagUParTAK4QvG.init(*fReader, "Jet_btagUParTAK4QvG");
    Jet_btagUParTAK4SvCB.init(*fReader, "Jet_btagUParTAK4SvCB");
    Jet_btagUParTAK4SvUDG.init(*fReader, "Jet_btagUParTAK4SvUDG");
    Jet_btagUParTAK4TauVJet.init(*fReader, "Jet_btagUParTAK4TauVJet");
    Jet_btagUParTAK4UDG.init(*fReader, "Jet_btagUParTAK4UDG");
    Jet_btagUParTAK4probb.init(*fReader, "Jet_btagUParTAK4probb");
    Jet_btagUParTAK4probbb.init(*fReader, "Jet_btagUParTAK4probbb");
    Jet_chEmEF.init(*fReader, "Jet_chEmEF");
    Jet_chHEF.init(*fReader, "Jet_chHEF");
    Jet_chMultiplicity.init(*fReader, "Jet_chMultiplicity");
    Jet_electronIdx1.init(*fReader, "Jet_electronIdx1");
    Jet_electronIdx2.init(*fReader, "Jet_electronIdx2");
    Jet_eta.init(*fReader, "Jet_eta");
    Jet_genJetIdx.init(*fReader, "Jet_genJetIdx");
    Jet_hadronFlavour.init(*fReader, "Jet_hadronFlavour");
    Jet_hfEmEF.init(*fReader, "Jet_hfEmEF");
    Jet_hfHEF.init(*fReader, "Jet_hfHEF");
    Jet_hfadjacentEtaStripsSize.init(*fReader, "Jet_hfadjacentEtaStripsSize");
    Jet_hfcentralEtaStripSize.init(*fReader, "Jet_hfcentralEtaStripSize");
    Jet_hfsigmaEtaEta.init(*fReader, "Jet_hfsigmaEtaEta");
    Jet_hfsigmaPhiPhi.init(*fReader, "Jet_hfsigmaPhiPhi");
    Jet_mass.init(*fReader, "Jet_mass");
    Jet_muEF.init(*fReader, "Jet_muEF");
    Jet_muonIdx1.init(*fReader, "Jet_muonIdx1");
    Jet_muonIdx2.init(*fReader, "Jet_muonIdx2");
    Jet_muonSubtrDeltaEta.init(*fReader, "Jet_muonSubtrDeltaEta");
    Jet_muonSubtrDeltaPhi.init(*fReader, "Jet_muonSubtrDeltaPhi");
    Jet_muonSubtrFactor.init(*fReader, "Jet_muonSubtrFactor");
    Jet_nConstituents.init(*fReader, "Jet_nConstituents");
    Jet_nElectrons.init(*fReader, "Jet_nElectrons");
    Jet_nMuons.init(*fReader, "Jet_nMuons");
    Jet_nSVs.init(*fReader, "Jet_nSVs");
    Jet_neEmEF.init(*fReader, "Jet_neEmEF");
    Jet_neHEF.init(*fReader, "Jet_neHEF");
    Jet_neMultiplicity.init(*fReader, "Jet_neMultiplicity");
    Jet_partonFlavour.init(*fReader, "Jet_partonFlavour");
    Jet_phi.init(*fReader, "Jet_phi");
    Jet_pt.init(*fReader, "Jet_pt");
    Jet_puIdDisc.init(*fReader, "Jet_puIdDisc");
    Jet_rawFactor.init(*fReader, "Jet_rawFactor");
    Jet_svIdx1.init(*fReader, "Jet_svIdx1");
    Jet_svIdx2.init(*fReader, "Jet_svIdx2");
    nJet.init(*fReader, "nJet");

    // LHE
    LHE_AlphaS.init(*fReader, "LHE_AlphaS");
    LHE_HT.init(*fReader, "LHE_HT");
    LHE_HTIncoming.init(*fReader, "LHE_HTIncoming");
    LHE_Nb.init(*fReader, "LHE_Nb");
    LHE_Nc.init(*fReader, "LHE_Nc");
    LHE_Nglu.init(*fReader, "LHE_Nglu");
    LHE_Njets.init(*fReader, "LHE_Njets");
    LHE_NpLO.init(*fReader, "LHE_NpLO");
    LHE_NpNLO.init(*fReader, "LHE_NpNLO");
    LHE_Nuds.init(*fReader, "LHE_Nuds");
    LHE_Vpt.init(*fReader, "LHE_Vpt");

    // LHEPart
    LHEPart_eta.init(*fReader, "LHEPart_eta");
    LHEPart_firstMotherIdx.init(*fReader, "LHEPart_firstMotherIdx");
    LHEPart_incomingpz.init(*fReader, "LHEPart_incomingpz");
    LHEPart_lastMotherIdx.init(*fReader, "LHEPart_lastMotherIdx");
    LHEPart_mass.init(*fReader, "LHEPart_mass");
    LHEPart_pdgId.init(*fReader, "LHEPart_pdgId");
    LHEPart_phi.init(*fReader, "LHEPart_phi");
    LHEPart_pt.init(*fReader, "LHEPart_pt");
    LHEPart_spin.init(*fReader, "LHEPart_spin");
    LHEPart_status.init(*fReader, "LHEPart_status");
    nLHEPart.init(*fReader, "nLHEPart");

    // LHEPdfWeight
    LHEPdfWeight.init(*fReader, "LHEPdfWeight");
    nLHEPdfWeight.init(*fReader, "nLHEPdfWeight");

    // LHEReweightingWeight
    LHEReweightingWeight.init(*fReader, "LHEReweightingWeight");
    nLHEReweightingWeight.init(*fReader, "nLHEReweightingWeight");

    // LHEScaleWeight
    LHEScaleWeight.init(*fReader, "LHEScaleWeight");
    nLHEScaleWeight.init(*fReader, "nLHEScaleWeight");

    // LHEWeight
    LHEWeight_originalXWGTUP.init(*fReader, "LHEWeight_originalXWGTUP");

    // LowPtElectron
    LowPtElectron_ID.init(*fReader, "LowPtElectron_ID");
    LowPtElectron_charge.init(*fReader, "LowPtElectron_charge");
    LowPtElectron_convVeto.init(*fReader, "LowPtElectron_convVeto");
    LowPtElectron_convVtxRadius.init(*fReader, "LowPtElectron_convVtxRadius");
    LowPtElectron_convWP.init(*fReader, "LowPtElectron_convWP");
    LowPtElectron_deltaEtaSC.init(*fReader, "LowPtElectron_deltaEtaSC");
    LowPtElectron_dxy.init(*fReader, "LowPtElectron_dxy");
    LowPtElectron_dxyErr.init(*fReader, "LowPtElectron_dxyErr");
    LowPtElectron_dz.init(*fReader, "LowPtElectron_dz");
    LowPtElectron_dzErr.init(*fReader, "LowPtElectron_dzErr");
    LowPtElectron_eInvMinusPInv.init(*fReader, "LowPtElectron_eInvMinusPInv");
    LowPtElectron_electronIdx.init(*fReader, "LowPtElectron_electronIdx");
    LowPtElectron_energyErr.init(*fReader, "LowPtElectron_energyErr");
    LowPtElectron_eta.init(*fReader, "LowPtElectron_eta");
    LowPtElectron_genPartFlav.init(*fReader, "LowPtElectron_genPartFlav");
    LowPtElectron_genPartIdx.init(*fReader, "LowPtElectron_genPartIdx");
    LowPtElectron_hoe.init(*fReader, "LowPtElectron_hoe");
    LowPtElectron_lostHits.init(*fReader, "LowPtElectron_lostHits");
    LowPtElectron_mass.init(*fReader, "LowPtElectron_mass");
    LowPtElectron_miniPFRelIso_all.init(*fReader, "LowPtElectron_miniPFRelIso_all");
    LowPtElectron_miniPFRelIso_chg.init(*fReader, "LowPtElectron_miniPFRelIso_chg");
    LowPtElectron_pdgId.init(*fReader, "LowPtElectron_pdgId");
    LowPtElectron_phi.init(*fReader, "LowPtElectron_phi");
    LowPtElectron_photonIdx.init(*fReader, "LowPtElectron_photonIdx");
    LowPtElectron_pt.init(*fReader, "LowPtElectron_pt");
    LowPtElectron_ptbiased.init(*fReader, "LowPtElectron_ptbiased");
    LowPtElectron_r9.init(*fReader, "LowPtElectron_r9");
    LowPtElectron_scEtOverPt.init(*fReader, "LowPtElectron_scEtOverPt");
    LowPtElectron_sieie.init(*fReader, "LowPtElectron_sieie");
    LowPtElectron_unbiased.init(*fReader, "LowPtElectron_unbiased");
    nLowPtElectron.init(*fReader, "nLowPtElectron");

    // Muon
    Muon_IPx.init(*fReader, "Muon_IPx");
    Muon_IPy.init(*fReader, "Muon_IPy");
    Muon_IPz.init(*fReader, "Muon_IPz");
    Muon_VXBS_Cov00.init(*fReader, "Muon_VXBS_Cov00");
    Muon_VXBS_Cov03.init(*fReader, "Muon_VXBS_Cov03");
    Muon_VXBS_Cov33.init(*fReader, "Muon_VXBS_Cov33");
    Muon_bestTrackType.init(*fReader, "Muon_bestTrackType");
    Muon_bsConstrainedChi2.init(*fReader, "Muon_bsConstrainedChi2");
    Muon_bsConstrainedPt.init(*fReader, "Muon_bsConstrainedPt");
    Muon_bsConstrainedPtErr.init(*fReader, "Muon_bsConstrainedPtErr");
    Muon_charge.init(*fReader, "Muon_charge");
    Muon_dxy.init(*fReader, "Muon_dxy");
    Muon_dxyErr.init(*fReader, "Muon_dxyErr");
    Muon_dxybs.init(*fReader, "Muon_dxybs");
    Muon_dxybsErr.init(*fReader, "Muon_dxybsErr");
    Muon_dz.init(*fReader, "Muon_dz");
    Muon_dzErr.init(*fReader, "Muon_dzErr");
    Muon_eta.init(*fReader, "Muon_eta");
    Muon_fsrPhotonIdx.init(*fReader, "Muon_fsrPhotonIdx");
    Muon_genPartFlav.init(*fReader, "Muon_genPartFlav");
    Muon_genPartIdx.init(*fReader, "Muon_genPartIdx");
    Muon_highPtId.init(*fReader, "Muon_highPtId");
    Muon_highPurity.init(*fReader, "Muon_highPurity");
    Muon_inTimeMuon.init(*fReader, "Muon_inTimeMuon");
    Muon_ip3d.init(*fReader, "Muon_ip3d");
    Muon_ipLengthSig.init(*fReader, "Muon_ipLengthSig");
    Muon_isGlobal.init(*fReader, "Muon_isGlobal");
    Muon_isPFcand.init(*fReader, "Muon_isPFcand");
    Muon_isStandalone.init(*fReader, "Muon_isStandalone");
    Muon_isTracker.init(*fReader, "Muon_isTracker");
    Muon_jetDF.init(*fReader, "Muon_jetDF");
    Muon_jetIdx.init(*fReader, "Muon_jetIdx");
    Muon_jetNDauCharged.init(*fReader, "Muon_jetNDauCharged");
    Muon_jetPtRelv2.init(*fReader, "Muon_jetPtRelv2");
    Muon_jetRelIso.init(*fReader, "Muon_jetRelIso");
    Muon_looseId.init(*fReader, "Muon_looseId");
    Muon_mass.init(*fReader, "Muon_mass");
    Muon_mediumId.init(*fReader, "Muon_mediumId");
    Muon_mediumPromptId.init(*fReader, "Muon_mediumPromptId");
    Muon_miniIsoId.init(*fReader, "Muon_miniIsoId");
    Muon_miniPFRelIso_all.init(*fReader, "Muon_miniPFRelIso_all");
    Muon_miniPFRelIso_chg.init(*fReader, "Muon_miniPFRelIso_chg");
    Muon_multiIsoId.init(*fReader, "Muon_multiIsoId");
    Muon_mvaLowPt.init(*fReader, "Muon_mvaLowPt");
    Muon_mvaMuID.init(*fReader, "Muon_mvaMuID");
    Muon_mvaMuID_WP.init(*fReader, "Muon_mvaMuID_WP");
    Muon_nStations.init(*fReader, "Muon_nStations");
    Muon_nTrackerLayers.init(*fReader, "Muon_nTrackerLayers");
    Muon_pdgId.init(*fReader, "Muon_pdgId");
    Muon_pfIsoId.init(*fReader, "Muon_pfIsoId");
    Muon_pfRelIso03_all.init(*fReader, "Muon_pfRelIso03_all");
    Muon_pfRelIso03_chg.init(*fReader, "Muon_pfRelIso03_chg");
    Muon_pfRelIso04_all.init(*fReader, "Muon_pfRelIso04_all");
    Muon_phi.init(*fReader, "Muon_phi");
    Muon_pnScore_heavy.init(*fReader, "Muon_pnScore_heavy");
    Muon_pnScore_light.init(*fReader, "Muon_pnScore_light");
    Muon_pnScore_prompt.init(*fReader, "Muon_pnScore_prompt");
    Muon_pnScore_tau.init(*fReader, "Muon_pnScore_tau");
    Muon_promptMVA.init(*fReader, "Muon_promptMVA");
    Muon_pt.init(*fReader, "Muon_pt");
    Muon_ptErr.init(*fReader, "Muon_ptErr");
    Muon_puppiIsoId.init(*fReader, "Muon_puppiIsoId");
    Muon_segmentComp.init(*fReader, "Muon_segmentComp");
    Muon_sip3d.init(*fReader, "Muon_sip3d");
    Muon_softId.init(*fReader, "Muon_softId");
    Muon_softMva.init(*fReader, "Muon_softMva");
    Muon_softMvaId.init(*fReader, "Muon_softMvaId");
    Muon_softMvaRun3.init(*fReader, "Muon_softMvaRun3");
    Muon_svIdx.init(*fReader, "Muon_svIdx");
    Muon_tightCharge.init(*fReader, "Muon_tightCharge");
    Muon_tightId.init(*fReader, "Muon_tightId");
    Muon_tkIsoId.init(*fReader, "Muon_tkIsoId");
    Muon_tkRelIso.init(*fReader, "Muon_tkRelIso");
    Muon_triggerIdLoose.init(*fReader, "Muon_triggerIdLoose");
    Muon_tuneP_charge.init(*fReader, "Muon_tuneP_charge");
    Muon_tuneP_pterr.init(*fReader, "Muon_tuneP_pterr");
    Muon_tunepRelPt.init(*fReader, "Muon_tunepRelPt");
    nMuon.init(*fReader, "nMuon");

    // OtherPV
    OtherPV_score.init(*fReader, "OtherPV_score");
    OtherPV_z.init(*fReader, "OtherPV_z");
    nOtherPV.init(*fReader, "nOtherPV");

    // PFCand
    PFCand_eta.init(*fReader, "PFCand_eta");
    PFCand_mass.init(*fReader, "PFCand_mass");
    PFCand_pdgId.init(*fReader, "PFCand_pdgId");
    PFCand_phi.init(*fReader, "PFCand_phi");
    PFCand_pt.init(*fReader, "PFCand_pt");
    nPFCand.init(*fReader, "nPFCand");

    // PFMET
    PFMET_covXX.init(*fReader, "PFMET_covXX");
    PFMET_covXY.init(*fReader, "PFMET_covXY");
    PFMET_covYY.init(*fReader, "PFMET_covYY");
    PFMET_phi.init(*fReader, "PFMET_phi");
    PFMET_phiUnclusteredDown.init(*fReader, "PFMET_phiUnclusteredDown");
    PFMET_phiUnclusteredUp.init(*fReader, "PFMET_phiUnclusteredUp");
    PFMET_pt.init(*fReader, "PFMET_pt");
    PFMET_ptUnclusteredDown.init(*fReader, "PFMET_ptUnclusteredDown");
    PFMET_ptUnclusteredUp.init(*fReader, "PFMET_ptUnclusteredUp");
    PFMET_significance.init(*fReader, "PFMET_significance");
    PFMET_sumEt.init(*fReader, "PFMET_sumEt");
    PFMET_sumPtUnclustered.init(*fReader, "PFMET_sumPtUnclustered");

    // PSWeight
    PSWeight.init(*fReader, "PSWeight");
    nPSWeight.init(*fReader, "nPSWeight");

    // PV
    PV_chi2.init(*fReader, "PV_chi2");
    PV_ndof.init(*fReader, "PV_ndof");
    PV_npvs.init(*fReader, "PV_npvs");
    PV_npvsGood.init(*fReader, "PV_npvsGood");
    PV_score.init(*fReader, "PV_score");
    PV_sumpt2.init(*fReader, "PV_sumpt2");
    PV_sumpx.init(*fReader, "PV_sumpx");
    PV_sumpy.init(*fReader, "PV_sumpy");
    PV_x.init(*fReader, "PV_x");
    PV_y.init(*fReader, "PV_y");
    PV_z.init(*fReader, "PV_z");

    // Photon
    Photon_cutBased.init(*fReader, "Photon_cutBased");
    Photon_ecalPFClusterIso.init(*fReader, "Photon_ecalPFClusterIso");
    Photon_electronIdx.init(*fReader, "Photon_electronIdx");
    Photon_electronVeto.init(*fReader, "Photon_electronVeto");
    Photon_energyErr.init(*fReader, "Photon_energyErr");
    Photon_energyRaw.init(*fReader, "Photon_energyRaw");
    Photon_esEffSigmaRR.init(*fReader, "Photon_esEffSigmaRR");
    Photon_esEnergyOverRawE.init(*fReader, "Photon_esEnergyOverRawE");
    Photon_eta.init(*fReader, "Photon_eta");
    Photon_etaWidth.init(*fReader, "Photon_etaWidth");
    Photon_genPartFlav.init(*fReader, "Photon_genPartFlav");
    Photon_genPartIdx.init(*fReader, "Photon_genPartIdx");
    Photon_haloTaggerMVAVal.init(*fReader, "Photon_haloTaggerMVAVal");
    Photon_hasConversionTracks.init(*fReader, "Photon_hasConversionTracks");
    Photon_hcalPFClusterIso.init(*fReader, "Photon_hcalPFClusterIso");
    Photon_hoe.init(*fReader, "Photon_hoe");
    Photon_hoe_PUcorr.init(*fReader, "Photon_hoe_PUcorr");
    Photon_hoe_Tower.init(*fReader, "Photon_hoe_Tower");
    Photon_isScEtaEB.init(*fReader, "Photon_isScEtaEB");
    Photon_isScEtaEE.init(*fReader, "Photon_isScEtaEE");
    Photon_jetIdx.init(*fReader, "Photon_jetIdx");
    Photon_mvaID.init(*fReader, "Photon_mvaID");
    Photon_mvaID_WP80.init(*fReader, "Photon_mvaID_WP80");
    Photon_mvaID_WP90.init(*fReader, "Photon_mvaID_WP90");
    Photon_pfChargedIso.init(*fReader, "Photon_pfChargedIso");
    Photon_pfChargedIsoPFPV.init(*fReader, "Photon_pfChargedIsoPFPV");
    Photon_pfChargedIsoWorstVtx.init(*fReader, "Photon_pfChargedIsoWorstVtx");
    Photon_pfPhoIso03.init(*fReader, "Photon_pfPhoIso03");
    Photon_pfRelIso03_all_quadratic.init(*fReader, "Photon_pfRelIso03_all_quadratic");
    Photon_pfRelIso03_chg_quadratic.init(*fReader, "Photon_pfRelIso03_chg_quadratic");
    Photon_phi.init(*fReader, "Photon_phi");
    Photon_phiWidth.init(*fReader, "Photon_phiWidth");
    Photon_pixelSeed.init(*fReader, "Photon_pixelSeed");
    Photon_pt.init(*fReader, "Photon_pt");
    Photon_r9.init(*fReader, "Photon_r9");
    Photon_s4.init(*fReader, "Photon_s4");
    Photon_seedGain.init(*fReader, "Photon_seedGain");
    Photon_seediEtaOriX.init(*fReader, "Photon_seediEtaOriX");
    Photon_seediPhiOriY.init(*fReader, "Photon_seediPhiOriY");
    Photon_sieie.init(*fReader, "Photon_sieie");
    Photon_sieip.init(*fReader, "Photon_sieip");
    Photon_sipip.init(*fReader, "Photon_sipip");
    Photon_superclusterEta.init(*fReader, "Photon_superclusterEta");
    Photon_trkSumPtHollowConeDR03.init(*fReader, "Photon_trkSumPtHollowConeDR03");
    Photon_trkSumPtSolidConeDR04.init(*fReader, "Photon_trkSumPtSolidConeDR04");
    Photon_vidNestedWPBitmap.init(*fReader, "Photon_vidNestedWPBitmap");
    Photon_x_calo.init(*fReader, "Photon_x_calo");
    Photon_y_calo.init(*fReader, "Photon_y_calo");
    Photon_z_calo.init(*fReader, "Photon_z_calo");
    nPhoton.init(*fReader, "nPhoton");

    // Pileup
    Pileup_gpudensity.init(*fReader, "Pileup_gpudensity");
    Pileup_nPU.init(*fReader, "Pileup_nPU");
    Pileup_nTrueInt.init(*fReader, "Pileup_nTrueInt");
    Pileup_pthatmax.init(*fReader, "Pileup_pthatmax");
    Pileup_pudensity.init(*fReader, "Pileup_pudensity");
    Pileup_sumEOOT.init(*fReader, "Pileup_sumEOOT");
    Pileup_sumLOOT.init(*fReader, "Pileup_sumLOOT");

    // PuppiMET
    PuppiMET_covXX.init(*fReader, "PuppiMET_covXX");
    PuppiMET_covXY.init(*fReader, "PuppiMET_covXY");
    PuppiMET_covYY.init(*fReader, "PuppiMET_covYY");
    PuppiMET_phi.init(*fReader, "PuppiMET_phi");
    PuppiMET_phiUnclusteredDown.init(*fReader, "PuppiMET_phiUnclusteredDown");
    PuppiMET_phiUnclusteredUp.init(*fReader, "PuppiMET_phiUnclusteredUp");
    PuppiMET_pt.init(*fReader, "PuppiMET_pt");
    PuppiMET_ptUnclusteredDown.init(*fReader, "PuppiMET_ptUnclusteredDown");
    PuppiMET_ptUnclusteredUp.init(*fReader, "PuppiMET_ptUnclusteredUp");
    PuppiMET_significance.init(*fReader, "PuppiMET_significance");
    PuppiMET_sumEt.init(*fReader, "PuppiMET_sumEt");
    PuppiMET_sumPtUnclustered.init(*fReader, "PuppiMET_sumPtUnclustered");

    // Rho
    Rho_fixedGridRhoAll.init(*fReader, "Rho_fixedGridRhoAll");
    Rho_fixedGridRhoFastjetAll.init(*fReader, "Rho_fixedGridRhoFastjetAll");
    Rho_fixedGridRhoFastjetCentral.init(*fReader, "Rho_fixedGridRhoFastjetCentral");
    Rho_fixedGridRhoFastjetCentralCalo.init(*fReader, "Rho_fixedGridRhoFastjetCentralCalo");
    Rho_fixedGridRhoFastjetCentralChargedPileUp.init(*fReader, "Rho_fixedGridRhoFastjetCentralChargedPileUp");
    Rho_fixedGridRhoFastjetCentralNeutral.init(*fReader, "Rho_fixedGridRhoFastjetCentralNeutral");

    // SV
    SV_charge.init(*fReader, "SV_charge");
    SV_chi2.init(*fReader, "SV_chi2");
    SV_dlen.init(*fReader, "SV_dlen");
    SV_dlenSig.init(*fReader, "SV_dlenSig");
    SV_dxy.init(*fReader, "SV_dxy");
    SV_dxySig.init(*fReader, "SV_dxySig");
    SV_eta.init(*fReader, "SV_eta");
    SV_mass.init(*fReader, "SV_mass");
    SV_ndof.init(*fReader, "SV_ndof");
    SV_ntracks.init(*fReader, "SV_ntracks");
    SV_pAngle.init(*fReader, "SV_pAngle");
    SV_phi.init(*fReader, "SV_phi");
    SV_pt.init(*fReader, "SV_pt");
    SV_x.init(*fReader, "SV_x");
    SV_y.init(*fReader, "SV_y");
    SV_z.init(*fReader, "SV_z");
    nSV.init(*fReader, "nSV");

    // SubGenJetAK8
    SubGenJetAK8_eta.init(*fReader, "SubGenJetAK8_eta");
    SubGenJetAK8_mass.init(*fReader, "SubGenJetAK8_mass");
    SubGenJetAK8_phi.init(*fReader, "SubGenJetAK8_phi");
    SubGenJetAK8_pt.init(*fReader, "SubGenJetAK8_pt");
    nSubGenJetAK8.init(*fReader, "nSubGenJetAK8");

    // SubJet
    SubJet_UParTAK4RegPtRawCorr.init(*fReader, "SubJet_UParTAK4RegPtRawCorr");
    SubJet_UParTAK4RegPtRawCorrNeutrino.init(*fReader, "SubJet_UParTAK4RegPtRawCorrNeutrino");
    SubJet_UParTAK4RegPtRawRes.init(*fReader, "SubJet_UParTAK4RegPtRawRes");
    SubJet_UParTAK4V1RegPtRawCorr.init(*fReader, "SubJet_UParTAK4V1RegPtRawCorr");
    SubJet_UParTAK4V1RegPtRawCorrNeutrino.init(*fReader, "SubJet_UParTAK4V1RegPtRawCorrNeutrino");
    SubJet_UParTAK4V1RegPtRawRes.init(*fReader, "SubJet_UParTAK4V1RegPtRawRes");
    SubJet_area.init(*fReader, "SubJet_area");
    SubJet_btagDeepFlavB.init(*fReader, "SubJet_btagDeepFlavB");
    SubJet_btagUParTAK4B.init(*fReader, "SubJet_btagUParTAK4B");
    SubJet_eta.init(*fReader, "SubJet_eta");
    SubJet_hadronFlavour.init(*fReader, "SubJet_hadronFlavour");
    SubJet_mass.init(*fReader, "SubJet_mass");
    SubJet_n2b1.init(*fReader, "SubJet_n2b1");
    SubJet_n3b1.init(*fReader, "SubJet_n3b1");
    SubJet_nBHadrons.init(*fReader, "SubJet_nBHadrons");
    SubJet_nCHadrons.init(*fReader, "SubJet_nCHadrons");
    SubJet_phi.init(*fReader, "SubJet_phi");
    SubJet_pt.init(*fReader, "SubJet_pt");
    SubJet_rawFactor.init(*fReader, "SubJet_rawFactor");
    SubJet_subGenJetAK8Idx.init(*fReader, "SubJet_subGenJetAK8Idx");
    SubJet_tau1.init(*fReader, "SubJet_tau1");
    SubJet_tau2.init(*fReader, "SubJet_tau2");
    SubJet_tau3.init(*fReader, "SubJet_tau3");
    SubJet_tau4.init(*fReader, "SubJet_tau4");
    nSubJet.init(*fReader, "nSubJet");

    // Tau
    Tau_IPx.init(*fReader, "Tau_IPx");
    Tau_IPy.init(*fReader, "Tau_IPy");
    Tau_IPz.init(*fReader, "Tau_IPz");
    Tau_charge.init(*fReader, "Tau_charge");
    Tau_chargedIso.init(*fReader, "Tau_chargedIso");
    Tau_decayMode.init(*fReader, "Tau_decayMode");
    Tau_decayModePNet.init(*fReader, "Tau_decayModePNet");
    Tau_decayModeUParT.init(*fReader, "Tau_decayModeUParT");
    Tau_dxy.init(*fReader, "Tau_dxy");
    Tau_dz.init(*fReader, "Tau_dz");
    Tau_eleIdx.init(*fReader, "Tau_eleIdx");
    Tau_eta.init(*fReader, "Tau_eta");
    Tau_genPartFlav.init(*fReader, "Tau_genPartFlav");
    Tau_genPartIdx.init(*fReader, "Tau_genPartIdx");
    Tau_hasRefitSV.init(*fReader, "Tau_hasRefitSV");
    Tau_idAntiEleDeadECal.init(*fReader, "Tau_idAntiEleDeadECal");
    Tau_idAntiMu.init(*fReader, "Tau_idAntiMu");
    Tau_idDecayModeNewDMs.init(*fReader, "Tau_idDecayModeNewDMs");
    Tau_idDecayModeOldDMs.init(*fReader, "Tau_idDecayModeOldDMs");
    Tau_idDeepTau2018v2p5VSe.init(*fReader, "Tau_idDeepTau2018v2p5VSe");
    Tau_idDeepTau2018v2p5VSjet.init(*fReader, "Tau_idDeepTau2018v2p5VSjet");
    Tau_idDeepTau2018v2p5VSmu.init(*fReader, "Tau_idDeepTau2018v2p5VSmu");
    Tau_ipLengthSig.init(*fReader, "Tau_ipLengthSig");
    Tau_jetIdx.init(*fReader, "Tau_jetIdx");
    Tau_leadTkDeltaEta.init(*fReader, "Tau_leadTkDeltaEta");
    Tau_leadTkDeltaPhi.init(*fReader, "Tau_leadTkDeltaPhi");
    Tau_leadTkPtOverTauPt.init(*fReader, "Tau_leadTkPtOverTauPt");
    Tau_mass.init(*fReader, "Tau_mass");
    Tau_muIdx.init(*fReader, "Tau_muIdx");
    Tau_nSVs.init(*fReader, "Tau_nSVs");
    Tau_neutralIso.init(*fReader, "Tau_neutralIso");
    Tau_phi.init(*fReader, "Tau_phi");
    Tau_photonsOutsideSignalCone.init(*fReader, "Tau_photonsOutsideSignalCone");
    Tau_probDM0PNet.init(*fReader, "Tau_probDM0PNet");
    Tau_probDM0UParT.init(*fReader, "Tau_probDM0UParT");
    Tau_probDM10PNet.init(*fReader, "Tau_probDM10PNet");
    Tau_probDM10UParT.init(*fReader, "Tau_probDM10UParT");
    Tau_probDM11PNet.init(*fReader, "Tau_probDM11PNet");
    Tau_probDM11UParT.init(*fReader, "Tau_probDM11UParT");
    Tau_probDM1PNet.init(*fReader, "Tau_probDM1PNet");
    Tau_probDM1UParT.init(*fReader, "Tau_probDM1UParT");
    Tau_probDM2PNet.init(*fReader, "Tau_probDM2PNet");
    Tau_probDM2UParT.init(*fReader, "Tau_probDM2UParT");
    Tau_pt.init(*fReader, "Tau_pt");
    Tau_ptCorrPNet.init(*fReader, "Tau_ptCorrPNet");
    Tau_ptCorrUParT.init(*fReader, "Tau_ptCorrUParT");
    Tau_puCorr.init(*fReader, "Tau_puCorr");
    Tau_qConfPNet.init(*fReader, "Tau_qConfPNet");
    Tau_qConfUParT.init(*fReader, "Tau_qConfUParT");
    Tau_rawDeepTau2018v2p5VSe.init(*fReader, "Tau_rawDeepTau2018v2p5VSe");
    Tau_rawDeepTau2018v2p5VSjet.init(*fReader, "Tau_rawDeepTau2018v2p5VSjet");
    Tau_rawDeepTau2018v2p5VSmu.init(*fReader, "Tau_rawDeepTau2018v2p5VSmu");
    Tau_rawIso.init(*fReader, "Tau_rawIso");
    Tau_rawIsodR03.init(*fReader, "Tau_rawIsodR03");
    Tau_rawPNetVSe.init(*fReader, "Tau_rawPNetVSe");
    Tau_rawPNetVSjet.init(*fReader, "Tau_rawPNetVSjet");
    Tau_rawPNetVSmu.init(*fReader, "Tau_rawPNetVSmu");
    Tau_rawUParTVSe.init(*fReader, "Tau_rawUParTVSe");
    Tau_rawUParTVSjet.init(*fReader, "Tau_rawUParTVSjet");
    Tau_rawUParTVSmu.init(*fReader, "Tau_rawUParTVSmu");
    Tau_refitSVchi2.init(*fReader, "Tau_refitSVchi2");
    Tau_refitSVcov00.init(*fReader, "Tau_refitSVcov00");
    Tau_refitSVcov10.init(*fReader, "Tau_refitSVcov10");
    Tau_refitSVcov11.init(*fReader, "Tau_refitSVcov11");
    Tau_refitSVcov20.init(*fReader, "Tau_refitSVcov20");
    Tau_refitSVcov21.init(*fReader, "Tau_refitSVcov21");
    Tau_refitSVcov22.init(*fReader, "Tau_refitSVcov22");
    Tau_refitSVx.init(*fReader, "Tau_refitSVx");
    Tau_refitSVy.init(*fReader, "Tau_refitSVy");
    Tau_refitSVz.init(*fReader, "Tau_refitSVz");
    Tau_svIdx1.init(*fReader, "Tau_svIdx1");
    Tau_svIdx2.init(*fReader, "Tau_svIdx2");
    nTau.init(*fReader, "nTau");

    // TauProd
    TauProd_eta.init(*fReader, "TauProd_eta");
    TauProd_pdgId.init(*fReader, "TauProd_pdgId");
    TauProd_phi.init(*fReader, "TauProd_phi");
    TauProd_pt.init(*fReader, "TauProd_pt");
    TauProd_tauIdx.init(*fReader, "TauProd_tauIdx");
    nTauProd.init(*fReader, "nTauProd");

    // TauSpinner
    TauSpinner_weight_cp_0.init(*fReader, "TauSpinner_weight_cp_0");
    TauSpinner_weight_cp_0_alt.init(*fReader, "TauSpinner_weight_cp_0_alt");
    TauSpinner_weight_cp_0p25.init(*fReader, "TauSpinner_weight_cp_0p25");
    TauSpinner_weight_cp_0p25_alt.init(*fReader, "TauSpinner_weight_cp_0p25_alt");
    TauSpinner_weight_cp_0p375.init(*fReader, "TauSpinner_weight_cp_0p375");
    TauSpinner_weight_cp_0p375_alt.init(*fReader, "TauSpinner_weight_cp_0p375_alt");
    TauSpinner_weight_cp_0p5.init(*fReader, "TauSpinner_weight_cp_0p5");
    TauSpinner_weight_cp_0p5_alt.init(*fReader, "TauSpinner_weight_cp_0p5_alt");
    TauSpinner_weight_cp_minus0p25.init(*fReader, "TauSpinner_weight_cp_minus0p25");
    TauSpinner_weight_cp_minus0p25_alt.init(*fReader, "TauSpinner_weight_cp_minus0p25_alt");

    // TrackGenJetAK4
    TrackGenJetAK4_eta.init(*fReader, "TrackGenJetAK4_eta");
    TrackGenJetAK4_phi.init(*fReader, "TrackGenJetAK4_phi");
    TrackGenJetAK4_pt.init(*fReader, "TrackGenJetAK4_pt");
    nTrackGenJetAK4.init(*fReader, "nTrackGenJetAK4");

    // TrigObj
    TrigObj_eta.init(*fReader, "TrigObj_eta");
    TrigObj_filterBits.init(*fReader, "TrigObj_filterBits");
    TrigObj_id.init(*fReader, "TrigObj_id");
    TrigObj_l1charge.init(*fReader, "TrigObj_l1charge");
    TrigObj_l1iso.init(*fReader, "TrigObj_l1iso");
    TrigObj_l1pt.init(*fReader, "TrigObj_l1pt");
    TrigObj_l1pt_2.init(*fReader, "TrigObj_l1pt_2");
    TrigObj_l2pt.init(*fReader, "TrigObj_l2pt");
    TrigObj_phi.init(*fReader, "TrigObj_phi");
    TrigObj_pt.init(*fReader, "TrigObj_pt");
    nTrigObj.init(*fReader, "nTrigObj");

    // TrkMET
    TrkMET_phi.init(*fReader, "TrkMET_phi");
    TrkMET_pt.init(*fReader, "TrkMET_pt");
    TrkMET_sumEt.init(*fReader, "TrkMET_sumEt");

    // boostedTau
    boostedTau_charge.init(*fReader, "boostedTau_charge");
    boostedTau_chargedIso.init(*fReader, "boostedTau_chargedIso");
    boostedTau_decayMode.init(*fReader, "boostedTau_decayMode");
    boostedTau_eta.init(*fReader, "boostedTau_eta");
    boostedTau_genPartFlav.init(*fReader, "boostedTau_genPartFlav");
    boostedTau_genPartIdx.init(*fReader, "boostedTau_genPartIdx");
    boostedTau_idAntiEle2018.init(*fReader, "boostedTau_idAntiEle2018");
    boostedTau_idAntiMu.init(*fReader, "boostedTau_idAntiMu");
    boostedTau_idMVAnewDM2017v2.init(*fReader, "boostedTau_idMVAnewDM2017v2");
    boostedTau_idMVAoldDM2017v2.init(*fReader, "boostedTau_idMVAoldDM2017v2");
    boostedTau_jetIdx.init(*fReader, "boostedTau_jetIdx");
    boostedTau_leadTkDeltaEta.init(*fReader, "boostedTau_leadTkDeltaEta");
    boostedTau_leadTkDeltaPhi.init(*fReader, "boostedTau_leadTkDeltaPhi");
    boostedTau_leadTkPtOverTauPt.init(*fReader, "boostedTau_leadTkPtOverTauPt");
    boostedTau_mass.init(*fReader, "boostedTau_mass");
    boostedTau_neutralIso.init(*fReader, "boostedTau_neutralIso");
    boostedTau_phi.init(*fReader, "boostedTau_phi");
    boostedTau_photonsOutsideSignalCone.init(*fReader, "boostedTau_photonsOutsideSignalCone");
    boostedTau_pt.init(*fReader, "boostedTau_pt");
    boostedTau_puCorr.init(*fReader, "boostedTau_puCorr");
    boostedTau_rawAntiEle2018.init(*fReader, "boostedTau_rawAntiEle2018");
    boostedTau_rawAntiEleCat2018.init(*fReader, "boostedTau_rawAntiEleCat2018");
    boostedTau_rawBoostedDeepTauRunIIv2p0VSe.init(*fReader, "boostedTau_rawBoostedDeepTauRunIIv2p0VSe");
    boostedTau_rawBoostedDeepTauRunIIv2p0VSjet.init(*fReader, "boostedTau_rawBoostedDeepTauRunIIv2p0VSjet");
    boostedTau_rawBoostedDeepTauRunIIv2p0VSmu.init(*fReader, "boostedTau_rawBoostedDeepTauRunIIv2p0VSmu");
    boostedTau_rawIso.init(*fReader, "boostedTau_rawIso");
    boostedTau_rawIsodR03.init(*fReader, "boostedTau_rawIsodR03");
    boostedTau_rawMVAnewDM2017v2.init(*fReader, "boostedTau_rawMVAnewDM2017v2");
    boostedTau_rawMVAoldDM2017v2.init(*fReader, "boostedTau_rawMVAoldDM2017v2");
    nboostedTau.init(*fReader, "nboostedTau");

    // bunchCrossing
    bunchCrossing.init(*fReader, "bunchCrossing");

    // event
    event.init(*fReader, "event");

    // genTtbarId
    genTtbarId.init(*fReader, "genTtbarId");

    // genWeight
    genWeight.init(*fReader, "genWeight");

    // luminosityBlock
    luminosityBlock.init(*fReader, "luminosityBlock");

    // orbitNumber
    orbitNumber.init(*fReader, "orbitNumber");

    // run
    RunNumber.init(*fReader, "run");
}
