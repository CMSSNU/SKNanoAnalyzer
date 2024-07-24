#include "Photon.h"

ClassImp(Photon)

Photon::Photon() : Particle() {
  
  j_cutBased = 0 ;
  j_mvaID_WP80 = false;
  j_mvaID_WP90 = false;
  j_pixelSeed = false;
  j_hoe = -999.;
  j_sieie = -999.;
  j_scEta = -999.;
  j_energyRaw = -999.;
}

Photon::~Photon(){

}
void Photon::SetSCEta(float eta, float phi, float PV_x, float PV_y, float PV_z, bool isScEtaEB, bool isScEtaEE) {
    float tg_theta_over_2 = exp(eta);
    float tg_theta        = 2 * tg_theta_over_2 / (1-tg_theta_over_2*tg_theta_over_2);
    float tg_sctheta;
    float SCEta = eta;
    if ( isScEtaEB ) {
        float R             = 130;
        float angle_x0_y0   = 0;
        if      (PV_x>0) angle_x0_y0 = atan(PV_y/PV_x);
        else if (PV_x<0) angle_x0_y0 = M_PI + atan(PV_y/PV_x);
        else if (PV_y>0) angle_x0_y0 = M_PI_2;
        else             angle_x0_y0 = M_PI_2;

        float alpha      = angle_x0_y0 + (M_PI - phi);
        float sin_beta   = sqrt(PV_x*PV_x + PV_y*PV_y) / R * sin(alpha);
        float beta       = abs(asin(sin_beta));
        float gamma      = M_PI_2 - alpha - beta;
        float l          = sqrt(R*R + (PV_x*PV_x + PV_y*PV_y) - 2*R*sqrt(PV_x*PV_x + PV_y*PV_y)*cos(gamma));
        
        float z0_zSC     = l/tg_theta;
        tg_sctheta       = R / (PV_z + z0_zSC);
        float sctheta = atan(tg_sctheta);
        if (sctheta<0) sctheta += M_PI;
        float tg_sctheta_over_2 = tan(sctheta/2);
        SCEta = -log(tg_sctheta_over_2);
    } else if ( isScEtaEE ) {
        float intersection_z;
        if ( eta > 0 ) intersection_z = 310;
        else intersection_z = -310;
        float base           = intersection_z - PV_z;
        float r              = base * tg_theta;

        float crystalX       = PV_x + r * cos(phi);
        float crystalY       = PV_y + r * sin(phi);
        tg_sctheta           = sqrt( crystalX*crystalX + crystalY*crystalY ) /intersection_z;
        float sctheta = atan(tg_sctheta);
        if (sctheta<0) sctheta += M_PI;
        float tg_sctheta_over_2 = tan(sctheta/2);
        SCEta = -log(tg_sctheta_over_2);
    }
    j_scEta = SCEta;
}

void Photon::SetBIDBit(BooleanID id, bool idbit) {
    switch (id) {
        case BooleanID::MVAIDWP80:            j_mvaID_WP80 = idbit; break;
        case BooleanID::MVAIDWP90:            j_mvaID_WP90 = idbit; break;
        default: break;
    }
}

void Photon::SetCBIDBit(CutBasedID id, unsigned int idbit) {
    switch (id) {
        case CutBasedID::CUTBASED:             j_cutBased = idbit; break;
        default: break;
    }
}

void Photon::SetMVA(MVATYPE type, float score) {
    switch (type) {
        case MVATYPE::MVAID:                    j_mvaID = score; break;
        default: break;
    }
}

bool Photon::PassID(const TString ID) const {
    // always veto gap
    // cout << "Eta: " << Eta() << "\t" << scEta() << endl;
    if (etaRegion() == ETAREGION::GAP) return false;

    // POG
    if (ID == "POGVeto")          return CutBased() == WORKINGPOINT::VETO;
    if (ID == "POGLoose")         return CutBased() == WORKINGPOINT::LOOSE;
    if (ID == "POGMedium")        return CutBased() == WORKINGPOINT::MEDIUM;
    if (ID == "POGTight")         return CutBased() == WORKINGPOINT::TIGHT;
    if (ID == "POGMVAIDWP80")    return isMVAIDWP80();
    if (ID == "POGMVAIDWP90")    return isMVAIDWP90();

    cerr << "[Photon::PassID] " << ID << " is not implemented" << endl;
    exit(ENODATA);

    return false;
}


