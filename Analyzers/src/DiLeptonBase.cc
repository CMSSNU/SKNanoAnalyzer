#include "DiLeptonBase.h"

DiLeptonBase::DiLeptonBase() {}
DiLeptonBase::~DiLeptonBase() {}

void DiLeptonBase::initializeAnalyzer() {
    // Flags
    RunDiMu = HasFlag("RunDiMu");
    RunEMu = HasFlag("RunEMu");
    RunNoVetoMap = HasFlag("RunNoVetoMap");
    MeasFakeMu8 = HasFlag("MeasFakeMu8");
    MeasFakeMu17 = HasFlag("MeasFakeMu17");
    MeasFakeEl8 = HasFlag("MeasFakeEl8");
    MeasFakeEl12 = HasFlag("MeasFakeEl12");
    MeasFakeEl23 = HasFlag("MeasFakeEl23");
    RunSyst = HasFlag("RunSyst");

    // Lepton IDs and triggers
    MuonIDs = new IDContainer("HcToWATight", "HcToWALoose");
    ElectronIDs = new IDContainer("HcToWATight", "HcToWALoose");
    if (DataEra == "2016preVFP") {
        DblMuTriggers = {
            "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL",
            "HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL "
        };
        EMuTriggers = {
            "HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL",
            //"HLT_Mu23_TrkIsoVVL_Ele8_CaloIdL_TrackIdL_IsoVL"
            "HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL"
        };
        isoMuTriggerName = "HLT_IsoMu24";
        triggerSafePtCut = 27.;
    } else if (DataEra == "2016postVFP") {
        DblMuTriggers = {
            "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ",
            "HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ",
            "HLT_TkMu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ"
        };
        EMuTriggers = {
            "HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ",
            "HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ"
        };
        isoMuTriggerName = "HLT_IsoMu24";
        triggerSafePtCut = 27.;
    } else if (DataEra == "2017") {
        DblMuTriggers = {
            "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ",
            "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8",
        };
        EMuTriggers = {
            "HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ",
            "HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ"
        };
        isoMuTriggerName = "HLT_IsoMu27";
        triggerSafePtCut = 30.;
    } else if (DataEra == "2018") {
        DblMuTriggers = {
            "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8",
        };
        EMuTriggers = {
            "HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ",
            "HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ"
        };
        isoMuTriggerName = "HLT_IsoMu24";
        triggerSafePtCut = 27.;
    } else {
       DblMuTriggers = {
           "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8",
       };
       EMuTriggers = {
           "HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ",
           "HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ"
       };
       isoMuTriggerName = "HLT_IsoMu24";
       triggerSafePtCut = 27.;
    }
    
    // Correction
    myCorr = new MyCorrection(DataEra, DataPeriod, IsDATA?DataStream:MCSample ,IsDATA);
    //const string SKNANO_HOME = getenv("SKNANO_HOME");
    //if (IsDATA) {
    //    systHelper = make_unique<SystematicHelper>(SKNANO_HOME + "/docs/noSyst.yaml", DataStream);
    //} else {
    //    systHelper = make_unique<SystematicHelper>(SKNANO_HOME + "/docs/DiLeptonSystematic.yaml", MCSample);
    //}
}

void DiLeptonBase::executeEvent() {
    return;
}

