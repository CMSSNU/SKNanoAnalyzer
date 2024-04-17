#ifndef AnalyzerCore_h
#define AnalyzerCore_h

#include <map>
#include <string>

#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TString.h"

#include "SKNanoLoader.h"
#include "Event.h"
#include "Particle.h"
#include "Lepton.h"
#include "Muon.h"
#include "Electron.h"
#include "Jet.h"
#include "GenJet.h"
#include "Tau.h"
#include "FatJet.h"

#include "LHAPDFHandler.h"
#include "PDFReweight.h"
#include "MCCorrection.h"

class AnalyzerCore: public SKNanoLoader {
public:
    AnalyzerCore();
    ~AnalyzerCore();

    virtual void initializeAnalyzer() {};
    virtual void executeEvent() {};

    inline static bool PtComparing(const Particle& p1, const Particle& p2) { return p1.Pt() > p2.Pt();}
    inline static bool PtComparingPtr(const Particle* p1, const Particle* p2) { return p1->Pt() > p2->Pt();}

    // PDF reweight
    PDFReweight *pdfReweight;
    float GetPDFWeight(LHAPDF::PDF *pdf_);
    float GetPDFReweight();
    float GetPDFReweight(int member);
    
    // MCCorrection
    MCCorrection *mcCorr;
    //unique_ptr<CorrectionSet> csetMuon;
    //unique_ptr<CorrectionSet> csetElectron;

    // MC weights
    float MCweight(bool usesign=true, bool norm_1invpb=true) const;
    float GetPileUpWeight(int sys);

    // Get objects
    Event GetEvent();
    RVec<Muon> GetAllMuons();
    RVec<Muon> GetMuons(const TString ID, const float ptmin, const float fetamax);
    RVec<Electron> GetAllElectrons();
    RVec<Jet> GetAllJets();
    RVec<Jet> GetJets(const TString id, const float ptmin, const float fetamax);
    RVec<Electron> GetElectrons(const TString id, const float ptmin, const float fetamax);
    RVec<Tau> GetAllTaus();
    RVec<FatJet> GetAllFatJets();
    RVec<GenJet> GetAllGenJets();

    // Select objects
    RVec<Muon> SelectMuons(const RVec<Muon> &muons, TString ID, const float ptmin, const float absetamax);
    RVec<Electron> SelectElectrons(const RVec<Electron> &electrons, const TString id, const float ptmin, const float absetamax);
    RVec<Tau> SelectTaus(const RVec<Tau> &taus, const TString ID, const float ptmin, const float absetamax);

    // Functions
    void SetOutfilePath(TString outpath);
    TH1F* GetHist1D(const string &histname);
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

    virtual void WriteHist();

private:
    unordered_map<string, TH1F*> histmap1d;
    unordered_map<string, TH2F*> histmap2d;
    unordered_map<string, TH3F*> histmap3d;
    TFile *outfile;
};

#endif
