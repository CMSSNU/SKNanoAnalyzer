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
using namespace ROOT::VecOps;

class SKNanoLoader {
public:
    SKNanoLoader();
    virtual ~SKNanoLoader();

    //virtual long GetEntry(long entry);
    virtual void SetTreeName(TString tname) { fChain = new TChain(tname); }
    virtual int AddFile(TString filename) { return fChain->Add(filename, -1); }

    long MaxEvent, NSkipEvent;
    int LogEvery;
    bool IsDATA;
    TString DataStream;
    TString MCSample;
    //bool IsFastSim;
    int DataYear;
    TString DataEra;
    float xsec, sumW, sumSign;
    RVec<TString> Userflags;

    virtual void Init();
    virtual void SetMaxLeafSize();
    virtual void Loop();

    //virtual void beginEvent(){};
    virtual void executeEvent(){};
    //virtual void endEvent(){};

    virtual void SetEra(TString era) {
        DataEra=era;
        DataYear=TString(era(0,4)).Atoi();
    }
    virtual TString GetEra() const { return DataEra; }
    //virtual TString GetEraShort() const;
    virtual int GetYear() const { return DataYear; }

    TChain *fChain=nullptr;
    ROOT::RDataFrame *df=nullptr;

    // Declaration of leaf types
    // PDFs
    Int_t Generator_id1;
    Int_t Generator_id2;
    Float_t Generator_x1;
    Float_t Generator_x2;
    Float_t Generator_xpdf1;
    Float_t Generator_xpdf2;
    Float_t Generator_scalePDF;
    Float_t Generator_weight;
    // Weights
    Float_t genWeight;
    Float_t LHEWeight_originalXWGTUP;
    static constexpr int nLHEPdfWeight = 110; // 325300 - 325402
    static constexpr int nLHEScaleWeight = 9;
    static constexpr int nPSWeight = 4; 
    Float_t LHEPdfWeight[nLHEPdfWeight];
    Float_t LHEScaleWeight[nLHEScaleWeight];
    Float_t PSWeight[nPSWeight];
    // Event
    Int_t Pileup_nPU;
    Float_t Pileup_nTrueInt;
    UChar_t PV_npvsGood;
    //Gen
    Int_t nGenPart;
    RVec<Int_t> GenPart_pdgId;
    RVec<Int_t> GenPart_status;
    RVec<UShort_t> GenPart_statusFlags;
    RVec<Short_t> GenPart_genPartIdxMother;
    RVec<Float_t> GenPart_pt;
    RVec<Float_t> GenPart_eta;
    RVec<Float_t> GenPart_phi;
    RVec<Float_t> GenPart_mass;

    //LHE
    Int_t nLHEPart;
    RVec<Float_t> LHEPart_pt;
    RVec<Float_t> LHEPart_eta;
    RVec<Float_t> LHEPart_phi;
    RVec<Float_t> LHEPart_mass;
    RVec<Float_t> LHEPart_incomingpz;
    RVec<Int_t> LHEPart_pdgId;
    RVec<Int_t> LHEPart_status;
    RVec<Int_t> LHEPart_spin;

    // Muon
    Int_t  nMuon;
    RVec<Float_t> Muon_pt;
    RVec<Float_t> Muon_eta;
    RVec<Float_t> Muon_phi;
    RVec<Float_t> Muon_mass;
    RVec<Int_t> Muon_charge;
    RVec<Float_t> Muon_dxy;
    RVec<Float_t> Muon_dxyErr;
    RVec<Float_t> Muon_dz;
    RVec<Float_t> Muon_dzErr;
    RVec<Float_t> Muon_ip3d;
    RVec<Float_t> Muon_sip3d;
    RVec<Float_t> Muon_pfRelIso03_all;
    RVec<Float_t> Muon_pfRelIso04_all;
    RVec<Float_t> Muon_miniPFRelIso_all;
    RVec<Float_t> Muon_tkRelIso;
    RVec<Bool_t> Muon_isTracker;
    RVec<Bool_t> Muon_isStandalone;
    RVec<Bool_t> Muon_isGlobal;
    RVec<Bool_t> Muon_looseId;
    RVec<Bool_t> Muon_mediumId;
    RVec<Bool_t> Muon_mediumPromptId;
    RVec<Bool_t> Muon_tightId;
    RVec<Bool_t> Muon_softId;
    RVec<Bool_t> Muon_softMvaId;
    RVec<Bool_t> Muon_triggerIdLoose;
    RVec<UChar_t> Muon_highPtId;
    RVec<UChar_t> Muon_miniIsoId;
    RVec<UChar_t> Muon_multiIsoId;
    RVec<UChar_t> Muon_mvaMuId;
    // RVec<UChar_t> Muon_mvaLowPtId; 
    RVec<UChar_t> Muon_pfIsoId;
    RVec<UChar_t> Muon_puppiIsoId;
    RVec<UChar_t> Muon_tkIsoId;
    RVec<Float_t> Muon_softMva;
    RVec<Float_t> Muon_mvaLowPt;
    RVec<Float_t> Muon_mvaTTH;

    // Electron
    Int_t nElectron;
    RVec<UChar_t> Electron_genPartFlav;
    RVec<Short_t> Electron_genPartIdx;
    RVec<Float_t> Electron_pt;
    RVec<Float_t> Electron_eta;
    RVec<Float_t> Electron_phi;
    RVec<Float_t> Electron_mass;
    RVec<Int_t> Electron_charge;
    RVec<Float_t> Electron_dxy;
    RVec<Float_t> Electron_dxyErr;
    RVec<Float_t> Electron_dz;
    RVec<Float_t> Electron_dzErr;
    RVec<Float_t> Electron_ip3d;
    RVec<Float_t> Electron_sip3d;
    RVec<Float_t> Electron_pfRelIso03_all;
    RVec<Float_t> Electron_miniPFRelIso_all;
    RVec<Float_t> Electron_energyErr;
    RVec<Bool_t> Electron_convVeto;
    RVec<UChar_t> Electron_lostHits;
    RVec<UChar_t> Electron_seedGain;
    RVec<UChar_t> Electron_tightCharge;
    RVec<Float_t> Electron_sieie;
    RVec<Float_t> Electron_hoe;
    RVec<Float_t> Electron_eInvMinusPInv;
    RVec<Float_t> Electron_dr03EcalRecHitSumEt;
    RVec<Float_t> Electron_dr03HcalDepth1TowerSumEt;
    RVec<Float_t> Electron_dr03TkSumPt;
    RVec<Float_t> Electron_dr03TkSumPtHEEP;
    RVec<Float_t> Electron_deltaEtaSC;
    RVec<Bool_t> Electron_mvaIso_WP80;
    RVec<Bool_t> Electron_mvaIso_WP90;
    RVec<Bool_t> Electron_mvaNoIso_WP80;
    RVec<Bool_t> Electron_mvaNoIso_WP90;
    RVec<Bool_t> Electron_cutBased_HEEP;
    RVec<UChar_t> Electron_cutBased;
    RVec<Float_t> Electron_mvaIso;
    RVec<Float_t> Electron_mvaNoIso;
    RVec<Float_t> Electron_mvaTTH;
    RVec<Float_t> Electron_r9;

    // Photon
    Int_t nPhoton;
    RVec<Float_t> Photon_pt;
    RVec<Float_t> Photon_eta;
    RVec<Float_t> Photon_phi;
    RVec<Float_t> Photon_energyRaw;
    RVec<Float_t> Photon_hoe;
    RVec<UChar_t> Photon_cutBased;
    RVec<Float_t> Photon_mvaID;
    RVec<Bool_t> Photon_mvaID_WP80;
    RVec<Bool_t> Photon_mvaID_WP90;
    RVec<Bool_t> Photon_pixelSeed;
    RVec<Float_t> Photon_sieie;
    RVec<Bool_t> Photon_isScEtaEB;
    RVec<Bool_t> Photon_isScEtaEE;
    // Jet
    Int_t nJet;
    RVec<Float_t> Jet_PNetRegPtRawCorr;
    RVec<Float_t> Jet_PNetRegPtRawCorrNeutrino;
    RVec<Float_t> Jet_PNetRegPtRawRes;
    RVec<Float_t> Jet_area;
    RVec<Float_t> Jet_btagDeepFlavB;
    RVec<Float_t> Jet_btagDeepFlavCvB;
    RVec<Float_t> Jet_btagDeepFlavCvL;
    RVec<Float_t> Jet_btagDeepFlavQG;
    RVec<Float_t> Jet_btagPNetB;
    RVec<Float_t> Jet_btagPNetCvB;
    RVec<Float_t> Jet_btagPNetCvL;
    RVec<Float_t> Jet_btagPNetQvG;
    RVec<Float_t> Jet_btagPNetTauVJet;
    RVec<Float_t> Jet_btagRobustParTAK4B;
    RVec<Float_t> Jet_btagRobustParTAK4CvB;
    RVec<Float_t> Jet_btagRobustParTAK4CvL;
    RVec<Float_t> Jet_btagRobustParTAK4QG;
    RVec<Float_t> Jet_chEmEF;
    RVec<Float_t> Jet_chHEF;
    RVec<Short_t> Jet_electronIdx1;
    RVec<Short_t> Jet_electronIdx2;
    RVec<Float_t> Jet_eta;
    RVec<Short_t> Jet_genJetIdx;
    RVec<UChar_t> Jet_hadronFlavour;
    RVec<Int_t> Jet_hfadjacentEtaStripsSize;
    RVec<Int_t> Jet_hfcentralEtaStripSize;
    RVec<Float_t> Jet_hfsigmaEtaEta;
    RVec<Float_t> Jet_hfsigmaPhiPhi;
    RVec<UChar_t> Jet_jetId;
    RVec<Float_t> Jet_mass;
    RVec<Float_t> Jet_muEF;
    RVec<Short_t> Jet_muonIdx1;
    RVec<Short_t> Jet_muonIdx2;
    RVec<Float_t> Jet_muonSubtrFactor;
    RVec<UChar_t> Jet_nConstituents;
    RVec<UChar_t> Jet_nElectrons;
    RVec<UChar_t> Jet_nMuons;
    RVec<UChar_t> Jet_nSVs;
    RVec<Float_t> Jet_neEmEF;
    RVec<Float_t> Jet_neHEF;
    RVec<Short_t> Jet_partonFlavour;
    RVec<Float_t> Jet_phi;
    RVec<Float_t> Jet_pt;
    RVec<Float_t> Jet_rawFactor;
    RVec<Short_t> Jet_svIdx1;
    RVec<Short_t> Jet_svIdx2;

    //Tau
    Int_t nTau;
    RVec<Float_t> Tau_pt;
    RVec<Float_t> Tau_eta;
    RVec<Float_t> Tau_phi;
    RVec<Float_t> Tau_mass;
    RVec<Short_t> Tau_charge;
    RVec<Float_t> Tau_dxy;
    RVec<Float_t> Tau_dz;
    RVec<UChar_t> Tau_idDeepTau2018v2p5VSe;
    RVec<UChar_t> Tau_idDeepTau2018v2p5VSjet;
    RVec<UChar_t> Tau_idDeepTau2018v2p5VSmu;
    RVec<UChar_t> Tau_decayMode;
    RVec<Bool_t> Tau_idDecayModeNewDMs;
    RVec<UChar_t> Tau_genPartFlav;
    RVec<Short_t> Tau_genPartIdx;

    // FatJet
    Int_t nFatJet;
    RVec<Float_t> FatJet_pt;
    RVec<Float_t> FatJet_eta;
    RVec<Float_t> FatJet_phi;
    RVec<Float_t> FatJet_mass;
    RVec<Float_t> FatJet_msoftdrop;
    RVec<Float_t> FatJet_area;
    RVec<Short_t> FatJet_genJetAK8Idx;
    RVec<UChar_t> FatJet_jetId;
    RVec<Float_t> FatJet_lsf3;
    RVec<UChar_t> FatJet_nBHadrons;
    RVec<UChar_t> FatJet_nCHadrons;
    RVec<UChar_t> FatJet_nConstituents;
    RVec<Float_t> FatJet_btagDDBvLV2;
    RVec<Float_t> FatJet_btagDDCvBV2;
    RVec<Float_t> FatJet_btagDDCvLV2;
    RVec<Float_t> FatJet_btagDeepB;
    RVec<Float_t> FatJet_btagHbb;
    RVec<Float_t> FatJet_particleNetWithMass_H4qvsQCD;
    RVec<Float_t> FatJet_particleNetWithMass_HccvsQCD;
    RVec<Float_t> FatJet_particleNetWithMass_HbbvsQCD;
    RVec<Float_t> FatJet_particleNetWithMass_QCD;
    RVec<Float_t> FatJet_particleNetWithMass_TvsQCD;
    RVec<Float_t> FatJet_particleNetWithMass_WvsQCD;
    RVec<Float_t> FatJet_particleNetWithMass_ZvsQCD;
    RVec<Float_t> FatJet_particleNet_QCD;
    RVec<Float_t> FatJet_particleNet_QCD0HF;
    RVec<Float_t> FatJet_particleNet_QCD1HF;
    RVec<Float_t> FatJet_particleNet_QCD2HF;
    RVec<Float_t> FatJet_particleNet_XbbVsQCD;
    RVec<Float_t> FatJet_particleNet_XccVsQCD;
    RVec<Float_t> FatJet_particleNet_XqqVsQCD;
    RVec<Float_t> FatJet_particleNet_XggVsQCD;
    RVec<Float_t> FatJet_particleNet_XteVsQCD;
    RVec<Float_t> FatJet_particleNet_XtmVsQCD;
    RVec<Float_t> FatJet_particleNet_XttVsQCD;
    RVec<Float_t> FatJet_particleNet_massCorr;
    RVec<Float_t> FatJet_tau1;
    RVec<Float_t> FatJet_tau2;
    RVec<Float_t> FatJet_tau3;
    RVec<Float_t> FatJet_tau4;
    RVec<Short_t> FatJet_subJetIdx1;
    RVec<Short_t> FatJet_subJetIdx2;

    // GenJet
    Int_t nGenJet;
    RVec<Float_t> GenJet_eta;
    RVec<Float_t> GenJet_mass;
    RVec<Float_t> GenJet_phi;
    RVec<Float_t> GenJet_pt;
    RVec<Short_t> GenJet_partonFlavour;
    RVec<UChar_t> GenJet_hadronFlavour;

    //MET
    Float_t PuppiMET_pt;
    Float_t PuppiMET_phi;
    Float_t PuppiMET_sumEt;
    Float_t PuppiMET_ptJERDown;
    Float_t PuppiMET_phiJERDown;
    Float_t PuppiMET_ptJERUp;
    Float_t PuppiMET_phiJERUp;
    Float_t PuppiMET_ptJESDown;
    Float_t PuppiMET_phiJESDown;
    Float_t PuppiMET_ptJESUp;
    Float_t PuppiMET_phiJESUp;
    Float_t PuppiMET_ptUnclusteredDown;
    Float_t PuppiMET_phiUnclusteredDown;
    Float_t PuppiMET_ptUnclusteredUp;
    Float_t PuppiMET_phiUnclusteredUp;
    // rho
    Float_t fixedGridRhoFastjetAll;
    // Flag
    Bool_t Flag_METFilters; // What is this?
    Bool_t Flag_goodVertices;
    Bool_t Flag_globalSuperTightHalo2016Filter;
    Bool_t Flag_ECalDeadCellTriggerPrimitiveFilter;
    Bool_t Flag_BadPFMuonFilter;
    Bool_t Flag_BadPFMuonDzFilter;
    Bool_t Flag_hfNoisyHitsFilter;
    Bool_t Flag_ecalBadCalibFilter;
    Bool_t Flag_eeBadScFilter;
    //Bool_t Flag_ecalBadCalibFilter;
    UInt_t Run;
    // Trigger Saving Map
    std::map<TString, Bool_t *>
        TriggerMap;
};

#endif
