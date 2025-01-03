#ifndef TFitParticlePz_hh
#define TFitParticlePz_hh


#include "TAbsFitParticle.h"
#include "TLorentzVector.h"
#include "TMatrixD.h"


class TFitParticlePz: public TAbsFitParticle {

public :
  TFitParticlePz();
  TFitParticlePz( const TFitParticlePz& fitParticle );
  TFitParticlePz(TLorentzVector* pini, const TMatrixD* theCovMatrix);
  TFitParticlePz(const TString &name, const TString &title, 
	       TLorentzVector* pini,
	       const TMatrixD* theCovMatrix);
  virtual ~TFitParticlePz();
  virtual TAbsFitParticle* clone( const TString& newname = "" ) const;

  // returns derivative dP/dy with P=(p,E) and y=(et, eta, phi) 
  // the free parameters of the fit. The columns of the matrix contain 
  // (dP/d(et), dP/d(eta), dP/d(phi)).
  virtual TMatrixD* getDerivative();
  virtual TMatrixD* transform(const TLorentzVector& vec);
  virtual void setIni4Vec(const TLorentzVector* pini);
  virtual TLorentzVector* calc4Vec( const TMatrixD* params );
  Double_t getIniPx() const { return _iniPx; }
  Double_t getIniPy() const { return _iniPy; }

protected :

  void init(TLorentzVector* pini, const TMatrixD* theCovMatrix);


private:
  Double_t _iniPx;
  Double_t _iniPy;
  ClassDef(TFitParticlePz,1)

};


#endif
