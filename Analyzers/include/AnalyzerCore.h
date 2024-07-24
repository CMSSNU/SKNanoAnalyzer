#ifndef AnalyzerCore_h
#define AnalyzerCore_h

#include <map>
#include <string>

#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TTree.h"
#include "TBranch.h"
#include "TString.h"
#include "TObjString.h"
#include "TRandom3.h"

#include "SKNanoLoader.h"
#include "Event.h"
#include "Particle.h"
#include "Lepton.h"
#include "Gen.h"
#include "LHE.h"
#include "Muon.h"
#include "Electron.h"
#include "Jet.h"
#include "GenJet.h"
#include "Tau.h"
#include "FatJet.h"
#include "Photon.h"
#include "Gen.h"

#include "LHAPDFHandler.h"
#include "PDFReweight.h"
#include "MCCorrection.h"
#include "JetTaggingParameter.h"

class AnalyzerCore: public SKNanoLoader {
public:
    AnalyzerCore();
    ~AnalyzerCore();

    virtual void initializeAnalyzer() {};
    virtual void executeEvent() {};

    inline static bool PtComparing(const Particle& p1, const Particle& p2) { return p1.Pt() > p2.Pt();}
    inline static bool PtComparingPtr(const Particle* p1, const Particle* p2) { return p1->Pt() > p2->Pt();}

    //MetFilter
    bool PassMetFilter(const RVec<Jet> &AllJets, const Event &ev);
    // PDF reweight
    PDFReweight *pdfReweight;
    float GetPDFWeight(LHAPDF::PDF *pdf_);
    float GetPDFReweight();
    float GetPDFReweight(int member);    
    // MCCorrection
    MCCorrection *mcCorr;
    //unique_ptr<CorrectionSet> csetMuon;
    //unique_ptr<CorrectionSet> csetElectron;;

    // MC weights
    float MCweight(bool usesign = true, bool norm_1invpb = true) const;

    // Get objects
    Event GetEvent(RVec<TString> HLT_List = {});
    RVec<Muon> GetAllMuons();
    RVec<Muon> GetMuons(const TString ID, const float ptmin, const float fetamax);
    RVec<Electron> GetAllElectrons();
    RVec<Jet> GetAllJets();
    RVec<Gen> GetAllGens();
    RVec<LHE> GetAllLHEs();
    RVec<Jet> GetJets(const TString id, const float ptmin, const float fetamax);
    RVec<Electron> GetElectrons(const TString id, const float ptmin, const float fetamax);
    RVec<Tau> GetAllTaus();
    RVec<FatJet> GetAllFatJets();
    RVec<GenJet> GetAllGenJets();
    RVec<Photon> GetAllPhotons();
    RVec<Photon> GetPhotons(TString id, double ptmin, double fetamax);

    // Select objects
    RVec<Muon> SelectMuons(const RVec<Muon> &muons, TString ID, const float ptmin, const float absetamax);
    RVec<Jet> SelectJets(const RVec<Jet> &jets, const TString id, const float ptmin, const float fetamax);
    RVec<Jet> JetsVetoLeptonInside(const RVec<Jet> &jets, const RVec<Electron> &electrons, const RVec<Muon> &muons, const float dR = 0.4);
    RVec<Electron> SelectElectrons(const RVec<Electron> &electrons, const TString id, const float ptmin, const float absetamax);
    RVec<Tau> SelectTaus(const RVec<Tau> &taus, const TString ID, const float ptmin, const float absetamax);
    // Functions
    float GetScaleVariation(const int &muF_syst, const int &muR_syst);
    inline float GetBTaggingWP(const JetTagging::JetFlavTagger &tagger, const JetTagging::JetFlavTaggerWP &wp) { return mcCorr->GetBTaggingWP(tagger, wp); }
    inline pair<float, float> GetCTaggingWP(const JetTagging::JetFlavTagger &tagger, const JetTagging::JetFlavTaggerWP &wp) { return mcCorr->GetCTaggingWP(tagger, wp); }
    inline float GetBTaggingWP(){ return mcCorr->GetBTaggingWP(); }
    inline pair<float, float> GetCTaggingWP(){ return mcCorr->GetCTaggingWP(); }

    unordered_map<int, int> GenJetMatching(const RVec<Jet> &jets, const RVec<GenJet> &genjets, const float &rho, const float dR = 0.2, const float pTJerCut = 3.);
    RVec<Jet> SmearJets(const RVec<Jet> &jets, const RVec<GenJet> &genjets);
    RVec<Jet> ScaleJets(const RVec<Jet> &jets, const int &syst, const TString &source = "");
    void SetOutfilePath(TString outpath);
    TH1F* GetHist1D(const string &histname);
    bool PassJetVetoMap(const RVec<Jet> &AllJet, const RVec<Muon> &AllMuon, const TString mapCategory = "jetvetomap");
    inline void FillCutFlow(const int &val,const int &maxCutN=10){
        FillHist("CutFlow", val, 1., maxCutN, 0, maxCutN);
    }
    void FillHist(const TString &histname, float value, float weight, int n_bin, float x_min, float x_max);
    void FillHist(const TString &histname, float value, float weight, int n_bin, float *xbins);
    void FillHist(const TString &histname, float value_x, float value_y, float weight, 
                                          int n_binx, float x_min, float x_max, 
                                          int n_biny, float y_min, float y_max);
    void FillHist(const TString &histname, float value_x, float value_y, float weight,
                                          int n_binx, float *xbins,
                                          int n_biny, float *ybins);
    void FillHist(const TString &histname, float value_x, float value_y, float value_z, float weight,
                                          int n_binx, float x_min, float x_max,
                                          int n_biny, float y_min, float y_max,
                                          int n_binz, float z_min, float z_max);
    void FillHist(const TString &histname, float value_x, float value_y, float value_z, float weight,
                                          int n_binx, float *xbins,
                                          int n_biny, float *ybins,
                                          int n_binz, float *zbins);

    TTree* NewTree(const TString &treename, const RVec<TString> &keeps = {}, const RVec<TString> &drops = {});
    TTree* GetTree(const TString &treename);
    inline void SetBranch(const TString &treename, const TString &branchname, float val) { this_floats.push_back(val); SetBranch(treename, branchname, (void*)(&this_floats.back()), branchname + "/F"); };
    inline void SetBranch(const TString &treename, const TString &branchname, double val) { this_floats.push_back(float(val)); SetBranch(treename, branchname, (void*)(&this_floats.back()), branchname + "/F"); };
    inline void SetBranch(const TString &treename, const TString &branchname, int val) { this_ints.push_back(val); SetBranch(treename, branchname, (void*)(&this_ints.back()), branchname + "/I"); };
    inline void SetBranch(const TString &treename, const TString &branchname, bool val) { SetBranch(treename, branchname, (void*)(&this_bools.back()), branchname + "/O"); };
    
    void FillTrees();
    virtual void WriteHist();

private:
    unordered_map<string, TH1F*> histmap1d;
    unordered_map<string, TH2F*> histmap2d;
    unordered_map<string, TH3F*> histmap3d;
    unordered_map<string, TTree*> treemap;
    unordered_map<TTree*, unordered_map<string, TBranch*>> branchmaps; 
    RVec<float> this_floats;
    RVec<int> this_ints;
    RVec<char> this_bools;
    TFile *outfile;
    void SetBranch(const TString &treename, const TString &branchname, void *address, const TString &leaflist);
};

#endif
