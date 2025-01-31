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

void Event::SetMET(RVec<float> MET_pt, RVec<float> MET_phi) {
    j_METVector_PUPPI.SetPtEtaPhiM(MET_pt[0], 0, MET_phi[0], 0);
    j_METVector_PUPPI_UE_UP.SetPtEtaPhiM(MET_pt[1], 0, MET_phi[1], 0);
    j_METVector_PUPPI_UE_Down.SetPtEtaPhiM(MET_pt[2], 0, MET_phi[2], 0);
    j_METVector_PUPPI_JER_UP.SetPtEtaPhiM(MET_pt[3], 0, MET_phi[3], 0);
    j_METVector_PUPPI_JER_Down.SetPtEtaPhiM(MET_pt[4], 0, MET_phi[4], 0);
    j_METVector_PUPPI_JES_UP.SetPtEtaPhiM(MET_pt[5], 0, MET_phi[5], 0);
    j_METVector_PUPPI_JES_Down.SetPtEtaPhiM(MET_pt[6], 0, MET_phi[6], 0);
}

Particle Event::GetMETVector(Event::MET_Type MET_type, MyCorrection::variation syst, Event::MET_Syst source) const
{
    if(MET_type!=MET_Type::PUPPI) {
        cerr << "[Event::GetMETVector] Only PUPPI MET is implemented" << endl;
        exit(EXIT_FAILURE);
    }
    // switch (syst) {
    //     case MET_Syst::CENTRAL:
    //         return j_METVector_PUPPI;
    //     case MET_Syst::UE_UP:
    //         return j_METVector_PUPPI_UE_UP;
    //     case MET_Syst::UE_DOWN:
    //         return j_METVector_PUPPI_UE_Down;
    //     case MET_Syst::JER_UP:
    //         return j_METVector_PUPPI_JER_UP;
    //     case MET_Syst::JER_DOWN:
    //         return j_METVector_PUPPI_JER_Down;
    //     case MET_Syst::JES_UP:
    //         return j_METVector_PUPPI_JES_UP;
    //     case MET_Syst::JES_DOWN:
    //         return j_METVector_PUPPI_JES_Down;
    // }
    switch (syst) {
        case MyCorrection::variation::nom:
            switch (source) {
                case MET_Syst::CENTRAL:
                    return j_METVector_PUPPI;
                default:
                    cerr << "[Event::GetMETVector] Source is not MET_Syst::CENTRAL but variation is nominal" << endl;
                    exit(EXIT_FAILURE);
            }
        case MyCorrection::variation::up:
            switch (source) {
                case MET_Syst::UE:
                    return j_METVector_PUPPI_UE_UP;
                case MET_Syst::JER:
                    return j_METVector_PUPPI_JER_UP;
                case MET_Syst::JES:
                    return j_METVector_PUPPI_JES_UP;
                default:
                    cerr << "[Event::GetMETVector] Source is not MET_Syst::UE, JER, or JES but variation is up" << endl;
                    exit(EXIT_FAILURE);
            }
        case MyCorrection::variation::down:
            switch (source) {
                case MET_Syst::UE:
                    return j_METVector_PUPPI_UE_Down;
                case MET_Syst::JER:
                    return j_METVector_PUPPI_JER_Down;
                case MET_Syst::JES:
                    return j_METVector_PUPPI_JES_Down;
                default:
                    cerr << "[Event::GetMETVector] Source is not MET_Syst::UE, JER, or JES but variation is down" << endl;
                    exit(EXIT_FAILURE);
            }
        default:
            cerr << "[Event::GetMETVector] Unknown variation" << endl;
            exit(EXIT_FAILURE);
    }
}
