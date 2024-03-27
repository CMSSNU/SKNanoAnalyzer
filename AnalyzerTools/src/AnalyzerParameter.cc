#include "AnalyzerParameter.h"

AnalyzerParameter::AnalyzerParameter() {
    Name = "";
    Electron_Tight_ID = "";
    Electron_Loose_ID = "";
    Electron_Veto_ID = "";
    Electron_ID_SF_Key = "";

    Muon_Tight_ID = "";
    Muon_Loose_ID = "";
    Muon_Veto_ID = "";
    Muon_RECO_SF_Key = "";
    Muon_ID_SF_Key = "";
    Muon_ISO_SF_Key = "";
    Muon_Trigger_SF_Key = "";

    Jet_ID = "";
    FatJet_ID = "";
}

AnalyzerParameter::~AnalyzerParameter() {}

void AnalyzerParameter::Clear() {
    Name = "";

    Electron_Tight_ID = "";
    Electron_Loose_ID = "";
    Electron_Veto_ID = "";
    Electron_ID_SF_Key = "";

    Muon_Tight_ID = "";
    Muon_Loose_ID = "";
    Muon_Veto_ID = "";
    Muon_RECO_SF_Key = "";
    Muon_ID_SF_Key = "";
    Muon_ISO_SF_Key = "";
    Muon_Trigger_SF_Key = "";
    Muon_Tight_RelIso = 999.;
    Muon_Loose_RelIso = 999.;
    Muon_Veto_RelIso = 999.;

    Jet_ID = "";
    FatJet_ID = "";
}
