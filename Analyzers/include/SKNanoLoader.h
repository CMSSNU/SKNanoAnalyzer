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
    static constexpr int kMaxMuon = 50;
    UInt_t  nMuon;
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

};

#endif
