#include "Photon.h"

ClassImp(Photon)

Photon::Photon() : Particle() {
  j_scEta = -999.;
  j_scPhi = -999.;
  j_scE = -999.;
  j_passLooseID = false;
  j_passMediumID = false;
  j_passTightID = false;
  j_passMVAID_WP80 = false;
  j_passMVAID_WP90 = false;
  j_Rho = -999.;
}

Photon::~Photon(){

}

void Photon::SetSC(double sceta, double scphi){
  j_scEta = sceta;
  j_scPhi = scphi;
}

void Photon::SetCutBasedIDVariables(double Full5x5_sigmaIetaIeta, double HoverE, double ChIsoWithEA, double NhIsoWithEA, double PhIsoWithEA){
  j_Full5x5_sigmaIetaIeta = Full5x5_sigmaIetaIeta;
  j_HoverE = HoverE;
  j_ChIsoWithEA = ChIsoWithEA;
  j_NhIsoWithEA = NhIsoWithEA;
  j_PhIsoWithEA = PhIsoWithEA;
}

void Photon::SetPOGIDs(std::vector<bool> bs){
  j_passLooseID = bs.at(0);
  j_passMediumID = bs.at(1);
  j_passTightID = bs.at(2);
  j_passMVAID_WP80 = bs.at(3);
  j_passMVAID_WP90 = bs.at(4);
  
}

bool Photon::PassID(TString ID){
  
  if(ID=="passLooseID") return passLooseID();
  if(ID=="passMediumID") return passMediumID();
  if(ID=="passTightID") return passTightID();
  if(ID=="passMVAID_WP80") return passMVAID_WP80();
  if(ID=="passMVAID_WP90") return passMVAID_WP90();

  cout << "[Photon::PassID] No id : " << ID << endl;
  exit(ENODATA);

  return false;

}

void Photon::SetRho(double r){
  j_Rho = r;
}
