#include "Gen.h"

ClassImp(Gen)

Gen::Gen() : Particle()
{
    j_motherIdx = -999;
    pdgId = -999;
    status = -999;
    statusFlags = 0;
}

Gen::~Gen() {}
