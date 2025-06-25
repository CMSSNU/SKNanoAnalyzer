#ifndef TestGetLeptonType_h
#define TestGetLeptonType_h

#include "AnalyzerCore.h"

class TestGetLeptonType : public AnalyzerCore {
public:
    void initializeAnalyzer();
    void executeEvent();

    RVec<Muon> AllMuons;
    RVec<Electron> AllElectrons;
    RVec<Gen> AllGens;

    TestGetLeptonType();
    ~TestGetLeptonType();
};

#endif