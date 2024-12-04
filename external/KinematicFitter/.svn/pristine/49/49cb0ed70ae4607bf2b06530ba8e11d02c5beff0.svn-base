#ifndef TFitConstraintMGaus_hh
#define TFitConstraintMGaus_hh

#include "TFitConstraintM.h"

#include <vector>

class TAbsFitParticle;

class TFitConstraintMGaus: public TFitConstraintM {

public :

  TFitConstraintMGaus();
  TFitConstraintMGaus(std::vector<TAbsFitParticle*>* ParList1,
		      std::vector<TAbsFitParticle*>* ParList2,
		      Double_t Mass = 0, Double_t Width = 0);
  TFitConstraintMGaus(const TString &name, const TString &title,
		      std::vector<TAbsFitParticle*>* ParList1,
		      std::vector<TAbsFitParticle*>* ParList2,
		      Double_t Mass = 0, Double_t Width = 0);

  virtual ~TFitConstraintMGaus();

  virtual Double_t getInitValue();
  virtual Double_t getCurrentValue();
  virtual Double_t getChi2(); //BHO
  virtual TMatrixD* getDerivativeAlpha();

  void setMassConstraint(Double_t Mass, Double_t Width);

  virtual TString getInfoString();
  virtual void print(); 

  //BHO
  void Clear(){ _ParList1.clear(); _ParList2.clear(); }
  double getWidth(){ return _width; }

protected :
  
  Double_t _width;

  void init();

  ClassDef(TFitConstraintMGaus,1)

};

#endif

