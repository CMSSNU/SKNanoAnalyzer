#ifndef AnalyzerParameter_h
#define AnalyzerParameter_h

// NOTE: This class is used to store the parameters of the analysis
// e.g. ID, SF keys...
// From Run3, users are recommended to inherit from this class and set the parameters for each analysis
// See ExampleParameter.h for an example

#include <iostream>
#include "TString.h"

using namespace std;

class AnalyzerParameter {
public:
    AnalyzerParameter();
    ~AnalyzerParameter();

    TString Name;

    // Electrons
    TString Electron_Tight_ID, Electron_Loose_ID, Electron_Veto_ID;
    TString Electron_ID_SF_Key, Electron_Trigger_SF_Key;

    // Muons
    TString Muon_Tight_ID, Muon_Loose_ID, Muon_Veto_ID;
    TString Muon_RECO_SF_Key, Muon_ID_SF_Key, Muon_ISO_SF_Key, Muon_Trigger_SF_Key;
    float Muon_Tight_RelIso, Muon_Loose_RelIso, Muon_Veto_RelIso;

    // Jets
    TString Jet_ID, FatJet_ID;
    virtual void Clear();
};

#endif
