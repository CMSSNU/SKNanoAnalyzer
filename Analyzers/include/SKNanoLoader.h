#ifndef SKNanoLoader_h
#define SKNanoLoader_h

#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

#include "TROOT.h"
#include "TChain.h"
#include "TFile.h"
#include "TString.h"
#include "ROOT/RVec.hxx"
#include "ROOT/RDataFrame.hxx"
#include <nlohmann/json.hpp>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
using namespace ROOT::VecOps;

template <typename T>
class TTreeReaderArrayWrapper
{
public:
    TTreeReaderArrayWrapper() = default;

    void init(TTreeReader &reader, const char *branchName)
    {
        myArray = std::make_unique<TTreeReaderArray<T>>(reader, branchName);
    }

    T operator[](std::size_t i) const
    {
        return (*myArray)[i];
    }

    std::size_t size() const
    {
        return myArray ? myArray->GetSize() : 0;
    }

private:
    std::unique_ptr<TTreeReaderArray<T>> myArray;
};

template <typename T>
class TTreeReaderValueWrapper
{
public:
    TTreeReaderValueWrapper() = default;

    void init(TTreeReader &reader, const char *branchName)
    {
        myValue = std::make_unique<TTreeReaderValue<T>>(reader, branchName);
    }

    operator T() const
    {
        return **myValue;
    }

private:
    std::unique_ptr<TTreeReaderValue<T>> myValue;
};

class SKNanoLoader
{
public:
    SKNanoLoader();
    virtual ~SKNanoLoader();

    // virtual long GetEntry(long entry);
    virtual void SetTreeName(TString tname) { fChain = new TChain(tname); }
    virtual int AddFile(TString filename) { return fChain->Add(filename, -1); }
    virtual void InitBranch();

    long MaxEvent, NSkipEvent;
    int LogEvery;
    bool IsDATA;
    TString DataStream;
    TString MCSample;
    TString Campaign;
    // bool IsFastSim;
    int DataYear;
    TString DataEra;
    int Run;
    float xsec, sumW, sumSign;
    RVec<TString> Userflags;

    virtual void Init();
    virtual void Loop();
    virtual void executeEvent() {};

    virtual void SetEra(TString era)
    {
        DataEra = era;
        DataYear = TString(era(0, 4)).Atoi();
        // 2016, 2017, 2018 are Run2
        if (DataYear == 2016 or DataYear == 2017 or DataYear == 2018)
            Run = 2;
        else if (DataYear == 2022 or DataYear == 2023 or DataYear == 2024 or DataYear == 2025)
            Run = 3;
    }
    virtual void SetCampaign(TString campaign) { Campaign = campaign; }

    virtual TString GetEra() const { return DataEra; }
    virtual TString GetCampaign() const { return Campaign; }
    virtual int GetYear() const { return DataYear; }

    TChain *fChain = nullptr;
    TTreeReader *fReader = nullptr;

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
    TTreeReaderArrayWrapper<Float_t> Electron_IPx;
    TTreeReaderArrayWrapper<Float_t> Electron_IPy;
    TTreeReaderArrayWrapper<Float_t> Electron_IPz;
    TTreeReaderArrayWrapper<Float_t> Electron_PreshowerEnergy;
    TTreeReaderArrayWrapper<Int_t> Electron_charge;
    TTreeReaderArrayWrapper<Bool_t> Electron_convVeto;
    TTreeReaderArrayWrapper<UChar_t> Electron_cutBased;
    TTreeReaderArrayWrapper<Bool_t> Electron_cutBased_HEEP;
    TTreeReaderArrayWrapper<Float_t> Electron_deltaEtaSC;
    TTreeReaderArrayWrapper<Float_t> Electron_dr03EcalRecHitSumEt;
    TTreeReaderArrayWrapper<Float_t> Electron_dr03HcalDepth1TowerSumEt;
    TTreeReaderArrayWrapper<Float_t> Electron_dr03TkSumPt;
    TTreeReaderArrayWrapper<Float_t> Electron_dr03TkSumPtHEEP;
    TTreeReaderArrayWrapper<Float_t> Electron_dxy;
    TTreeReaderArrayWrapper<Float_t> Electron_dxyErr;
    TTreeReaderArrayWrapper<Float_t> Electron_dz;
    TTreeReaderArrayWrapper<Float_t> Electron_dzErr;
    TTreeReaderArrayWrapper<Float_t> Electron_eInvMinusPInv;
    TTreeReaderArrayWrapper<Float_t> Electron_ecalEnergy;
    TTreeReaderArrayWrapper<Float_t> Electron_ecalEnergyError;
    TTreeReaderArrayWrapper<Float_t> Electron_energyErr;
    TTreeReaderArrayWrapper<Float_t> Electron_eta;
    TTreeReaderArrayWrapper<Float_t> Electron_fbrem;
    TTreeReaderArrayWrapper<Short_t> Electron_fsrPhotonIdx;
    TTreeReaderArrayWrapper<UChar_t> Electron_genPartFlav;
    TTreeReaderArrayWrapper<Short_t> Electron_genPartIdx;
    TTreeReaderArrayWrapper<Float_t> Electron_gsfTrketaMode;
    TTreeReaderArrayWrapper<Float_t> Electron_gsfTrkpMode;
    TTreeReaderArrayWrapper<Float_t> Electron_gsfTrkpModeErr;
    TTreeReaderArrayWrapper<Float_t> Electron_gsfTrkphiMode;
    TTreeReaderArrayWrapper<Float_t> Electron_hoe;
    TTreeReaderArrayWrapper<Float_t> Electron_ip3d;
    TTreeReaderArrayWrapper<Float_t> Electron_ipLengthSig;
    TTreeReaderArrayWrapper<Bool_t> Electron_isEB;
    TTreeReaderArrayWrapper<Bool_t> Electron_isEcalDriven;
    TTreeReaderArrayWrapper<Bool_t> Electron_isPFcand;
    TTreeReaderArrayWrapper<Float_t> Electron_jetDF;
    TTreeReaderArrayWrapper<Short_t> Electron_jetIdx;
    TTreeReaderArrayWrapper<UChar_t> Electron_jetNDauCharged;
    TTreeReaderArrayWrapper<Float_t> Electron_jetPtRelv2;
    TTreeReaderArrayWrapper<Float_t> Electron_jetRelIso;
    TTreeReaderArrayWrapper<UChar_t> Electron_lostHits;
    TTreeReaderArrayWrapper<Float_t> Electron_mass;
    TTreeReaderArrayWrapper<Float_t> Electron_miniPFRelIso_all;
    TTreeReaderArrayWrapper<Float_t> Electron_miniPFRelIso_chg;
    TTreeReaderArrayWrapper<Float_t> Electron_mvaHZZIso;
    TTreeReaderArrayWrapper<Float_t> Electron_mvaIso;
    TTreeReaderArrayWrapper<Bool_t> Electron_mvaIso_WP80;
    TTreeReaderArrayWrapper<Bool_t> Electron_mvaIso_WP90;
    TTreeReaderArrayWrapper<Bool_t> Electron_mvaIso_WPHZZ;
    TTreeReaderArrayWrapper<Float_t> Electron_mvaNoIso;
    TTreeReaderArrayWrapper<Bool_t> Electron_mvaNoIso_WP80;
    TTreeReaderArrayWrapper<Bool_t> Electron_mvaNoIso_WP90;
    TTreeReaderArrayWrapper<Int_t> Electron_pdgId;
    TTreeReaderArrayWrapper<Float_t> Electron_pfRelIso03_all;
    TTreeReaderArrayWrapper<Float_t> Electron_pfRelIso03_chg;
    TTreeReaderArrayWrapper<Float_t> Electron_pfRelIso04_all;
    TTreeReaderArrayWrapper<Float_t> Electron_phi;
    TTreeReaderArrayWrapper<Short_t> Electron_photonIdx;
    TTreeReaderArrayWrapper<Float_t> Electron_promptMVA;
    TTreeReaderArrayWrapper<Float_t> Electron_pt;
    TTreeReaderArrayWrapper<Float_t> Electron_r9;
    TTreeReaderArrayWrapper<Float_t> Electron_rawEnergy;
    TTreeReaderArrayWrapper<Float_t> Electron_scEtOverPt;
    TTreeReaderArrayWrapper<UChar_t> Electron_seedGain;
    TTreeReaderArrayWrapper<Short_t> Electron_seediEtaOriX;
    TTreeReaderArrayWrapper<Short_t> Electron_seediPhiOriY;
    TTreeReaderArrayWrapper<Float_t> Electron_sieie;
    TTreeReaderArrayWrapper<Float_t> Electron_sip3d;
    TTreeReaderArrayWrapper<Float_t> Electron_superclusterEta;
    TTreeReaderArrayWrapper<Short_t> Electron_svIdx;
    TTreeReaderArrayWrapper<UChar_t> Electron_tightCharge;
    TTreeReaderArrayWrapper<Int_t> Electron_vidNestedWPBitmap;
    TTreeReaderArrayWrapper<Int_t> Electron_vidNestedWPBitmapHEEP;
    TTreeReaderValueWrapper<Int_t> nElectron;

    // FatJet
    TTreeReaderArrayWrapper<Float_t> FatJet_area;
    TTreeReaderArrayWrapper<Float_t> FatJet_chEmEF;
    TTreeReaderArrayWrapper<Float_t> FatJet_chHEF;
    TTreeReaderArrayWrapper<Short_t> FatJet_chMultiplicity;
    TTreeReaderArrayWrapper<Short_t> FatJet_electronIdx3SJ;
    TTreeReaderArrayWrapper<Float_t> FatJet_eta;
    TTreeReaderArrayWrapper<Short_t> FatJet_genJetAK8Idx;
    TTreeReaderArrayWrapper<Float_t> FatJet_globalParT3_QCD;
    TTreeReaderArrayWrapper<Float_t> FatJet_globalParT3_TopbWev;
    TTreeReaderArrayWrapper<Float_t> FatJet_globalParT3_TopbWmv;
    TTreeReaderArrayWrapper<Float_t> FatJet_globalParT3_TopbWq;
    TTreeReaderArrayWrapper<Float_t> FatJet_globalParT3_TopbWqq;
    TTreeReaderArrayWrapper<Float_t> FatJet_globalParT3_TopbWtauhv;
    TTreeReaderArrayWrapper<Float_t> FatJet_globalParT3_WvsQCD;
    TTreeReaderArrayWrapper<Float_t> FatJet_globalParT3_XWW3q;
    TTreeReaderArrayWrapper<Float_t> FatJet_globalParT3_XWW4q;
    TTreeReaderArrayWrapper<Float_t> FatJet_globalParT3_XWWqqev;
    TTreeReaderArrayWrapper<Float_t> FatJet_globalParT3_XWWqqmv;
    TTreeReaderArrayWrapper<Float_t> FatJet_globalParT3_Xbb;
    TTreeReaderArrayWrapper<Float_t> FatJet_globalParT3_Xcc;
    TTreeReaderArrayWrapper<Float_t> FatJet_globalParT3_Xcs;
    TTreeReaderArrayWrapper<Float_t> FatJet_globalParT3_Xqq;
    TTreeReaderArrayWrapper<Float_t> FatJet_globalParT3_Xtauhtaue;
    TTreeReaderArrayWrapper<Float_t> FatJet_globalParT3_Xtauhtauh;
    TTreeReaderArrayWrapper<Float_t> FatJet_globalParT3_Xtauhtaum;
    TTreeReaderArrayWrapper<Float_t> FatJet_globalParT3_massCorrGeneric;
    TTreeReaderArrayWrapper<Float_t> FatJet_globalParT3_massCorrX2p;
    TTreeReaderArrayWrapper<Float_t> FatJet_globalParT3_withMassTopvsQCD;
    TTreeReaderArrayWrapper<Float_t> FatJet_globalParT3_withMassWvsQCD;
    TTreeReaderArrayWrapper<Float_t> FatJet_globalParT3_withMassZvsQCD;
    TTreeReaderArrayWrapper<UChar_t> FatJet_hadronFlavour;
    TTreeReaderArrayWrapper<Float_t> FatJet_hfEmEF;
    TTreeReaderArrayWrapper<Float_t> FatJet_hfHEF;
    TTreeReaderArrayWrapper<Float_t> FatJet_lsf3;
    TTreeReaderArrayWrapper<Float_t> FatJet_mass;
    TTreeReaderArrayWrapper<Float_t> FatJet_msoftdrop;
    TTreeReaderArrayWrapper<Float_t> FatJet_muEF;
    TTreeReaderArrayWrapper<Short_t> FatJet_muonIdx3SJ;
    TTreeReaderArrayWrapper<Float_t> FatJet_n2b1;
    TTreeReaderArrayWrapper<Float_t> FatJet_n3b1;
    TTreeReaderArrayWrapper<UChar_t> FatJet_nConstituents;
    TTreeReaderArrayWrapper<Float_t> FatJet_neEmEF;
    TTreeReaderArrayWrapper<Float_t> FatJet_neHEF;
    TTreeReaderArrayWrapper<Short_t> FatJet_neMultiplicity;
    TTreeReaderArrayWrapper<Float_t> FatJet_particleNetLegacy_QCD;
    TTreeReaderArrayWrapper<Float_t> FatJet_particleNetLegacy_Xbb;
    TTreeReaderArrayWrapper<Float_t> FatJet_particleNetLegacy_Xcc;
    TTreeReaderArrayWrapper<Float_t> FatJet_particleNetLegacy_Xqq;
    TTreeReaderArrayWrapper<Float_t> FatJet_particleNetLegacy_mass;
    TTreeReaderArrayWrapper<Float_t> FatJet_particleNetWithMass_H4qvsQCD;
    TTreeReaderArrayWrapper<Float_t> FatJet_particleNetWithMass_HbbvsQCD;
    TTreeReaderArrayWrapper<Float_t> FatJet_particleNetWithMass_HccvsQCD;
    TTreeReaderArrayWrapper<Float_t> FatJet_particleNetWithMass_QCD;
    TTreeReaderArrayWrapper<Float_t> FatJet_particleNetWithMass_TvsQCD;
    TTreeReaderArrayWrapper<Float_t> FatJet_particleNetWithMass_WvsQCD;
    TTreeReaderArrayWrapper<Float_t> FatJet_particleNetWithMass_ZvsQCD;
    TTreeReaderArrayWrapper<Float_t> FatJet_particleNet_QCD;
    TTreeReaderArrayWrapper<Float_t> FatJet_particleNet_QCD0HF;
    TTreeReaderArrayWrapper<Float_t> FatJet_particleNet_QCD1HF;
    TTreeReaderArrayWrapper<Float_t> FatJet_particleNet_QCD2HF;
    TTreeReaderArrayWrapper<Float_t> FatJet_particleNet_WVsQCD;
    TTreeReaderArrayWrapper<Float_t> FatJet_particleNet_XbbVsQCD;
    TTreeReaderArrayWrapper<Float_t> FatJet_particleNet_XccVsQCD;
    TTreeReaderArrayWrapper<Float_t> FatJet_particleNet_XggVsQCD;
    TTreeReaderArrayWrapper<Float_t> FatJet_particleNet_XqqVsQCD;
    TTreeReaderArrayWrapper<Float_t> FatJet_particleNet_XteVsQCD;
    TTreeReaderArrayWrapper<Float_t> FatJet_particleNet_XtmVsQCD;
    TTreeReaderArrayWrapper<Float_t> FatJet_particleNet_XttVsQCD;
    TTreeReaderArrayWrapper<Float_t> FatJet_particleNet_massCorr;
    TTreeReaderArrayWrapper<Float_t> FatJet_phi;
    TTreeReaderArrayWrapper<Float_t> FatJet_pt;
    TTreeReaderArrayWrapper<Float_t> FatJet_rawFactor;
    TTreeReaderArrayWrapper<Short_t> FatJet_subJetIdx1;
    TTreeReaderArrayWrapper<Short_t> FatJet_subJetIdx2;
    TTreeReaderArrayWrapper<Float_t> FatJet_tau1;
    TTreeReaderArrayWrapper<Float_t> FatJet_tau2;
    TTreeReaderArrayWrapper<Float_t> FatJet_tau3;
    TTreeReaderArrayWrapper<Float_t> FatJet_tau4;
    TTreeReaderValueWrapper<Int_t> nFatJet;

    // FatJetPFCand
    TTreeReaderArrayWrapper<Int_t> FatJetPFCand_jetIdx;
    TTreeReaderArrayWrapper<Int_t> FatJetPFCand_pfCandIdx;
    TTreeReaderValueWrapper<Int_t> nFatJetPFCand;

    // Flag
    TTreeReaderValueWrapper<Bool_t> Flag_BadChargedCandidateFilter;
    TTreeReaderValueWrapper<Bool_t> Flag_BadChargedCandidateSummer16Filter;
    TTreeReaderValueWrapper<Bool_t> Flag_BadPFMuonDzFilter;
    TTreeReaderValueWrapper<Bool_t> Flag_BadPFMuonFilter;
    TTreeReaderValueWrapper<Bool_t> Flag_BadPFMuonSummer16Filter;
    TTreeReaderValueWrapper<Bool_t> Flag_CSCTightHalo2015Filter;
    TTreeReaderValueWrapper<Bool_t> Flag_CSCTightHaloFilter;
    TTreeReaderValueWrapper<Bool_t> Flag_CSCTightHaloTrkMuUnvetoFilter;
    TTreeReaderValueWrapper<Bool_t> Flag_EcalDeadCellBoundaryEnergyFilter;
    TTreeReaderValueWrapper<Bool_t> Flag_EcalDeadCellTriggerPrimitiveFilter;
    TTreeReaderValueWrapper<Bool_t> Flag_HBHENoiseFilter;
    TTreeReaderValueWrapper<Bool_t> Flag_HBHENoiseIsoFilter;
    TTreeReaderValueWrapper<Bool_t> Flag_HcalStripHaloFilter;
    TTreeReaderValueWrapper<Bool_t> Flag_chargedHadronTrackResolutionFilter;
    TTreeReaderValueWrapper<Bool_t> Flag_ecalBadCalibFilter;
    TTreeReaderValueWrapper<Bool_t> Flag_ecalLaserCorrFilter;
    TTreeReaderValueWrapper<Bool_t> Flag_eeBadScFilter;
    TTreeReaderValueWrapper<Bool_t> Flag_globalSuperTightHalo2016Filter;
    TTreeReaderValueWrapper<Bool_t> Flag_globalTightHalo2016Filter;
    TTreeReaderValueWrapper<Bool_t> Flag_goodVertices;
    TTreeReaderValueWrapper<Bool_t> Flag_hcalLaserEventFilter;
    TTreeReaderValueWrapper<Bool_t> Flag_hfNoisyHitsFilter;
    TTreeReaderValueWrapper<Bool_t> Flag_muonBadTrackFilter;
    TTreeReaderValueWrapper<Bool_t> Flag_trkPOGFilters;
    TTreeReaderValueWrapper<Bool_t> Flag_trkPOG_logErrorTooManyClusters;
    TTreeReaderValueWrapper<Bool_t> Flag_trkPOG_manystripclus53X;
    TTreeReaderValueWrapper<Bool_t> Flag_trkPOG_toomanystripclus53X;

    // FsrPhoton
    TTreeReaderArrayWrapper<Float_t> FsrPhoton_dROverEt2;
    TTreeReaderArrayWrapper<Short_t> FsrPhoton_electronIdx;
    TTreeReaderArrayWrapper<Float_t> FsrPhoton_eta;
    TTreeReaderArrayWrapper<Short_t> FsrPhoton_muonIdx;
    TTreeReaderArrayWrapper<Float_t> FsrPhoton_phi;
    TTreeReaderArrayWrapper<Float_t> FsrPhoton_pt;
    TTreeReaderArrayWrapper<Float_t> FsrPhoton_relIso03;
    TTreeReaderValueWrapper<Int_t> nFsrPhoton;

    // GenDressedLepton
    TTreeReaderArrayWrapper<Float_t> GenDressedLepton_eta;
    TTreeReaderArrayWrapper<Bool_t> GenDressedLepton_hasTauAnc;
    TTreeReaderArrayWrapper<Float_t> GenDressedLepton_mass;
    TTreeReaderArrayWrapper<Int_t> GenDressedLepton_pdgId;
    TTreeReaderArrayWrapper<Float_t> GenDressedLepton_phi;
    TTreeReaderArrayWrapper<Float_t> GenDressedLepton_pt;
    TTreeReaderValueWrapper<Int_t> nGenDressedLepton;

    // GenIsolatedPhoton
    TTreeReaderArrayWrapper<Float_t> GenIsolatedPhoton_eta;
    TTreeReaderArrayWrapper<Float_t> GenIsolatedPhoton_mass;
    TTreeReaderArrayWrapper<Float_t> GenIsolatedPhoton_phi;
    TTreeReaderArrayWrapper<Float_t> GenIsolatedPhoton_pt;
    TTreeReaderValueWrapper<Int_t> nGenIsolatedPhoton;

    // GenJet
    TTreeReaderArrayWrapper<Float_t> GenJet_eta;
    TTreeReaderArrayWrapper<UChar_t> GenJet_hadronFlavour;
    TTreeReaderArrayWrapper<Float_t> GenJet_mass;
    TTreeReaderArrayWrapper<UChar_t> GenJet_nBHadrons;
    TTreeReaderArrayWrapper<UChar_t> GenJet_nCHadrons;
    TTreeReaderArrayWrapper<Short_t> GenJet_partonFlavour;
    TTreeReaderArrayWrapper<Float_t> GenJet_phi;
    TTreeReaderArrayWrapper<Float_t> GenJet_pt;
    TTreeReaderValueWrapper<Int_t> nGenJet;

    // GenJetAK8
    TTreeReaderArrayWrapper<Float_t> GenJetAK8_eta;
    TTreeReaderArrayWrapper<UChar_t> GenJetAK8_hadronFlavour;
    TTreeReaderArrayWrapper<Float_t> GenJetAK8_mass;
    TTreeReaderArrayWrapper<UChar_t> GenJetAK8_nBHadrons;
    TTreeReaderArrayWrapper<UChar_t> GenJetAK8_nCHadrons;
    TTreeReaderArrayWrapper<Short_t> GenJetAK8_partonFlavour;
    TTreeReaderArrayWrapper<Float_t> GenJetAK8_phi;
    TTreeReaderArrayWrapper<Float_t> GenJetAK8_pt;
    TTreeReaderValueWrapper<Int_t> nGenJetAK8;

    // GenMET
    TTreeReaderValueWrapper<Float_t> GenMET_phi;
    TTreeReaderValueWrapper<Float_t> GenMET_pt;

    // GenPart
    TTreeReaderArrayWrapper<Float_t> GenPart_eta;
    TTreeReaderArrayWrapper<Short_t> GenPart_genPartIdxMother;
    TTreeReaderArrayWrapper<Float_t> GenPart_iso;
    TTreeReaderArrayWrapper<Float_t> GenPart_mass;
    TTreeReaderArrayWrapper<Int_t> GenPart_pdgId;
    TTreeReaderArrayWrapper<Float_t> GenPart_phi;
    TTreeReaderArrayWrapper<Float_t> GenPart_pt;
    TTreeReaderArrayWrapper<Int_t> GenPart_status;
    TTreeReaderArrayWrapper<UShort_t> GenPart_statusFlags;
    TTreeReaderValueWrapper<Int_t> nGenPart;

    // GenVisTau
    TTreeReaderArrayWrapper<Short_t> GenVisTau_charge;
    TTreeReaderArrayWrapper<Float_t> GenVisTau_eta;
    TTreeReaderArrayWrapper<Short_t> GenVisTau_genPartIdxMother;
    TTreeReaderArrayWrapper<Float_t> GenVisTau_mass;
    TTreeReaderArrayWrapper<Float_t> GenVisTau_phi;
    TTreeReaderArrayWrapper<Float_t> GenVisTau_pt;
    TTreeReaderArrayWrapper<UChar_t> GenVisTau_status;
    TTreeReaderValueWrapper<Int_t> nGenVisTau;

    // GenVtx
    TTreeReaderValueWrapper<Float_t> GenVtx_t0;
    TTreeReaderValueWrapper<Float_t> GenVtx_x;
    TTreeReaderValueWrapper<Float_t> GenVtx_y;
    TTreeReaderValueWrapper<Float_t> GenVtx_z;

    // Generator
    TTreeReaderValueWrapper<Float_t> Generator_binvar;
    TTreeReaderValueWrapper<Int_t> Generator_id1;
    TTreeReaderValueWrapper<Int_t> Generator_id2;
    TTreeReaderValueWrapper<Float_t> Generator_scalePDF;
    TTreeReaderValueWrapper<Float_t> Generator_weight;
    TTreeReaderValueWrapper<Float_t> Generator_x1;
    TTreeReaderValueWrapper<Float_t> Generator_x2;
    TTreeReaderValueWrapper<Float_t> Generator_xpdf1;
    TTreeReaderValueWrapper<Float_t> Generator_xpdf2;

    // IsoTrack
    TTreeReaderArrayWrapper<Short_t> IsoTrack_charge;
    TTreeReaderArrayWrapper<Float_t> IsoTrack_dxy;
    TTreeReaderArrayWrapper<Float_t> IsoTrack_dz;
    TTreeReaderArrayWrapper<Float_t> IsoTrack_eta;
    TTreeReaderArrayWrapper<Short_t> IsoTrack_fromPV;
    TTreeReaderArrayWrapper<Bool_t> IsoTrack_isFromLostTrack;
    TTreeReaderArrayWrapper<Bool_t> IsoTrack_isHighPurityTrack;
    TTreeReaderArrayWrapper<Bool_t> IsoTrack_isPFcand;
    TTreeReaderArrayWrapper<Float_t> IsoTrack_miniPFRelIso_all;
    TTreeReaderArrayWrapper<Float_t> IsoTrack_miniPFRelIso_chg;
    TTreeReaderArrayWrapper<Int_t> IsoTrack_pdgId;
    TTreeReaderArrayWrapper<Float_t> IsoTrack_pfRelIso03_all;
    TTreeReaderArrayWrapper<Float_t> IsoTrack_pfRelIso03_chg;
    TTreeReaderArrayWrapper<Float_t> IsoTrack_phi;
    TTreeReaderArrayWrapper<Float_t> IsoTrack_pt;
    TTreeReaderValueWrapper<Int_t> nIsoTrack;

    // Jet
    TTreeReaderArrayWrapper<Float_t> Jet_PNetRegPtRawCorr;
    TTreeReaderArrayWrapper<Float_t> Jet_PNetRegPtRawCorrNeutrino;
    TTreeReaderArrayWrapper<Float_t> Jet_PNetRegPtRawRes;
    TTreeReaderArrayWrapper<Float_t> Jet_UParTAK4RegPtRawCorr;
    TTreeReaderArrayWrapper<Float_t> Jet_UParTAK4RegPtRawCorrNeutrino;
    TTreeReaderArrayWrapper<Float_t> Jet_UParTAK4RegPtRawRes;
    TTreeReaderArrayWrapper<Float_t> Jet_UParTAK4V1RegPtRawCorr;
    TTreeReaderArrayWrapper<Float_t> Jet_UParTAK4V1RegPtRawCorrNeutrino;
    TTreeReaderArrayWrapper<Float_t> Jet_UParTAK4V1RegPtRawRes;
    TTreeReaderArrayWrapper<Float_t> Jet_area;
    TTreeReaderArrayWrapper<Float_t> Jet_btagDeepFlavB;
    TTreeReaderArrayWrapper<Float_t> Jet_btagDeepFlavCvB;
    TTreeReaderArrayWrapper<Float_t> Jet_btagDeepFlavCvL;
    TTreeReaderArrayWrapper<Float_t> Jet_btagDeepFlavQG;
    TTreeReaderArrayWrapper<Float_t> Jet_btagPNetB;
    TTreeReaderArrayWrapper<Float_t> Jet_btagPNetCvB;
    TTreeReaderArrayWrapper<Float_t> Jet_btagPNetCvL;
    TTreeReaderArrayWrapper<Float_t> Jet_btagPNetCvNotB;
    TTreeReaderArrayWrapper<Float_t> Jet_btagPNetQvG;
    TTreeReaderArrayWrapper<Float_t> Jet_btagPNetTauVJet;
    TTreeReaderArrayWrapper<Float_t> Jet_btagUParTAK4B;
    TTreeReaderArrayWrapper<Float_t> Jet_btagUParTAK4CvB;
    TTreeReaderArrayWrapper<Float_t> Jet_btagUParTAK4CvL;
    TTreeReaderArrayWrapper<Float_t> Jet_btagUParTAK4CvNotB;
    TTreeReaderArrayWrapper<Float_t> Jet_btagUParTAK4Ele;
    TTreeReaderArrayWrapper<Float_t> Jet_btagUParTAK4Mu;
    TTreeReaderArrayWrapper<Float_t> Jet_btagUParTAK4QvG;
    TTreeReaderArrayWrapper<Float_t> Jet_btagUParTAK4SvCB;
    TTreeReaderArrayWrapper<Float_t> Jet_btagUParTAK4SvUDG;
    TTreeReaderArrayWrapper<Float_t> Jet_btagUParTAK4TauVJet;
    TTreeReaderArrayWrapper<Float_t> Jet_btagUParTAK4UDG;
    TTreeReaderArrayWrapper<Float_t> Jet_btagUParTAK4probb;
    TTreeReaderArrayWrapper<Float_t> Jet_btagUParTAK4probbb;
    TTreeReaderArrayWrapper<Float_t> Jet_chEmEF;
    TTreeReaderArrayWrapper<Float_t> Jet_chHEF;
    TTreeReaderArrayWrapper<UChar_t> Jet_chMultiplicity;
    TTreeReaderArrayWrapper<Short_t> Jet_electronIdx1;
    TTreeReaderArrayWrapper<Short_t> Jet_electronIdx2;
    TTreeReaderArrayWrapper<Float_t> Jet_eta;
    TTreeReaderArrayWrapper<Short_t> Jet_genJetIdx;
    TTreeReaderArrayWrapper<UChar_t> Jet_hadronFlavour;
    TTreeReaderArrayWrapper<Float_t> Jet_hfEmEF;
    TTreeReaderArrayWrapper<Float_t> Jet_hfHEF;
    TTreeReaderArrayWrapper<Int_t> Jet_hfadjacentEtaStripsSize;
    TTreeReaderArrayWrapper<Int_t> Jet_hfcentralEtaStripSize;
    TTreeReaderArrayWrapper<Float_t> Jet_hfsigmaEtaEta;
    TTreeReaderArrayWrapper<Float_t> Jet_hfsigmaPhiPhi;
    TTreeReaderArrayWrapper<Float_t> Jet_mass;
    TTreeReaderArrayWrapper<Float_t> Jet_muEF;
    TTreeReaderArrayWrapper<Short_t> Jet_muonIdx1;
    TTreeReaderArrayWrapper<Short_t> Jet_muonIdx2;
    TTreeReaderArrayWrapper<Float_t> Jet_muonSubtrDeltaEta;
    TTreeReaderArrayWrapper<Float_t> Jet_muonSubtrDeltaPhi;
    TTreeReaderArrayWrapper<Float_t> Jet_muonSubtrFactor;
    TTreeReaderArrayWrapper<UChar_t> Jet_nConstituents;
    TTreeReaderArrayWrapper<UChar_t> Jet_nElectrons;
    TTreeReaderArrayWrapper<UChar_t> Jet_nMuons;
    TTreeReaderArrayWrapper<UChar_t> Jet_nSVs;
    TTreeReaderArrayWrapper<Float_t> Jet_neEmEF;
    TTreeReaderArrayWrapper<Float_t> Jet_neHEF;
    TTreeReaderArrayWrapper<UChar_t> Jet_neMultiplicity;
    TTreeReaderArrayWrapper<Short_t> Jet_partonFlavour;
    TTreeReaderArrayWrapper<Float_t> Jet_phi;
    TTreeReaderArrayWrapper<Float_t> Jet_pt;
    TTreeReaderArrayWrapper<Float_t> Jet_puIdDisc;
    TTreeReaderArrayWrapper<Float_t> Jet_rawFactor;
    TTreeReaderArrayWrapper<Short_t> Jet_svIdx1;
    TTreeReaderArrayWrapper<Short_t> Jet_svIdx2;
    TTreeReaderValueWrapper<Int_t> nJet;

    // LHE
    TTreeReaderValueWrapper<Float_t> LHE_AlphaS;
    TTreeReaderValueWrapper<Float_t> LHE_HT;
    TTreeReaderValueWrapper<Float_t> LHE_HTIncoming;
    TTreeReaderValueWrapper<UChar_t> LHE_Nb;
    TTreeReaderValueWrapper<UChar_t> LHE_Nc;
    TTreeReaderValueWrapper<UChar_t> LHE_Nglu;
    TTreeReaderValueWrapper<UChar_t> LHE_Njets;
    TTreeReaderValueWrapper<UChar_t> LHE_NpLO;
    TTreeReaderValueWrapper<UChar_t> LHE_NpNLO;
    TTreeReaderValueWrapper<UChar_t> LHE_Nuds;
    TTreeReaderValueWrapper<Float_t> LHE_Vpt;

    // LHEPart
    TTreeReaderArrayWrapper<Float_t> LHEPart_eta;
    TTreeReaderArrayWrapper<Short_t> LHEPart_firstMotherIdx;
    TTreeReaderArrayWrapper<Float_t> LHEPart_incomingpz;
    TTreeReaderArrayWrapper<Short_t> LHEPart_lastMotherIdx;
    TTreeReaderArrayWrapper<Float_t> LHEPart_mass;
    TTreeReaderArrayWrapper<Int_t> LHEPart_pdgId;
    TTreeReaderArrayWrapper<Float_t> LHEPart_phi;
    TTreeReaderArrayWrapper<Float_t> LHEPart_pt;
    TTreeReaderArrayWrapper<Int_t> LHEPart_spin;
    TTreeReaderArrayWrapper<Int_t> LHEPart_status;
    TTreeReaderValueWrapper<Int_t> nLHEPart;

    // LHEPdfWeight
    TTreeReaderArrayWrapper<Float_t> LHEPdfWeight;
    TTreeReaderValueWrapper<Int_t> nLHEPdfWeight;

    // LHEReweightingWeight
    TTreeReaderArrayWrapper<Float_t> LHEReweightingWeight;
    TTreeReaderValueWrapper<Int_t> nLHEReweightingWeight;

    // LHEScaleWeight
    TTreeReaderArrayWrapper<Float_t> LHEScaleWeight;
    TTreeReaderValueWrapper<Int_t> nLHEScaleWeight;

    // LHEWeight
    TTreeReaderValueWrapper<Float_t> LHEWeight_originalXWGTUP;

    // LowPtElectron
    TTreeReaderArrayWrapper<Float_t> LowPtElectron_ID;
    TTreeReaderArrayWrapper<Int_t> LowPtElectron_charge;
    TTreeReaderArrayWrapper<Bool_t> LowPtElectron_convVeto;
    TTreeReaderArrayWrapper<Float_t> LowPtElectron_convVtxRadius;
    TTreeReaderArrayWrapper<UChar_t> LowPtElectron_convWP;
    TTreeReaderArrayWrapper<Float_t> LowPtElectron_deltaEtaSC;
    TTreeReaderArrayWrapper<Float_t> LowPtElectron_dxy;
    TTreeReaderArrayWrapper<Float_t> LowPtElectron_dxyErr;
    TTreeReaderArrayWrapper<Float_t> LowPtElectron_dz;
    TTreeReaderArrayWrapper<Float_t> LowPtElectron_dzErr;
    TTreeReaderArrayWrapper<Float_t> LowPtElectron_eInvMinusPInv;
    TTreeReaderArrayWrapper<Short_t> LowPtElectron_electronIdx;
    TTreeReaderArrayWrapper<Float_t> LowPtElectron_energyErr;
    TTreeReaderArrayWrapper<Float_t> LowPtElectron_eta;
    TTreeReaderArrayWrapper<UChar_t> LowPtElectron_genPartFlav;
    TTreeReaderArrayWrapper<Short_t> LowPtElectron_genPartIdx;
    TTreeReaderArrayWrapper<Float_t> LowPtElectron_hoe;
    TTreeReaderArrayWrapper<UChar_t> LowPtElectron_lostHits;
    TTreeReaderArrayWrapper<Float_t> LowPtElectron_mass;
    TTreeReaderArrayWrapper<Float_t> LowPtElectron_miniPFRelIso_all;
    TTreeReaderArrayWrapper<Float_t> LowPtElectron_miniPFRelIso_chg;
    TTreeReaderArrayWrapper<Int_t> LowPtElectron_pdgId;
    TTreeReaderArrayWrapper<Float_t> LowPtElectron_phi;
    TTreeReaderArrayWrapper<Short_t> LowPtElectron_photonIdx;
    TTreeReaderArrayWrapper<Float_t> LowPtElectron_pt;
    TTreeReaderArrayWrapper<Float_t> LowPtElectron_ptbiased;
    TTreeReaderArrayWrapper<Float_t> LowPtElectron_r9;
    TTreeReaderArrayWrapper<Float_t> LowPtElectron_scEtOverPt;
    TTreeReaderArrayWrapper<Float_t> LowPtElectron_sieie;
    TTreeReaderArrayWrapper<Float_t> LowPtElectron_unbiased;
    TTreeReaderValueWrapper<Int_t> nLowPtElectron;

    // Muon
    TTreeReaderArrayWrapper<Float_t> Muon_IPx;
    TTreeReaderArrayWrapper<Float_t> Muon_IPy;
    TTreeReaderArrayWrapper<Float_t> Muon_IPz;
    TTreeReaderArrayWrapper<Float_t> Muon_VXBS_Cov00;
    TTreeReaderArrayWrapper<Float_t> Muon_VXBS_Cov03;
    TTreeReaderArrayWrapper<Float_t> Muon_VXBS_Cov33;
    TTreeReaderArrayWrapper<UChar_t> Muon_bestTrackType;
    TTreeReaderArrayWrapper<Float_t> Muon_bsConstrainedChi2;
    TTreeReaderArrayWrapper<Float_t> Muon_bsConstrainedPt;
    TTreeReaderArrayWrapper<Float_t> Muon_bsConstrainedPtErr;
    TTreeReaderArrayWrapper<Int_t> Muon_charge;
    TTreeReaderArrayWrapper<Float_t> Muon_dxy;
    TTreeReaderArrayWrapper<Float_t> Muon_dxyErr;
    TTreeReaderArrayWrapper<Float_t> Muon_dxybs;
    TTreeReaderArrayWrapper<Float_t> Muon_dxybsErr;
    TTreeReaderArrayWrapper<Float_t> Muon_dz;
    TTreeReaderArrayWrapper<Float_t> Muon_dzErr;
    TTreeReaderArrayWrapper<Float_t> Muon_eta;
    TTreeReaderArrayWrapper<Short_t> Muon_fsrPhotonIdx;
    TTreeReaderArrayWrapper<UChar_t> Muon_genPartFlav;
    TTreeReaderArrayWrapper<Short_t> Muon_genPartIdx;
    TTreeReaderArrayWrapper<UChar_t> Muon_highPtId;
    TTreeReaderArrayWrapper<Bool_t> Muon_highPurity;
    TTreeReaderArrayWrapper<Bool_t> Muon_inTimeMuon;
    TTreeReaderArrayWrapper<Float_t> Muon_ip3d;
    TTreeReaderArrayWrapper<Float_t> Muon_ipLengthSig;
    TTreeReaderArrayWrapper<Bool_t> Muon_isGlobal;
    TTreeReaderArrayWrapper<Bool_t> Muon_isPFcand;
    TTreeReaderArrayWrapper<Bool_t> Muon_isStandalone;
    TTreeReaderArrayWrapper<Bool_t> Muon_isTracker;
    TTreeReaderArrayWrapper<Float_t> Muon_jetDF;
    TTreeReaderArrayWrapper<Short_t> Muon_jetIdx;
    TTreeReaderArrayWrapper<UChar_t> Muon_jetNDauCharged;
    TTreeReaderArrayWrapper<Float_t> Muon_jetPtRelv2;
    TTreeReaderArrayWrapper<Float_t> Muon_jetRelIso;
    TTreeReaderArrayWrapper<Bool_t> Muon_looseId;
    TTreeReaderArrayWrapper<Float_t> Muon_mass;
    TTreeReaderArrayWrapper<Bool_t> Muon_mediumId;
    TTreeReaderArrayWrapper<Bool_t> Muon_mediumPromptId;
    TTreeReaderArrayWrapper<UChar_t> Muon_miniIsoId;
    TTreeReaderArrayWrapper<Float_t> Muon_miniPFRelIso_all;
    TTreeReaderArrayWrapper<Float_t> Muon_miniPFRelIso_chg;
    TTreeReaderArrayWrapper<UChar_t> Muon_multiIsoId;
    TTreeReaderArrayWrapper<Float_t> Muon_mvaLowPt;
    TTreeReaderArrayWrapper<Float_t> Muon_mvaMuID;
    TTreeReaderArrayWrapper<UChar_t> Muon_mvaMuID_WP;
    TTreeReaderArrayWrapper<UChar_t> Muon_nStations;
    TTreeReaderArrayWrapper<UChar_t> Muon_nTrackerLayers;
    TTreeReaderArrayWrapper<Int_t> Muon_pdgId;
    TTreeReaderArrayWrapper<UChar_t> Muon_pfIsoId;
    TTreeReaderArrayWrapper<Float_t> Muon_pfRelIso03_all;
    TTreeReaderArrayWrapper<Float_t> Muon_pfRelIso03_chg;
    TTreeReaderArrayWrapper<Float_t> Muon_pfRelIso04_all;
    TTreeReaderArrayWrapper<Float_t> Muon_phi;
    TTreeReaderArrayWrapper<Float_t> Muon_pnScore_heavy;
    TTreeReaderArrayWrapper<Float_t> Muon_pnScore_light;
    TTreeReaderArrayWrapper<Float_t> Muon_pnScore_prompt;
    TTreeReaderArrayWrapper<Float_t> Muon_pnScore_tau;
    TTreeReaderArrayWrapper<Float_t> Muon_promptMVA;
    TTreeReaderArrayWrapper<Float_t> Muon_pt;
    TTreeReaderArrayWrapper<Float_t> Muon_ptErr;
    TTreeReaderArrayWrapper<UChar_t> Muon_puppiIsoId;
    TTreeReaderArrayWrapper<Float_t> Muon_segmentComp;
    TTreeReaderArrayWrapper<Float_t> Muon_sip3d;
    TTreeReaderArrayWrapper<Bool_t> Muon_softId;
    TTreeReaderArrayWrapper<Float_t> Muon_softMva;
    TTreeReaderArrayWrapper<Bool_t> Muon_softMvaId;
    TTreeReaderArrayWrapper<Float_t> Muon_softMvaRun3;
    TTreeReaderArrayWrapper<Short_t> Muon_svIdx;
    TTreeReaderArrayWrapper<UChar_t> Muon_tightCharge;
    TTreeReaderArrayWrapper<Bool_t> Muon_tightId;
    TTreeReaderArrayWrapper<UChar_t> Muon_tkIsoId;
    TTreeReaderArrayWrapper<Float_t> Muon_tkRelIso;
    TTreeReaderArrayWrapper<Bool_t> Muon_triggerIdLoose;
    TTreeReaderArrayWrapper<Float_t> Muon_tuneP_charge;
    TTreeReaderArrayWrapper<Float_t> Muon_tuneP_pterr;
    TTreeReaderArrayWrapper<Float_t> Muon_tunepRelPt;
    TTreeReaderValueWrapper<Int_t> nMuon;

    // OtherPV
    TTreeReaderArrayWrapper<Float_t> OtherPV_score;
    TTreeReaderArrayWrapper<Float_t> OtherPV_z;
    TTreeReaderValueWrapper<Int_t> nOtherPV;

    // PFCand
    TTreeReaderArrayWrapper<Float_t> PFCand_eta;
    TTreeReaderArrayWrapper<Float_t> PFCand_mass;
    TTreeReaderArrayWrapper<Int_t> PFCand_pdgId;
    TTreeReaderArrayWrapper<Float_t> PFCand_phi;
    TTreeReaderArrayWrapper<Float_t> PFCand_pt;
    TTreeReaderValueWrapper<Int_t> nPFCand;

    // PFMET
    TTreeReaderValueWrapper<Float_t> PFMET_covXX;
    TTreeReaderValueWrapper<Float_t> PFMET_covXY;
    TTreeReaderValueWrapper<Float_t> PFMET_covYY;
    TTreeReaderValueWrapper<Float_t> PFMET_phi;
    TTreeReaderValueWrapper<Float_t> PFMET_phiUnclusteredDown;
    TTreeReaderValueWrapper<Float_t> PFMET_phiUnclusteredUp;
    TTreeReaderValueWrapper<Float_t> PFMET_pt;
    TTreeReaderValueWrapper<Float_t> PFMET_ptUnclusteredDown;
    TTreeReaderValueWrapper<Float_t> PFMET_ptUnclusteredUp;
    TTreeReaderValueWrapper<Float_t> PFMET_significance;
    TTreeReaderValueWrapper<Float_t> PFMET_sumEt;
    TTreeReaderValueWrapper<Float_t> PFMET_sumPtUnclustered;

    // PSWeight
    TTreeReaderArrayWrapper<Float_t> PSWeight;
    TTreeReaderValueWrapper<Int_t> nPSWeight;

    // PV
    TTreeReaderValueWrapper<Float_t> PV_chi2;
    TTreeReaderValueWrapper<Float_t> PV_ndof;
    TTreeReaderValueWrapper<UChar_t> PV_npvs;
    TTreeReaderValueWrapper<UChar_t> PV_npvsGood;
    TTreeReaderValueWrapper<Float_t> PV_score;
    TTreeReaderValueWrapper<Float_t> PV_sumpt2;
    TTreeReaderValueWrapper<Float_t> PV_sumpx;
    TTreeReaderValueWrapper<Float_t> PV_sumpy;
    TTreeReaderValueWrapper<Float_t> PV_x;
    TTreeReaderValueWrapper<Float_t> PV_y;
    TTreeReaderValueWrapper<Float_t> PV_z;

    // Photon
    TTreeReaderArrayWrapper<UChar_t> Photon_cutBased;
    TTreeReaderArrayWrapper<Float_t> Photon_ecalPFClusterIso;
    TTreeReaderArrayWrapper<Short_t> Photon_electronIdx;
    TTreeReaderArrayWrapper<Bool_t> Photon_electronVeto;
    TTreeReaderArrayWrapper<Float_t> Photon_energyErr;
    TTreeReaderArrayWrapper<Float_t> Photon_energyRaw;
    TTreeReaderArrayWrapper<Float_t> Photon_esEffSigmaRR;
    TTreeReaderArrayWrapper<Float_t> Photon_esEnergyOverRawE;
    TTreeReaderArrayWrapper<Float_t> Photon_eta;
    TTreeReaderArrayWrapper<Float_t> Photon_etaWidth;
    TTreeReaderArrayWrapper<UChar_t> Photon_genPartFlav;
    TTreeReaderArrayWrapper<Short_t> Photon_genPartIdx;
    TTreeReaderArrayWrapper<Float_t> Photon_haloTaggerMVAVal;
    TTreeReaderArrayWrapper<Bool_t> Photon_hasConversionTracks;
    TTreeReaderArrayWrapper<Float_t> Photon_hcalPFClusterIso;
    TTreeReaderArrayWrapper<Float_t> Photon_hoe;
    TTreeReaderArrayWrapper<Float_t> Photon_hoe_PUcorr;
    TTreeReaderArrayWrapper<Float_t> Photon_hoe_Tower;
    TTreeReaderArrayWrapper<Bool_t> Photon_isScEtaEB;
    TTreeReaderArrayWrapper<Bool_t> Photon_isScEtaEE;
    TTreeReaderArrayWrapper<Short_t> Photon_jetIdx;
    TTreeReaderArrayWrapper<Float_t> Photon_mvaID;
    TTreeReaderArrayWrapper<Bool_t> Photon_mvaID_WP80;
    TTreeReaderArrayWrapper<Bool_t> Photon_mvaID_WP90;
    TTreeReaderArrayWrapper<Float_t> Photon_pfChargedIso;
    TTreeReaderArrayWrapper<Float_t> Photon_pfChargedIsoPFPV;
    TTreeReaderArrayWrapper<Float_t> Photon_pfChargedIsoWorstVtx;
    TTreeReaderArrayWrapper<Float_t> Photon_pfPhoIso03;
    TTreeReaderArrayWrapper<Float_t> Photon_pfRelIso03_all_quadratic;
    TTreeReaderArrayWrapper<Float_t> Photon_pfRelIso03_chg_quadratic;
    TTreeReaderArrayWrapper<Float_t> Photon_phi;
    TTreeReaderArrayWrapper<Float_t> Photon_phiWidth;
    TTreeReaderArrayWrapper<Bool_t> Photon_pixelSeed;
    TTreeReaderArrayWrapper<Float_t> Photon_pt;
    TTreeReaderArrayWrapper<Float_t> Photon_r9;
    TTreeReaderArrayWrapper<Float_t> Photon_s4;
    TTreeReaderArrayWrapper<UChar_t> Photon_seedGain;
    TTreeReaderArrayWrapper<Short_t> Photon_seediEtaOriX;
    TTreeReaderArrayWrapper<Short_t> Photon_seediPhiOriY;
    TTreeReaderArrayWrapper<Float_t> Photon_sieie;
    TTreeReaderArrayWrapper<Float_t> Photon_sieip;
    TTreeReaderArrayWrapper<Float_t> Photon_sipip;
    TTreeReaderArrayWrapper<Float_t> Photon_superclusterEta;
    TTreeReaderArrayWrapper<Float_t> Photon_trkSumPtHollowConeDR03;
    TTreeReaderArrayWrapper<Float_t> Photon_trkSumPtSolidConeDR04;
    TTreeReaderArrayWrapper<Int_t> Photon_vidNestedWPBitmap;
    TTreeReaderArrayWrapper<Float_t> Photon_x_calo;
    TTreeReaderArrayWrapper<Float_t> Photon_y_calo;
    TTreeReaderArrayWrapper<Float_t> Photon_z_calo;
    TTreeReaderValueWrapper<Int_t> nPhoton;

    // Pileup
    TTreeReaderValueWrapper<Float_t> Pileup_gpudensity;
    TTreeReaderValueWrapper<Int_t> Pileup_nPU;
    TTreeReaderValueWrapper<Float_t> Pileup_nTrueInt;
    TTreeReaderValueWrapper<Float_t> Pileup_pthatmax;
    TTreeReaderValueWrapper<Float_t> Pileup_pudensity;
    TTreeReaderValueWrapper<Int_t> Pileup_sumEOOT;
    TTreeReaderValueWrapper<Int_t> Pileup_sumLOOT;

    // PuppiMET
    TTreeReaderValueWrapper<Float_t> PuppiMET_covXX;
    TTreeReaderValueWrapper<Float_t> PuppiMET_covXY;
    TTreeReaderValueWrapper<Float_t> PuppiMET_covYY;
    TTreeReaderValueWrapper<Float_t> PuppiMET_phi;
    TTreeReaderValueWrapper<Float_t> PuppiMET_phiUnclusteredDown;
    TTreeReaderValueWrapper<Float_t> PuppiMET_phiUnclusteredUp;
    TTreeReaderValueWrapper<Float_t> PuppiMET_pt;
    TTreeReaderValueWrapper<Float_t> PuppiMET_ptUnclusteredDown;
    TTreeReaderValueWrapper<Float_t> PuppiMET_ptUnclusteredUp;
    TTreeReaderValueWrapper<Float_t> PuppiMET_significance;
    TTreeReaderValueWrapper<Float_t> PuppiMET_sumEt;
    TTreeReaderValueWrapper<Float_t> PuppiMET_sumPtUnclustered;

    // Rho
    TTreeReaderValueWrapper<Float_t> Rho_fixedGridRhoAll;
    TTreeReaderValueWrapper<Float_t> Rho_fixedGridRhoFastjetAll;
    TTreeReaderValueWrapper<Float_t> Rho_fixedGridRhoFastjetCentral;
    TTreeReaderValueWrapper<Float_t> Rho_fixedGridRhoFastjetCentralCalo;
    TTreeReaderValueWrapper<Float_t> Rho_fixedGridRhoFastjetCentralChargedPileUp;
    TTreeReaderValueWrapper<Float_t> Rho_fixedGridRhoFastjetCentralNeutral;

    // SV
    TTreeReaderArrayWrapper<Short_t> SV_charge;
    TTreeReaderArrayWrapper<Float_t> SV_chi2;
    TTreeReaderArrayWrapper<Float_t> SV_dlen;
    TTreeReaderArrayWrapper<Float_t> SV_dlenSig;
    TTreeReaderArrayWrapper<Float_t> SV_dxy;
    TTreeReaderArrayWrapper<Float_t> SV_dxySig;
    TTreeReaderArrayWrapper<Float_t> SV_eta;
    TTreeReaderArrayWrapper<Float_t> SV_mass;
    TTreeReaderArrayWrapper<Float_t> SV_ndof;
    TTreeReaderArrayWrapper<UChar_t> SV_ntracks;
    TTreeReaderArrayWrapper<Float_t> SV_pAngle;
    TTreeReaderArrayWrapper<Float_t> SV_phi;
    TTreeReaderArrayWrapper<Float_t> SV_pt;
    TTreeReaderArrayWrapper<Float_t> SV_x;
    TTreeReaderArrayWrapper<Float_t> SV_y;
    TTreeReaderArrayWrapper<Float_t> SV_z;
    TTreeReaderValueWrapper<Int_t> nSV;

    // SubGenJetAK8
    TTreeReaderArrayWrapper<Float_t> SubGenJetAK8_eta;
    TTreeReaderArrayWrapper<Float_t> SubGenJetAK8_mass;
    TTreeReaderArrayWrapper<Float_t> SubGenJetAK8_phi;
    TTreeReaderArrayWrapper<Float_t> SubGenJetAK8_pt;
    TTreeReaderValueWrapper<Int_t> nSubGenJetAK8;

    // SubJet
    TTreeReaderArrayWrapper<Float_t> SubJet_UParTAK4RegPtRawCorr;
    TTreeReaderArrayWrapper<Float_t> SubJet_UParTAK4RegPtRawCorrNeutrino;
    TTreeReaderArrayWrapper<Float_t> SubJet_UParTAK4RegPtRawRes;
    TTreeReaderArrayWrapper<Float_t> SubJet_UParTAK4V1RegPtRawCorr;
    TTreeReaderArrayWrapper<Float_t> SubJet_UParTAK4V1RegPtRawCorrNeutrino;
    TTreeReaderArrayWrapper<Float_t> SubJet_UParTAK4V1RegPtRawRes;
    TTreeReaderArrayWrapper<Float_t> SubJet_area;
    TTreeReaderArrayWrapper<Float_t> SubJet_btagDeepFlavB;
    TTreeReaderArrayWrapper<Float_t> SubJet_btagUParTAK4B;
    TTreeReaderArrayWrapper<Float_t> SubJet_eta;
    TTreeReaderArrayWrapper<UChar_t> SubJet_hadronFlavour;
    TTreeReaderArrayWrapper<Float_t> SubJet_mass;
    TTreeReaderArrayWrapper<Float_t> SubJet_n2b1;
    TTreeReaderArrayWrapper<Float_t> SubJet_n3b1;
    TTreeReaderArrayWrapper<UChar_t> SubJet_nBHadrons;
    TTreeReaderArrayWrapper<UChar_t> SubJet_nCHadrons;
    TTreeReaderArrayWrapper<Float_t> SubJet_phi;
    TTreeReaderArrayWrapper<Float_t> SubJet_pt;
    TTreeReaderArrayWrapper<Float_t> SubJet_rawFactor;
    TTreeReaderArrayWrapper<Short_t> SubJet_subGenJetAK8Idx;
    TTreeReaderArrayWrapper<Float_t> SubJet_tau1;
    TTreeReaderArrayWrapper<Float_t> SubJet_tau2;
    TTreeReaderArrayWrapper<Float_t> SubJet_tau3;
    TTreeReaderArrayWrapper<Float_t> SubJet_tau4;
    TTreeReaderValueWrapper<Int_t> nSubJet;

    // Tau
    TTreeReaderArrayWrapper<Float_t> Tau_IPx;
    TTreeReaderArrayWrapper<Float_t> Tau_IPy;
    TTreeReaderArrayWrapper<Float_t> Tau_IPz;
    TTreeReaderArrayWrapper<Short_t> Tau_charge;
    TTreeReaderArrayWrapper<Float_t> Tau_chargedIso;
    TTreeReaderArrayWrapper<UChar_t> Tau_decayMode;
    TTreeReaderArrayWrapper<Short_t> Tau_decayModePNet;
    TTreeReaderArrayWrapper<Short_t> Tau_decayModeUParT;
    TTreeReaderArrayWrapper<Float_t> Tau_dxy;
    TTreeReaderArrayWrapper<Float_t> Tau_dz;
    TTreeReaderArrayWrapper<Short_t> Tau_eleIdx;
    TTreeReaderArrayWrapper<Float_t> Tau_eta;
    TTreeReaderArrayWrapper<UChar_t> Tau_genPartFlav;
    TTreeReaderArrayWrapper<Short_t> Tau_genPartIdx;
    TTreeReaderArrayWrapper<Bool_t> Tau_hasRefitSV;
    TTreeReaderArrayWrapper<Bool_t> Tau_idAntiEleDeadECal;
    TTreeReaderArrayWrapper<UChar_t> Tau_idAntiMu;
    TTreeReaderArrayWrapper<Bool_t> Tau_idDecayModeNewDMs;
    TTreeReaderArrayWrapper<Bool_t> Tau_idDecayModeOldDMs;
    TTreeReaderArrayWrapper<UChar_t> Tau_idDeepTau2018v2p5VSe;
    TTreeReaderArrayWrapper<UChar_t> Tau_idDeepTau2018v2p5VSjet;
    TTreeReaderArrayWrapper<UChar_t> Tau_idDeepTau2018v2p5VSmu;
    TTreeReaderArrayWrapper<Float_t> Tau_ipLengthSig;
    TTreeReaderArrayWrapper<Short_t> Tau_jetIdx;
    TTreeReaderArrayWrapper<Float_t> Tau_leadTkDeltaEta;
    TTreeReaderArrayWrapper<Float_t> Tau_leadTkDeltaPhi;
    TTreeReaderArrayWrapper<Float_t> Tau_leadTkPtOverTauPt;
    TTreeReaderArrayWrapper<Float_t> Tau_mass;
    TTreeReaderArrayWrapper<Short_t> Tau_muIdx;
    TTreeReaderArrayWrapper<UChar_t> Tau_nSVs;
    TTreeReaderArrayWrapper<Float_t> Tau_neutralIso;
    TTreeReaderArrayWrapper<Float_t> Tau_phi;
    TTreeReaderArrayWrapper<Float_t> Tau_photonsOutsideSignalCone;
    TTreeReaderArrayWrapper<Float_t> Tau_probDM0PNet;
    TTreeReaderArrayWrapper<Float_t> Tau_probDM0UParT;
    TTreeReaderArrayWrapper<Float_t> Tau_probDM10PNet;
    TTreeReaderArrayWrapper<Float_t> Tau_probDM10UParT;
    TTreeReaderArrayWrapper<Float_t> Tau_probDM11PNet;
    TTreeReaderArrayWrapper<Float_t> Tau_probDM11UParT;
    TTreeReaderArrayWrapper<Float_t> Tau_probDM1PNet;
    TTreeReaderArrayWrapper<Float_t> Tau_probDM1UParT;
    TTreeReaderArrayWrapper<Float_t> Tau_probDM2PNet;
    TTreeReaderArrayWrapper<Float_t> Tau_probDM2UParT;
    TTreeReaderArrayWrapper<Float_t> Tau_pt;
    TTreeReaderArrayWrapper<Float_t> Tau_ptCorrPNet;
    TTreeReaderArrayWrapper<Float_t> Tau_ptCorrUParT;
    TTreeReaderArrayWrapper<Float_t> Tau_puCorr;
    TTreeReaderArrayWrapper<Float_t> Tau_qConfPNet;
    TTreeReaderArrayWrapper<Float_t> Tau_qConfUParT;
    TTreeReaderArrayWrapper<Float_t> Tau_rawDeepTau2018v2p5VSe;
    TTreeReaderArrayWrapper<Float_t> Tau_rawDeepTau2018v2p5VSjet;
    TTreeReaderArrayWrapper<Float_t> Tau_rawDeepTau2018v2p5VSmu;
    TTreeReaderArrayWrapper<Float_t> Tau_rawIso;
    TTreeReaderArrayWrapper<Float_t> Tau_rawIsodR03;
    TTreeReaderArrayWrapper<Float_t> Tau_rawPNetVSe;
    TTreeReaderArrayWrapper<Float_t> Tau_rawPNetVSjet;
    TTreeReaderArrayWrapper<Float_t> Tau_rawPNetVSmu;
    TTreeReaderArrayWrapper<Float_t> Tau_rawUParTVSe;
    TTreeReaderArrayWrapper<Float_t> Tau_rawUParTVSjet;
    TTreeReaderArrayWrapper<Float_t> Tau_rawUParTVSmu;
    TTreeReaderArrayWrapper<Float_t> Tau_refitSVchi2;
    TTreeReaderArrayWrapper<Float_t> Tau_refitSVcov00;
    TTreeReaderArrayWrapper<Float_t> Tau_refitSVcov10;
    TTreeReaderArrayWrapper<Float_t> Tau_refitSVcov11;
    TTreeReaderArrayWrapper<Float_t> Tau_refitSVcov20;
    TTreeReaderArrayWrapper<Float_t> Tau_refitSVcov21;
    TTreeReaderArrayWrapper<Float_t> Tau_refitSVcov22;
    TTreeReaderArrayWrapper<Float_t> Tau_refitSVx;
    TTreeReaderArrayWrapper<Float_t> Tau_refitSVy;
    TTreeReaderArrayWrapper<Float_t> Tau_refitSVz;
    TTreeReaderArrayWrapper<Short_t> Tau_svIdx1;
    TTreeReaderArrayWrapper<Short_t> Tau_svIdx2;
    TTreeReaderValueWrapper<Int_t> nTau;

    // TauProd
    TTreeReaderArrayWrapper<Float_t> TauProd_eta;
    TTreeReaderArrayWrapper<Int_t> TauProd_pdgId;
    TTreeReaderArrayWrapper<Float_t> TauProd_phi;
    TTreeReaderArrayWrapper<Float_t> TauProd_pt;
    TTreeReaderArrayWrapper<Short_t> TauProd_tauIdx;
    TTreeReaderValueWrapper<Int_t> nTauProd;

    // TauSpinner
    TTreeReaderValueWrapper<Double_t> TauSpinner_weight_cp_0;
    TTreeReaderValueWrapper<Double_t> TauSpinner_weight_cp_0_alt;
    TTreeReaderValueWrapper<Double_t> TauSpinner_weight_cp_0p25;
    TTreeReaderValueWrapper<Double_t> TauSpinner_weight_cp_0p25_alt;
    TTreeReaderValueWrapper<Double_t> TauSpinner_weight_cp_0p375;
    TTreeReaderValueWrapper<Double_t> TauSpinner_weight_cp_0p375_alt;
    TTreeReaderValueWrapper<Double_t> TauSpinner_weight_cp_0p5;
    TTreeReaderValueWrapper<Double_t> TauSpinner_weight_cp_0p5_alt;
    TTreeReaderValueWrapper<Double_t> TauSpinner_weight_cp_minus0p25;
    TTreeReaderValueWrapper<Double_t> TauSpinner_weight_cp_minus0p25_alt;

    // TrackGenJetAK4
    TTreeReaderArrayWrapper<Float_t> TrackGenJetAK4_eta;
    TTreeReaderArrayWrapper<Float_t> TrackGenJetAK4_phi;
    TTreeReaderArrayWrapper<Float_t> TrackGenJetAK4_pt;
    TTreeReaderValueWrapper<Int_t> nTrackGenJetAK4;

    // TrigObj
    TTreeReaderArrayWrapper<Float_t> TrigObj_eta;
    TTreeReaderArrayWrapper<ULong64_t> TrigObj_filterBits;
    TTreeReaderArrayWrapper<UShort_t> TrigObj_id;
    TTreeReaderArrayWrapper<Short_t> TrigObj_l1charge;
    TTreeReaderArrayWrapper<Int_t> TrigObj_l1iso;
    TTreeReaderArrayWrapper<Float_t> TrigObj_l1pt;
    TTreeReaderArrayWrapper<Float_t> TrigObj_l1pt_2;
    TTreeReaderArrayWrapper<Float_t> TrigObj_l2pt;
    TTreeReaderArrayWrapper<Float_t> TrigObj_phi;
    TTreeReaderArrayWrapper<Float_t> TrigObj_pt;
    TTreeReaderValueWrapper<Int_t> nTrigObj;

    // TrkMET
    TTreeReaderValueWrapper<Float_t> TrkMET_phi;
    TTreeReaderValueWrapper<Float_t> TrkMET_pt;
    TTreeReaderValueWrapper<Float_t> TrkMET_sumEt;

    // boostedTau
    TTreeReaderArrayWrapper<Int_t> boostedTau_charge;
    TTreeReaderArrayWrapper<Float_t> boostedTau_chargedIso;
    TTreeReaderArrayWrapper<Int_t> boostedTau_decayMode;
    TTreeReaderArrayWrapper<Float_t> boostedTau_eta;
    TTreeReaderArrayWrapper<UChar_t> boostedTau_genPartFlav;
    TTreeReaderArrayWrapper<Short_t> boostedTau_genPartIdx;
    TTreeReaderArrayWrapper<UChar_t> boostedTau_idAntiEle2018;
    TTreeReaderArrayWrapper<UChar_t> boostedTau_idAntiMu;
    TTreeReaderArrayWrapper<UChar_t> boostedTau_idMVAnewDM2017v2;
    TTreeReaderArrayWrapper<UChar_t> boostedTau_idMVAoldDM2017v2;
    TTreeReaderArrayWrapper<Short_t> boostedTau_jetIdx;
    TTreeReaderArrayWrapper<Float_t> boostedTau_leadTkDeltaEta;
    TTreeReaderArrayWrapper<Float_t> boostedTau_leadTkDeltaPhi;
    TTreeReaderArrayWrapper<Float_t> boostedTau_leadTkPtOverTauPt;
    TTreeReaderArrayWrapper<Float_t> boostedTau_mass;
    TTreeReaderArrayWrapper<Float_t> boostedTau_neutralIso;
    TTreeReaderArrayWrapper<Float_t> boostedTau_phi;
    TTreeReaderArrayWrapper<Float_t> boostedTau_photonsOutsideSignalCone;
    TTreeReaderArrayWrapper<Float_t> boostedTau_pt;
    TTreeReaderArrayWrapper<Float_t> boostedTau_puCorr;
    TTreeReaderArrayWrapper<Float_t> boostedTau_rawAntiEle2018;
    TTreeReaderArrayWrapper<Short_t> boostedTau_rawAntiEleCat2018;
    TTreeReaderArrayWrapper<Float_t> boostedTau_rawBoostedDeepTauRunIIv2p0VSe;
    TTreeReaderArrayWrapper<Float_t> boostedTau_rawBoostedDeepTauRunIIv2p0VSjet;
    TTreeReaderArrayWrapper<Float_t> boostedTau_rawBoostedDeepTauRunIIv2p0VSmu;
    TTreeReaderArrayWrapper<Float_t> boostedTau_rawIso;
    TTreeReaderArrayWrapper<Float_t> boostedTau_rawIsodR03;
    TTreeReaderArrayWrapper<Float_t> boostedTau_rawMVAnewDM2017v2;
    TTreeReaderArrayWrapper<Float_t> boostedTau_rawMVAoldDM2017v2;
    TTreeReaderValueWrapper<Int_t> nboostedTau;

    // bunchCrossing
    TTreeReaderValueWrapper<UInt_t> bunchCrossing;

    // event
    TTreeReaderValueWrapper<ULong64_t> event;

    // genTtbarId
    TTreeReaderValueWrapper<Int_t> genTtbarId;

    // genWeight
    TTreeReaderValueWrapper<Float_t> genWeight;

    // luminosityBlock
    TTreeReaderValueWrapper<UInt_t> luminosityBlock;

    // orbitNumber
    TTreeReaderValueWrapper<UInt_t> orbitNumber;

    // run
    TTreeReaderValueWrapper<UInt_t> RunNumber;

    //=================================================
    std::map<TString, pair<Bool_t *, float>> TriggerMap;
};

#endif
