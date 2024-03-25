#ifndef ExampleRun_h
#define ExampleRun_h

#include "AnalyzerCore.h"

class ExampleRun : public AnalyzerCore {
public:
    void initializeAnalyzer();
    void executeEvent();
    void executeEventFromParameter(AnalyzerParameter param);

    bool RunSyst;
    bool RunNewPDF;
    bool RunXSecSyst;

    TString IsoMuTriggerName;
    float TriggerSafePtCut;

    RVec<TString> MuonIDs, MuonIDSFKeys;
    RVec<Muon> AllMuons;
    //RVec<Jet> AllJets;

    float weight_Prefire;

    ExampleRun();
    ~ExampleRun();
};

#endif

