// Classname: TFitParticlePt
// Author: S.Paktinat(IPM, CMS)     
// 27 July 2005


//________________________________________________________________
// 
// TFitParticlePt::
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
#include "TFitParticlePt.h"

ClassImp(TFitParticlePt)

#include "TMath.h"
#include <cmath>

//----------------
// Constructor --
//----------------
TFitParticlePt::TFitParticlePt()
  :TAbsFitParticle()  
{
  init(0, 0);
}

TFitParticlePt::TFitParticlePt( const TFitParticlePt& fitParticle )
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
  _iniEta = fitParticle.getIniEta();
  _iniPhi = fitParticle.getIniPhi();
  _iniM = fitParticle.getIniM();

}

TFitParticlePt::TFitParticlePt(TLorentzVector* pini, const TMatrixD* theCovMatrix)
  :TAbsFitParticle()  
{
  init(pini, theCovMatrix);
}

TFitParticlePt::TFitParticlePt(const TString &name, const TString &title, 
			   TLorentzVector* pini, const TMatrixD* theCovMatrix)
  :TAbsFitParticle(name, title)  
{
  init(pini, theCovMatrix);
}

TAbsFitParticle* TFitParticlePt::clone( const TString& newname ) const {
  // Returns a copy of itself
  
  TAbsFitParticle* myclone = new TFitParticlePt( *this );
  if ( newname.Length() > 0 ) myclone->SetName(newname);
  return myclone;

}

//--------------
// Destructor --
//--------------
TFitParticlePt::~TFitParticlePt() {

}

//--------------
// Operations --
//--------------
void TFitParticlePt::init(TLorentzVector* pini, const TMatrixD* theCovMatrix ) {

  _nPar = 1;
  setIni4Vec(pini);
  setCovMatrix(theCovMatrix);

}

TLorentzVector* TFitParticlePt::calc4Vec( const TMatrixD* params ) {
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

  Double_t pt = (*params)(0,0);
  Double_t eta = _iniEta;
  Double_t phi = _iniPhi;
  Double_t m = _iniM;

  Double_t X = pt*TMath::Cos(phi);
  Double_t Y = pt*TMath::Sin(phi);
  Double_t Z = pt*TMath::SinH(eta);
  Double_t E = TMath::Sqrt( X*X + Y*Y + Z*Z + m*m );
		
  TLorentzVector* vec = new TLorentzVector( X, Y, Z, E );
  return vec;

}

void TFitParticlePt::setIni4Vec(const TLorentzVector* pini) {
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
    
    Double_t pt = pini->Pt();
    _iniEta = pini->Eta();
    _iniPhi = pini->Phi();
    _iniM = pini->M();

    _pini = (*pini);
    _pcurr = _pini;
    
    _iniparameters.ResizeTo(_nPar,1);
    _iniparameters(0,0) = pt;
    
    _parameters.ResizeTo(_nPar,1);
    _parameters = _iniparameters;

    _u1.SetXYZ( TMath::Cos(_iniPhi), TMath::Sin(_iniPhi), 0.); // the base vector of Pt
    _u2.SetXYZ( -1.*TMath::Cos(_iniPhi)*TMath::TanH(_iniEta), -1.*TMath::Sin(_iniPhi)*TMath::TanH(_iniEta), 1./TMath::CosH(_iniEta) );// the base vector of Eta ( same as the base vector for Theta)
    _u3.SetXYZ( -1.*TMath::Sin(_iniPhi), TMath::Cos(_iniPhi), 0. );// the base vector of Phi

  }

}

TMatrixD* TFitParticlePt::getDerivative() {
  // returns derivative dP/dy with P=(p,E) and y=(pt, eta, phi) 
  // the free parameters of the fit. The columns of the matrix contain 
  // (dP/d(pt), dP/d(eta), dP/d(phi)).

  TMatrixD* DerivativeMatrix = new TMatrixD(4,1);
  (*DerivativeMatrix) *= 0.;

  Double_t eta = _iniEta;
  Double_t phi = _iniPhi;

  //1st column: dP/d(pt)
  (*DerivativeMatrix)(0,0) = TMath::Cos(phi);
  (*DerivativeMatrix)(1,0) = TMath::Sin(phi);
  (*DerivativeMatrix)(2,0) = TMath::SinH(eta);
  Double_t cosh_eta = TMath::CosH(eta);
  (*DerivativeMatrix)(3,0) = _parameters(0,0)*cosh_eta*cosh_eta/_pcurr.E();

   return DerivativeMatrix;

}

TMatrixD* TFitParticlePt::transform(const TLorentzVector& vec) {
  // Returns the parameters corresponding to the given 
  // 4vector

  // retrieve parameters
  TMatrixD* tparams = new TMatrixD( _nPar, 1 );
  (*tparams)(0,0) = vec.Pt();

  return tparams;

}
