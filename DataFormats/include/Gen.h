#ifndef Gen_h
#define Gen_h

#include "Particle.h"

class Gen : public Particle {
    
public :    
    Gen();
    ~Gen();
    inline void SetMotherIdx(short idx) { j_motherIdx = idx; }
    inline short MotherIdx() const { return j_motherIdx; }
    inline void SetPdgId(int pdg) { pdgId = pdg; }
    inline int PdgId() const { return pdgId; }
    inline void SetStatus(int stat) { status = stat; }
    inline int Status() const { return status; }
    inline void SetStatusFlags(unsigned short statFlags) { statusFlags = statFlags; }
    inline unsigned short StatusFlags() const { return statusFlags; }

private :
    short j_motherIdx;
    int j_pdgId;
    int j_status;
    unsigned short j_statusFlags;

    ClassDef(Gen, 1);
};

#endif