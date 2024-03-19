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

    //virtual TString GetEra() const { return DataEra; }
    //virtual TString GetEraShort() const;
    //virtual int GetYear() const { return DataYear; }

    TChain *fChain=nullptr;

    // Declaration of leaf types
    // Muon
    static constexpr int kMaxMuon = 20;
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
    static constexpr int kMaxElectron = 20;
    Int_t nElectron;
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

};

#endif
