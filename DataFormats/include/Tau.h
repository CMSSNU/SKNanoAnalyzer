#ifndef Tau_h
#define Tau_h

#include "TString.h"
#include "Tau.h"

class Tau : public Lepton {
    
public :
    
    Tau();
    ~Tau();

    void SetIdDecayModeNewDMs(bool id) {j_idDecayModeNewDMs = id;}
    inline bool passDecayModeNewDMs() const {return j_idDecayModeNewDMs;}

    void SetDecayMode(unsigned char decayMode) {j_decayMode = decayMode;}
    inline unsigned char DecayMode() const {return j_decayMode;}

    void SetGenPartFlav(unsigned char genPartFlav) {j_genPartFlav = genPartFlav;}
    inline unsigned char GenPartFlav() const {return j_genPartFlav;}
    
    void SetIdDeepTau2018v2p5VSjet(unsigned char id) {j_idDeepTau2018v2p5VSjet = id;}
    inline bool passVVVLIDvJet() const {return j_idDeepTau2018v2p5VSjet == 1;}
    inline bool passVVLIDvJet()  const {return j_idDeepTau2018v2p5VSjet == 2;}
    inline bool passVLIDvJet()   const {return j_idDeepTau2018v2p5VSjet == 3;}
    inline bool passLIDvJet()    const {return j_idDeepTau2018v2p5VSjet == 4;}
    inline bool passMIDvJet()    const {return j_idDeepTau2018v2p5VSjet == 5;}
    inline bool passTIDvJet()    const {return j_idDeepTau2018v2p5VSjet == 6;}
    inline bool passVTIDvJet()   const {return j_idDeepTau2018v2p5VSjet == 7;}
    inline bool passVVTIDvJet()  const {return j_idDeepTau2018v2p5VSjet == 8;}

    void SetIdDeepTau2018v2p5VSe(unsigned char id) {j_idDeepTau2018v2p5VSe = id;}
    inline bool passVVVLIDvEl()  const {return j_idDeepTau2018v2p5VSe == 1;}
    inline bool passVVLIDvEl()   const {return j_idDeepTau2018v2p5VSe == 2;}
    inline bool passVLIDvEl()    const {return j_idDeepTau2018v2p5VSe == 3;}
    inline bool passLIDvEl()     const {return j_idDeepTau2018v2p5VSe == 4;}
    inline bool passMIDvEl()     const {return j_idDeepTau2018v2p5VSe == 5;}
    inline bool passTIDvEl()     const {return j_idDeepTau2018v2p5VSe == 6;}
    inline bool passVTIDvEl()    const {return j_idDeepTau2018v2p5VSe == 7;}
    inline bool passVVTIDvEl()   const {return j_idDeepTau2018v2p5VSe == 8;}

    void SetIdDeepTau2018v2p5VSmu(unsigned char id) {j_idDeepTau2018v2p5VSmu = id;}
    inline bool passVLIDvMu()    const {return j_idDeepTau2018v2p5VSmu == 1;}
    inline bool passLIDvMu()     const {return j_idDeepTau2018v2p5VSmu == 2;}
    inline bool passMIDvMu()     const {return j_idDeepTau2018v2p5VSmu == 3;}
    inline bool passTIDvMu()     const {return j_idDeepTau2018v2p5VSmu == 4;}

    bool PassID(const TString ID) const;

private :
    
    bool j_idDecayModeNewDMs;
    unsigned char j_decayMode, j_genPartFlav;
    unsigned char j_idDeepTau2018v2p5VSe, j_idDeepTau2018v2p5VSjet, j_idDeepTau2018v2p5VSmu;
    float j_rawDeepTau2018v2p5VSe, j_rawDeepTau2018v2p5VSjet, j_rawDeepTau2018v2p5VSmu;
    int j_decayModePNet;
    ClassDef(Tau, 1);

};

#endif