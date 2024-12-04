// Classname: TFitParticlePz
// Author: S.Paktinat(IPM, CMS)     
// 27 July 2005


//________________________________________________________________
// 
// TFitParticlePz::
// --------------------
//
// Particle with a special parametrization useful in hadron 
// colliders (3 free parameters (Et, Eta, Phi). The parametrization is 
// chosen as follows:
//
// p = (EtCosPhi, EtSinPhi, EtSinhEta)
// E =  EtCoshEta
//

#include <iostream>
//#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "TFitParticlePz.h"

ClassImp(TFitParticlePz)

#include "TMath.h"
#include <cmath>

//----------------
// Constructor --
//----------------
TFitParticlePz::TFitParticlePz()
  :TAbsFitParticle()  
{
  init(0, 0);
}

TFitParticlePz::TFitParticlePz( const TFitParticlePz& fitParticle )
  :TAbsFitParticle( fitParticle.GetName(), fitParticle.GetTitle() )
{

  _nPar = fitParticle._nPar;
  _u1 = fitParticle._u1;
  _u2 = fitParticle._u2;
  _u3 = fitParticle._u3;
  _covMatrix.ResizeTo(  fitParticle._covMatrix );
  _covMatrix = fitParticle._covMatrix;
  _iniparameters.ResizeTo( fitParticle._iniparameters );
  _iniparameters = fitParticle._iniparameters;
  _parameters.ResizeTo( fitParticle._parameters );
  _parameters = fitParticle._parameters;
  _pini = fitParticle._pini;
  _pcurr = fitParticle._pcurr;
  _iniPx = fitParticle.getIniPx();
  _iniPy = fitParticle.getIniPy();

}

TFitParticlePz::TFitParticlePz(TLorentzVector* pini, const TMatrixD* theCovMatrix)
  :TAbsFitParticle()  
{
  init(pini, theCovMatrix);
}

TFitParticlePz::TFitParticlePz(const TString &name, const TString &title, 
			   TLorentzVector* pini, const TMatrixD* theCovMatrix)
  :TAbsFitParticle(name, title)  
{
  init(pini, theCovMatrix);
}

TAbsFitParticle* TFitParticlePz::clone( const TString& newname ) const {
  // Returns a copy of itself
  
  TAbsFitParticle* myclone = new TFitParticlePz( *this );
  if ( newname.Length() > 0 ) myclone->SetName(newname);
  return myclone;

}

//--------------
// Destructor --
//--------------
TFitParticlePz::~TFitParticlePz() {

}

//--------------
// Operations --
//--------------
void TFitParticlePz::init(TLorentzVector* pini, const TMatrixD* theCovMatrix ) {

  _nPar = 1;
  setIni4Vec(pini);
  setCovMatrix(theCovMatrix);

}

TLorentzVector* TFitParticlePz::calc4Vec( const TMatrixD* params ) {
  // Calculates a 4vector corresponding to the given
  // parameter values

  if (params == 0) {
    return 0;
  }

  if ( params->GetNcols() != 1 || params->GetNrows() !=_nPar ) {
    //edm::LogError ("WrongMatrixSize")
    //  << GetName() << "::calc4Vec - Parameter matrix has wrong size.";
    return 0;
  }


  Double_t X = _iniPx;
  Double_t Y = _iniPy;
  Double_t Z = (*params)(0,0);
  Double_t E = TMath::Sqrt(X*X + Y*Y + Z*Z + _pini.M2() );
		
  TLorentzVector* vec = new TLorentzVector( X, Y, Z, E );
  return vec;

}

void TFitParticlePz::setIni4Vec(const TLorentzVector* pini) {
  // Set the initial 4vector. Will also set the 
  // inital parameter values 

  if (pini == 0) {

    _u1.SetXYZ(0., 0., 0.);
    _u3.SetXYZ(0., 0., 0.);
    _u2.SetXYZ(0., 0., 0.);
    _pini.SetXYZT(0., 0., 0., 0.);
    _pcurr = _pini;

    _iniparameters.ResizeTo(_nPar,1);
    _iniparameters(0,0) = 0.;
    
    _parameters.ResizeTo(_nPar,1);
    _parameters(0,0) = 0.;
    
  } else {
    
    Double_t pz = pini->Pz();
    _iniPx = pini->Px();
    _iniPy = pini->Py();
    
    _pini = (*pini);
    _pcurr = _pini;
    
    _iniparameters.ResizeTo(_nPar,1);
    _iniparameters(0,0) = pz;
    
    _parameters.ResizeTo(_nPar,1);
    _parameters = _iniparameters;

    _u1.SetXYZ( 1., 0., 0.); // the base vector of x
    _u2.SetXYZ( 0., 1., 0. );// the base vector of y    
    _u3.SetXYZ( 0., 0., 1. );// the base vector of z

  }

}

TMatrixD* TFitParticlePz::getDerivative() {
  // returns derivative dP/dy with P=(p,E) and y=(et, eta, phi) 
  // the free parameters of the fit. The columns of the matrix contain 
  // (dP/d(et), dP/d(eta), dP/d(phi)).

  TMatrixD* DerivativeMatrix = new TMatrixD(4,1);
  (*DerivativeMatrix) *= 0.;

  Double_t pz = _parameters(0,0);
  Double_t px = _iniPx;
  Double_t py = _iniPy;

  //1st column: dP/d(pz)
  (*DerivativeMatrix)(0,0) = 0.;
  (*DerivativeMatrix)(1,0) = 0.;
  (*DerivativeMatrix)(2,0) = 1.;
  (*DerivativeMatrix)(3,0) = pz/_pcurr.E();

   return DerivativeMatrix;

}

TMatrixD* TFitParticlePz::transform(const TLorentzVector& vec) {
  // Returns the parameters corresponding to the given 
  // 4vector

  // retrieve parameters
  TMatrixD* tparams = new TMatrixD( _nPar, 1 );
  (*tparams)(0,0) = vec.Pz();

  return tparams;

}
