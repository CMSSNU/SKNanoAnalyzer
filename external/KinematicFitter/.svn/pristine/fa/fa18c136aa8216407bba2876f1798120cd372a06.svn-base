// Classname: TFitConstraintM2Gaus
// Author: Jan E. Sundermann, Verena Klose (TU Dresden)      


//________________________________________________________________
// 
// TFitConstraintM2Gaus::
// --------------------
//
// Fit constraint: mass conservation ( m_i - m_j - alpha * MassConstraint == 0 )
//

#include <iostream>
#include <iomanip>
#include "TFitConstraintM2Gaus.h"

ClassImp(TFitConstraintM2Gaus)

//#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "TClass.h"


//----------------
// Constructor --
//----------------
TFitConstraintM2Gaus::TFitConstraintM2Gaus()
  : TFitConstraintM() 
{

  init();

}

TFitConstraintM2Gaus::TFitConstraintM2Gaus(std::vector<TAbsFitParticle*>* ParList1,
					 std::vector<TAbsFitParticle*>* ParList2, 
					 Double_t Mass,
					 Double_t Width)
  : TFitConstraintM(ParList1, ParList2, Mass ) 
{

  init();
  setMassConstraint( Mass, Width );

}

TFitConstraintM2Gaus::TFitConstraintM2Gaus(const TString &name, const TString &title,
					 std::vector<TAbsFitParticle*>* ParList1,
					 std::vector<TAbsFitParticle*>* ParList2, 
					 Double_t Mass,
					 Double_t Width)
  : TFitConstraintM( name, title, ParList1, ParList2, Mass )
{

  init();
  setMassConstraint( Mass, Width );

}

void
TFitConstraintM2Gaus::init() {

  _nPar = 1;
  _iniparameters.ResizeTo(1,1);
  _iniparameters(0,0) = 1.;
  _parameters.ResizeTo(1,1);
  _parameters = _iniparameters;

}

//--------------
// Destructor --
//--------------
TFitConstraintM2Gaus::~TFitConstraintM2Gaus() {

}

//--------------
// Operations --
//--------------

void TFitConstraintM2Gaus::setMassConstraint(Double_t Mass, Double_t Width) { 
  
  _TheMassConstraint = Mass*Mass;
  _width = Width*Width;
  setCovMatrix( 0 );
  if(!Mass) std::cout
    << "Error occured!\n"
    << "Object type : TFitConstraintM2Gaus\n"
    << "Object name : " << GetName() << "\n"
    << "Object title: " << GetTitle() << "\n"
    << "Mass of 0 GeV not supported, please choose a larger mass!\n";
  _covMatrix(0,0) = (Width*Width*Width*Width) / (Mass * Mass * Mass * Mass);

}

Double_t TFitConstraintM2Gaus::getInitValue() {
  // Get initial value of constraint (before the fit)

  Double_t initMass = CalcMass( &_ParList1, true ) - 
                      CalcMass( &_ParList2, true );
  Double_t InitValue = initMass*initMass;
  InitValue -= _iniparameters(0,0) * _TheMassConstraint;

  return InitValue;

}

Double_t TFitConstraintM2Gaus::getCurrentValue() {
  // Get value of constraint after the fit

  Double_t currMass = CalcMass(&_ParList1,false) - 
                      CalcMass(&_ParList2,false);
  Double_t CurrentValue = currMass*currMass;
  CurrentValue -= _parameters(0,0) * _TheMassConstraint;

  return CurrentValue;

}

TMatrixD* TFitConstraintM2Gaus::getDerivativeAlpha() { 
  // Calculate dF/dAlpha = -1 * M^2

  TMatrixD* DerivativeMatrix = new TMatrixD(1,1);
  DerivativeMatrix->Zero();

  (*DerivativeMatrix)(0,0) = -1. * _TheMassConstraint;

  return DerivativeMatrix;

}

TString TFitConstraintM2Gaus::getInfoString() {
  // Collect information to be used for printout

  std::stringstream info;
  info << std::scientific << std::setprecision(6);

  info << "__________________________" << std::endl
       << std::endl;
  info << "OBJ: " << IsA()->GetName() << "\t" << GetName() << "\t" << GetTitle() << std::endl;

  info << "initial value: " << getInitValue() << std::endl;
  info << "current value: " << getCurrentValue() << std::endl;
  info << "mean mass: " << _TheMassConstraint << std::endl;
  info << "width: " << _width << std::endl;
  info << "initial mass: " << _iniparameters(0,0)*_TheMassConstraint  << std::endl;
  info << "current mass: " << _parameters(0,0)*_TheMassConstraint  << std::endl;

  return info.str();

}

void TFitConstraintM2Gaus::print() {
  // Print constraint contents

  //edm::LogVerbatim("KinFitter") << this->getInfoString();

}

