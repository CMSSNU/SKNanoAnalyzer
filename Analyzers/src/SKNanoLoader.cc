#define SKNanoLoader_cxx
#include "SKNanoLoader.h"
using json = nlohmann::json;

SKNanoLoader::SKNanoLoader() : // ==============================================================
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
                               Electron_IPx(TTreeReaderArray<Float_t>(*fReader, "Electron_IPx")),
                               Electron_IPy(TTreeReaderArray<Float_t>(*fReader, "Electron_IPy")),
                               Electron_IPz(TTreeReaderArray<Float_t>(*fReader, "Electron_IPz")),
                               Electron_PreshowerEnergy(TTreeReaderArray<Float_t>(*fReader, "Electron_PreshowerEnergy")),
                               Electron_charge(TTreeReaderArray<Int_t>(*fReader, "Electron_charge")),
                               Electron_convVeto(TTreeReaderArray<Bool_t>(*fReader, "Electron_convVeto")),
                               Electron_cutBased(TTreeReaderArray<UChar_t>(*fReader, "Electron_cutBased")),
                               Electron_cutBased_HEEP(TTreeReaderArray<Bool_t>(*fReader, "Electron_cutBased_HEEP")),
                               Electron_deltaEtaSC(TTreeReaderArray<Float_t>(*fReader, "Electron_deltaEtaSC")),
                               Electron_dr03EcalRecHitSumEt(TTreeReaderArray<Float_t>(*fReader, "Electron_dr03EcalRecHitSumEt")),
                               Electron_dr03HcalDepth1TowerSumEt(TTreeReaderArray<Float_t>(*fReader, "Electron_dr03HcalDepth1TowerSumEt")),
                               Electron_dr03TkSumPt(TTreeReaderArray<Float_t>(*fReader, "Electron_dr03TkSumPt")),
                               Electron_dr03TkSumPtHEEP(TTreeReaderArray<Float_t>(*fReader, "Electron_dr03TkSumPtHEEP")),
                               Electron_dxy(TTreeReaderArray<Float_t>(*fReader, "Electron_dxy")),
                               Electron_dxyErr(TTreeReaderArray<Float_t>(*fReader, "Electron_dxyErr")),
                               Electron_dz(TTreeReaderArray<Float_t>(*fReader, "Electron_dz")),
                               Electron_dzErr(TTreeReaderArray<Float_t>(*fReader, "Electron_dzErr")),
                               Electron_eInvMinusPInv(TTreeReaderArray<Float_t>(*fReader, "Electron_eInvMinusPInv")),
                               Electron_ecalEnergy(TTreeReaderArray<Float_t>(*fReader, "Electron_ecalEnergy")),
                               Electron_ecalEnergyError(TTreeReaderArray<Float_t>(*fReader, "Electron_ecalEnergyError")),
                               Electron_energyErr(TTreeReaderArray<Float_t>(*fReader, "Electron_energyErr")),
                               Electron_eta(TTreeReaderArray<Float_t>(*fReader, "Electron_eta")),
                               Electron_fbrem(TTreeReaderArray<Float_t>(*fReader, "Electron_fbrem")),
                               Electron_fsrPhotonIdx(TTreeReaderArray<Short_t>(*fReader, "Electron_fsrPhotonIdx")),
                               Electron_genPartFlav(TTreeReaderArray<UChar_t>(*fReader, "Electron_genPartFlav")),
                               Electron_genPartIdx(TTreeReaderArray<Short_t>(*fReader, "Electron_genPartIdx")),
                               Electron_gsfTrketaMode(TTreeReaderArray<Float_t>(*fReader, "Electron_gsfTrketaMode")),
                               Electron_gsfTrkpMode(TTreeReaderArray<Float_t>(*fReader, "Electron_gsfTrkpMode")),
                               Electron_gsfTrkpModeErr(TTreeReaderArray<Float_t>(*fReader, "Electron_gsfTrkpModeErr")),
                               Electron_gsfTrkphiMode(TTreeReaderArray<Float_t>(*fReader, "Electron_gsfTrkphiMode")),
                               Electron_hoe(TTreeReaderArray<Float_t>(*fReader, "Electron_hoe")),
                               Electron_ip3d(TTreeReaderArray<Float_t>(*fReader, "Electron_ip3d")),
                               Electron_ipLengthSig(TTreeReaderArray<Float_t>(*fReader, "Electron_ipLengthSig")),
                               Electron_isEB(TTreeReaderArray<Bool_t>(*fReader, "Electron_isEB")),
                               Electron_isEcalDriven(TTreeReaderArray<Bool_t>(*fReader, "Electron_isEcalDriven")),
                               Electron_isPFcand(TTreeReaderArray<Bool_t>(*fReader, "Electron_isPFcand")),
                               Electron_jetDF(TTreeReaderArray<Float_t>(*fReader, "Electron_jetDF")),
                               Electron_jetIdx(TTreeReaderArray<Short_t>(*fReader, "Electron_jetIdx")),
                               Electron_jetNDauCharged(TTreeReaderArray<UChar_t>(*fReader, "Electron_jetNDauCharged")),
                               Electron_jetPtRelv2(TTreeReaderArray<Float_t>(*fReader, "Electron_jetPtRelv2")),
                               Electron_jetRelIso(TTreeReaderArray<Float_t>(*fReader, "Electron_jetRelIso")),
                               Electron_lostHits(TTreeReaderArray<UChar_t>(*fReader, "Electron_lostHits")),
                               Electron_mass(TTreeReaderArray<Float_t>(*fReader, "Electron_mass")),
                               Electron_miniPFRelIso_all(TTreeReaderArray<Float_t>(*fReader, "Electron_miniPFRelIso_all")),
                               Electron_miniPFRelIso_chg(TTreeReaderArray<Float_t>(*fReader, "Electron_miniPFRelIso_chg")),
                               Electron_mvaHZZIso(TTreeReaderArray<Float_t>(*fReader, "Electron_mvaHZZIso")),
                               Electron_mvaIso(TTreeReaderArray<Float_t>(*fReader, "Electron_mvaIso")),
                               Electron_mvaIso_WP80(TTreeReaderArray<Bool_t>(*fReader, "Electron_mvaIso_WP80")),
                               Electron_mvaIso_WP90(TTreeReaderArray<Bool_t>(*fReader, "Electron_mvaIso_WP90")),
                               Electron_mvaIso_WPHZZ(TTreeReaderArray<Bool_t>(*fReader, "Electron_mvaIso_WPHZZ")),
                               Electron_mvaNoIso(TTreeReaderArray<Float_t>(*fReader, "Electron_mvaNoIso")),
                               Electron_mvaNoIso_WP80(TTreeReaderArray<Bool_t>(*fReader, "Electron_mvaNoIso_WP80")),
                               Electron_mvaNoIso_WP90(TTreeReaderArray<Bool_t>(*fReader, "Electron_mvaNoIso_WP90")),
                               Electron_pdgId(TTreeReaderArray<Int_t>(*fReader, "Electron_pdgId")),
                               Electron_pfRelIso03_all(TTreeReaderArray<Float_t>(*fReader, "Electron_pfRelIso03_all")),
                               Electron_pfRelIso03_chg(TTreeReaderArray<Float_t>(*fReader, "Electron_pfRelIso03_chg")),
                               Electron_pfRelIso04_all(TTreeReaderArray<Float_t>(*fReader, "Electron_pfRelIso04_all")),
                               Electron_phi(TTreeReaderArray<Float_t>(*fReader, "Electron_phi")),
                               Electron_photonIdx(TTreeReaderArray<Short_t>(*fReader, "Electron_photonIdx")),
                               Electron_promptMVA(TTreeReaderArray<Float_t>(*fReader, "Electron_promptMVA")),
                               Electron_pt(TTreeReaderArray<Float_t>(*fReader, "Electron_pt")),
                               Electron_r9(TTreeReaderArray<Float_t>(*fReader, "Electron_r9")),
                               Electron_rawEnergy(TTreeReaderArray<Float_t>(*fReader, "Electron_rawEnergy")),
                               Electron_scEtOverPt(TTreeReaderArray<Float_t>(*fReader, "Electron_scEtOverPt")),
                               Electron_seedGain(TTreeReaderArray<UChar_t>(*fReader, "Electron_seedGain")),
                               Electron_seediEtaOriX(TTreeReaderArray<Short_t>(*fReader, "Electron_seediEtaOriX")),
                               Electron_seediPhiOriY(TTreeReaderArray<Short_t>(*fReader, "Electron_seediPhiOriY")),
                               Electron_sieie(TTreeReaderArray<Float_t>(*fReader, "Electron_sieie")),
                               Electron_sip3d(TTreeReaderArray<Float_t>(*fReader, "Electron_sip3d")),
                               Electron_superclusterEta(TTreeReaderArray<Float_t>(*fReader, "Electron_superclusterEta")),
                               Electron_svIdx(TTreeReaderArray<Short_t>(*fReader, "Electron_svIdx")),
                               Electron_tightCharge(TTreeReaderArray<UChar_t>(*fReader, "Electron_tightCharge")),
                               Electron_vidNestedWPBitmap(TTreeReaderArray<Int_t>(*fReader, "Electron_vidNestedWPBitmap")),
                               Electron_vidNestedWPBitmapHEEP(TTreeReaderArray<Int_t>(*fReader, "Electron_vidNestedWPBitmapHEEP")),
                               nElectron_Ptr(TTreeReaderValue<Int_t>(*fReader, "nElectron")),
                               nElectron(*nElectron_Ptr),

                               // FatJet
                               FatJet_area(TTreeReaderArray<Float_t>(*fReader, "FatJet_area")),
                               FatJet_chEmEF(TTreeReaderArray<Float_t>(*fReader, "FatJet_chEmEF")),
                               FatJet_chHEF(TTreeReaderArray<Float_t>(*fReader, "FatJet_chHEF")),
                               FatJet_chMultiplicity(TTreeReaderArray<Short_t>(*fReader, "FatJet_chMultiplicity")),
                               FatJet_electronIdx3SJ(TTreeReaderArray<Short_t>(*fReader, "FatJet_electronIdx3SJ")),
                               FatJet_eta(TTreeReaderArray<Float_t>(*fReader, "FatJet_eta")),
                               FatJet_genJetAK8Idx(TTreeReaderArray<Short_t>(*fReader, "FatJet_genJetAK8Idx")),
                               FatJet_globalParT3_QCD(TTreeReaderArray<Float_t>(*fReader, "FatJet_globalParT3_QCD")),
                               FatJet_globalParT3_TopbWev(TTreeReaderArray<Float_t>(*fReader, "FatJet_globalParT3_TopbWev")),
                               FatJet_globalParT3_TopbWmv(TTreeReaderArray<Float_t>(*fReader, "FatJet_globalParT3_TopbWmv")),
                               FatJet_globalParT3_TopbWq(TTreeReaderArray<Float_t>(*fReader, "FatJet_globalParT3_TopbWq")),
                               FatJet_globalParT3_TopbWqq(TTreeReaderArray<Float_t>(*fReader, "FatJet_globalParT3_TopbWqq")),
                               FatJet_globalParT3_TopbWtauhv(TTreeReaderArray<Float_t>(*fReader, "FatJet_globalParT3_TopbWtauhv")),
                               FatJet_globalParT3_WvsQCD(TTreeReaderArray<Float_t>(*fReader, "FatJet_globalParT3_WvsQCD")),
                               FatJet_globalParT3_XWW3q(TTreeReaderArray<Float_t>(*fReader, "FatJet_globalParT3_XWW3q")),
                               FatJet_globalParT3_XWW4q(TTreeReaderArray<Float_t>(*fReader, "FatJet_globalParT3_XWW4q")),
                               FatJet_globalParT3_XWWqqev(TTreeReaderArray<Float_t>(*fReader, "FatJet_globalParT3_XWWqqev")),
                               FatJet_globalParT3_XWWqqmv(TTreeReaderArray<Float_t>(*fReader, "FatJet_globalParT3_XWWqqmv")),
                               FatJet_globalParT3_Xbb(TTreeReaderArray<Float_t>(*fReader, "FatJet_globalParT3_Xbb")),
                               FatJet_globalParT3_Xcc(TTreeReaderArray<Float_t>(*fReader, "FatJet_globalParT3_Xcc")),
                               FatJet_globalParT3_Xcs(TTreeReaderArray<Float_t>(*fReader, "FatJet_globalParT3_Xcs")),
                               FatJet_globalParT3_Xqq(TTreeReaderArray<Float_t>(*fReader, "FatJet_globalParT3_Xqq")),
                               FatJet_globalParT3_Xtauhtaue(TTreeReaderArray<Float_t>(*fReader, "FatJet_globalParT3_Xtauhtaue")),
                               FatJet_globalParT3_Xtauhtauh(TTreeReaderArray<Float_t>(*fReader, "FatJet_globalParT3_Xtauhtauh")),
                               FatJet_globalParT3_Xtauhtaum(TTreeReaderArray<Float_t>(*fReader, "FatJet_globalParT3_Xtauhtaum")),
                               FatJet_globalParT3_massCorrGeneric(TTreeReaderArray<Float_t>(*fReader, "FatJet_globalParT3_massCorrGeneric")),
                               FatJet_globalParT3_massCorrX2p(TTreeReaderArray<Float_t>(*fReader, "FatJet_globalParT3_massCorrX2p")),
                               FatJet_globalParT3_withMassTopvsQCD(TTreeReaderArray<Float_t>(*fReader, "FatJet_globalParT3_withMassTopvsQCD")),
                               FatJet_globalParT3_withMassWvsQCD(TTreeReaderArray<Float_t>(*fReader, "FatJet_globalParT3_withMassWvsQCD")),
                               FatJet_globalParT3_withMassZvsQCD(TTreeReaderArray<Float_t>(*fReader, "FatJet_globalParT3_withMassZvsQCD")),
                               FatJet_hadronFlavour(TTreeReaderArray<UChar_t>(*fReader, "FatJet_hadronFlavour")),
                               FatJet_hfEmEF(TTreeReaderArray<Float_t>(*fReader, "FatJet_hfEmEF")),
                               FatJet_hfHEF(TTreeReaderArray<Float_t>(*fReader, "FatJet_hfHEF")),
                               FatJet_lsf3(TTreeReaderArray<Float_t>(*fReader, "FatJet_lsf3")),
                               FatJet_mass(TTreeReaderArray<Float_t>(*fReader, "FatJet_mass")),
                               FatJet_msoftdrop(TTreeReaderArray<Float_t>(*fReader, "FatJet_msoftdrop")),
                               FatJet_muEF(TTreeReaderArray<Float_t>(*fReader, "FatJet_muEF")),
                               FatJet_muonIdx3SJ(TTreeReaderArray<Short_t>(*fReader, "FatJet_muonIdx3SJ")),
                               FatJet_n2b1(TTreeReaderArray<Float_t>(*fReader, "FatJet_n2b1")),
                               FatJet_n3b1(TTreeReaderArray<Float_t>(*fReader, "FatJet_n3b1")),
                               FatJet_nConstituents(TTreeReaderArray<UChar_t>(*fReader, "FatJet_nConstituents")),
                               FatJet_neEmEF(TTreeReaderArray<Float_t>(*fReader, "FatJet_neEmEF")),
                               FatJet_neHEF(TTreeReaderArray<Float_t>(*fReader, "FatJet_neHEF")),
                               FatJet_neMultiplicity(TTreeReaderArray<Short_t>(*fReader, "FatJet_neMultiplicity")),
                               FatJet_particleNetLegacy_QCD(TTreeReaderArray<Float_t>(*fReader, "FatJet_particleNetLegacy_QCD")),
                               FatJet_particleNetLegacy_Xbb(TTreeReaderArray<Float_t>(*fReader, "FatJet_particleNetLegacy_Xbb")),
                               FatJet_particleNetLegacy_Xcc(TTreeReaderArray<Float_t>(*fReader, "FatJet_particleNetLegacy_Xcc")),
                               FatJet_particleNetLegacy_Xqq(TTreeReaderArray<Float_t>(*fReader, "FatJet_particleNetLegacy_Xqq")),
                               FatJet_particleNetLegacy_mass(TTreeReaderArray<Float_t>(*fReader, "FatJet_particleNetLegacy_mass")),
                               FatJet_particleNetWithMass_H4qvsQCD(TTreeReaderArray<Float_t>(*fReader, "FatJet_particleNetWithMass_H4qvsQCD")),
                               FatJet_particleNetWithMass_HbbvsQCD(TTreeReaderArray<Float_t>(*fReader, "FatJet_particleNetWithMass_HbbvsQCD")),
                               FatJet_particleNetWithMass_HccvsQCD(TTreeReaderArray<Float_t>(*fReader, "FatJet_particleNetWithMass_HccvsQCD")),
                               FatJet_particleNetWithMass_QCD(TTreeReaderArray<Float_t>(*fReader, "FatJet_particleNetWithMass_QCD")),
                               FatJet_particleNetWithMass_TvsQCD(TTreeReaderArray<Float_t>(*fReader, "FatJet_particleNetWithMass_TvsQCD")),
                               FatJet_particleNetWithMass_WvsQCD(TTreeReaderArray<Float_t>(*fReader, "FatJet_particleNetWithMass_WvsQCD")),
                               FatJet_particleNetWithMass_ZvsQCD(TTreeReaderArray<Float_t>(*fReader, "FatJet_particleNetWithMass_ZvsQCD")),
                               FatJet_particleNet_QCD(TTreeReaderArray<Float_t>(*fReader, "FatJet_particleNet_QCD")),
                               FatJet_particleNet_QCD0HF(TTreeReaderArray<Float_t>(*fReader, "FatJet_particleNet_QCD0HF")),
                               FatJet_particleNet_QCD1HF(TTreeReaderArray<Float_t>(*fReader, "FatJet_particleNet_QCD1HF")),
                               FatJet_particleNet_QCD2HF(TTreeReaderArray<Float_t>(*fReader, "FatJet_particleNet_QCD2HF")),
                               FatJet_particleNet_WVsQCD(TTreeReaderArray<Float_t>(*fReader, "FatJet_particleNet_WVsQCD")),
                               FatJet_particleNet_XbbVsQCD(TTreeReaderArray<Float_t>(*fReader, "FatJet_particleNet_XbbVsQCD")),
                               FatJet_particleNet_XccVsQCD(TTreeReaderArray<Float_t>(*fReader, "FatJet_particleNet_XccVsQCD")),
                               FatJet_particleNet_XggVsQCD(TTreeReaderArray<Float_t>(*fReader, "FatJet_particleNet_XggVsQCD")),
                               FatJet_particleNet_XqqVsQCD(TTreeReaderArray<Float_t>(*fReader, "FatJet_particleNet_XqqVsQCD")),
                               FatJet_particleNet_XteVsQCD(TTreeReaderArray<Float_t>(*fReader, "FatJet_particleNet_XteVsQCD")),
                               FatJet_particleNet_XtmVsQCD(TTreeReaderArray<Float_t>(*fReader, "FatJet_particleNet_XtmVsQCD")),
                               FatJet_particleNet_XttVsQCD(TTreeReaderArray<Float_t>(*fReader, "FatJet_particleNet_XttVsQCD")),
                               FatJet_particleNet_massCorr(TTreeReaderArray<Float_t>(*fReader, "FatJet_particleNet_massCorr")),
                               FatJet_phi(TTreeReaderArray<Float_t>(*fReader, "FatJet_phi")),
                               FatJet_pt(TTreeReaderArray<Float_t>(*fReader, "FatJet_pt")),
                               FatJet_rawFactor(TTreeReaderArray<Float_t>(*fReader, "FatJet_rawFactor")),
                               FatJet_subJetIdx1(TTreeReaderArray<Short_t>(*fReader, "FatJet_subJetIdx1")),
                               FatJet_subJetIdx2(TTreeReaderArray<Short_t>(*fReader, "FatJet_subJetIdx2")),
                               FatJet_tau1(TTreeReaderArray<Float_t>(*fReader, "FatJet_tau1")),
                               FatJet_tau2(TTreeReaderArray<Float_t>(*fReader, "FatJet_tau2")),
                               FatJet_tau3(TTreeReaderArray<Float_t>(*fReader, "FatJet_tau3")),
                               FatJet_tau4(TTreeReaderArray<Float_t>(*fReader, "FatJet_tau4")),
                               nFatJet_Ptr(TTreeReaderValue<Int_t>(*fReader, "nFatJet")),
                               nFatJet(*nFatJet_Ptr),

                               // FatJetPFCand
                               FatJetPFCand_jetIdx(TTreeReaderArray<Int_t>(*fReader, "FatJetPFCand_jetIdx")),
                               FatJetPFCand_pfCandIdx(TTreeReaderArray<Int_t>(*fReader, "FatJetPFCand_pfCandIdx")),
                               nFatJetPFCand_Ptr(TTreeReaderValue<Int_t>(*fReader, "nFatJetPFCand")),
                               nFatJetPFCand(*nFatJetPFCand_Ptr),

                               // Flag
                               Flag_BadChargedCandidateFilter_Ptr(TTreeReaderValue<Bool_t>(*fReader, "Flag_BadChargedCandidateFilter")),
                               Flag_BadChargedCandidateFilter(*Flag_BadChargedCandidateFilter_Ptr),
                               Flag_BadChargedCandidateSummer16Filter_Ptr(TTreeReaderValue<Bool_t>(*fReader, "Flag_BadChargedCandidateSummer16Filter")),
                               Flag_BadChargedCandidateSummer16Filter(*Flag_BadChargedCandidateSummer16Filter_Ptr),
                               Flag_BadPFMuonDzFilter_Ptr(TTreeReaderValue<Bool_t>(*fReader, "Flag_BadPFMuonDzFilter")),
                               Flag_BadPFMuonDzFilter(*Flag_BadPFMuonDzFilter_Ptr),
                               Flag_BadPFMuonFilter_Ptr(TTreeReaderValue<Bool_t>(*fReader, "Flag_BadPFMuonFilter")),
                               Flag_BadPFMuonFilter(*Flag_BadPFMuonFilter_Ptr),
                               Flag_BadPFMuonSummer16Filter_Ptr(TTreeReaderValue<Bool_t>(*fReader, "Flag_BadPFMuonSummer16Filter")),
                               Flag_BadPFMuonSummer16Filter(*Flag_BadPFMuonSummer16Filter_Ptr),
                               Flag_CSCTightHalo2015Filter_Ptr(TTreeReaderValue<Bool_t>(*fReader, "Flag_CSCTightHalo2015Filter")),
                               Flag_CSCTightHalo2015Filter(*Flag_CSCTightHalo2015Filter_Ptr),
                               Flag_CSCTightHaloFilter_Ptr(TTreeReaderValue<Bool_t>(*fReader, "Flag_CSCTightHaloFilter")),
                               Flag_CSCTightHaloFilter(*Flag_CSCTightHaloFilter_Ptr),
                               Flag_CSCTightHaloTrkMuUnvetoFilter_Ptr(TTreeReaderValue<Bool_t>(*fReader, "Flag_CSCTightHaloTrkMuUnvetoFilter")),
                               Flag_CSCTightHaloTrkMuUnvetoFilter(*Flag_CSCTightHaloTrkMuUnvetoFilter_Ptr),
                               Flag_EcalDeadCellBoundaryEnergyFilter_Ptr(TTreeReaderValue<Bool_t>(*fReader, "Flag_EcalDeadCellBoundaryEnergyFilter")),
                               Flag_EcalDeadCellBoundaryEnergyFilter(*Flag_EcalDeadCellBoundaryEnergyFilter_Ptr),
                               Flag_EcalDeadCellTriggerPrimitiveFilter_Ptr(TTreeReaderValue<Bool_t>(*fReader, "Flag_EcalDeadCellTriggerPrimitiveFilter")),
                               Flag_EcalDeadCellTriggerPrimitiveFilter(*Flag_EcalDeadCellTriggerPrimitiveFilter_Ptr),
                               Flag_HBHENoiseFilter_Ptr(TTreeReaderValue<Bool_t>(*fReader, "Flag_HBHENoiseFilter")),
                               Flag_HBHENoiseFilter(*Flag_HBHENoiseFilter_Ptr),
                               Flag_HBHENoiseIsoFilter_Ptr(TTreeReaderValue<Bool_t>(*fReader, "Flag_HBHENoiseIsoFilter")),
                               Flag_HBHENoiseIsoFilter(*Flag_HBHENoiseIsoFilter_Ptr),
                               Flag_HcalStripHaloFilter_Ptr(TTreeReaderValue<Bool_t>(*fReader, "Flag_HcalStripHaloFilter")),
                               Flag_HcalStripHaloFilter(*Flag_HcalStripHaloFilter_Ptr),
                               Flag_chargedHadronTrackResolutionFilter_Ptr(TTreeReaderValue<Bool_t>(*fReader, "Flag_chargedHadronTrackResolutionFilter")),
                               Flag_chargedHadronTrackResolutionFilter(*Flag_chargedHadronTrackResolutionFilter_Ptr),
                               Flag_ecalBadCalibFilter_Ptr(TTreeReaderValue<Bool_t>(*fReader, "Flag_ecalBadCalibFilter")),
                               Flag_ecalBadCalibFilter(*Flag_ecalBadCalibFilter_Ptr),
                               Flag_ecalLaserCorrFilter_Ptr(TTreeReaderValue<Bool_t>(*fReader, "Flag_ecalLaserCorrFilter")),
                               Flag_ecalLaserCorrFilter(*Flag_ecalLaserCorrFilter_Ptr),
                               Flag_eeBadScFilter_Ptr(TTreeReaderValue<Bool_t>(*fReader, "Flag_eeBadScFilter")),
                               Flag_eeBadScFilter(*Flag_eeBadScFilter_Ptr),
                               Flag_globalSuperTightHalo2016Filter_Ptr(TTreeReaderValue<Bool_t>(*fReader, "Flag_globalSuperTightHalo2016Filter")),
                               Flag_globalSuperTightHalo2016Filter(*Flag_globalSuperTightHalo2016Filter_Ptr),
                               Flag_globalTightHalo2016Filter_Ptr(TTreeReaderValue<Bool_t>(*fReader, "Flag_globalTightHalo2016Filter")),
                               Flag_globalTightHalo2016Filter(*Flag_globalTightHalo2016Filter_Ptr),
                               Flag_goodVertices_Ptr(TTreeReaderValue<Bool_t>(*fReader, "Flag_goodVertices")),
                               Flag_goodVertices(*Flag_goodVertices_Ptr),
                               Flag_hcalLaserEventFilter_Ptr(TTreeReaderValue<Bool_t>(*fReader, "Flag_hcalLaserEventFilter")),
                               Flag_hcalLaserEventFilter(*Flag_hcalLaserEventFilter_Ptr),
                               Flag_hfNoisyHitsFilter_Ptr(TTreeReaderValue<Bool_t>(*fReader, "Flag_hfNoisyHitsFilter")),
                               Flag_hfNoisyHitsFilter(*Flag_hfNoisyHitsFilter_Ptr),
                               Flag_muonBadTrackFilter_Ptr(TTreeReaderValue<Bool_t>(*fReader, "Flag_muonBadTrackFilter")),
                               Flag_muonBadTrackFilter(*Flag_muonBadTrackFilter_Ptr),
                               Flag_trkPOGFilters_Ptr(TTreeReaderValue<Bool_t>(*fReader, "Flag_trkPOGFilters")),
                               Flag_trkPOGFilters(*Flag_trkPOGFilters_Ptr),
                               Flag_trkPOG_logErrorTooManyClusters_Ptr(TTreeReaderValue<Bool_t>(*fReader, "Flag_trkPOG_logErrorTooManyClusters")),
                               Flag_trkPOG_logErrorTooManyClusters(*Flag_trkPOG_logErrorTooManyClusters_Ptr),
                               Flag_trkPOG_manystripclus53X_Ptr(TTreeReaderValue<Bool_t>(*fReader, "Flag_trkPOG_manystripclus53X")),
                               Flag_trkPOG_manystripclus53X(*Flag_trkPOG_manystripclus53X_Ptr),
                               Flag_trkPOG_toomanystripclus53X_Ptr(TTreeReaderValue<Bool_t>(*fReader, "Flag_trkPOG_toomanystripclus53X")),
                               Flag_trkPOG_toomanystripclus53X(*Flag_trkPOG_toomanystripclus53X_Ptr),

                               // FsrPhoton
                               FsrPhoton_dROverEt2(TTreeReaderArray<Float_t>(*fReader, "FsrPhoton_dROverEt2")),
                               FsrPhoton_electronIdx(TTreeReaderArray<Short_t>(*fReader, "FsrPhoton_electronIdx")),
                               FsrPhoton_eta(TTreeReaderArray<Float_t>(*fReader, "FsrPhoton_eta")),
                               FsrPhoton_muonIdx(TTreeReaderArray<Short_t>(*fReader, "FsrPhoton_muonIdx")),
                               FsrPhoton_phi(TTreeReaderArray<Float_t>(*fReader, "FsrPhoton_phi")),
                               FsrPhoton_pt(TTreeReaderArray<Float_t>(*fReader, "FsrPhoton_pt")),
                               FsrPhoton_relIso03(TTreeReaderArray<Float_t>(*fReader, "FsrPhoton_relIso03")),
                               nFsrPhoton_Ptr(TTreeReaderValue<Int_t>(*fReader, "nFsrPhoton")),
                               nFsrPhoton(*nFsrPhoton_Ptr),

                               // GenDressedLepton
                               GenDressedLepton_eta(TTreeReaderArray<Float_t>(*fReader, "GenDressedLepton_eta")),
                               GenDressedLepton_hasTauAnc(TTreeReaderArray<Bool_t>(*fReader, "GenDressedLepton_hasTauAnc")),
                               GenDressedLepton_mass(TTreeReaderArray<Float_t>(*fReader, "GenDressedLepton_mass")),
                               GenDressedLepton_pdgId(TTreeReaderArray<Int_t>(*fReader, "GenDressedLepton_pdgId")),
                               GenDressedLepton_phi(TTreeReaderArray<Float_t>(*fReader, "GenDressedLepton_phi")),
                               GenDressedLepton_pt(TTreeReaderArray<Float_t>(*fReader, "GenDressedLepton_pt")),
                               nGenDressedLepton_Ptr(TTreeReaderValue<Int_t>(*fReader, "nGenDressedLepton")),
                               nGenDressedLepton(*nGenDressedLepton_Ptr),

                               // GenIsolatedPhoton
                               GenIsolatedPhoton_eta(TTreeReaderArray<Float_t>(*fReader, "GenIsolatedPhoton_eta")),
                               GenIsolatedPhoton_mass(TTreeReaderArray<Float_t>(*fReader, "GenIsolatedPhoton_mass")),
                               GenIsolatedPhoton_phi(TTreeReaderArray<Float_t>(*fReader, "GenIsolatedPhoton_phi")),
                               GenIsolatedPhoton_pt(TTreeReaderArray<Float_t>(*fReader, "GenIsolatedPhoton_pt")),
                               nGenIsolatedPhoton_Ptr(TTreeReaderValue<Int_t>(*fReader, "nGenIsolatedPhoton")),
                               nGenIsolatedPhoton(*nGenIsolatedPhoton_Ptr),

                               // GenJet
                               GenJet_eta(TTreeReaderArray<Float_t>(*fReader, "GenJet_eta")),
                               GenJet_hadronFlavour(TTreeReaderArray<UChar_t>(*fReader, "GenJet_hadronFlavour")),
                               GenJet_mass(TTreeReaderArray<Float_t>(*fReader, "GenJet_mass")),
                               GenJet_nBHadrons(TTreeReaderArray<UChar_t>(*fReader, "GenJet_nBHadrons")),
                               GenJet_nCHadrons(TTreeReaderArray<UChar_t>(*fReader, "GenJet_nCHadrons")),
                               GenJet_partonFlavour(TTreeReaderArray<Short_t>(*fReader, "GenJet_partonFlavour")),
                               GenJet_phi(TTreeReaderArray<Float_t>(*fReader, "GenJet_phi")),
                               GenJet_pt(TTreeReaderArray<Float_t>(*fReader, "GenJet_pt")),
                               nGenJet_Ptr(TTreeReaderValue<Int_t>(*fReader, "nGenJet")),
                               nGenJet(*nGenJet_Ptr),

                               // GenJetAK8
                               GenJetAK8_eta(TTreeReaderArray<Float_t>(*fReader, "GenJetAK8_eta")),
                               GenJetAK8_hadronFlavour(TTreeReaderArray<UChar_t>(*fReader, "GenJetAK8_hadronFlavour")),
                               GenJetAK8_mass(TTreeReaderArray<Float_t>(*fReader, "GenJetAK8_mass")),
                               GenJetAK8_nBHadrons(TTreeReaderArray<UChar_t>(*fReader, "GenJetAK8_nBHadrons")),
                               GenJetAK8_nCHadrons(TTreeReaderArray<UChar_t>(*fReader, "GenJetAK8_nCHadrons")),
                               GenJetAK8_partonFlavour(TTreeReaderArray<Short_t>(*fReader, "GenJetAK8_partonFlavour")),
                               GenJetAK8_phi(TTreeReaderArray<Float_t>(*fReader, "GenJetAK8_phi")),
                               GenJetAK8_pt(TTreeReaderArray<Float_t>(*fReader, "GenJetAK8_pt")),
                               nGenJetAK8_Ptr(TTreeReaderValue<Int_t>(*fReader, "nGenJetAK8")),
                               nGenJetAK8(*nGenJetAK8_Ptr),

                               // GenMET
                               GenMET_phi_Ptr(TTreeReaderValue<Float_t>(*fReader, "GenMET_phi")),
                               GenMET_phi(*GenMET_phi_Ptr),
                               GenMET_pt_Ptr(TTreeReaderValue<Float_t>(*fReader, "GenMET_pt")),
                               GenMET_pt(*GenMET_pt_Ptr),

                               // GenPart
                               GenPart_eta(TTreeReaderArray<Float_t>(*fReader, "GenPart_eta")),
                               GenPart_genPartIdxMother(TTreeReaderArray<Short_t>(*fReader, "GenPart_genPartIdxMother")),
                               GenPart_iso(TTreeReaderArray<Float_t>(*fReader, "GenPart_iso")),
                               GenPart_mass(TTreeReaderArray<Float_t>(*fReader, "GenPart_mass")),
                               GenPart_pdgId(TTreeReaderArray<Int_t>(*fReader, "GenPart_pdgId")),
                               GenPart_phi(TTreeReaderArray<Float_t>(*fReader, "GenPart_phi")),
                               GenPart_pt(TTreeReaderArray<Float_t>(*fReader, "GenPart_pt")),
                               GenPart_status(TTreeReaderArray<Int_t>(*fReader, "GenPart_status")),
                               GenPart_statusFlags(TTreeReaderArray<UShort_t>(*fReader, "GenPart_statusFlags")),
                               nGenPart_Ptr(TTreeReaderValue<Int_t>(*fReader, "nGenPart")),
                               nGenPart(*nGenPart_Ptr),

                               // GenVisTau
                               GenVisTau_charge(TTreeReaderArray<Short_t>(*fReader, "GenVisTau_charge")),
                               GenVisTau_eta(TTreeReaderArray<Float_t>(*fReader, "GenVisTau_eta")),
                               GenVisTau_genPartIdxMother(TTreeReaderArray<Short_t>(*fReader, "GenVisTau_genPartIdxMother")),
                               GenVisTau_mass(TTreeReaderArray<Float_t>(*fReader, "GenVisTau_mass")),
                               GenVisTau_phi(TTreeReaderArray<Float_t>(*fReader, "GenVisTau_phi")),
                               GenVisTau_pt(TTreeReaderArray<Float_t>(*fReader, "GenVisTau_pt")),
                               GenVisTau_status(TTreeReaderArray<UChar_t>(*fReader, "GenVisTau_status")),
                               nGenVisTau_Ptr(TTreeReaderValue<Int_t>(*fReader, "nGenVisTau")),
                               nGenVisTau(*nGenVisTau_Ptr),

                               // GenVtx
                               GenVtx_t0_Ptr(TTreeReaderValue<Float_t>(*fReader, "GenVtx_t0")),
                               GenVtx_t0(*GenVtx_t0_Ptr),
                               GenVtx_x_Ptr(TTreeReaderValue<Float_t>(*fReader, "GenVtx_x")),
                               GenVtx_x(*GenVtx_x_Ptr),
                               GenVtx_y_Ptr(TTreeReaderValue<Float_t>(*fReader, "GenVtx_y")),
                               GenVtx_y(*GenVtx_y_Ptr),
                               GenVtx_z_Ptr(TTreeReaderValue<Float_t>(*fReader, "GenVtx_z")),
                               GenVtx_z(*GenVtx_z_Ptr),

                               // Generator
                               Generator_binvar_Ptr(TTreeReaderValue<Float_t>(*fReader, "Generator_binvar")),
                               Generator_binvar(*Generator_binvar_Ptr),
                               Generator_id1_Ptr(TTreeReaderValue<Int_t>(*fReader, "Generator_id1")),
                               Generator_id1(*Generator_id1_Ptr),
                               Generator_id2_Ptr(TTreeReaderValue<Int_t>(*fReader, "Generator_id2")),
                               Generator_id2(*Generator_id2_Ptr),
                               Generator_scalePDF_Ptr(TTreeReaderValue<Float_t>(*fReader, "Generator_scalePDF")),
                               Generator_scalePDF(*Generator_scalePDF_Ptr),
                               Generator_weight_Ptr(TTreeReaderValue<Float_t>(*fReader, "Generator_weight")),
                               Generator_weight(*Generator_weight_Ptr),
                               Generator_x1_Ptr(TTreeReaderValue<Float_t>(*fReader, "Generator_x1")),
                               Generator_x1(*Generator_x1_Ptr),
                               Generator_x2_Ptr(TTreeReaderValue<Float_t>(*fReader, "Generator_x2")),
                               Generator_x2(*Generator_x2_Ptr),
                               Generator_xpdf1_Ptr(TTreeReaderValue<Float_t>(*fReader, "Generator_xpdf1")),
                               Generator_xpdf1(*Generator_xpdf1_Ptr),
                               Generator_xpdf2_Ptr(TTreeReaderValue<Float_t>(*fReader, "Generator_xpdf2")),
                               Generator_xpdf2(*Generator_xpdf2_Ptr),

                               // IsoTrack
                               IsoTrack_charge(TTreeReaderArray<Short_t>(*fReader, "IsoTrack_charge")),
                               IsoTrack_dxy(TTreeReaderArray<Float_t>(*fReader, "IsoTrack_dxy")),
                               IsoTrack_dz(TTreeReaderArray<Float_t>(*fReader, "IsoTrack_dz")),
                               IsoTrack_eta(TTreeReaderArray<Float_t>(*fReader, "IsoTrack_eta")),
                               IsoTrack_fromPV(TTreeReaderArray<Short_t>(*fReader, "IsoTrack_fromPV")),
                               IsoTrack_isFromLostTrack(TTreeReaderArray<Bool_t>(*fReader, "IsoTrack_isFromLostTrack")),
                               IsoTrack_isHighPurityTrack(TTreeReaderArray<Bool_t>(*fReader, "IsoTrack_isHighPurityTrack")),
                               IsoTrack_isPFcand(TTreeReaderArray<Bool_t>(*fReader, "IsoTrack_isPFcand")),
                               IsoTrack_miniPFRelIso_all(TTreeReaderArray<Float_t>(*fReader, "IsoTrack_miniPFRelIso_all")),
                               IsoTrack_miniPFRelIso_chg(TTreeReaderArray<Float_t>(*fReader, "IsoTrack_miniPFRelIso_chg")),
                               IsoTrack_pdgId(TTreeReaderArray<Int_t>(*fReader, "IsoTrack_pdgId")),
                               IsoTrack_pfRelIso03_all(TTreeReaderArray<Float_t>(*fReader, "IsoTrack_pfRelIso03_all")),
                               IsoTrack_pfRelIso03_chg(TTreeReaderArray<Float_t>(*fReader, "IsoTrack_pfRelIso03_chg")),
                               IsoTrack_phi(TTreeReaderArray<Float_t>(*fReader, "IsoTrack_phi")),
                               IsoTrack_pt(TTreeReaderArray<Float_t>(*fReader, "IsoTrack_pt")),
                               nIsoTrack_Ptr(TTreeReaderValue<Int_t>(*fReader, "nIsoTrack")),
                               nIsoTrack(*nIsoTrack_Ptr),

                               // Jet
                               Jet_PNetRegPtRawCorr(TTreeReaderArray<Float_t>(*fReader, "Jet_PNetRegPtRawCorr")),
                               Jet_PNetRegPtRawCorrNeutrino(TTreeReaderArray<Float_t>(*fReader, "Jet_PNetRegPtRawCorrNeutrino")),
                               Jet_PNetRegPtRawRes(TTreeReaderArray<Float_t>(*fReader, "Jet_PNetRegPtRawRes")),
                               Jet_UParTAK4RegPtRawCorr(TTreeReaderArray<Float_t>(*fReader, "Jet_UParTAK4RegPtRawCorr")),
                               Jet_UParTAK4RegPtRawCorrNeutrino(TTreeReaderArray<Float_t>(*fReader, "Jet_UParTAK4RegPtRawCorrNeutrino")),
                               Jet_UParTAK4RegPtRawRes(TTreeReaderArray<Float_t>(*fReader, "Jet_UParTAK4RegPtRawRes")),
                               Jet_UParTAK4V1RegPtRawCorr(TTreeReaderArray<Float_t>(*fReader, "Jet_UParTAK4V1RegPtRawCorr")),
                               Jet_UParTAK4V1RegPtRawCorrNeutrino(TTreeReaderArray<Float_t>(*fReader, "Jet_UParTAK4V1RegPtRawCorrNeutrino")),
                               Jet_UParTAK4V1RegPtRawRes(TTreeReaderArray<Float_t>(*fReader, "Jet_UParTAK4V1RegPtRawRes")),
                               Jet_area(TTreeReaderArray<Float_t>(*fReader, "Jet_area")),
                               Jet_btagDeepFlavB(TTreeReaderArray<Float_t>(*fReader, "Jet_btagDeepFlavB")),
                               Jet_btagDeepFlavCvB(TTreeReaderArray<Float_t>(*fReader, "Jet_btagDeepFlavCvB")),
                               Jet_btagDeepFlavCvL(TTreeReaderArray<Float_t>(*fReader, "Jet_btagDeepFlavCvL")),
                               Jet_btagDeepFlavQG(TTreeReaderArray<Float_t>(*fReader, "Jet_btagDeepFlavQG")),
                               Jet_btagPNetB(TTreeReaderArray<Float_t>(*fReader, "Jet_btagPNetB")),
                               Jet_btagPNetCvB(TTreeReaderArray<Float_t>(*fReader, "Jet_btagPNetCvB")),
                               Jet_btagPNetCvL(TTreeReaderArray<Float_t>(*fReader, "Jet_btagPNetCvL")),
                               Jet_btagPNetCvNotB(TTreeReaderArray<Float_t>(*fReader, "Jet_btagPNetCvNotB")),
                               Jet_btagPNetQvG(TTreeReaderArray<Float_t>(*fReader, "Jet_btagPNetQvG")),
                               Jet_btagPNetTauVJet(TTreeReaderArray<Float_t>(*fReader, "Jet_btagPNetTauVJet")),
                               Jet_btagUParTAK4B(TTreeReaderArray<Float_t>(*fReader, "Jet_btagUParTAK4B")),
                               Jet_btagUParTAK4CvB(TTreeReaderArray<Float_t>(*fReader, "Jet_btagUParTAK4CvB")),
                               Jet_btagUParTAK4CvL(TTreeReaderArray<Float_t>(*fReader, "Jet_btagUParTAK4CvL")),
                               Jet_btagUParTAK4CvNotB(TTreeReaderArray<Float_t>(*fReader, "Jet_btagUParTAK4CvNotB")),
                               Jet_btagUParTAK4Ele(TTreeReaderArray<Float_t>(*fReader, "Jet_btagUParTAK4Ele")),
                               Jet_btagUParTAK4Mu(TTreeReaderArray<Float_t>(*fReader, "Jet_btagUParTAK4Mu")),
                               Jet_btagUParTAK4QvG(TTreeReaderArray<Float_t>(*fReader, "Jet_btagUParTAK4QvG")),
                               Jet_btagUParTAK4SvCB(TTreeReaderArray<Float_t>(*fReader, "Jet_btagUParTAK4SvCB")),
                               Jet_btagUParTAK4SvUDG(TTreeReaderArray<Float_t>(*fReader, "Jet_btagUParTAK4SvUDG")),
                               Jet_btagUParTAK4TauVJet(TTreeReaderArray<Float_t>(*fReader, "Jet_btagUParTAK4TauVJet")),
                               Jet_btagUParTAK4UDG(TTreeReaderArray<Float_t>(*fReader, "Jet_btagUParTAK4UDG")),
                               Jet_btagUParTAK4probb(TTreeReaderArray<Float_t>(*fReader, "Jet_btagUParTAK4probb")),
                               Jet_btagUParTAK4probbb(TTreeReaderArray<Float_t>(*fReader, "Jet_btagUParTAK4probbb")),
                               Jet_chEmEF(TTreeReaderArray<Float_t>(*fReader, "Jet_chEmEF")),
                               Jet_chHEF(TTreeReaderArray<Float_t>(*fReader, "Jet_chHEF")),
                               Jet_chMultiplicity(TTreeReaderArray<UChar_t>(*fReader, "Jet_chMultiplicity")),
                               Jet_electronIdx1(TTreeReaderArray<Short_t>(*fReader, "Jet_electronIdx1")),
                               Jet_electronIdx2(TTreeReaderArray<Short_t>(*fReader, "Jet_electronIdx2")),
                               Jet_eta(TTreeReaderArray<Float_t>(*fReader, "Jet_eta")),
                               Jet_genJetIdx(TTreeReaderArray<Short_t>(*fReader, "Jet_genJetIdx")),
                               Jet_hadronFlavour(TTreeReaderArray<UChar_t>(*fReader, "Jet_hadronFlavour")),
                               Jet_hfEmEF(TTreeReaderArray<Float_t>(*fReader, "Jet_hfEmEF")),
                               Jet_hfHEF(TTreeReaderArray<Float_t>(*fReader, "Jet_hfHEF")),
                               Jet_hfadjacentEtaStripsSize(TTreeReaderArray<Int_t>(*fReader, "Jet_hfadjacentEtaStripsSize")),
                               Jet_hfcentralEtaStripSize(TTreeReaderArray<Int_t>(*fReader, "Jet_hfcentralEtaStripSize")),
                               Jet_hfsigmaEtaEta(TTreeReaderArray<Float_t>(*fReader, "Jet_hfsigmaEtaEta")),
                               Jet_hfsigmaPhiPhi(TTreeReaderArray<Float_t>(*fReader, "Jet_hfsigmaPhiPhi")),
                               Jet_mass(TTreeReaderArray<Float_t>(*fReader, "Jet_mass")),
                               Jet_muEF(TTreeReaderArray<Float_t>(*fReader, "Jet_muEF")),
                               Jet_muonIdx1(TTreeReaderArray<Short_t>(*fReader, "Jet_muonIdx1")),
                               Jet_muonIdx2(TTreeReaderArray<Short_t>(*fReader, "Jet_muonIdx2")),
                               Jet_muonSubtrDeltaEta(TTreeReaderArray<Float_t>(*fReader, "Jet_muonSubtrDeltaEta")),
                               Jet_muonSubtrDeltaPhi(TTreeReaderArray<Float_t>(*fReader, "Jet_muonSubtrDeltaPhi")),
                               Jet_muonSubtrFactor(TTreeReaderArray<Float_t>(*fReader, "Jet_muonSubtrFactor")),
                               Jet_nConstituents(TTreeReaderArray<UChar_t>(*fReader, "Jet_nConstituents")),
                               Jet_nElectrons(TTreeReaderArray<UChar_t>(*fReader, "Jet_nElectrons")),
                               Jet_nMuons(TTreeReaderArray<UChar_t>(*fReader, "Jet_nMuons")),
                               Jet_nSVs(TTreeReaderArray<UChar_t>(*fReader, "Jet_nSVs")),
                               Jet_neEmEF(TTreeReaderArray<Float_t>(*fReader, "Jet_neEmEF")),
                               Jet_neHEF(TTreeReaderArray<Float_t>(*fReader, "Jet_neHEF")),
                               Jet_neMultiplicity(TTreeReaderArray<UChar_t>(*fReader, "Jet_neMultiplicity")),
                               Jet_partonFlavour(TTreeReaderArray<Short_t>(*fReader, "Jet_partonFlavour")),
                               Jet_phi(TTreeReaderArray<Float_t>(*fReader, "Jet_phi")),
                               Jet_pt(TTreeReaderArray<Float_t>(*fReader, "Jet_pt")),
                               Jet_puIdDisc(TTreeReaderArray<Float_t>(*fReader, "Jet_puIdDisc")),
                               Jet_rawFactor(TTreeReaderArray<Float_t>(*fReader, "Jet_rawFactor")),
                               Jet_svIdx1(TTreeReaderArray<Short_t>(*fReader, "Jet_svIdx1")),
                               Jet_svIdx2(TTreeReaderArray<Short_t>(*fReader, "Jet_svIdx2")),
                               nJet_Ptr(TTreeReaderValue<Int_t>(*fReader, "nJet")),
                               nJet(*nJet_Ptr),

                               // LHE
                               LHE_AlphaS_Ptr(TTreeReaderValue<Float_t>(*fReader, "LHE_AlphaS")),
                               LHE_AlphaS(*LHE_AlphaS_Ptr),
                               LHE_HT_Ptr(TTreeReaderValue<Float_t>(*fReader, "LHE_HT")),
                               LHE_HT(*LHE_HT_Ptr),
                               LHE_HTIncoming_Ptr(TTreeReaderValue<Float_t>(*fReader, "LHE_HTIncoming")),
                               LHE_HTIncoming(*LHE_HTIncoming_Ptr),
                               LHE_Nb_Ptr(TTreeReaderValue<UChar_t>(*fReader, "LHE_Nb")),
                               LHE_Nb(*LHE_Nb_Ptr),
                               LHE_Nc_Ptr(TTreeReaderValue<UChar_t>(*fReader, "LHE_Nc")),
                               LHE_Nc(*LHE_Nc_Ptr),
                               LHE_Nglu_Ptr(TTreeReaderValue<UChar_t>(*fReader, "LHE_Nglu")),
                               LHE_Nglu(*LHE_Nglu_Ptr),
                               LHE_Njets_Ptr(TTreeReaderValue<UChar_t>(*fReader, "LHE_Njets")),
                               LHE_Njets(*LHE_Njets_Ptr),
                               LHE_NpLO_Ptr(TTreeReaderValue<UChar_t>(*fReader, "LHE_NpLO")),
                               LHE_NpLO(*LHE_NpLO_Ptr),
                               LHE_NpNLO_Ptr(TTreeReaderValue<UChar_t>(*fReader, "LHE_NpNLO")),
                               LHE_NpNLO(*LHE_NpNLO_Ptr),
                               LHE_Nuds_Ptr(TTreeReaderValue<UChar_t>(*fReader, "LHE_Nuds")),
                               LHE_Nuds(*LHE_Nuds_Ptr),
                               LHE_Vpt_Ptr(TTreeReaderValue<Float_t>(*fReader, "LHE_Vpt")),
                               LHE_Vpt(*LHE_Vpt_Ptr),

                               // LHEPart
                               LHEPart_eta(TTreeReaderArray<Float_t>(*fReader, "LHEPart_eta")),
                               LHEPart_firstMotherIdx(TTreeReaderArray<Short_t>(*fReader, "LHEPart_firstMotherIdx")),
                               LHEPart_incomingpz(TTreeReaderArray<Float_t>(*fReader, "LHEPart_incomingpz")),
                               LHEPart_lastMotherIdx(TTreeReaderArray<Short_t>(*fReader, "LHEPart_lastMotherIdx")),
                               LHEPart_mass(TTreeReaderArray<Float_t>(*fReader, "LHEPart_mass")),
                               LHEPart_pdgId(TTreeReaderArray<Int_t>(*fReader, "LHEPart_pdgId")),
                               LHEPart_phi(TTreeReaderArray<Float_t>(*fReader, "LHEPart_phi")),
                               LHEPart_pt(TTreeReaderArray<Float_t>(*fReader, "LHEPart_pt")),
                               LHEPart_spin(TTreeReaderArray<Int_t>(*fReader, "LHEPart_spin")),
                               LHEPart_status(TTreeReaderArray<Int_t>(*fReader, "LHEPart_status")),
                               nLHEPart_Ptr(TTreeReaderValue<Int_t>(*fReader, "nLHEPart")),
                               nLHEPart(*nLHEPart_Ptr),

                               // LHEPdfWeight
                               LHEPdfWeight(TTreeReaderArray<Float_t>(*fReader, "LHEPdfWeight")),
                               nLHEPdfWeight_Ptr(TTreeReaderValue<Int_t>(*fReader, "nLHEPdfWeight")),
                               nLHEPdfWeight(*nLHEPdfWeight_Ptr),

                               // LHEReweightingWeight
                               LHEReweightingWeight(TTreeReaderArray<Float_t>(*fReader, "LHEReweightingWeight")),
                               nLHEReweightingWeight_Ptr(TTreeReaderValue<Int_t>(*fReader, "nLHEReweightingWeight")),
                               nLHEReweightingWeight(*nLHEReweightingWeight_Ptr),

                               // LHEScaleWeight
                               LHEScaleWeight(TTreeReaderArray<Float_t>(*fReader, "LHEScaleWeight")),
                               nLHEScaleWeight_Ptr(TTreeReaderValue<Int_t>(*fReader, "nLHEScaleWeight")),
                               nLHEScaleWeight(*nLHEScaleWeight_Ptr),

                               // LHEWeight
                               LHEWeight_originalXWGTUP_Ptr(TTreeReaderValue<Float_t>(*fReader, "LHEWeight_originalXWGTUP")),
                               LHEWeight_originalXWGTUP(*LHEWeight_originalXWGTUP_Ptr),

                               // LowPtElectron
                               LowPtElectron_ID(TTreeReaderArray<Float_t>(*fReader, "LowPtElectron_ID")),
                               LowPtElectron_charge(TTreeReaderArray<Int_t>(*fReader, "LowPtElectron_charge")),
                               LowPtElectron_convVeto(TTreeReaderArray<Bool_t>(*fReader, "LowPtElectron_convVeto")),
                               LowPtElectron_convVtxRadius(TTreeReaderArray<Float_t>(*fReader, "LowPtElectron_convVtxRadius")),
                               LowPtElectron_convWP(TTreeReaderArray<UChar_t>(*fReader, "LowPtElectron_convWP")),
                               LowPtElectron_deltaEtaSC(TTreeReaderArray<Float_t>(*fReader, "LowPtElectron_deltaEtaSC")),
                               LowPtElectron_dxy(TTreeReaderArray<Float_t>(*fReader, "LowPtElectron_dxy")),
                               LowPtElectron_dxyErr(TTreeReaderArray<Float_t>(*fReader, "LowPtElectron_dxyErr")),
                               LowPtElectron_dz(TTreeReaderArray<Float_t>(*fReader, "LowPtElectron_dz")),
                               LowPtElectron_dzErr(TTreeReaderArray<Float_t>(*fReader, "LowPtElectron_dzErr")),
                               LowPtElectron_eInvMinusPInv(TTreeReaderArray<Float_t>(*fReader, "LowPtElectron_eInvMinusPInv")),
                               LowPtElectron_electronIdx(TTreeReaderArray<Short_t>(*fReader, "LowPtElectron_electronIdx")),
                               LowPtElectron_energyErr(TTreeReaderArray<Float_t>(*fReader, "LowPtElectron_energyErr")),
                               LowPtElectron_eta(TTreeReaderArray<Float_t>(*fReader, "LowPtElectron_eta")),
                               LowPtElectron_genPartFlav(TTreeReaderArray<UChar_t>(*fReader, "LowPtElectron_genPartFlav")),
                               LowPtElectron_genPartIdx(TTreeReaderArray<Short_t>(*fReader, "LowPtElectron_genPartIdx")),
                               LowPtElectron_hoe(TTreeReaderArray<Float_t>(*fReader, "LowPtElectron_hoe")),
                               LowPtElectron_lostHits(TTreeReaderArray<UChar_t>(*fReader, "LowPtElectron_lostHits")),
                               LowPtElectron_mass(TTreeReaderArray<Float_t>(*fReader, "LowPtElectron_mass")),
                               LowPtElectron_miniPFRelIso_all(TTreeReaderArray<Float_t>(*fReader, "LowPtElectron_miniPFRelIso_all")),
                               LowPtElectron_miniPFRelIso_chg(TTreeReaderArray<Float_t>(*fReader, "LowPtElectron_miniPFRelIso_chg")),
                               LowPtElectron_pdgId(TTreeReaderArray<Int_t>(*fReader, "LowPtElectron_pdgId")),
                               LowPtElectron_phi(TTreeReaderArray<Float_t>(*fReader, "LowPtElectron_phi")),
                               LowPtElectron_photonIdx(TTreeReaderArray<Short_t>(*fReader, "LowPtElectron_photonIdx")),
                               LowPtElectron_pt(TTreeReaderArray<Float_t>(*fReader, "LowPtElectron_pt")),
                               LowPtElectron_ptbiased(TTreeReaderArray<Float_t>(*fReader, "LowPtElectron_ptbiased")),
                               LowPtElectron_r9(TTreeReaderArray<Float_t>(*fReader, "LowPtElectron_r9")),
                               LowPtElectron_scEtOverPt(TTreeReaderArray<Float_t>(*fReader, "LowPtElectron_scEtOverPt")),
                               LowPtElectron_sieie(TTreeReaderArray<Float_t>(*fReader, "LowPtElectron_sieie")),
                               LowPtElectron_unbiased(TTreeReaderArray<Float_t>(*fReader, "LowPtElectron_unbiased")),
                               nLowPtElectron_Ptr(TTreeReaderValue<Int_t>(*fReader, "nLowPtElectron")),
                               nLowPtElectron(*nLowPtElectron_Ptr),

                               // Muon
                               Muon_IPx(TTreeReaderArray<Float_t>(*fReader, "Muon_IPx")),
                               Muon_IPy(TTreeReaderArray<Float_t>(*fReader, "Muon_IPy")),
                               Muon_IPz(TTreeReaderArray<Float_t>(*fReader, "Muon_IPz")),
                               Muon_VXBS_Cov00(TTreeReaderArray<Float_t>(*fReader, "Muon_VXBS_Cov00")),
                               Muon_VXBS_Cov03(TTreeReaderArray<Float_t>(*fReader, "Muon_VXBS_Cov03")),
                               Muon_VXBS_Cov33(TTreeReaderArray<Float_t>(*fReader, "Muon_VXBS_Cov33")),
                               Muon_bestTrackType(TTreeReaderArray<UChar_t>(*fReader, "Muon_bestTrackType")),
                               Muon_bsConstrainedChi2(TTreeReaderArray<Float_t>(*fReader, "Muon_bsConstrainedChi2")),
                               Muon_bsConstrainedPt(TTreeReaderArray<Float_t>(*fReader, "Muon_bsConstrainedPt")),
                               Muon_bsConstrainedPtErr(TTreeReaderArray<Float_t>(*fReader, "Muon_bsConstrainedPtErr")),
                               Muon_charge(TTreeReaderArray<Int_t>(*fReader, "Muon_charge")),
                               Muon_dxy(TTreeReaderArray<Float_t>(*fReader, "Muon_dxy")),
                               Muon_dxyErr(TTreeReaderArray<Float_t>(*fReader, "Muon_dxyErr")),
                               Muon_dxybs(TTreeReaderArray<Float_t>(*fReader, "Muon_dxybs")),
                               Muon_dxybsErr(TTreeReaderArray<Float_t>(*fReader, "Muon_dxybsErr")),
                               Muon_dz(TTreeReaderArray<Float_t>(*fReader, "Muon_dz")),
                               Muon_dzErr(TTreeReaderArray<Float_t>(*fReader, "Muon_dzErr")),
                               Muon_eta(TTreeReaderArray<Float_t>(*fReader, "Muon_eta")),
                               Muon_fsrPhotonIdx(TTreeReaderArray<Short_t>(*fReader, "Muon_fsrPhotonIdx")),
                               Muon_genPartFlav(TTreeReaderArray<UChar_t>(*fReader, "Muon_genPartFlav")),
                               Muon_genPartIdx(TTreeReaderArray<Short_t>(*fReader, "Muon_genPartIdx")),
                               Muon_highPtId(TTreeReaderArray<UChar_t>(*fReader, "Muon_highPtId")),
                               Muon_highPurity(TTreeReaderArray<Bool_t>(*fReader, "Muon_highPurity")),
                               Muon_inTimeMuon(TTreeReaderArray<Bool_t>(*fReader, "Muon_inTimeMuon")),
                               Muon_ip3d(TTreeReaderArray<Float_t>(*fReader, "Muon_ip3d")),
                               Muon_ipLengthSig(TTreeReaderArray<Float_t>(*fReader, "Muon_ipLengthSig")),
                               Muon_isGlobal(TTreeReaderArray<Bool_t>(*fReader, "Muon_isGlobal")),
                               Muon_isPFcand(TTreeReaderArray<Bool_t>(*fReader, "Muon_isPFcand")),
                               Muon_isStandalone(TTreeReaderArray<Bool_t>(*fReader, "Muon_isStandalone")),
                               Muon_isTracker(TTreeReaderArray<Bool_t>(*fReader, "Muon_isTracker")),
                               Muon_jetDF(TTreeReaderArray<Float_t>(*fReader, "Muon_jetDF")),
                               Muon_jetIdx(TTreeReaderArray<Short_t>(*fReader, "Muon_jetIdx")),
                               Muon_jetNDauCharged(TTreeReaderArray<UChar_t>(*fReader, "Muon_jetNDauCharged")),
                               Muon_jetPtRelv2(TTreeReaderArray<Float_t>(*fReader, "Muon_jetPtRelv2")),
                               Muon_jetRelIso(TTreeReaderArray<Float_t>(*fReader, "Muon_jetRelIso")),
                               Muon_looseId(TTreeReaderArray<Bool_t>(*fReader, "Muon_looseId")),
                               Muon_mass(TTreeReaderArray<Float_t>(*fReader, "Muon_mass")),
                               Muon_mediumId(TTreeReaderArray<Bool_t>(*fReader, "Muon_mediumId")),
                               Muon_mediumPromptId(TTreeReaderArray<Bool_t>(*fReader, "Muon_mediumPromptId")),
                               Muon_miniIsoId(TTreeReaderArray<UChar_t>(*fReader, "Muon_miniIsoId")),
                               Muon_miniPFRelIso_all(TTreeReaderArray<Float_t>(*fReader, "Muon_miniPFRelIso_all")),
                               Muon_miniPFRelIso_chg(TTreeReaderArray<Float_t>(*fReader, "Muon_miniPFRelIso_chg")),
                               Muon_multiIsoId(TTreeReaderArray<UChar_t>(*fReader, "Muon_multiIsoId")),
                               Muon_mvaLowPt(TTreeReaderArray<Float_t>(*fReader, "Muon_mvaLowPt")),
                               Muon_mvaMuID(TTreeReaderArray<Float_t>(*fReader, "Muon_mvaMuID")),
                               Muon_mvaMuID_WP(TTreeReaderArray<UChar_t>(*fReader, "Muon_mvaMuID_WP")),
                               Muon_nStations(TTreeReaderArray<UChar_t>(*fReader, "Muon_nStations")),
                               Muon_nTrackerLayers(TTreeReaderArray<UChar_t>(*fReader, "Muon_nTrackerLayers")),
                               Muon_pdgId(TTreeReaderArray<Int_t>(*fReader, "Muon_pdgId")),
                               Muon_pfIsoId(TTreeReaderArray<UChar_t>(*fReader, "Muon_pfIsoId")),
                               Muon_pfRelIso03_all(TTreeReaderArray<Float_t>(*fReader, "Muon_pfRelIso03_all")),
                               Muon_pfRelIso03_chg(TTreeReaderArray<Float_t>(*fReader, "Muon_pfRelIso03_chg")),
                               Muon_pfRelIso04_all(TTreeReaderArray<Float_t>(*fReader, "Muon_pfRelIso04_all")),
                               Muon_phi(TTreeReaderArray<Float_t>(*fReader, "Muon_phi")),
                               Muon_pnScore_heavy(TTreeReaderArray<Float_t>(*fReader, "Muon_pnScore_heavy")),
                               Muon_pnScore_light(TTreeReaderArray<Float_t>(*fReader, "Muon_pnScore_light")),
                               Muon_pnScore_prompt(TTreeReaderArray<Float_t>(*fReader, "Muon_pnScore_prompt")),
                               Muon_pnScore_tau(TTreeReaderArray<Float_t>(*fReader, "Muon_pnScore_tau")),
                               Muon_promptMVA(TTreeReaderArray<Float_t>(*fReader, "Muon_promptMVA")),
                               Muon_pt(TTreeReaderArray<Float_t>(*fReader, "Muon_pt")),
                               Muon_ptErr(TTreeReaderArray<Float_t>(*fReader, "Muon_ptErr")),
                               Muon_puppiIsoId(TTreeReaderArray<UChar_t>(*fReader, "Muon_puppiIsoId")),
                               Muon_segmentComp(TTreeReaderArray<Float_t>(*fReader, "Muon_segmentComp")),
                               Muon_sip3d(TTreeReaderArray<Float_t>(*fReader, "Muon_sip3d")),
                               Muon_softId(TTreeReaderArray<Bool_t>(*fReader, "Muon_softId")),
                               Muon_softMva(TTreeReaderArray<Float_t>(*fReader, "Muon_softMva")),
                               Muon_softMvaId(TTreeReaderArray<Bool_t>(*fReader, "Muon_softMvaId")),
                               Muon_softMvaRun3(TTreeReaderArray<Float_t>(*fReader, "Muon_softMvaRun3")),
                               Muon_svIdx(TTreeReaderArray<Short_t>(*fReader, "Muon_svIdx")),
                               Muon_tightCharge(TTreeReaderArray<UChar_t>(*fReader, "Muon_tightCharge")),
                               Muon_tightId(TTreeReaderArray<Bool_t>(*fReader, "Muon_tightId")),
                               Muon_tkIsoId(TTreeReaderArray<UChar_t>(*fReader, "Muon_tkIsoId")),
                               Muon_tkRelIso(TTreeReaderArray<Float_t>(*fReader, "Muon_tkRelIso")),
                               Muon_triggerIdLoose(TTreeReaderArray<Bool_t>(*fReader, "Muon_triggerIdLoose")),
                               Muon_tuneP_charge(TTreeReaderArray<Float_t>(*fReader, "Muon_tuneP_charge")),
                               Muon_tuneP_pterr(TTreeReaderArray<Float_t>(*fReader, "Muon_tuneP_pterr")),
                               Muon_tunepRelPt(TTreeReaderArray<Float_t>(*fReader, "Muon_tunepRelPt")),
                               nMuon_Ptr(TTreeReaderValue<Int_t>(*fReader, "nMuon")),
                               nMuon(*nMuon_Ptr),

                               // OtherPV
                               OtherPV_score(TTreeReaderArray<Float_t>(*fReader, "OtherPV_score")),
                               OtherPV_z(TTreeReaderArray<Float_t>(*fReader, "OtherPV_z")),
                               nOtherPV_Ptr(TTreeReaderValue<Int_t>(*fReader, "nOtherPV")),
                               nOtherPV(*nOtherPV_Ptr),

                               // PFCand
                               PFCand_eta(TTreeReaderArray<Float_t>(*fReader, "PFCand_eta")),
                               PFCand_mass(TTreeReaderArray<Float_t>(*fReader, "PFCand_mass")),
                               PFCand_pdgId(TTreeReaderArray<Int_t>(*fReader, "PFCand_pdgId")),
                               PFCand_phi(TTreeReaderArray<Float_t>(*fReader, "PFCand_phi")),
                               PFCand_pt(TTreeReaderArray<Float_t>(*fReader, "PFCand_pt")),
                               nPFCand_Ptr(TTreeReaderValue<Int_t>(*fReader, "nPFCand")),
                               nPFCand(*nPFCand_Ptr),

                               // PFMET
                               PFMET_covXX_Ptr(TTreeReaderValue<Float_t>(*fReader, "PFMET_covXX")),
                               PFMET_covXX(*PFMET_covXX_Ptr),
                               PFMET_covXY_Ptr(TTreeReaderValue<Float_t>(*fReader, "PFMET_covXY")),
                               PFMET_covXY(*PFMET_covXY_Ptr),
                               PFMET_covYY_Ptr(TTreeReaderValue<Float_t>(*fReader, "PFMET_covYY")),
                               PFMET_covYY(*PFMET_covYY_Ptr),
                               PFMET_phi_Ptr(TTreeReaderValue<Float_t>(*fReader, "PFMET_phi")),
                               PFMET_phi(*PFMET_phi_Ptr),
                               PFMET_phiUnclusteredDown_Ptr(TTreeReaderValue<Float_t>(*fReader, "PFMET_phiUnclusteredDown")),
                               PFMET_phiUnclusteredDown(*PFMET_phiUnclusteredDown_Ptr),
                               PFMET_phiUnclusteredUp_Ptr(TTreeReaderValue<Float_t>(*fReader, "PFMET_phiUnclusteredUp")),
                               PFMET_phiUnclusteredUp(*PFMET_phiUnclusteredUp_Ptr),
                               PFMET_pt_Ptr(TTreeReaderValue<Float_t>(*fReader, "PFMET_pt")),
                               PFMET_pt(*PFMET_pt_Ptr),
                               PFMET_ptUnclusteredDown_Ptr(TTreeReaderValue<Float_t>(*fReader, "PFMET_ptUnclusteredDown")),
                               PFMET_ptUnclusteredDown(*PFMET_ptUnclusteredDown_Ptr),
                               PFMET_ptUnclusteredUp_Ptr(TTreeReaderValue<Float_t>(*fReader, "PFMET_ptUnclusteredUp")),
                               PFMET_ptUnclusteredUp(*PFMET_ptUnclusteredUp_Ptr),
                               PFMET_significance_Ptr(TTreeReaderValue<Float_t>(*fReader, "PFMET_significance")),
                               PFMET_significance(*PFMET_significance_Ptr),
                               PFMET_sumEt_Ptr(TTreeReaderValue<Float_t>(*fReader, "PFMET_sumEt")),
                               PFMET_sumEt(*PFMET_sumEt_Ptr),
                               PFMET_sumPtUnclustered_Ptr(TTreeReaderValue<Float_t>(*fReader, "PFMET_sumPtUnclustered")),
                               PFMET_sumPtUnclustered(*PFMET_sumPtUnclustered_Ptr),

                               // PSWeight
                               PSWeight(TTreeReaderArray<Float_t>(*fReader, "PSWeight")),
                               nPSWeight_Ptr(TTreeReaderValue<Int_t>(*fReader, "nPSWeight")),
                               nPSWeight(*nPSWeight_Ptr),

                               // PV
                               PV_chi2_Ptr(TTreeReaderValue<Float_t>(*fReader, "PV_chi2")),
                               PV_chi2(*PV_chi2_Ptr),
                               PV_ndof_Ptr(TTreeReaderValue<Float_t>(*fReader, "PV_ndof")),
                               PV_ndof(*PV_ndof_Ptr),
                               PV_npvs_Ptr(TTreeReaderValue<UChar_t>(*fReader, "PV_npvs")),
                               PV_npvs(*PV_npvs_Ptr),
                               PV_npvsGood_Ptr(TTreeReaderValue<UChar_t>(*fReader, "PV_npvsGood")),
                               PV_npvsGood(*PV_npvsGood_Ptr),
                               PV_score_Ptr(TTreeReaderValue<Float_t>(*fReader, "PV_score")),
                               PV_score(*PV_score_Ptr),
                               PV_sumpt2_Ptr(TTreeReaderValue<Float_t>(*fReader, "PV_sumpt2")),
                               PV_sumpt2(*PV_sumpt2_Ptr),
                               PV_sumpx_Ptr(TTreeReaderValue<Float_t>(*fReader, "PV_sumpx")),
                               PV_sumpx(*PV_sumpx_Ptr),
                               PV_sumpy_Ptr(TTreeReaderValue<Float_t>(*fReader, "PV_sumpy")),
                               PV_sumpy(*PV_sumpy_Ptr),
                               PV_x_Ptr(TTreeReaderValue<Float_t>(*fReader, "PV_x")),
                               PV_x(*PV_x_Ptr),
                               PV_y_Ptr(TTreeReaderValue<Float_t>(*fReader, "PV_y")),
                               PV_y(*PV_y_Ptr),
                               PV_z_Ptr(TTreeReaderValue<Float_t>(*fReader, "PV_z")),
                               PV_z(*PV_z_Ptr),

                               // Photon
                               Photon_cutBased(TTreeReaderArray<UChar_t>(*fReader, "Photon_cutBased")),
                               Photon_ecalPFClusterIso(TTreeReaderArray<Float_t>(*fReader, "Photon_ecalPFClusterIso")),
                               Photon_electronIdx(TTreeReaderArray<Short_t>(*fReader, "Photon_electronIdx")),
                               Photon_electronVeto(TTreeReaderArray<Bool_t>(*fReader, "Photon_electronVeto")),
                               Photon_energyErr(TTreeReaderArray<Float_t>(*fReader, "Photon_energyErr")),
                               Photon_energyRaw(TTreeReaderArray<Float_t>(*fReader, "Photon_energyRaw")),
                               Photon_esEffSigmaRR(TTreeReaderArray<Float_t>(*fReader, "Photon_esEffSigmaRR")),
                               Photon_esEnergyOverRawE(TTreeReaderArray<Float_t>(*fReader, "Photon_esEnergyOverRawE")),
                               Photon_eta(TTreeReaderArray<Float_t>(*fReader, "Photon_eta")),
                               Photon_etaWidth(TTreeReaderArray<Float_t>(*fReader, "Photon_etaWidth")),
                               Photon_genPartFlav(TTreeReaderArray<UChar_t>(*fReader, "Photon_genPartFlav")),
                               Photon_genPartIdx(TTreeReaderArray<Short_t>(*fReader, "Photon_genPartIdx")),
                               Photon_haloTaggerMVAVal(TTreeReaderArray<Float_t>(*fReader, "Photon_haloTaggerMVAVal")),
                               Photon_hasConversionTracks(TTreeReaderArray<Bool_t>(*fReader, "Photon_hasConversionTracks")),
                               Photon_hcalPFClusterIso(TTreeReaderArray<Float_t>(*fReader, "Photon_hcalPFClusterIso")),
                               Photon_hoe(TTreeReaderArray<Float_t>(*fReader, "Photon_hoe")),
                               Photon_hoe_PUcorr(TTreeReaderArray<Float_t>(*fReader, "Photon_hoe_PUcorr")),
                               Photon_hoe_Tower(TTreeReaderArray<Float_t>(*fReader, "Photon_hoe_Tower")),
                               Photon_isScEtaEB(TTreeReaderArray<Bool_t>(*fReader, "Photon_isScEtaEB")),
                               Photon_isScEtaEE(TTreeReaderArray<Bool_t>(*fReader, "Photon_isScEtaEE")),
                               Photon_jetIdx(TTreeReaderArray<Short_t>(*fReader, "Photon_jetIdx")),
                               Photon_mvaID(TTreeReaderArray<Float_t>(*fReader, "Photon_mvaID")),
                               Photon_mvaID_WP80(TTreeReaderArray<Bool_t>(*fReader, "Photon_mvaID_WP80")),
                               Photon_mvaID_WP90(TTreeReaderArray<Bool_t>(*fReader, "Photon_mvaID_WP90")),
                               Photon_pfChargedIso(TTreeReaderArray<Float_t>(*fReader, "Photon_pfChargedIso")),
                               Photon_pfChargedIsoPFPV(TTreeReaderArray<Float_t>(*fReader, "Photon_pfChargedIsoPFPV")),
                               Photon_pfChargedIsoWorstVtx(TTreeReaderArray<Float_t>(*fReader, "Photon_pfChargedIsoWorstVtx")),
                               Photon_pfPhoIso03(TTreeReaderArray<Float_t>(*fReader, "Photon_pfPhoIso03")),
                               Photon_pfRelIso03_all_quadratic(TTreeReaderArray<Float_t>(*fReader, "Photon_pfRelIso03_all_quadratic")),
                               Photon_pfRelIso03_chg_quadratic(TTreeReaderArray<Float_t>(*fReader, "Photon_pfRelIso03_chg_quadratic")),
                               Photon_phi(TTreeReaderArray<Float_t>(*fReader, "Photon_phi")),
                               Photon_phiWidth(TTreeReaderArray<Float_t>(*fReader, "Photon_phiWidth")),
                               Photon_pixelSeed(TTreeReaderArray<Bool_t>(*fReader, "Photon_pixelSeed")),
                               Photon_pt(TTreeReaderArray<Float_t>(*fReader, "Photon_pt")),
                               Photon_r9(TTreeReaderArray<Float_t>(*fReader, "Photon_r9")),
                               Photon_s4(TTreeReaderArray<Float_t>(*fReader, "Photon_s4")),
                               Photon_seedGain(TTreeReaderArray<UChar_t>(*fReader, "Photon_seedGain")),
                               Photon_seediEtaOriX(TTreeReaderArray<Short_t>(*fReader, "Photon_seediEtaOriX")),
                               Photon_seediPhiOriY(TTreeReaderArray<Short_t>(*fReader, "Photon_seediPhiOriY")),
                               Photon_sieie(TTreeReaderArray<Float_t>(*fReader, "Photon_sieie")),
                               Photon_sieip(TTreeReaderArray<Float_t>(*fReader, "Photon_sieip")),
                               Photon_sipip(TTreeReaderArray<Float_t>(*fReader, "Photon_sipip")),
                               Photon_superclusterEta(TTreeReaderArray<Float_t>(*fReader, "Photon_superclusterEta")),
                               Photon_trkSumPtHollowConeDR03(TTreeReaderArray<Float_t>(*fReader, "Photon_trkSumPtHollowConeDR03")),
                               Photon_trkSumPtSolidConeDR04(TTreeReaderArray<Float_t>(*fReader, "Photon_trkSumPtSolidConeDR04")),
                               Photon_vidNestedWPBitmap(TTreeReaderArray<Int_t>(*fReader, "Photon_vidNestedWPBitmap")),
                               Photon_x_calo(TTreeReaderArray<Float_t>(*fReader, "Photon_x_calo")),
                               Photon_y_calo(TTreeReaderArray<Float_t>(*fReader, "Photon_y_calo")),
                               Photon_z_calo(TTreeReaderArray<Float_t>(*fReader, "Photon_z_calo")),
                               nPhoton_Ptr(TTreeReaderValue<Int_t>(*fReader, "nPhoton")),
                               nPhoton(*nPhoton_Ptr),

                               // Pileup
                               Pileup_gpudensity_Ptr(TTreeReaderValue<Float_t>(*fReader, "Pileup_gpudensity")),
                               Pileup_gpudensity(*Pileup_gpudensity_Ptr),
                               Pileup_nPU_Ptr(TTreeReaderValue<Int_t>(*fReader, "Pileup_nPU")),
                               Pileup_nPU(*Pileup_nPU_Ptr),
                               Pileup_nTrueInt_Ptr(TTreeReaderValue<Float_t>(*fReader, "Pileup_nTrueInt")),
                               Pileup_nTrueInt(*Pileup_nTrueInt_Ptr),
                               Pileup_pthatmax_Ptr(TTreeReaderValue<Float_t>(*fReader, "Pileup_pthatmax")),
                               Pileup_pthatmax(*Pileup_pthatmax_Ptr),
                               Pileup_pudensity_Ptr(TTreeReaderValue<Float_t>(*fReader, "Pileup_pudensity")),
                               Pileup_pudensity(*Pileup_pudensity_Ptr),
                               Pileup_sumEOOT_Ptr(TTreeReaderValue<Int_t>(*fReader, "Pileup_sumEOOT")),
                               Pileup_sumEOOT(*Pileup_sumEOOT_Ptr),
                               Pileup_sumLOOT_Ptr(TTreeReaderValue<Int_t>(*fReader, "Pileup_sumLOOT")),
                               Pileup_sumLOOT(*Pileup_sumLOOT_Ptr),

                               // PuppiMET
                               PuppiMET_covXX_Ptr(TTreeReaderValue<Float_t>(*fReader, "PuppiMET_covXX")),
                               PuppiMET_covXX(*PuppiMET_covXX_Ptr),
                               PuppiMET_covXY_Ptr(TTreeReaderValue<Float_t>(*fReader, "PuppiMET_covXY")),
                               PuppiMET_covXY(*PuppiMET_covXY_Ptr),
                               PuppiMET_covYY_Ptr(TTreeReaderValue<Float_t>(*fReader, "PuppiMET_covYY")),
                               PuppiMET_covYY(*PuppiMET_covYY_Ptr),
                               PuppiMET_phi_Ptr(TTreeReaderValue<Float_t>(*fReader, "PuppiMET_phi")),
                               PuppiMET_phi(*PuppiMET_phi_Ptr),
                               PuppiMET_phiUnclusteredDown_Ptr(TTreeReaderValue<Float_t>(*fReader, "PuppiMET_phiUnclusteredDown")),
                               PuppiMET_phiUnclusteredDown(*PuppiMET_phiUnclusteredDown_Ptr),
                               PuppiMET_phiUnclusteredUp_Ptr(TTreeReaderValue<Float_t>(*fReader, "PuppiMET_phiUnclusteredUp")),
                               PuppiMET_phiUnclusteredUp(*PuppiMET_phiUnclusteredUp_Ptr),
                               PuppiMET_pt_Ptr(TTreeReaderValue<Float_t>(*fReader, "PuppiMET_pt")),
                               PuppiMET_pt(*PuppiMET_pt_Ptr),
                               PuppiMET_ptUnclusteredDown_Ptr(TTreeReaderValue<Float_t>(*fReader, "PuppiMET_ptUnclusteredDown")),
                               PuppiMET_ptUnclusteredDown(*PuppiMET_ptUnclusteredDown_Ptr),
                               PuppiMET_ptUnclusteredUp_Ptr(TTreeReaderValue<Float_t>(*fReader, "PuppiMET_ptUnclusteredUp")),
                               PuppiMET_ptUnclusteredUp(*PuppiMET_ptUnclusteredUp_Ptr),
                               PuppiMET_significance_Ptr(TTreeReaderValue<Float_t>(*fReader, "PuppiMET_significance")),
                               PuppiMET_significance(*PuppiMET_significance_Ptr),
                               PuppiMET_sumEt_Ptr(TTreeReaderValue<Float_t>(*fReader, "PuppiMET_sumEt")),
                               PuppiMET_sumEt(*PuppiMET_sumEt_Ptr),
                               PuppiMET_sumPtUnclustered_Ptr(TTreeReaderValue<Float_t>(*fReader, "PuppiMET_sumPtUnclustered")),
                               PuppiMET_sumPtUnclustered(*PuppiMET_sumPtUnclustered_Ptr),

                               // Rho
                               Rho_fixedGridRhoAll_Ptr(TTreeReaderValue<Float_t>(*fReader, "Rho_fixedGridRhoAll")),
                               Rho_fixedGridRhoAll(*Rho_fixedGridRhoAll_Ptr),
                               Rho_fixedGridRhoFastjetAll_Ptr(TTreeReaderValue<Float_t>(*fReader, "Rho_fixedGridRhoFastjetAll")),
                               Rho_fixedGridRhoFastjetAll(*Rho_fixedGridRhoFastjetAll_Ptr),
                               Rho_fixedGridRhoFastjetCentral_Ptr(TTreeReaderValue<Float_t>(*fReader, "Rho_fixedGridRhoFastjetCentral")),
                               Rho_fixedGridRhoFastjetCentral(*Rho_fixedGridRhoFastjetCentral_Ptr),
                               Rho_fixedGridRhoFastjetCentralCalo_Ptr(TTreeReaderValue<Float_t>(*fReader, "Rho_fixedGridRhoFastjetCentralCalo")),
                               Rho_fixedGridRhoFastjetCentralCalo(*Rho_fixedGridRhoFastjetCentralCalo_Ptr),
                               Rho_fixedGridRhoFastjetCentralChargedPileUp_Ptr(TTreeReaderValue<Float_t>(*fReader, "Rho_fixedGridRhoFastjetCentralChargedPileUp")),
                               Rho_fixedGridRhoFastjetCentralChargedPileUp(*Rho_fixedGridRhoFastjetCentralChargedPileUp_Ptr),
                               Rho_fixedGridRhoFastjetCentralNeutral_Ptr(TTreeReaderValue<Float_t>(*fReader, "Rho_fixedGridRhoFastjetCentralNeutral")),
                               Rho_fixedGridRhoFastjetCentralNeutral(*Rho_fixedGridRhoFastjetCentralNeutral_Ptr),

                               // SV
                               SV_charge(TTreeReaderArray<Short_t>(*fReader, "SV_charge")),
                               SV_chi2(TTreeReaderArray<Float_t>(*fReader, "SV_chi2")),
                               SV_dlen(TTreeReaderArray<Float_t>(*fReader, "SV_dlen")),
                               SV_dlenSig(TTreeReaderArray<Float_t>(*fReader, "SV_dlenSig")),
                               SV_dxy(TTreeReaderArray<Float_t>(*fReader, "SV_dxy")),
                               SV_dxySig(TTreeReaderArray<Float_t>(*fReader, "SV_dxySig")),
                               SV_eta(TTreeReaderArray<Float_t>(*fReader, "SV_eta")),
                               SV_mass(TTreeReaderArray<Float_t>(*fReader, "SV_mass")),
                               SV_ndof(TTreeReaderArray<Float_t>(*fReader, "SV_ndof")),
                               SV_ntracks(TTreeReaderArray<UChar_t>(*fReader, "SV_ntracks")),
                               SV_pAngle(TTreeReaderArray<Float_t>(*fReader, "SV_pAngle")),
                               SV_phi(TTreeReaderArray<Float_t>(*fReader, "SV_phi")),
                               SV_pt(TTreeReaderArray<Float_t>(*fReader, "SV_pt")),
                               SV_x(TTreeReaderArray<Float_t>(*fReader, "SV_x")),
                               SV_y(TTreeReaderArray<Float_t>(*fReader, "SV_y")),
                               SV_z(TTreeReaderArray<Float_t>(*fReader, "SV_z")),
                               nSV_Ptr(TTreeReaderValue<Int_t>(*fReader, "nSV")),
                               nSV(*nSV_Ptr),

                               // SubGenJetAK8
                               SubGenJetAK8_eta(TTreeReaderArray<Float_t>(*fReader, "SubGenJetAK8_eta")),
                               SubGenJetAK8_mass(TTreeReaderArray<Float_t>(*fReader, "SubGenJetAK8_mass")),
                               SubGenJetAK8_phi(TTreeReaderArray<Float_t>(*fReader, "SubGenJetAK8_phi")),
                               SubGenJetAK8_pt(TTreeReaderArray<Float_t>(*fReader, "SubGenJetAK8_pt")),
                               nSubGenJetAK8_Ptr(TTreeReaderValue<Int_t>(*fReader, "nSubGenJetAK8")),
                               nSubGenJetAK8(*nSubGenJetAK8_Ptr),

                               // SubJet
                               SubJet_UParTAK4RegPtRawCorr(TTreeReaderArray<Float_t>(*fReader, "SubJet_UParTAK4RegPtRawCorr")),
                               SubJet_UParTAK4RegPtRawCorrNeutrino(TTreeReaderArray<Float_t>(*fReader, "SubJet_UParTAK4RegPtRawCorrNeutrino")),
                               SubJet_UParTAK4RegPtRawRes(TTreeReaderArray<Float_t>(*fReader, "SubJet_UParTAK4RegPtRawRes")),
                               SubJet_UParTAK4V1RegPtRawCorr(TTreeReaderArray<Float_t>(*fReader, "SubJet_UParTAK4V1RegPtRawCorr")),
                               SubJet_UParTAK4V1RegPtRawCorrNeutrino(TTreeReaderArray<Float_t>(*fReader, "SubJet_UParTAK4V1RegPtRawCorrNeutrino")),
                               SubJet_UParTAK4V1RegPtRawRes(TTreeReaderArray<Float_t>(*fReader, "SubJet_UParTAK4V1RegPtRawRes")),
                               SubJet_area(TTreeReaderArray<Float_t>(*fReader, "SubJet_area")),
                               SubJet_btagDeepFlavB(TTreeReaderArray<Float_t>(*fReader, "SubJet_btagDeepFlavB")),
                               SubJet_btagUParTAK4B(TTreeReaderArray<Float_t>(*fReader, "SubJet_btagUParTAK4B")),
                               SubJet_eta(TTreeReaderArray<Float_t>(*fReader, "SubJet_eta")),
                               SubJet_hadronFlavour(TTreeReaderArray<UChar_t>(*fReader, "SubJet_hadronFlavour")),
                               SubJet_mass(TTreeReaderArray<Float_t>(*fReader, "SubJet_mass")),
                               SubJet_n2b1(TTreeReaderArray<Float_t>(*fReader, "SubJet_n2b1")),
                               SubJet_n3b1(TTreeReaderArray<Float_t>(*fReader, "SubJet_n3b1")),
                               SubJet_nBHadrons(TTreeReaderArray<UChar_t>(*fReader, "SubJet_nBHadrons")),
                               SubJet_nCHadrons(TTreeReaderArray<UChar_t>(*fReader, "SubJet_nCHadrons")),
                               SubJet_phi(TTreeReaderArray<Float_t>(*fReader, "SubJet_phi")),
                               SubJet_pt(TTreeReaderArray<Float_t>(*fReader, "SubJet_pt")),
                               SubJet_rawFactor(TTreeReaderArray<Float_t>(*fReader, "SubJet_rawFactor")),
                               SubJet_subGenJetAK8Idx(TTreeReaderArray<Short_t>(*fReader, "SubJet_subGenJetAK8Idx")),
                               SubJet_tau1(TTreeReaderArray<Float_t>(*fReader, "SubJet_tau1")),
                               SubJet_tau2(TTreeReaderArray<Float_t>(*fReader, "SubJet_tau2")),
                               SubJet_tau3(TTreeReaderArray<Float_t>(*fReader, "SubJet_tau3")),
                               SubJet_tau4(TTreeReaderArray<Float_t>(*fReader, "SubJet_tau4")),
                               nSubJet_Ptr(TTreeReaderValue<Int_t>(*fReader, "nSubJet")),
                               nSubJet(*nSubJet_Ptr),

                               // Tau
                               Tau_IPx(TTreeReaderArray<Float_t>(*fReader, "Tau_IPx")),
                               Tau_IPy(TTreeReaderArray<Float_t>(*fReader, "Tau_IPy")),
                               Tau_IPz(TTreeReaderArray<Float_t>(*fReader, "Tau_IPz")),
                               Tau_charge(TTreeReaderArray<Short_t>(*fReader, "Tau_charge")),
                               Tau_chargedIso(TTreeReaderArray<Float_t>(*fReader, "Tau_chargedIso")),
                               Tau_decayMode(TTreeReaderArray<UChar_t>(*fReader, "Tau_decayMode")),
                               Tau_decayModePNet(TTreeReaderArray<Short_t>(*fReader, "Tau_decayModePNet")),
                               Tau_decayModeUParT(TTreeReaderArray<Short_t>(*fReader, "Tau_decayModeUParT")),
                               Tau_dxy(TTreeReaderArray<Float_t>(*fReader, "Tau_dxy")),
                               Tau_dz(TTreeReaderArray<Float_t>(*fReader, "Tau_dz")),
                               Tau_eleIdx(TTreeReaderArray<Short_t>(*fReader, "Tau_eleIdx")),
                               Tau_eta(TTreeReaderArray<Float_t>(*fReader, "Tau_eta")),
                               Tau_genPartFlav(TTreeReaderArray<UChar_t>(*fReader, "Tau_genPartFlav")),
                               Tau_genPartIdx(TTreeReaderArray<Short_t>(*fReader, "Tau_genPartIdx")),
                               Tau_hasRefitSV(TTreeReaderArray<Bool_t>(*fReader, "Tau_hasRefitSV")),
                               Tau_idAntiEleDeadECal(TTreeReaderArray<Bool_t>(*fReader, "Tau_idAntiEleDeadECal")),
                               Tau_idAntiMu(TTreeReaderArray<UChar_t>(*fReader, "Tau_idAntiMu")),
                               Tau_idDecayModeNewDMs(TTreeReaderArray<Bool_t>(*fReader, "Tau_idDecayModeNewDMs")),
                               Tau_idDecayModeOldDMs(TTreeReaderArray<Bool_t>(*fReader, "Tau_idDecayModeOldDMs")),
                               Tau_idDeepTau2018v2p5VSe(TTreeReaderArray<UChar_t>(*fReader, "Tau_idDeepTau2018v2p5VSe")),
                               Tau_idDeepTau2018v2p5VSjet(TTreeReaderArray<UChar_t>(*fReader, "Tau_idDeepTau2018v2p5VSjet")),
                               Tau_idDeepTau2018v2p5VSmu(TTreeReaderArray<UChar_t>(*fReader, "Tau_idDeepTau2018v2p5VSmu")),
                               Tau_ipLengthSig(TTreeReaderArray<Float_t>(*fReader, "Tau_ipLengthSig")),
                               Tau_jetIdx(TTreeReaderArray<Short_t>(*fReader, "Tau_jetIdx")),
                               Tau_leadTkDeltaEta(TTreeReaderArray<Float_t>(*fReader, "Tau_leadTkDeltaEta")),
                               Tau_leadTkDeltaPhi(TTreeReaderArray<Float_t>(*fReader, "Tau_leadTkDeltaPhi")),
                               Tau_leadTkPtOverTauPt(TTreeReaderArray<Float_t>(*fReader, "Tau_leadTkPtOverTauPt")),
                               Tau_mass(TTreeReaderArray<Float_t>(*fReader, "Tau_mass")),
                               Tau_muIdx(TTreeReaderArray<Short_t>(*fReader, "Tau_muIdx")),
                               Tau_nSVs(TTreeReaderArray<UChar_t>(*fReader, "Tau_nSVs")),
                               Tau_neutralIso(TTreeReaderArray<Float_t>(*fReader, "Tau_neutralIso")),
                               Tau_phi(TTreeReaderArray<Float_t>(*fReader, "Tau_phi")),
                               Tau_photonsOutsideSignalCone(TTreeReaderArray<Float_t>(*fReader, "Tau_photonsOutsideSignalCone")),
                               Tau_probDM0PNet(TTreeReaderArray<Float_t>(*fReader, "Tau_probDM0PNet")),
                               Tau_probDM0UParT(TTreeReaderArray<Float_t>(*fReader, "Tau_probDM0UParT")),
                               Tau_probDM10PNet(TTreeReaderArray<Float_t>(*fReader, "Tau_probDM10PNet")),
                               Tau_probDM10UParT(TTreeReaderArray<Float_t>(*fReader, "Tau_probDM10UParT")),
                               Tau_probDM11PNet(TTreeReaderArray<Float_t>(*fReader, "Tau_probDM11PNet")),
                               Tau_probDM11UParT(TTreeReaderArray<Float_t>(*fReader, "Tau_probDM11UParT")),
                               Tau_probDM1PNet(TTreeReaderArray<Float_t>(*fReader, "Tau_probDM1PNet")),
                               Tau_probDM1UParT(TTreeReaderArray<Float_t>(*fReader, "Tau_probDM1UParT")),
                               Tau_probDM2PNet(TTreeReaderArray<Float_t>(*fReader, "Tau_probDM2PNet")),
                               Tau_probDM2UParT(TTreeReaderArray<Float_t>(*fReader, "Tau_probDM2UParT")),
                               Tau_pt(TTreeReaderArray<Float_t>(*fReader, "Tau_pt")),
                               Tau_ptCorrPNet(TTreeReaderArray<Float_t>(*fReader, "Tau_ptCorrPNet")),
                               Tau_ptCorrUParT(TTreeReaderArray<Float_t>(*fReader, "Tau_ptCorrUParT")),
                               Tau_puCorr(TTreeReaderArray<Float_t>(*fReader, "Tau_puCorr")),
                               Tau_qConfPNet(TTreeReaderArray<Float_t>(*fReader, "Tau_qConfPNet")),
                               Tau_qConfUParT(TTreeReaderArray<Float_t>(*fReader, "Tau_qConfUParT")),
                               Tau_rawDeepTau2018v2p5VSe(TTreeReaderArray<Float_t>(*fReader, "Tau_rawDeepTau2018v2p5VSe")),
                               Tau_rawDeepTau2018v2p5VSjet(TTreeReaderArray<Float_t>(*fReader, "Tau_rawDeepTau2018v2p5VSjet")),
                               Tau_rawDeepTau2018v2p5VSmu(TTreeReaderArray<Float_t>(*fReader, "Tau_rawDeepTau2018v2p5VSmu")),
                               Tau_rawIso(TTreeReaderArray<Float_t>(*fReader, "Tau_rawIso")),
                               Tau_rawIsodR03(TTreeReaderArray<Float_t>(*fReader, "Tau_rawIsodR03")),
                               Tau_rawPNetVSe(TTreeReaderArray<Float_t>(*fReader, "Tau_rawPNetVSe")),
                               Tau_rawPNetVSjet(TTreeReaderArray<Float_t>(*fReader, "Tau_rawPNetVSjet")),
                               Tau_rawPNetVSmu(TTreeReaderArray<Float_t>(*fReader, "Tau_rawPNetVSmu")),
                               Tau_rawUParTVSe(TTreeReaderArray<Float_t>(*fReader, "Tau_rawUParTVSe")),
                               Tau_rawUParTVSjet(TTreeReaderArray<Float_t>(*fReader, "Tau_rawUParTVSjet")),
                               Tau_rawUParTVSmu(TTreeReaderArray<Float_t>(*fReader, "Tau_rawUParTVSmu")),
                               Tau_refitSVchi2(TTreeReaderArray<Float_t>(*fReader, "Tau_refitSVchi2")),
                               Tau_refitSVcov00(TTreeReaderArray<Float_t>(*fReader, "Tau_refitSVcov00")),
                               Tau_refitSVcov10(TTreeReaderArray<Float_t>(*fReader, "Tau_refitSVcov10")),
                               Tau_refitSVcov11(TTreeReaderArray<Float_t>(*fReader, "Tau_refitSVcov11")),
                               Tau_refitSVcov20(TTreeReaderArray<Float_t>(*fReader, "Tau_refitSVcov20")),
                               Tau_refitSVcov21(TTreeReaderArray<Float_t>(*fReader, "Tau_refitSVcov21")),
                               Tau_refitSVcov22(TTreeReaderArray<Float_t>(*fReader, "Tau_refitSVcov22")),
                               Tau_refitSVx(TTreeReaderArray<Float_t>(*fReader, "Tau_refitSVx")),
                               Tau_refitSVy(TTreeReaderArray<Float_t>(*fReader, "Tau_refitSVy")),
                               Tau_refitSVz(TTreeReaderArray<Float_t>(*fReader, "Tau_refitSVz")),
                               Tau_svIdx1(TTreeReaderArray<Short_t>(*fReader, "Tau_svIdx1")),
                               Tau_svIdx2(TTreeReaderArray<Short_t>(*fReader, "Tau_svIdx2")),
                               nTau_Ptr(TTreeReaderValue<Int_t>(*fReader, "nTau")),
                               nTau(*nTau_Ptr),

                               // TauProd
                               TauProd_eta(TTreeReaderArray<Float_t>(*fReader, "TauProd_eta")),
                               TauProd_pdgId(TTreeReaderArray<Int_t>(*fReader, "TauProd_pdgId")),
                               TauProd_phi(TTreeReaderArray<Float_t>(*fReader, "TauProd_phi")),
                               TauProd_pt(TTreeReaderArray<Float_t>(*fReader, "TauProd_pt")),
                               TauProd_tauIdx(TTreeReaderArray<Short_t>(*fReader, "TauProd_tauIdx")),
                               nTauProd_Ptr(TTreeReaderValue<Int_t>(*fReader, "nTauProd")),
                               nTauProd(*nTauProd_Ptr),

                               // TauSpinner
                               TauSpinner_weight_cp_0_Ptr(TTreeReaderValue<Double_t>(*fReader, "TauSpinner_weight_cp_0")),
                               TauSpinner_weight_cp_0(*TauSpinner_weight_cp_0_Ptr),
                               TauSpinner_weight_cp_0_alt_Ptr(TTreeReaderValue<Double_t>(*fReader, "TauSpinner_weight_cp_0_alt")),
                               TauSpinner_weight_cp_0_alt(*TauSpinner_weight_cp_0_alt_Ptr),
                               TauSpinner_weight_cp_0p25_Ptr(TTreeReaderValue<Double_t>(*fReader, "TauSpinner_weight_cp_0p25")),
                               TauSpinner_weight_cp_0p25(*TauSpinner_weight_cp_0p25_Ptr),
                               TauSpinner_weight_cp_0p25_alt_Ptr(TTreeReaderValue<Double_t>(*fReader, "TauSpinner_weight_cp_0p25_alt")),
                               TauSpinner_weight_cp_0p25_alt(*TauSpinner_weight_cp_0p25_alt_Ptr),
                               TauSpinner_weight_cp_0p375_Ptr(TTreeReaderValue<Double_t>(*fReader, "TauSpinner_weight_cp_0p375")),
                               TauSpinner_weight_cp_0p375(*TauSpinner_weight_cp_0p375_Ptr),
                               TauSpinner_weight_cp_0p375_alt_Ptr(TTreeReaderValue<Double_t>(*fReader, "TauSpinner_weight_cp_0p375_alt")),
                               TauSpinner_weight_cp_0p375_alt(*TauSpinner_weight_cp_0p375_alt_Ptr),
                               TauSpinner_weight_cp_0p5_Ptr(TTreeReaderValue<Double_t>(*fReader, "TauSpinner_weight_cp_0p5")),
                               TauSpinner_weight_cp_0p5(*TauSpinner_weight_cp_0p5_Ptr),
                               TauSpinner_weight_cp_0p5_alt_Ptr(TTreeReaderValue<Double_t>(*fReader, "TauSpinner_weight_cp_0p5_alt")),
                               TauSpinner_weight_cp_0p5_alt(*TauSpinner_weight_cp_0p5_alt_Ptr),
                               TauSpinner_weight_cp_minus0p25_Ptr(TTreeReaderValue<Double_t>(*fReader, "TauSpinner_weight_cp_minus0p25")),
                               TauSpinner_weight_cp_minus0p25(*TauSpinner_weight_cp_minus0p25_Ptr),
                               TauSpinner_weight_cp_minus0p25_alt_Ptr(TTreeReaderValue<Double_t>(*fReader, "TauSpinner_weight_cp_minus0p25_alt")),
                               TauSpinner_weight_cp_minus0p25_alt(*TauSpinner_weight_cp_minus0p25_alt_Ptr),

                               // TrackGenJetAK4
                               TrackGenJetAK4_eta(TTreeReaderArray<Float_t>(*fReader, "TrackGenJetAK4_eta")),
                               TrackGenJetAK4_phi(TTreeReaderArray<Float_t>(*fReader, "TrackGenJetAK4_phi")),
                               TrackGenJetAK4_pt(TTreeReaderArray<Float_t>(*fReader, "TrackGenJetAK4_pt")),
                               nTrackGenJetAK4_Ptr(TTreeReaderValue<Int_t>(*fReader, "nTrackGenJetAK4")),
                               nTrackGenJetAK4(*nTrackGenJetAK4_Ptr),

                               // TrigObj
                               TrigObj_eta(TTreeReaderArray<Float_t>(*fReader, "TrigObj_eta")),
                               TrigObj_filterBits(TTreeReaderArray<ULong64_t>(*fReader, "TrigObj_filterBits")),
                               TrigObj_id(TTreeReaderArray<UShort_t>(*fReader, "TrigObj_id")),
                               TrigObj_l1charge(TTreeReaderArray<Short_t>(*fReader, "TrigObj_l1charge")),
                               TrigObj_l1iso(TTreeReaderArray<Int_t>(*fReader, "TrigObj_l1iso")),
                               TrigObj_l1pt(TTreeReaderArray<Float_t>(*fReader, "TrigObj_l1pt")),
                               TrigObj_l1pt_2(TTreeReaderArray<Float_t>(*fReader, "TrigObj_l1pt_2")),
                               TrigObj_l2pt(TTreeReaderArray<Float_t>(*fReader, "TrigObj_l2pt")),
                               TrigObj_phi(TTreeReaderArray<Float_t>(*fReader, "TrigObj_phi")),
                               TrigObj_pt(TTreeReaderArray<Float_t>(*fReader, "TrigObj_pt")),
                               nTrigObj_Ptr(TTreeReaderValue<Int_t>(*fReader, "nTrigObj")),
                               nTrigObj(*nTrigObj_Ptr),

                               // TrkMET
                               TrkMET_phi_Ptr(TTreeReaderValue<Float_t>(*fReader, "TrkMET_phi")),
                               TrkMET_phi(*TrkMET_phi_Ptr),
                               TrkMET_pt_Ptr(TTreeReaderValue<Float_t>(*fReader, "TrkMET_pt")),
                               TrkMET_pt(*TrkMET_pt_Ptr),
                               TrkMET_sumEt_Ptr(TTreeReaderValue<Float_t>(*fReader, "TrkMET_sumEt")),
                               TrkMET_sumEt(*TrkMET_sumEt_Ptr),

                               // boostedTau
                               boostedTau_charge(TTreeReaderArray<Int_t>(*fReader, "boostedTau_charge")),
                               boostedTau_chargedIso(TTreeReaderArray<Float_t>(*fReader, "boostedTau_chargedIso")),
                               boostedTau_decayMode(TTreeReaderArray<Int_t>(*fReader, "boostedTau_decayMode")),
                               boostedTau_eta(TTreeReaderArray<Float_t>(*fReader, "boostedTau_eta")),
                               boostedTau_genPartFlav(TTreeReaderArray<UChar_t>(*fReader, "boostedTau_genPartFlav")),
                               boostedTau_genPartIdx(TTreeReaderArray<Short_t>(*fReader, "boostedTau_genPartIdx")),
                               boostedTau_idAntiEle2018(TTreeReaderArray<UChar_t>(*fReader, "boostedTau_idAntiEle2018")),
                               boostedTau_idAntiMu(TTreeReaderArray<UChar_t>(*fReader, "boostedTau_idAntiMu")),
                               boostedTau_idMVAnewDM2017v2(TTreeReaderArray<UChar_t>(*fReader, "boostedTau_idMVAnewDM2017v2")),
                               boostedTau_idMVAoldDM2017v2(TTreeReaderArray<UChar_t>(*fReader, "boostedTau_idMVAoldDM2017v2")),
                               boostedTau_jetIdx(TTreeReaderArray<Short_t>(*fReader, "boostedTau_jetIdx")),
                               boostedTau_leadTkDeltaEta(TTreeReaderArray<Float_t>(*fReader, "boostedTau_leadTkDeltaEta")),
                               boostedTau_leadTkDeltaPhi(TTreeReaderArray<Float_t>(*fReader, "boostedTau_leadTkDeltaPhi")),
                               boostedTau_leadTkPtOverTauPt(TTreeReaderArray<Float_t>(*fReader, "boostedTau_leadTkPtOverTauPt")),
                               boostedTau_mass(TTreeReaderArray<Float_t>(*fReader, "boostedTau_mass")),
                               boostedTau_neutralIso(TTreeReaderArray<Float_t>(*fReader, "boostedTau_neutralIso")),
                               boostedTau_phi(TTreeReaderArray<Float_t>(*fReader, "boostedTau_phi")),
                               boostedTau_photonsOutsideSignalCone(TTreeReaderArray<Float_t>(*fReader, "boostedTau_photonsOutsideSignalCone")),
                               boostedTau_pt(TTreeReaderArray<Float_t>(*fReader, "boostedTau_pt")),
                               boostedTau_puCorr(TTreeReaderArray<Float_t>(*fReader, "boostedTau_puCorr")),
                               boostedTau_rawAntiEle2018(TTreeReaderArray<Float_t>(*fReader, "boostedTau_rawAntiEle2018")),
                               boostedTau_rawAntiEleCat2018(TTreeReaderArray<Short_t>(*fReader, "boostedTau_rawAntiEleCat2018")),
                               boostedTau_rawBoostedDeepTauRunIIv2p0VSe(TTreeReaderArray<Float_t>(*fReader, "boostedTau_rawBoostedDeepTauRunIIv2p0VSe")),
                               boostedTau_rawBoostedDeepTauRunIIv2p0VSjet(TTreeReaderArray<Float_t>(*fReader, "boostedTau_rawBoostedDeepTauRunIIv2p0VSjet")),
                               boostedTau_rawBoostedDeepTauRunIIv2p0VSmu(TTreeReaderArray<Float_t>(*fReader, "boostedTau_rawBoostedDeepTauRunIIv2p0VSmu")),
                               boostedTau_rawIso(TTreeReaderArray<Float_t>(*fReader, "boostedTau_rawIso")),
                               boostedTau_rawIsodR03(TTreeReaderArray<Float_t>(*fReader, "boostedTau_rawIsodR03")),
                               boostedTau_rawMVAnewDM2017v2(TTreeReaderArray<Float_t>(*fReader, "boostedTau_rawMVAnewDM2017v2")),
                               boostedTau_rawMVAoldDM2017v2(TTreeReaderArray<Float_t>(*fReader, "boostedTau_rawMVAoldDM2017v2")),
                               nboostedTau_Ptr(TTreeReaderValue<Int_t>(*fReader, "nboostedTau")),
                               nboostedTau(*nboostedTau_Ptr),

                               // bunchCrossing
                               bunchCrossing_Ptr(TTreeReaderValue<UInt_t>(*fReader, "bunchCrossing")),
                               bunchCrossing(*bunchCrossing_Ptr),

                               // event
                               event_Ptr(TTreeReaderValue<ULong64_t>(*fReader, "event")),
                               event(*event_Ptr),

                               // genTtbarId
                               genTtbarId_Ptr(TTreeReaderValue<Int_t>(*fReader, "genTtbarId")),
                               genTtbarId(*genTtbarId_Ptr),

                               // genWeight
                               genWeight_Ptr(TTreeReaderValue<Float_t>(*fReader, "genWeight")),
                               genWeight(*genWeight_Ptr),

                               // luminosityBlock
                               luminosityBlock_Ptr(TTreeReaderValue<UInt_t>(*fReader, "luminosityBlock")),
                               luminosityBlock(*luminosityBlock_Ptr),

                               // orbitNumber
                               orbitNumber_Ptr(TTreeReaderValue<UInt_t>(*fReader, "orbitNumber")),
                               orbitNumber(*orbitNumber_Ptr),

                               // run
                               RunNumber_Ptr(TTreeReaderValue<UInt_t>(*fReader, "run")),
                               RunNumber(*RunNumber_Ptr)

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
    while(fReader->Next())
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
