#include "AnalyzerParameter.h"

AnalyzerParameter::AnalyzerParameter() {
    Name = "";
    MCCorrectionIgnoreNoHist = false;
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

    syst = SYST::Central;
}

AnalyzerParameter::~AnalyzerParameter() {}

void AnalyzerParameter::Clear() {
    Name = "";
    MCCorrectionIgnoreNoHist = false;

    Electron_Tight_ID = "";
    Electron_Loose_ID = "";
    Electron_Veto_ID = "";
    Electron_ID_SF_Key = "";
    Electron_FR_ID = "";
    Electron_FR_Key = "";
    Electron_CF_ID = "";
    Electron_CF_Key = "";
    Electron_Tight_RelIso = 999.;
    Electron_Loose_RelIso = 999.;
    Electron_Veto_RelIso = 999.;
    Electron_UseMini = false;
    Electron_UsePtCone = false;
    Electron_MinPt = 10.;

    Muon_Tight_ID = "";
    Muon_Loose_ID = "";
    Muon_Veto_ID = "";
    Muon_RECO_SF_Key = "";
    Muon_ID_SF_Key = "";
    Muon_ISO_SF_Key = "";
    Muon_Trigger_SF_Key = "";
    Muon_FR_ID = "";
    Muon_FR_Key = "";
    Muon_CF_ID = "";
    Muon_CF_Key = "";
    Muon_Tight_RelIso = 999.;
    Muon_Loose_RelIso = 999.;
    Muon_Veto_RelIso = 999.;
    Muon_UseMini = false;
    Muon_UsePtCone = false;
    Muon_UseTuneP = false;
    Muon_MinPt = 10.;

    Jet_ID = "";
    FatJet_ID = "";

    syst = SYST::Central;
}

TString AnalyzerParameter::GetSystType() {
    switch (syst) {
        case SYST::Central:
            return "Central"; break;
        case SYST::JetResUp:
            return "JetResUp"; break;
        case SYST::JetResDown:
            return "JetResDown"; break;
        case SYST::JetEnUp:
            return "JetEnUp"; break;
        case SYST::JetEnDown:
            return "JetEnDown"; break;
        case SYST::MuonEnUp:
            return "MuonEnUp"; break;
        case SYST::MuonEnDown:
            return "MuonEnDown"; break;
        case SYST::ElectronResUp:
            return "ElectronResUp"; break;
        case SYST::ElectronResDown:
            return "ElectronResDown"; break;
        case SYST::ElectronEnUp:
            return "ElectronEnUp"; break;
        case SYST::ElectronEnDown:
            return "ElectronEnDown"; break;
        default:
            cerr << "[AnalyzerParameter::GetSystType] ERROR: Unknown systematic type!" << endl;
            exit(ENODATA);
            return "error";
    }
}

