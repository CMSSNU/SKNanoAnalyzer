#include "Event.h"

ClassImp(Event)

Event::Event() {
    j_nPV = -999;
    j_DataYear = -999;
    j_DataEra = "";
}

Event::~Event() {}

bool Event::PassTrigger(TString trig) {
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

// NOTE
// trigger lumi calcuated from brilcalc
// e.g. brilcalc lumi -b "STABLE BEAMS" \
//               -u /pb -i /afs/cern.ch/user/c/choij/private/brilcalc/json/Run3/2023.json \
//               --hltpath "HLT_IsoMu24_v*"
// No /cvmfs/cms-bril.cern.ch/cms-lumi-pog/Normtags/normtag_PHYSICS.json for Run3?
float Event::GetTriggerLumi(TString trig) {
    if (GetEra() == "2022") {
        if (trig=="Full") return 7865.511617201;
        else return -999.;
    } else if (GetEra() == "2022EE") {
        if (trig == "Full") return 26337.274444149;
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
