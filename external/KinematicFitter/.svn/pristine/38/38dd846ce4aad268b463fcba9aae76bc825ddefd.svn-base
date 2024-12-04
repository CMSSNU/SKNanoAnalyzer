#ifndef TFitParticlePt_hh
#define TFitParticlePt_hh


#include "TAbsFitParticle.h"
#include "TLorentzVector.h"
#include "TMatrixD.h"


class TFitParticlePt: public TAbsFitParticle {

public :
  TFitParticlePt();
  TFitParticlePt( const TFitParticlePt& fitParticle );
  TFitParticlePt(TLorentzVector* pini, const TMatrixD* theCovMatrix);
  TFitParticlePt(const TString &name, const TString &title, 
	       TLorentzVector* pini,
	       const TMatrixD* theCovMatrix);
  virtual ~TFitParticlePt();
  virtual TAbsFitParticle* clone( const TString& newname = "" ) const;

  // returns derivative dP/dy with P=(p,E) and y=(et, eta, phi) 
  // the free parameters of the fit. The columns of the matrix contain 
  // (dP/d(et), dP/d(eta), dP/d(phi)).
  virtual TMatrixD* getDerivative();
  virtual TMatrixD* transform(const TLorentzVector& vec);
  virtual void setIni4Vec(const TLorentzVector* pini);
  virtual TLorentzVector* calc4Vec( const TMatrixD* params );
  Double_t getIniEta() const { return _iniEta; }
  Double_t getIniPhi() const { return _iniPhi; }
  Double_t getIniM() const { return _iniM; }

protected :

  void init(TLorentzVector* pini, const TMatrixD* theCovMatrix);


private:
  Double_t _iniEta;
  Double_t _iniPhi;
  Double_t _iniM;
  ClassDef(TFitParticlePt,1)

};


#endif
