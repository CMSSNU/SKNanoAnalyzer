#ifndef Photon_h
#define Photon_h

#include "Particle.h"

class Photon: public Particle {
public:

  Photon();
  ~Photon();
  
  
  void SetSC(double sceta, double scphi);
  inline double scEta() const { return j_scEta; }
//  inline double scPhi() const { return j_scPhi; }
    
//  void SetCutBasedIDVariables(double Full5x5_sigmaIetaIeta, double HoverE, double ChIsoWithEA, double NhIsoWithEA, double PhIsoWithEA);
  inline double Full5x5_sigmaIetaIeta() const { return j_Full5x5_sigmaIetaIeta; }
  inline double HoverE() const { return j_HoverE; }
//  inline double ChIsoWithEA() const { return j_ChIsoWithEA; }
//  inline double NhIsoWithEA() const { return j_NhIsoWithEA; }
//  inline double PhIsoWithEA() const { return j_PhIsoWithEA; }
  

  void SetPOGIDs(std::vector<bool> bs);
  inline bool passLooseID() const { return j_passLooseID; }
  inline bool passMediumID() const { return j_passMediumID; }
  inline bool passTightID() const { return j_passTightID; }
  inline bool passMVAID_WP80() const { return j_passMVAID_WP80; }
  inline bool passMVAID_WP90() const { return j_passMVAID_WP90; }
  
  
  // === ID
  bool PassID(TString ID);
  bool Pass_CutBasedLoose();
  bool Pass_CutBasedMedium();
  bool Pass_CutBasedTight();
  
  bool Pass_Prefire_ID();

  void SetRho(double r);
  inline double Rho() const { return j_Rho; }
  

private:
  double j_scEta;
//  double j_scPhi;
  double j_scE;
  double j_Full5x5_sigmaIetaIeta;
  double j_HoverE;
//  double j_ChIsoWithEA;
//  double j_NhIsoWithEA;
//  double j_PhIsoWithEA;
  bool j_passLooseID;
  bool j_passMediumID;
  bool j_passTightID;
  bool j_passMVAID_WP80;
  bool j_passMVAID_WP90;

  double j_Rho;

  ClassDef(Photon,1)

};

#endif
