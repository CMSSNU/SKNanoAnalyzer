#include "Event.h"

ClassImp(Event)

Event::Event() {
    j_nPV = -999;
    j_nTrueInt = -999.;
    j_nPVsGood = -999;
    j_DataYear = -999;
    j_DataEra = "";
}

Event::~Event() {}

void Event::SetTrigger(RVec<TString> HLT_TriggerName, std::map<TString, Bool_t*> TriggerMap) {
    for(auto trig : HLT_TriggerName) {
        j_HLT_PassTrigger[trig] = TriggerMap[trig];
    }
}

void Event::SetTrigger(TString HLT_TriggerName, std::map<TString, Bool_t*> TriggerMap) {
    j_HLT_PassTrigger[HLT_TriggerName] = TriggerMap[HLT_TriggerName];
}
/*bool Event::PassTrigger(TString trig) {
    RVec<TString> tmp_vec;
    tmp_vec.push_back(trig);
    return PassTrigger(tmp_vec);
}

bool Event::PassTrigger(RVec<TString> trigs) {
    for (auto trig : trigs) {
        if (find(j_HLT_TriggerName.begin(), j_HLT_TriggerName.end(), trig) != j_HLT_TriggerName.end()) {
            return true;
        }
    }
    return false;
}
*/
bool Event::PassTrigger(TString trig) {
    if (j_HLT_PassTrigger.find(trig) == j_HLT_PassTrigger.end()) {
        cerr << "[Event::PassTrigger] Trigger " << trig << " not found" << endl;
        return false;
    }
    return *j_HLT_PassTrigger[trig];
}


// NOTE
// trigger lumi calcuated from brilcalc
// e.g. brilcalc lumi -b "STABLE BEAMS" \
//               -u /pb -i /afs/cern.ch/user/c/choij/private/brilcalc/json/Run3/2023.json \
//               --hltpath "HLT_IsoMu24_v*"
// /cvmfs/cms-bril.cern.ch/cms-lumi-pog/Normtags/normtag_PHYSICS.json for 2022, normtag_BRIL.json for 2023 (2024.07.11)
float Event::GetTriggerLumi(TString trig) {
    if (GetEra() == "2022") {
        if (trig=="Full") return 8077.009684657;
        else return -999.;
    } else if (GetEra() == "2022EE") {
        if (trig == "Full") return 26671.609707229;
        else return -999.;
    } else if (GetEra() == "2023") {
        if (trig == "Full") return 30000.;
        else return -999.;
    }
    else {
        cerr << "[Event::GetTriggerLumi] No era " << GetEra() << "for Run3" << endl;
        return -999.;
    }
}

bool Event::IsPDForTrigger(TString trig, TString PD) {
    cerr << "WARNING: IsPDForTrigger() not implemented yet" << endl;
    return false;
}

void Event::SetMET(float pt, float phi) {
    j_METVector.SetPtEtaPhiM(pt, 0, phi, 0);
}
