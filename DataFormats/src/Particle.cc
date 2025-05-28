#include "Particle.h"
ClassImp(Particle);

Particle::Particle(): fLorentzVector(), j_Charge(0) {}
Particle::Particle(const fLorentzVector &p): fLorentzVector(p), j_Charge(0) {}
Particle::Particle(const Particle &p): fLorentzVector(p), j_Charge(p.Charge()) {}
//Particle::Particle(float px, float py, float pz, float e): LorentzVector<PtEtaPhiM4D<float>>(px, py, pz, e), j_Charge(0) {}
Particle::~Particle() {}

Particle &Particle::operator+=(const Particle &p) {
    fLorentzVector::operator+=(p);
    j_Charge += p.j_Charge;
    return *this;
}

Particle &Particle::operator=(const Particle &p) {
    if (this != &p) {
        fLorentzVector::operator=(p);
        j_Charge = p.j_Charge;
    }
    return *this;
}

bool Particle::operator<(const Particle &p) const {
    return Pt() < p.Pt();
}

bool Particle::operator>(const Particle &p) const {
    return Pt() > p.Pt();
}

void Particle::SetPtEtaPhiM(float pt, float eta, float phi, float m) {
    fLorentzVector::SetXYZT(pt, eta, phi, m);
}

void Particle::SetCharge(float q) {
    j_Charge = q;
}

float Particle::DeltaPhi(const Particle &p) const {
    float dPhi = Phi() - p.Phi();
    if (dPhi > TMath::Pi() || dPhi <= -TMath::Pi()) {
        if (dPhi > 0) {
            dPhi -= TMath::TwoPi();
        } else {
            dPhi += TMath::TwoPi();
        }
    }
    return dPhi;
}


float Particle::DeltaR(const Particle &p) const {
    float dEta = Eta() - p.Eta();
    float dPhi = Phi() - p.Phi();
    
    // Normalize dPhi to be between -π and π
    if (dPhi > TMath::Pi() || dPhi <= -TMath::Pi()) {
        if (dPhi > 0) {
            int n = static_cast<int>(dPhi / TMath::TwoPi() + 0.5);
            dPhi -= n * TMath::TwoPi();
        } else {
            int n = static_cast<int>(0.5 - dPhi / TMath::TwoPi());
            dPhi -= n * TMath::TwoPi();
        }
    }
    return std::sqrt(dEta*dEta + dPhi*dPhi);
}

void Particle::Print() const {
    cout << "(Pt, Eta, Phi, M, Charge) = " << Pt() << "\t" << Eta() << "\t" << Phi() << "\t" << M() << "\t" << Charge() << endl;
}
