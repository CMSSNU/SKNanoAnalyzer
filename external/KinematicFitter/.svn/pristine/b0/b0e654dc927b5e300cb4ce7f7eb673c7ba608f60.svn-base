#ifndef TFitParticleEtPhi_hh
#define TFitParticleEtPhi_hh


#include "TAbsFitParticle.h"
#include "TLorentzVector.h"
#include "TMatrixD.h"


class TFitParticleEtPhi: public TAbsFitParticle {

public :
  TFitParticleEtPhi();
  TFitParticleEtPhi( const TFitParticleEtPhi& fitParticle );
  TFitParticleEtPhi(TLorentzVector* pini, const TMatrixD* theCovMatrix);
  TFitParticleEtPhi(const TString &name, const TString &title, 
	       TLorentzVector* pini,
	       const TMatrixD* theCovMatrix);
  virtual ~TFitParticleEtPhi();
  virtual TAbsFitParticle* clone( const TString& newname = "" ) const;

  // returns derivative dP/dy with P=(p,E) and y=(et, eta, phi) 
  // the free parameters of the fit. The columns of the matrix contain 
  // (dP/d(et), dP/d(eta), dP/d(phi)).
  virtual TMatrixD* getDerivative();
  virtual TMatrixD* transform(const TLorentzVector& vec);
  virtual void setIni4Vec(const TLorentzVector* pini);
  virtual TLorentzVector* calc4Vec( const TMatrixD* params );

  Double_t getIniEta() const { return _iniEta; }

protected :

  void init(TLorentzVector* pini, const TMatrixD* theCovMatrix);


private:
  Double_t _iniEta; 
  ClassDef(TFitParticleEtPhi,1)

};


#endif
