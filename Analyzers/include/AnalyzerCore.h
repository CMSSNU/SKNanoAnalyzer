#ifndef AnalyzerCore_h
#define AnalyzerCore_h

#include <iostream>
#include <vector>
#include <map>
using namespace std;

#include "TFile.h"
#include "TH1F.h"

#include "SKNanoLoader.h"
#include "Event.h"
#include "Particle.h"
#include "Lepton.h"
#include "Muon.h"

class AnalyzerCore: public SKNanoLoader {
public:
    AnalyzerCore();
    ~AnalyzerCore();

    //virtual void initializeAnalyzer() {};
    //virtual void executeEvent() {};

    inline static bool PtComparing(const Particle& p1, const Particle& p2) { return p1.Pt() > p2.Pt();}
    inline static bool PtComparing(const Particle* p1, const Particle* p2) { return p1->Pt() > p2->Pt();}

    // Get muons
    //Event GetEvent();
    RVec<Muon> GetAllMuons();

    // Functions
    void SetOutfilePath(TString outpath);
    TH1F* GetHist1D(TString histname);
    void FillHist(TString histname, float value, float weight, int n_bin, float x_min, float x_max);
    virtual void WriteHist();

private:
    map<TString, TH1F*> histmap;
    TFile *outfile;
};

#endif
