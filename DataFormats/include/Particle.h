#ifndef Particle_h
#define Particle_h

#include <TBuffer.h>
#include <TLorentzVector.h>
#include <TString.h>
#include <iostream>
using namespace std;

class Particle: public TLorentzVector{
public:
    // Default constructor with p4
    Particle();
    // TLorentzVector copy constructor
    Particle(const TLorentzVector &p);
    // Copy constructor
    Particle(const Particle &p);
    // Constructor from 4-vector components
    Particle(double px, double py, double pz, double e);

    // Charge operator
    Particle &operator+=(const Particle &p);

    // Assignment operator
    Particle &operator=(const Particle &p);

    // destructor
    ~Particle();

    // set functions
    void SetCharge(double q);
    inline double Charge() const { return j_Charge; }

    // print p4
    virtual void Print();

private:
    double j_Charge;
    ClassDef(Particle, 1)

};

#endif
