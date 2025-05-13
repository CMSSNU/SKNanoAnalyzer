#include "MeasTrigEff.h"

MeasTrigEff::MeasTrigEff() {}
MeasTrigEff::~MeasTrigEff() {}


void MeasTrigEff::initializeAnalyzer() {
    // Userflags
    MeasMuLegs = HasFlag("MeasMuLegs");
    MeasElLegs = HasFlag("MeasElLegs");
    MeasDblMuDZ = HasFlag("MeasDblMuDZ");
    MeasEMuDZ = HasFlag("MeasEMuDZ");

    // ID settings
    if (DataEra == "2016preVFP") {
        MuID = IDContainer("HcToWATight", "HcToWALoose");
        ElID = IDContainer("HcToWATight_16a", "HcToWALoose_16a");

        // Triggers and filters
        Triggers.SetSglMuTrigs({"HLT_IsoMu24_v", "HLT_IsoTkMu24_v"});
        Triggers.SetSglElTrigs({"HLT_Ele27_WPTight_Gsf_v"});
        Triggers.SetDblMuTrigs({"HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_v", "HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_v"});
        Triggers.SetDblMuDZTrigs({"HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_v", "HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ_v"});
        Triggers.SetDblMuDZMTrigs({});
        Triggers.SetEMuTrigs({"HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_v", "HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_v"});
        Triggers.SetEMuDZTrigs({});
        Triggers.SetMu23El12_MuLegFilter("hltMu23TrkIsoVVLEle8CaloIdLTrackIdLIsoVLMuonlegL3IsoFiltered23");
        Triggers.SetMu8El23_MuLegFilter("hltMu8TrkIsoVVLEle23CaloIdLTrackIdLIsoVLMuonlegL3IsoFiltered8");
        Triggers.SetMu23El12_ElLegFilter("hltMu23TrkIsoVVLEle8CaloIdLTrackIdLIsoVLElectronlegTrackIsoFilter");
        Triggers.SetMu8El23_ElLegFilter("hltMu8TrkIsoVVLEle23CaloIdLTrackIdLIsoVLElectronlegTrackIsoFilter");
    
    } else if (DataEra == "2016postVFP") {
        MuID = IDContainer("HcToWATight", "HcToWALoose");
        ElID = IDContainer("HcToWATight_16b", "HcToWALoose_16b");
    } else if (DataEra == "2017") {
        MuID = IDContainer("HcToWATight", "HcToWALoose");
        ElID = IDContainer("HcToWATight_17", "HcToWALoose_17");
    } else if (DataEra == "2018") {
        MuID = IDContainer("HcToWATight", "HcToWALoose");
        ElID = IDContainer("HcToWATight_18", "HcToWALoose_18");
    } else {
        cerr << "[MeasTrigEff::initializeAnalyzer] Wrong era " << DataEra << endl;
        exit(EXIT_FAILURE);
    }

    // Triggers and filters
}