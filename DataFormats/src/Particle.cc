#include "Particle.h"

ClassImp(Particle);

Particle::Particle(): TLorentzVector(), j_Charge(0) {}
Particle::Particle(const TLorentzVector &p): TLorentzVector(p), j_Charge(0) {}
Particle::Particle(const Particle &p): TLorentzVector(p), j_Charge(p.Charge()) {}
Particle::Particle(double px, double py, double pz, double e): TLorentzVector(px, py, pz, e), j_Charge(0) {}
Particle::~Particle() {}

Particle &Particle::operator+=(const Particle &p) {
    TLorentzVector::operator+=(p);
    j_Charge += p.j_Charge;
    return *this;
}

Particle &Particle::operator=(const Particle &p) {
    if (this != &p) {
        TLorentzVector::operator=(p);
        j_Charge = p.j_Charge;
    }
    return *this;
}

void Particle::SetCharge(double q) {
    j_Charge = q;
}

void Particle::Print() {
    cout << "(Pt, Eta, Phi, M, Charge) = " << Pt() << "\t" << Eta() << "\t" << Phi() << "\t" << M() << "\t" << Charge() << endl;
}
