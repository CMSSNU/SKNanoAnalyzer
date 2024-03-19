#ifndef AnalyzerCore_h
#define AnalyzerCore_h

#include <iostream>
#include <map>
#include <string>
using namespace std;

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

class AnalyzerCore: public SKNanoLoader {
public:
    AnalyzerCore();
    ~AnalyzerCore();

    //virtual void initializeAnalyzer() {};
    //virtual void executeEvent() {};

    inline static bool PtComparing(const Particle& p1, const Particle& p2) { return p1.Pt() > p2.Pt();}
    inline static bool PtComparing(const Particle* p1, const Particle* p2) { return p1->Pt() > p2->Pt();}

    // Get objects
    //Event GetEvent();
    RVec<Muon> GetAllMuons();
    RVec<Electron> GetAllElectrons();

    // Functions
    void SetOutfilePath(TString outpath);
    TH1F* GetHist1D(const string &histname);
    void FillHist(const string &histname, float value, float weight, int n_bin, float x_min, float x_max);
    void FillHist(const string &histname, float value, float weight, int n_bin, float *xbins);
    void FillHist(const string &histname, float value_x, float value_y, float weight, 
                                          int n_binx, float x_min, float x_max, 
                                          int n_biny, float y_min, float y_max);
    void FillHist(const string &histname, float value_x, float value_y, float weight,
                                          int n_binx, float *xbins,
                                          int n_biny, float *ybins);
    void FillHist(const string &histname, float value_x, float value_y, float value_z, float weight,
                                          int n_binx, float x_min, float x_max,
                                          int n_biny, float y_min, float y_max,
                                          int n_binz, float z_min, float z_max);
    void FillHist(const string &histname, float value_x, float value_y, float value_z, float weight,
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
