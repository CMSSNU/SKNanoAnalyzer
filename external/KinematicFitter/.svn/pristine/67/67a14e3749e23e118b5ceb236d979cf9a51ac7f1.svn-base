// Classname: TFitParticlePxPy
// Author: Jan E. Sundermann, Verena Klose (TU Dresden)      


//________________________________________________________________
// 
// TFitParticlePxPy::
// --------------------
//
// Particle with cartesian 4vector parametrization and constrained mass
// [three free parameters (px, py, pz) with initial values
//  (px, py, pz)]
//
// p = px*u1 + py*u2 + pz*u3
// E = Sqrt( |p|^2 + m^2 )
//

#include <iostream>
//#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "TFitParticlePxPy.h"

ClassImp(TFitParticlePxPy)

#include "TMath.h"


//----------------
// Constructor --
//----------------
TFitParticlePxPy::TFitParticlePxPy()
  :TAbsFitParticle()
{ 
  init( 0, 0., 0);
}

TFitParticlePxPy::TFitParticlePxPy( const TFitParticlePxPy& fitParticle )
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
  _pZ = fitParticle.getPz();

}

TFitParticlePxPy::TFitParticlePxPy(TVector3* p, Double_t M, const TMatrixD* theCovMatrix)
  :TAbsFitParticle()
{ 
  init(p, M, theCovMatrix);
}

TFitParticlePxPy::TFitParticlePxPy(const TString &name, const TString &title,
			       TVector3* p, Double_t M, const TMatrixD* theCovMatrix)
  :TAbsFitParticle(name, title)
{ 
  init(p, M, theCovMatrix);
}

TFitParticlePxPy::TFitParticlePxPy(const TString &name, const TString &title,
			       TLorentzVector* pini, const TMatrixD* theCovMatrix)
  :TAbsFitParticle(name, title)
{ 
  init(pini, theCovMatrix);
}

TAbsFitParticle* TFitParticlePxPy::clone( const TString& newname ) const {
  // Returns a copy of itself

  TAbsFitParticle* myclone = new TFitParticlePxPy( *this );
  if ( newname.Length() > 0 ) myclone->SetName(newname);
  return myclone;

}

//--------------
// Destructor --
//--------------
TFitParticlePxPy::~TFitParticlePxPy() {

}


//--------------
// Operations --
//--------------
void TFitParticlePxPy::init(TVector3* p, Double_t M, const TMatrixD* theCovMatrix) {

  _nPar = 2;
  setIni4Vec(p, M);
  setCovMatrix(theCovMatrix);
  
}

void TFitParticlePxPy::init(TLorentzVector* pini, const TMatrixD* theCovMatrix) {

  _nPar = 2;
  setIni4Vec(pini);
  setCovMatrix(theCovMatrix);
  
}


TLorentzVector* TFitParticlePxPy::calc4Vec( const TMatrixD* params ) {
  // Calculates a 4vector corresponding to the given
  // parameter values

  if (params == 0) {
    return 0;
  }

  if ( params->GetNcols() != 1 || params->GetNrows() !=_nPar ) {
    //edm::LogError ("WrongMatrixSize")
      //<< GetName() << "::calc4Vec - Parameter matrix has wrong size.";
    return 0;
  }
  
  Double_t X = (*params)(0,0);
  Double_t Y = (*params)(1,0); 
  Double_t Z = _pZ;
  Double_t E = TMath::Sqrt(  X*X + Y*Y + Z*Z + _pini.M2() );

  TLorentzVector* vec = new TLorentzVector( X, Y, Z, E );
  return vec;

}

void TFitParticlePxPy::setIni4Vec(const TLorentzVector* pini) {
  // Set the initial 4vector. Will also set the 
  // inital parameter values 

  TVector3 vec( pini->Vect() );
  setIni4Vec( &vec, pini->M() );

}

void TFitParticlePxPy::setIni4Vec(const TVector3* p, Double_t M) {
  // Set the initial 4vector. Will also set the 
  // inital parameter values

  if ( p == 0 ) {

    _iniparameters.ResizeTo(_nPar,1);
    _iniparameters(0,0) = 0.;
    _iniparameters(1,0) = 0.;
    _pZ = 0.;
    _parameters.ResizeTo(_nPar,1);
    _parameters = _iniparameters;

    _pini.SetXYZM( 0., 0., 0., M);
    _pcurr = _pini;

  } else {

    _iniparameters.ResizeTo(_nPar,1);
    _iniparameters(0,0) = p->x();
    _iniparameters(1,0) = p->y();
    _pZ = p->z();
    _parameters.ResizeTo(_nPar,1);
    _parameters = _iniparameters;

    _pini.SetXYZM( p->x(), p->y(), p->z(), M);
    _pcurr = _pini;

    _u1.SetXYZ( 1., 0., 0. );
    _u2.SetXYZ( 0., 1., 0. );
    _u3.SetXYZ( 0., 0., 1. );
  
  }

}

TMatrixD* TFitParticlePxPy::getDerivative() {
  // returns derivative dP/dy with P=(p,E) and y=(px, py, pz) 
  // the free parameters of the fit. The columns of the matrix contain 
  // (dP/dpx, dP/dpy, ...).

  TMatrixD* DerivativeMatrix = new TMatrixD(4,2);
  (*DerivativeMatrix) *= 0.;

  //1st column: dP/dx
  (*DerivativeMatrix)(0,0) =  1.;
  (*DerivativeMatrix)(1,0) =  0.;
  (*DerivativeMatrix)(2,0) =  0.;
  (*DerivativeMatrix)(3,0) = _parameters(0,0)/_pcurr.E();

  //2nd column: dP/dy
  (*DerivativeMatrix)(0,1) = 0.;
  (*DerivativeMatrix)(1,1) = 1.;
  (*DerivativeMatrix)(2,1) = 0.;
  (*DerivativeMatrix)(3,1) = _parameters(1,0)/_pcurr.E();


  return DerivativeMatrix;  

}

TMatrixD* TFitParticlePxPy::transform(const TLorentzVector& vec) {
  // Returns the parameters corresponding to the given 
  // 4vector

  TVector3 vec3( vec.Vect() );

  // retrieve parameters
  TMatrixD* tparams = new TMatrixD( _nPar, 1 );
  (*tparams)(0,0) = vec3.x();
  (*tparams)(1,0) = vec3.y();

  return tparams;

}
