#ifndef LHE_h 
#define LHE_h

#include "Particle.h"

class LHE : public Particle {
    
public :    
    LHE();
    ~LHE();
    inline void SetIncomingPz(float incomingPz) { j_incomingPz = incomingPz; }
    inline float IncomingPz() const { return j_incomingPz; }
    inline void SetSpin(int spin) { j_spin = spin; }
    inline int Spin() const { return j_spin; }
    inline void SetStatus(int status) { j_status = status; }
    inline int Status() const { return j_status; }

private :
    float j_incomingPz;
    int j_spin;
    int j_status;
    ClassDef(LHE, 1);
};

#endif