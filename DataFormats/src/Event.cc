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

float Event::GetTriggerLumi(TString trig) {
    cerr << "WARNING: GetTriggerLumi() not implemented yet" << endl;
    return -999.;
}

bool Event::IsPDForTrigger(TString trig, TString PD) {
    cerr << "WARNING: IsPDForTrigger() not implemented yet" << endl;
    return false;
}

void Event::SetMET(float pt, float phi) {
    j_METVector.SetPtEtaPhiM(pt, 0, phi, 0);
}