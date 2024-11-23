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

const void Event::SetTrigger(const std::map<TString, pair<Bool_t*,float>>& TriggerMap) {
    j_HLT_TriggerMapPtr = &(TriggerMap);
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
const bool Event::PassTrigger(TString trig) {

    if (j_HLT_TriggerMapPtr->find(trig) == j_HLT_TriggerMapPtr->end()) {
        cerr << "[Event::PassTrigger] Trigger " << trig << " not found" << endl;
        exit(ENODATA);
    }
    return *((j_HLT_TriggerMapPtr->at(trig)).first);
}


// NOTE
// trigger lumi calcuated from brilcalc
// e.g. brilcalc lumi -b "STABLE BEAMS" \
//               -u /pb -i /afs/cern.ch/user/c/choij/private/brilcalc/json/Run3/2023.json \
//               --hltpath "HLT_IsoMu24_v*"
// /cvmfs/cms-bril.cern.ch/cms-lumi-pog/Normtags/normtag_PHYSICS.json for 2022, normtag_BRIL.json for 2023 (2024.07.11)
float Event::GetTriggerLumi(TString trig) {
    if(j_HLT_TriggerMapPtr->find(trig) == j_HLT_TriggerMapPtr->end()) {
        cerr << "[Event::GetTriggerLumi] Trigger " << trig << " not found" << endl;
        return -999.;
    }
    return (j_HLT_TriggerMapPtr->at(trig)).second;
    // I put full trigger luminosity in HLT_Path.json
    // If you are using prescaled one, you have to modify 
}

bool Event::IsPDForTrigger(TString trig, TString PD) {
    cerr << "WARNING: IsPDForTrigger() not implemented yet" << endl;
    return false;
}

void Event::SetMET(float pt, float phi) {
    j_METVector.SetPtEtaPhiM(pt, 0, phi, 0);
}
