#include "GenJet.h"

ClassImp(GenJet)

GenJet::GenJet() : Particle() {
  j_hadronFlavour = 99;    // unsigned char
  j_partonFlavour = -999;   // short
}

GenJet::~GenJet() {}

//////////
