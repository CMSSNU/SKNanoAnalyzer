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
    j_deltaEtaInSC = -999.;
    j_deltaEtaInSeed = -999.;
    j_deltaPhiInSC = -999.;
    j_deltaPhiInSeed = -999.;
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
    j_rho = -999.;
    j_genPartFlav = 0;
    j_genPartIdx = -1;
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

void Electron::SetCBIDBit(CutBasedID id, unsigned int idbit) {
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

bool Electron::PassID(const TString ID) const {
    // always veto gap
    // cout << "Eta: " << Eta() << "\t" << scEta() << endl;
    if (etaRegion() == ETAREGION::GAP) return false;

    // POG
    if (ID == "")                 return true;
    if (ID == "POGVeto")          return (int)(CutBased()) >= (int)(WORKINGPOINT::VETO);
    if (ID == "POGLoose")         return (int)(CutBased()) >= (int)(WORKINGPOINT::LOOSE);
    if (ID == "POGMedium")        return (int)(CutBased()) >= (int)(WORKINGPOINT::MEDIUM);
    if (ID == "POGTight")         return (int)(CutBased()) >= (int)(WORKINGPOINT::TIGHT);
    if (ID == "POGHEEP")          return isCutBasedHEEP();
    if (ID == "POGMVAIsoWP80")    return isMVAIsoWP80();
    if (ID == "POGMVAIsoWP90")    return isMVAIsoWP90();
    if (ID == "POGMVANoIsoWP80")  return isMVANoIsoWP80();
    if (ID == "POGMVANoIsoWP90")  return isMVANoIsoWP90();
    if (ID == "HcToWATight_16a")   return Pass_HcToWATight16a();
    if (ID == "HcToWATight_16b")   return Pass_HcToWATight16b();
    if (ID == "HcToWATight_17")    return Pass_HcToWATight17();
    if (ID == "HcToWATight_18")    return Pass_HcToWATight18();
    if (ID == "HcToWALoose_16a")   return Pass_HcToWALoose16a();
    if (ID == "HcToWALoose_16b")   return Pass_HcToWALoose16b();
    if (ID == "HcToWALoose_17")    return Pass_HcToWALoose17();
    if (ID == "HcToWALoose_18")    return Pass_HcToWALoose18();

    cerr << "[Electron::PassID] " << ID << " is not implemented" << endl;
    exit(ENODATA);

    return false;
}

bool Electron::PassID(ElectronID ID) const {
    switch (ID) {
        case ElectronID::POG_VETO:         return PassID("POGVeto");
        case ElectronID::POG_LOOSE:        return PassID("POGLoose");
        case ElectronID::POG_MEDIUM:       return PassID("POGMedium");
        case ElectronID::POG_TIGHT:        return PassID("POGTight");
        case ElectronID::POG_HEEP:         return PassID("POGHEEP");
        case ElectronID::POG_MVAISO_WP80:      return PassID("POGMVAIsoWP80");
        case ElectronID::POG_MVAISO_WP90:      return PassID("POGMVAIsoWP90");
        case ElectronID::POG_MVANOISO_WP80:    return PassID("POGMVANoIsoWP80");
        case ElectronID::POG_MVANOISO_WP90:    return PassID("POGMVANoIsoWP90");
        default: break;
    }
    return false;
}

// Private IDs
bool Electron::Pass_CaloIdL_TrackIdL_IsoVL() const {
    // checked in https://cmshltinfo.app.cern.ch/path/HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v
    const bool ApplyEA = true;
    
    // Check eta region first
    if (etaRegion() == ETAREGION::GAP) {
        return false;
    } else if  (etaRegion() == ETAREGION::IB || etaRegion() == ETAREGION::OB) {
        if (! (sieie() < 0.013)) return false;
        if (! (fabs(deltaEtaInSC()) < 0.01)) return false;
        if (! (fabs(deltaPhiInSeed()) < 0.07)) return false;
        if (! (hoe() < 0.13)) return false;
        if (! (max(0., ecalPFClusterIso() - rho()*(ApplyEA? 0.16544: 0.)) < 0.5*Pt())) return false;
        if (! (max(0., hcalPFClusterIso() - rho()*(ApplyEA? 0.05956: 0.)) < 0.3*Pt())) return false;
        if (! (dr03TkSumPt() < 0.2*Pt())) return false;
        return true;
    } else {
        if (! (sieie() < 0.035)) return false;
        if (! (fabs(deltaEtaInSC()) < 0.015)) return false;
        if (! (fabs(deltaPhiInSeed()) < 0.1)) return false;
        if (! (hoe() < 0.13)) return false;
        if (! (max(0., ecalPFClusterIso() - rho()*(ApplyEA? 0.13212: 0.)) < 0.5*Pt())) return false;
        if (! (max(0., hcalPFClusterIso() - rho()*(ApplyEA? 0.13052: 0.)) < 0.3*Pt())) return false;
        if (! (dr03TkSumPt() < 0.2*Pt())) return false;
        return true;
    }
}

bool Electron::Pass_HcToWABaseline() const {
    if (! Pass_CaloIdL_TrackIdL_IsoVL()) return false;
    if (! ConvVeto()) return false;
    if (! (LostHits() < 2)) return false;
    if (! (fabs(dZ()) < 0.1)) return false;
    return true;
}

bool Electron::Pass_HcToWA(const TString &era, const Electron::WORKINGPOINT &wp) const {
    if (wp == WORKINGPOINT::TIGHT) {
        if (! isMVANoIsoWP90()) return false;
        if (! (SIP3D() < 4.)) return false;
        if (! (MiniPFRelIso() < 0.1*Pt())) return false;
    } else if (wp == WORKINGPOINT::LOOSE) {
        const float cutIB=0.985, cutOB=0.96, cutEC=0.85;
        bool passMVAIDNoIsoCut = false;
        switch(etaRegion()) {
            case ETAREGION::IB:
                if (! (MvaNoIso() > cutIB)) passMVAIDNoIsoCut = true;
                break;
            case ETAREGION::OB:
                if (! (MvaNoIso() > cutOB)) passMVAIDNoIsoCut = true;
                break;
            case ETAREGION::EC:
                if (! (MvaNoIso() > cutEC)) passMVAIDNoIsoCut = true;
                break;
            default:
                // Other eta region are already rejected
                break;
        }
        if (! (isMVANoIsoWP90() || passMVAIDNoIsoCut)) return false;
        if (! (SIP3D() < 8.)) return false;
        if (! (MiniPFRelIso() < 0.4*Pt())) return false;
    } else {
        cerr << "[Electron::Pass_HcToWA] Only tight and loose WP are implemented" << endl;
        exit(EXIT_FAILURE);
    }
    return true;
}

bool Electron::Pass_HcToWATight16a() const {
    if (! Pass_HcToWABaseline()) return false;
    if (! Pass_HcToWA("2016a", WORKINGPOINT::TIGHT)) return false;
    return true;
}

bool Electron::Pass_HcToWALoose16a() const {
    if (! Pass_HcToWABaseline()) return false;
    if (! Pass_HcToWA("2016a", WORKINGPOINT::LOOSE)) return false;
    return true;
}

bool Electron::Pass_HcToWATight16b() const {
    if (! Pass_HcToWABaseline()) return false;
    if (! Pass_HcToWA("2016b", WORKINGPOINT::TIGHT)) return false;
    return true;
}

bool Electron::Pass_HcToWALoose16b() const {
    if (! Pass_HcToWABaseline()) return false;
    if (! Pass_HcToWA("2016b", WORKINGPOINT::LOOSE)) return false;
    return true;
}

bool Electron::Pass_HcToWATight17() const {
    if (! Pass_HcToWABaseline()) return false;
    if (! Pass_HcToWA("2017", WORKINGPOINT::TIGHT)) return false;
    return true;
}


bool Electron::Pass_HcToWALoose17() const {
    if (! Pass_HcToWABaseline()) return false;
    if (! Pass_HcToWA("2017", WORKINGPOINT::LOOSE)) return false;
    return true;
}

bool Electron::Pass_HcToWATight18() const {
    if (! Pass_HcToWABaseline()) return false;
    if (! Pass_HcToWA("2018", WORKINGPOINT::TIGHT)) return false;
    return true;
}

bool Electron::Pass_HcToWALoose18() const {
    if (! Pass_HcToWABaseline()) return false;
    if (! Pass_HcToWA("2018", WORKINGPOINT::LOOSE)) return false;
    return true;
}


