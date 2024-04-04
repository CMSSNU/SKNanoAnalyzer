#include "GenJet.h"

ClassImp(GenJet)

GenJet::GenJet() : Particle() {
  j_hadronFlavour = -999;
  j_partonFlavour = -999;
}

GenJet::~GenJet() {}

//////////
