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
    static constexpr int kMaxGen = 50;
    Int_t nGenPart;
    Int_t GenPart_pdgId[kMaxGen];
    Int_t GenPart_status[kMaxGen];
    UShort_t GenPart_statusFlags[kMaxGen];
    Short_t GenPart_genPartIdxMother[kMaxGen];
    Float_t GenPart_pt[kMaxGen];
    Float_t GenPart_eta[kMaxGen];
    Float_t GenPart_phi[kMaxGen];
    Float_t GenPart_mass[kMaxGen];
    //LHE
    static constexpr int kMaxLHE = 50;
    Int_t nLHEPart;
    Float_t LHEPart_pt[kMaxLHE];
    Float_t LHEPart_eta[kMaxLHE];
    Float_t LHEPart_phi[kMaxLHE];
    Float_t LHEPart_mass[kMaxLHE];
    Float_t LHEPart_incomingpz[kMaxLHE];
    Int_t LHEPart_pdgId[kMaxLHE];
    Int_t LHEPart_status[kMaxLHE];
    Int_t LHEPart_spin[kMaxLHE];

    // Muon
    //static constexpr int kMaxMuon = 8;
    static constexpr int kMaxMuon = 50;
    Int_t  nMuon;
    Float_t Muon_pt[kMaxMuon];
    Float_t Muon_eta[kMaxMuon];
    Float_t Muon_phi[kMaxMuon];
    Float_t Muon_mass[kMaxMuon];
    Int_t   Muon_charge[kMaxMuon];
    Float_t Muon_dxy[kMaxMuon];
    Float_t Muon_dxyErr[kMaxMuon];
    Float_t Muon_dz[kMaxMuon];
    Float_t Muon_dzErr[kMaxMuon];
    Float_t Muon_ip3d[kMaxMuon];
    Float_t Muon_sip3d[kMaxMuon];
    Float_t Muon_pfRelIso03_all[kMaxMuon];
    Float_t Muon_pfRelIso04_all[kMaxMuon];
    Float_t Muon_miniPFRelIso_all[kMaxMuon];
    Float_t Muon_tkRelIso[kMaxMuon];
    Bool_t  Muon_isTracker[kMaxMuon];
    Bool_t  Muon_isStandalone[kMaxMuon];
    Bool_t  Muon_isGlobal[kMaxMuon];
    Bool_t  Muon_looseId[kMaxMuon];
    Bool_t  Muon_mediumId[kMaxMuon];
    Bool_t  Muon_mediumPromptId[kMaxMuon];
    Bool_t  Muon_tightId[kMaxMuon];
    Bool_t  Muon_softId[kMaxMuon];
    Bool_t  Muon_softMvaId[kMaxMuon];
    Bool_t  Muon_triggerIdLoose[kMaxMuon];
    UChar_t Muon_highPtId[kMaxMuon];
    UChar_t Muon_miniIsoId[kMaxMuon];
    UChar_t Muon_multiIsoId[kMaxMuon];
    UChar_t Muon_mvaMuId[kMaxMuon];
    //UChar_t Muon_mvaLowPtId[kMaxMuon];
    UChar_t Muon_pfIsoId[kMaxMuon];
    UChar_t Muon_puppiIsoId[kMaxMuon];
    UChar_t Muon_tkIsoId[kMaxMuon];
    Float_t Muon_softMva[kMaxMuon];
    Float_t Muon_mvaLowPt[kMaxMuon];
    Float_t Muon_mvaTTH[kMaxMuon];
    // Electron
    //static constexpr int kMaxElectron = 8;
    static constexpr int kMaxElectron = 50;
    Int_t nElectron;
    UChar_t Electron_genPartFlav[kMaxElectron];
    Short_t Electron_genPartIdx[kMaxElectron];
    Float_t Electron_pt[kMaxElectron];
    Float_t Electron_eta[kMaxElectron];
    Float_t Electron_phi[kMaxElectron];
    Float_t Electron_mass[kMaxElectron];
    Int_t   Electron_charge[kMaxElectron];
    Float_t Electron_dxy[kMaxElectron];
    Float_t Electron_dxyErr[kMaxElectron];
    Float_t Electron_dz[kMaxElectron];
    Float_t Electron_dzErr[kMaxElectron];
    Float_t Electron_ip3d[kMaxElectron];
    Float_t Electron_sip3d[kMaxElectron];
    Float_t Electron_pfRelIso03_all[kMaxElectron];
    Float_t Electron_miniPFRelIso_all[kMaxElectron];
    Float_t Electron_energyErr[kMaxElectron];
    Bool_t  Electron_convVeto[kMaxElectron];
    UChar_t Electron_lostHits[kMaxElectron];
    UChar_t Electron_seedGain[kMaxElectron];
    UChar_t Electron_tightCharge[kMaxElectron];
    Float_t Electron_sieie[kMaxElectron];
    Float_t Electron_hoe[kMaxElectron];
    Float_t Electron_eInvMinusPInv[kMaxElectron];
    Float_t Electron_dr03EcalRecHitSumEt[kMaxElectron];
    Float_t Electron_dr03HcalDepth1TowerSumEt[kMaxElectron];
    Float_t Electron_dr03TkSumPt[kMaxElectron];
    Float_t Electron_dr03TkSumPtHEEP[kMaxElectron];
    Float_t Electron_deltaEtaSC[kMaxElectron];
    Bool_t  Electron_mvaIso_WP80[kMaxElectron];
    Bool_t  Electron_mvaIso_WP90[kMaxElectron];
    Bool_t  Electron_mvaNoIso_WP80[kMaxElectron];
    Bool_t  Electron_mvaNoIso_WP90[kMaxElectron];
    Bool_t  Electron_cutBased_HEEP[kMaxElectron];
    UChar_t Electron_cutBased[kMaxElectron];
    Float_t Electron_mvaIso[kMaxElectron];
    Float_t Electron_mvaNoIso[kMaxElectron];
    Float_t Electron_mvaTTH[kMaxElectron];
    Float_t Electron_r9[kMaxElectron];
    // Jet
    // static constexpr int kMaxJet = 20;
    static constexpr int kMaxJet = 50;
    Int_t nJet; 
    Float_t Jet_PNetRegPtRawCorr[kMaxJet];
    Float_t Jet_PNetRegPtRawCorrNeutrino[kMaxJet];
    Float_t Jet_PNetRegPtRawRes[kMaxJet];
    Float_t Jet_area[kMaxJet];
    Float_t Jet_btagDeepFlavB[kMaxJet];
    Float_t Jet_btagDeepFlavCvB[kMaxJet];
    Float_t Jet_btagDeepFlavCvL[kMaxJet];
    Float_t Jet_btagDeepFlavQG[kMaxJet];
    Float_t Jet_btagPNetB[kMaxJet];
    Float_t Jet_btagPNetCvB[kMaxJet];
    Float_t Jet_btagPNetCvL[kMaxJet];
    Float_t Jet_btagPNetQvG[kMaxJet];
    Float_t Jet_btagPNetTauVJet[kMaxJet];
    Float_t Jet_btagRobustParTAK4B[kMaxJet];
    Float_t Jet_btagRobustParTAK4CvB[kMaxJet];
    Float_t Jet_btagRobustParTAK4CvL[kMaxJet];
    Float_t Jet_btagRobustParTAK4QG[kMaxJet];
    Float_t Jet_chEmEF[kMaxJet];
    Float_t Jet_chHEF[kMaxJet];
    Short_t Jet_electronIdx1[kMaxJet];
    Short_t Jet_electronIdx2[kMaxJet];
    Float_t Jet_eta[kMaxJet];
    Short_t Jet_genJetIdx[kMaxJet];
    UChar_t Jet_hadronFlavour[kMaxJet];
    Int_t Jet_hfadjacentEtaStripsSize[kMaxJet];
    Int_t Jet_hfcentralEtaStripSize[kMaxJet];
    Float_t Jet_hfsigmaEtaEta[kMaxJet];
    Float_t Jet_hfsigmaPhiPhi[kMaxJet];
    UChar_t Jet_jetId[kMaxJet];
    Float_t Jet_mass[kMaxJet];
    Float_t Jet_muEF[kMaxJet];
    Short_t Jet_muonIdx1[kMaxJet];
    Short_t Jet_muonIdx2[kMaxJet];
    Float_t Jet_muonSubtrFactor[kMaxJet];
    UChar_t Jet_nConstituents[kMaxJet];
    UChar_t Jet_nElectrons[kMaxJet];
    UChar_t Jet_nMuons[kMaxJet];
    UChar_t Jet_nSVs[kMaxJet];
    Float_t Jet_neEmEF[kMaxJet];
    Float_t Jet_neHEF[kMaxJet];
    Short_t Jet_partonFlavour[kMaxJet];
    Float_t Jet_phi[kMaxJet];
    Float_t Jet_pt[kMaxJet];
    Float_t Jet_rawFactor[kMaxJet];
    Short_t Jet_svIdx1[kMaxJet];
    Short_t Jet_svIdx2[kMaxJet];
    //Tau
    //static constexpr int kMaxTau = 7;
    static constexpr int kMaxTau = 20;
    Int_t nTau;
    Float_t Tau_pt[kMaxTau];
    Float_t Tau_eta[kMaxTau];
    Float_t Tau_phi[kMaxTau];
    Float_t Tau_mass[kMaxTau];
    Short_t Tau_charge[kMaxTau];
    Float_t Tau_dxy[kMaxTau];
    Float_t Tau_dz[kMaxTau];
    UChar_t Tau_idDeepTau2018v2p5VSe[kMaxTau];
    UChar_t Tau_idDeepTau2018v2p5VSjet[kMaxTau];
    UChar_t Tau_idDeepTau2018v2p5VSmu[kMaxTau];
    UChar_t Tau_decayMode[kMaxTau];
    Bool_t  Tau_idDecayModeNewDMs[kMaxTau];
    UChar_t Tau_genPartFlav[kMaxTau];
    Short_t Tau_genPartIdx[kMaxTau];
    // FatJet
    //static constexpr int kMaxFatJet = 6;
    static constexpr int kMaxFatJet = 20;
    Int_t nFatJet;
    Float_t FatJet_pt[kMaxFatJet];
    Float_t FatJet_eta[kMaxFatJet];
    Float_t FatJet_phi[kMaxFatJet];
    Float_t FatJet_mass[kMaxFatJet];
    Float_t FatJet_msoftdrop[kMaxFatJet];
    Float_t FatJet_area[kMaxFatJet];
    Short_t FatJet_genJetAK8Idx[kMaxFatJet];
    UChar_t FatJet_jetId[kMaxFatJet];
    Float_t FatJet_lsf3[kMaxFatJet];
    UChar_t FatJet_nBHadrons[kMaxFatJet];
    UChar_t FatJet_nCHadrons[kMaxFatJet];
    UChar_t FatJet_nConstituents[kMaxFatJet];
    Float_t FatJet_btagDDBvLV2[kMaxFatJet];
    Float_t FatJet_btagDDCvBV2[kMaxFatJet];
    Float_t FatJet_btagDDCvLV2[kMaxFatJet];
    Float_t FatJet_btagDeepB[kMaxFatJet];
    Float_t FatJet_btagHbb[kMaxFatJet];
    Float_t FatJet_particleNetWithMass_H4qvsQCD[kMaxFatJet];
    Float_t FatJet_particleNetWithMass_HccvsQCD[kMaxFatJet];
    Float_t FatJet_particleNetWithMass_HbbvsQCD[kMaxFatJet];
    Float_t FatJet_particleNetWithMass_QCD[kMaxFatJet];
    Float_t FatJet_particleNetWithMass_TvsQCD[kMaxFatJet];
    Float_t FatJet_particleNetWithMass_WvsQCD[kMaxFatJet];
    Float_t FatJet_particleNetWithMass_ZvsQCD[kMaxFatJet];
    Float_t FatJet_particleNet_QCD[kMaxFatJet];
    Float_t FatJet_particleNet_QCD0HF[kMaxFatJet];
    Float_t FatJet_particleNet_QCD1HF[kMaxFatJet];
    Float_t FatJet_particleNet_QCD2HF[kMaxFatJet];
    Float_t FatJet_particleNet_XbbVsQCD[kMaxFatJet];
    Float_t FatJet_particleNet_XccVsQCD[kMaxFatJet];
    Float_t FatJet_particleNet_XqqVsQCD[kMaxFatJet];
    Float_t FatJet_particleNet_XggVsQCD[kMaxFatJet];
    Float_t FatJet_particleNet_XteVsQCD[kMaxFatJet];
    Float_t FatJet_particleNet_XtmVsQCD[kMaxFatJet];
    Float_t FatJet_particleNet_XttVsQCD[kMaxFatJet];
    Float_t FatJet_particleNet_massCorr[kMaxFatJet];
    Float_t FatJet_tau1[kMaxFatJet];
    Float_t FatJet_tau2[kMaxFatJet];
    Float_t FatJet_tau3[kMaxFatJet];
    Float_t FatJet_tau4[kMaxFatJet];
    Short_t FatJet_subJetIdx1[kMaxFatJet];
    Short_t FatJet_subJetIdx2[kMaxFatJet];

    // GenJet
    //static constexpr int kMaxGenJet = 25;
    static constexpr int kMaxGenJet = 50;
    Int_t nGenJet;
    Float_t GenJet_eta[kMaxGenJet];
    Float_t GenJet_mass[kMaxGenJet];
    Float_t GenJet_phi[kMaxGenJet];
    Float_t GenJet_pt[kMaxGenJet];
    Short_t GenJet_partonFlavour[kMaxGenJet];
    UChar_t GenJet_hadronFlavour[kMaxGenJet];
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
