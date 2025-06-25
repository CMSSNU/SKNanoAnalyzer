#ifndef ParseMuIDVariables_h
#define ParseMuIDVariables_h

#include "AnalyzerCore.h"

class ParseMuIDVariables: public AnalyzerCore {
private:
    TTree *Events;
    // Event variables
    float genWeight;
    unsigned int nMuons;
    float pt[20];
    float eta[20];
    int lepType[20];
    int nearestJetFlavour[20];
    // ID variables
    float isPOGMediumId[20];
    float dZ[20];
    float sip3d[20];
    float tkRelIso[20];
    float miniPFRelIso[20];

    // Trigger matching variables
    bool isTrigMatched[20];
    
    // For event selection
    RVec<TString> EMuTriggers;

public:
    ParseMuIDVariables();
    ~ParseMuIDVariables();

    void initializeAnalyzer();
    void executeEvent();
    void WriteHist();
};

#endif