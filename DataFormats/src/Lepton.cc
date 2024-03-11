#include "Lepton.h"

ClassImp(Lepton)

Lepton::Lepton(): Particle() {
    j_dXY = -999.;
    j_dXYerr = -999.;
    j_dZ = -999.;
    j_dZerr = -999.;
    j_IP3D = -999.;
    j_IP3Derr = -999.;
    j_RelIso = -999.;
    j_MiniRelIso = -999.;
    j_ptcone = -999.;
    j_LeptonFlavour = NONE;
}

Lepton::~Lepton() {}

float Lepton::ConeSizeMiniIso() const {
    float minDeltaR = 0.05;
    float maxDeltaR = 0.2;
    float kt_scale = 10.0;
    return max(minDeltaR, min(maxDeltaR, float(kt_scale/this->Pt())));
}
float Lepton::CalcPtCone(float thisIso, float tightIso) {
    return (this->Pt())*(1.+max(float(0.), (thisIso-tightIso)));
}

void Lepton::Print(){
  if (IsElectron()) cout << "ELECTRON\t";
  else if (IsMuon()) cout << "MUON\t";
  else if (IsTau()) cout << "TAU\t";
  else cout << "NONE\t";
  cout << "(Pt, Eta, Phi, M, Charge) = " << Pt() << "\t" << Eta() << "\t" << Phi() << "\t" << M() << "\t" << Charge() << endl;
}