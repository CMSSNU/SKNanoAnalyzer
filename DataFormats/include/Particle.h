#ifndef Particle_h
#define Particle_h

#include <iostream>
#include "TLorentzVector.h"
#include "TString.h"
#include "ROOT/RVec.hxx"
using namespace std;
using namespace ROOT::VecOps;

class Particle: public TLorentzVector{
public:
    // Default constructor with p4
    Particle();
    // TLorentzVector copy constructor
    Particle(const TLorentzVector &p);
    // Copy constructor
    Particle(const Particle &p);
    // Constructor from 4-vector components
    Particle(float px, float py, float pz, float e);

    // Charge operator
    Particle &operator+=(const Particle &p);

    // Assignment operator
    Particle &operator=(const Particle &p);

    // Comparison operators
    bool operator<(const Particle &p) const;
    bool operator>(const Particle &p) const;

    // destructor
    ~Particle();

    // set functions
    void SetCharge(float q);
    inline float Charge() const { return j_Charge; }

    // print p4
    virtual void Print();

private:
    float j_Charge;
    ClassDef(Particle, 1)

};

#endif
