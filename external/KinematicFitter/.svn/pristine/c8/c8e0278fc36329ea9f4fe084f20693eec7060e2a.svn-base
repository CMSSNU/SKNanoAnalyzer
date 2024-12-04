#ifndef TFitParticlePxPy_hh
#define TFitParticlePxPy_hh

#include "TMatrixD.h"
#include "TAbsFitParticle.h"
#include "TLorentzVector.h"
#include "TVector3.h"

class TFitParticlePxPy: public TAbsFitParticle {

public :

  TFitParticlePxPy();
  TFitParticlePxPy( const TFitParticlePxPy& fitParticle );
  TFitParticlePxPy(TVector3* p, Double_t M, const TMatrixD* theCovMatrix);
  TFitParticlePxPy(const TString &name, const TString &title, 
		 TVector3* p, Double_t M, const TMatrixD* theCovMatrix);
  TFitParticlePxPy(const TString &name, const TString &title, 
		 TLorentzVector* pini, const TMatrixD* theCovMatrix);
  virtual ~TFitParticlePxPy();
  virtual TAbsFitParticle* clone( const TString& newname = TString("") ) const;

  // returns derivative dP/dy with P=(p,E) and y=(r, theta, phi, ...) 
  // the free parameters of the fit. The columns of the matrix contain 
  // (dP/dr, dP/dtheta, ...).
  virtual TMatrixD* getDerivative();
  virtual TMatrixD* transform(const TLorentzVector& vec);
  virtual void setIni4Vec(const TLorentzVector* pini);
  void setIni4Vec(const TVector3* p, Double_t M);
  virtual TLorentzVector* calc4Vec( const TMatrixD* params );
  Double_t getPz() const{ return _pZ; }

protected :

  void init(TVector3* p, Double_t M, const TMatrixD* theCovMatrix);
  void init(TLorentzVector* pini, const TMatrixD* theCovMatrix);

  
private:
  Double_t _pZ;
  ClassDef(TFitParticlePxPy,1)

};

#endif
