#ifndef Gen_h
#define Gen_h

#include "Particle.h"

class Gen: public Particle {
public:

    Gen();
    ~Gen();

    //==== For empty Gen
    void SetIsEmpty(bool b);
    inline bool IsEmpty() const { return j_IsEmpty; }

    void SetIndexPIDStatus(int i, int p, int t);
    inline int Index() const { return j_index; }
    inline int PID() const { return j_PID; }
    inline int Status() const { return j_status; }
    void SetMother(int mindex);
    inline int MotherIndex() const { return j_mother_index; }

    //==== Gen Status Flags
    void SetGenStatusFlags(unsigned short f);

    inline bool isPrompt() const { return j_status_flags & 1UL<<0; }
    inline bool isDecayedLeptonHadron() const { return j_status_flags & 1UL<<1; }
    inline bool isTauDecayProduct() const { return j_status_flags & 1UL<<2; }
    inline bool isPromptTauDecayProduct() const { return j_status_flags & 1UL<<3; }
    inline bool isDirectTauDecayProduct() const { return j_status_flags & 1UL<<4; }
    inline bool isDirectPromptTauDecayProduct() const { return j_status_flags & 1UL<<5; }
    inline bool isDirectHadronDecayProduct() const { return j_status_flags & 1UL<<6; }
    inline bool isHardProcess() const { return j_status_flags & 1UL<<7; }
    inline bool fromHardProcess() const { return j_status_flags & 1UL<<8; }
    inline bool isHardProcessTauDecayProduct() const { return j_status_flags & 1UL<<9; }
    inline bool isDirectHardProcessTauDecayProduct() const { return j_status_flags & 1UL<<10; }
    inline bool fromHardProcessBeforeFSR() const { return j_status_flags & 1UL<<11; }
    inline bool isFirstCopy() const { return j_status_flags & 1UL<<12; }
    inline bool isLastCopy() const { return j_status_flags & 1UL<<13; }
    inline bool isLastCopyBeforeFSR() const { return j_status_flags & 1UL<<14; }

    virtual void Print() const;

private:

    bool j_IsEmpty;
    int j_index, j_PID, j_status;
    int j_mother_index;
    unsigned short j_status_flags;
    bool j_isPrompt, j_isDecayedLeptonHadron, j_isTauDecayProduct, j_isPromptTauDecayProduct, j_isDirectTauDecayProduct, j_isDirectPromptTauDecayProduct, j_isDirectHadronDecayProduct, j_isHardProcess, j_fromHardProcess, j_isHardProcessTauDecayProduct, j_isDirectHardProcessTauDecayProduct, j_fromHardProcessBeforeFSR, j_isFirstCopy, j_isLastCopy, j_isLastCopyBeforeFSR;

    ClassDef(Gen,1)
};

#endif
