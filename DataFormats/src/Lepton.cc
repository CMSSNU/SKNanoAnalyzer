#include "Lepton.h"

ClassImp(Lepton)

Lepton::Lepton(): Particle() {
    j_dxy = -999.;
    j_dxyErr = -999.;
    j_dz = -999.;
    j_dzErr = -999.;
    j_ip3d = -999.;
    j_sip3d = -999.;
    j_tkRelIso = -999.;
    j_pfRelIso03_all = -999.;
    j_pfRelIso04_all = -999.;
    j_miniPFRelIso_all = -999.;
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