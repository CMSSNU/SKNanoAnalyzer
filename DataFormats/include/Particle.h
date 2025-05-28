#ifndef Particle_h
#define Particle_h

#include <iostream>
//#include "TLorentzVector.h"
#include "TMath.h"
#include "Math/LorentzVector.h"
#include "Math/Vector4D.h"
#include "TString.h"
#include "ROOT/RVec.hxx"
using namespace std;
using namespace ROOT::VecOps;

typedef ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<float>> fLorentzVector;

class Particle: public fLorentzVector {
public:
    // Default constructor with p4
    Particle();
    // LorentzVector copy constructor
    Particle(const fLorentzVector &p);
    // Copy constructor
    Particle(const Particle &p);
    // Constructor from 4-vector components
    // Particle(float px, float py, float pz, float e);

    // Charge operator
    Particle &operator+=(const Particle &p);

    // Assignment operator
    Particle &operator=(const Particle &p);

    // Comparison operators
    bool operator<(const Particle &p) const;
    bool operator>(const Particle &p) const;

    // set functions
    void SetPtEtaPhiM(float pt, float eta, float phi, float m);
    void SetCharge(float q);
    inline float Charge() const { return j_Charge; }
    
    // Calculate DeltaR between this particle and another
    float DeltaR(const Particle &p) const;

    // Calculate DeltaPhi between this particle and another
    float DeltaPhi(const Particle &p) const;

    // print p4
    virtual void Print() const;

    // destructor
    ~Particle();

private:
    float j_Charge;
    ClassDef(Particle, 1)

};

#endif
