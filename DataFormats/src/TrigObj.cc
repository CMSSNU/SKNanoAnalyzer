#include "TrigObj.h"

ClassImp(TrigObj)

TrigObj::TrigObj() : Particle() {
    j_id = -999;
    j_filterBits = 0;
    j_run = 2; // Default to Run 2
}

TrigObj::~TrigObj() {}

//////////
