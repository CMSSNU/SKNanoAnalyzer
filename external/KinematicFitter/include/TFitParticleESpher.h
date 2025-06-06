#ifndef TFitParticleESpher_hh
#define TFitParticleESpher_hh


#include "TAbsFitParticle.h"
#include "TLorentzVector.h"
#include "TMatrixD.h"

class TFitParticleESpher: public TAbsFitParticle {

public :

  TFitParticleESpher();
  TFitParticleESpher( const TFitParticleESpher& fitParticle );
  TFitParticleESpher(TLorentzVector* pini, const TMatrixD* theCovMatrix);
  TFitParticleESpher(const TString &name, const TString &title, 
	       TLorentzVector* pini,
	       const TMatrixD* theCovMatrix);
  virtual ~TFitParticleESpher();
  virtual TAbsFitParticle* clone( const TString& newname = TString("") ) const;

  // returns derivative dP/dy with P=(p,E) and y=(r, theta, phi, ...) 
  // the free parameters of the fit. The columns of the matrix contain 
  // (dP/dr, dP/dtheta, ...).
  virtual TMatrixD* getDerivative();
  virtual TMatrixD* transform(const TLorentzVector& vec);
  virtual void setIni4Vec(const TLorentzVector* pini);
  virtual TLorentzVector* calc4Vec( const TMatrixD* params );
  ClassDef(TFitParticleESpher, 1);
protected :

  void init(TLorentzVector* pini, const TMatrixD* theCovMatrix);


private:
  

};

#endif
