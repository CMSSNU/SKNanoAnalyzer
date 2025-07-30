#include "LHE.h"

ClassImp(LHE)

LHE::LHE() : Particle() {
    j_IsEmpty = true;
    j_incomingPz = -999.;
    j_spin = -999;
    j_status = -999;
    j_pdgId = -999;
}

LHE::~LHE() {}

void LHE::Print() const {
    cout << "(PID, Status, Pt, Eta, Phi, M, Spin) = " << PdgId() << "\t" << Status() << "\t" << Pt() << "\t" << Eta() << "\t" << Phi() << "\t" << M() << "\t" << Spin() << endl;
}
