#include "Electron.h"

ClassImp(Electron)

Electron::Electron() {
    this->SetLeptonFlavour(ELECTRON);
    // uncertainties
    j_energyErr = -999.;
    // ID variables
    j_convVeto = false;
    j_lostHits = 0;
    j_seedGain = 0;
    j_tightCharge = 0;
    j_sieie = -999.;
    j_hoe = -999.;
    j_eInvMinusPInv = -999.;
    j_dr03EcalRecHitSumEt = -999.;
    j_dr03HcalDepth1TowerSumEt = -999.;
    j_dr03TkSumPt = -999.;
    j_dr03TkSumPtHEEP = -999.;
    // supercluster
    j_deltaEtaSC = -999.;
    // ID bits
    j_mvaIso_WP80 = false;
    j_mvaIso_WP90 = false;
    j_mvaNoIso_WP80 = false;
    j_mvaNoIso_WP90 = false;
    j_cutBased_HEEP = false;
    j_cutBased = 0;
    j_mvaIso = -999.;
    j_mvaNoIso = -999.;
    j_mvaTTH = -999.;
    // others
    j_r9 = -999.;
}

Electron::~Electron() {}

void Electron::SetBIDBit(BooleanID id, bool idbit) {
    switch (id) {
        case BooleanID::MVAISOWP80:            j_mvaIso_WP80 = idbit; break;
        case BooleanID::MVAISOWP90:            j_mvaIso_WP90 = idbit; break;
        case BooleanID::MVANOISOWP80:          j_mvaNoIso_WP80 = idbit; break;
        case BooleanID::MVANOISOWP90:          j_mvaNoIso_WP90 = idbit; break;
        case BooleanID::CUTBASEDHEEP:          j_cutBased_HEEP = idbit; break;
        default: break;
    }
}

void Electron::SetCBIDBit(CutBasedID id, bool idbit) {
    switch (id) {
        case CutBasedID::CUTBASED:             j_cutBased = idbit; break;
        default: break;
    }
}

void Electron::SetMVA(MVATYPE type, float score) {
    switch (type) {
        case MVATYPE::MVAISO:                    j_mvaIso = score; break;
        case MVATYPE::MVANOISO:                  j_mvaNoIso = score; break;
        case MVATYPE::MVATTH:                    j_mvaTTH = score; break;
        default: break;
    }
}
