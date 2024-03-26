#ifndef AnalyzerParameter_h
#define AnalyzerParameter_h

#include <iostream>
#include "TString.h"

using namespace std;

class AnalyzerParameter {
public:
    AnalyzerParameter();
    ~AnalyzerParameter();

    TString Name;
    bool MCCorrectionIgnoreNoHist;

    // Electrons
    TString Electron_Tight_ID, Electron_Loose_ID, Electron_Veto_ID;
    TString Electron_ID_SF_Key, Electron_Trigger_SF_Key;
    TString Electron_FR_ID, Electron_FR_Key;
    TString Electron_CF_ID, Electron_CF_Key;
    float Electron_Tight_RelIso, Electron_Loose_RelIso, Electron_Veto_RelIso;
    float Electron_MinPt;
    bool Electron_UseMini, Electron_UsePtCone;

    // Muons
    TString Muon_Tight_ID, Muon_Loose_ID, Muon_Veto_ID;
    TString Muon_RECO_SF_Key, Muon_ID_SF_Key, Muon_ISO_SF_Key, Muon_Trigger_SF_Key;
    TString Muon_FR_ID, Muon_FR_Key;
    TString Muon_CF_ID, Muon_CF_Key;
    float Muon_Tight_RelIso, Muon_Loose_RelIso, Muon_Veto_RelIso;
    float Muon_MinPt;
    bool Muon_UseMini, Muon_UsePtCone, Muon_UseTuneP;

    // Jets
    TString Jet_ID, FatJet_ID;

    enum class SYST {
        Central,
        JetResUp, JetResDown,
        JetEnUp, JetEnDown,
        MuonEnUp, MuonEnDown,
        ElectronResUp, ElectronResDown,
        ElectronEnUp, ElectronEnDown,
        NSyst
    };
    SYST syst;
    TString GetSystType();

    void Clear();

};

#endif
