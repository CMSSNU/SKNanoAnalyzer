#ifndef Gen_h
#define Gen_h

#include "Particle.h"

class Gen : public Particle {
    
public :    
    Gen();
    ~Gen();
    inline void SetMotherIdx(short idx) { j_motherIdx = idx; }
    inline short MotherIdx() const { return j_motherIdx; }
    inline void SetPdgId(int pdg) { j_pdgId = pdg; }
    inline int PdgId() const { return j_pdgId; }
    inline void SetStatus(int stat) { j_status = stat; }
    inline int Status() const { return j_status; }
    inline void SetStatusFlags(unsigned short statFlags) { j_statusFlags = statFlags; }
    inline unsigned short StatusFlags() const { return j_statusFlags; }
    enum class statusFlag {
        isPrompt = 0,
        isDecayedLeptonHadron = 1,
        isTauDecayProduct = 2,
        isPromptTauDecayProduct = 3,
        isDirectTauDecayProduct = 4,
        isDirectPromptTauDecayProduct = 5,
        isDirectHadronDecayProduct = 6,
        isHardProcess = 7,
        fromHardProcess = 8,
        isHardProcessTauDecayProduct = 9,
        isDirectHardProcessTauDecayProduct = 10,
        fromHardProcessBeforeFSR = 11,
        isFirstCopy = 12,
        isLastCopy = 13,
        isLastCopyBeforeFSR = 14
    };
private :
    short j_motherIdx;
    int j_pdgId;
    int j_status;
    unsigned short j_statusFlags;

    ClassDef(Gen, 1);
};

#endif