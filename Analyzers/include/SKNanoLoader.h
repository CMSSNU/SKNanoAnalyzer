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
    //virtual void executeEvent(){};
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
    RVec<Float_t> *Muon_pt;
    RVec<Float_t> *Muon_eta;
    RVec<Float_t> *Muon_phi;
    RVec<Float_t> *Muon_mass;
    RVec<Int_t>   *Muon_charge;
    RVec<Float_t> *Muon_dxy;
    RVec<Float_t> *Muon_dxyErr;
    RVec<Float_t> *Muon_dz;
    RVec<Float_t> *Muon_dzErr;
    RVec<Float_t> *Muon_ip3d;
    RVec<Float_t> *Muon_sip3d;
    RVec<Float_t> *Muon_pfRelIso03_all;
    RVec<Float_t> *Muon_pfRelIso04_all;
    RVec<Float_t> *Muon_miniPFRelIso_all;
    RVec<Float_t> *Muon_tkRelIso;

    // list of branches
    TBranch *b_Muon_pt;
    TBranch *b_Muon_eta;
    TBranch *b_Muon_phi;
    TBranch *b_Muon_mass;
    TBranch *b_Muon_charge;
    TBranch *b_Muon_dxy;
    TBranch *b_Muon_dxyErr;
    TBranch *b_Muon_dz;
    TBranch *b_Muon_dzErr;
    TBranch *b_Muon_ip3d;
    TBranch *b_Muon_sip3d;
    TBranch *b_Muon_pfRelIso03_all;
    TBranch *b_Muon_pfRelIso04_all;
    TBranch *b_Muon_miniPFRelIso_all;
    TBranch *b_Muon_tkRelIso;

};

#endif
