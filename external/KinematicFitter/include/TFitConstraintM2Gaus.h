#ifndef TFitConstraintM2Gaus_hh
#define TFitConstraintM2Gaus_hh

#include "TFitConstraintM.h"

#include <vector>
class TAbsFitParticle;

class TFitConstraintM2Gaus: public TFitConstraintM {

public :

  TFitConstraintM2Gaus();
  TFitConstraintM2Gaus(std::vector<TAbsFitParticle*>* ParList1,
		      std::vector<TAbsFitParticle*>* ParList2,
		      Double_t Mass = 0, Double_t Width = 0);
  TFitConstraintM2Gaus(const TString &name, const TString &title,
		      std::vector<TAbsFitParticle*>* ParList1,
		      std::vector<TAbsFitParticle*>* ParList2,
		      Double_t Mass = 0, Double_t Width = 0);

  virtual ~TFitConstraintM2Gaus();

  virtual Double_t getInitValue();
  virtual Double_t getCurrentValue();
  virtual TMatrixD* getDerivativeAlpha();

  void setMassConstraint(Double_t Mass, Double_t Width);

  virtual TString getInfoString();
  virtual void print(); 

  double getWidth(){ return _width; }
  ClassDef(TFitConstraintM2Gaus, 1)
protected :
  
  Double_t _width;

  void init();


};

#endif

