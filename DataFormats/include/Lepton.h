#ifndef Lepton_h
#define Lepton_h

#include "Particle.h"

class Lepton: public Particle {
public:
    Lepton();
    ~Lepton();

    void SetdXY(float dxy, float dxyErr) { j_dxy = dxy; j_dxyErr = dxyErr;}
    void SetdXY(float dxy) { j_dxy = dxy;}
    inline float dXY() const {return j_dxy;}
    inline float dXYerr() const {return j_dxyErr;}

    void SetdZ(float dz, float dzErr) { j_dz = dz; j_dzErr = dzErr;}
    void SetdZ(float dz) { j_dz = dz;}
    inline float dZ() const {return j_dz;}
    inline float dZerr() const {return j_dzErr;}

    void SetIP3D(float ip3d, float sip3d) { j_ip3d = ip3d; j_sip3d = sip3d;}
    inline float IP3D() const {return j_ip3d;}
    inline float SIP3D() const {return j_sip3d;}

    //==== AbsIso will be set in Muon/Electron,
    //==== and use SetRelIso to save calculated RelIso
    void SetTkRelIso(float r) {j_tkRelIso = r;}
    void SetPfRelIso03(float r) {j_pfRelIso03_all = r;}
    void SetPfRelIso04(float r) {j_pfRelIso04_all = r;}
    void SetMiniPFRelIso(float r) {j_miniPFRelIso_all = r;}
    inline float TkRelIso() const {return j_tkRelIso;}
    inline float PfRelIso03() const {return j_pfRelIso03_all;}
    inline float PfRelIso04() const {return j_pfRelIso04_all;}
    inline float MiniPFRelIso() const {return j_miniPFRelIso_all;}

    // UPDATE: miniIsoDr to ConeSizeMiniIso
    float ConeSizeMiniIso() const;

    // UPDATE: add typedef for readability, no practical change
    enum FLAVOR{NONE, ELECTRON, MUON, TAU};
    typedef FLAVOR Flavour;
    inline Flavour LeptonFlavour() const {return j_LeptonFlavour;}
    void SetLeptonFlavour(Flavour f) {j_LeptonFlavour = f;}
    inline bool IsElectron() const {return j_LeptonFlavour == ELECTRON;}
    inline bool IsMuon() const {return j_LeptonFlavour == MUON;}
    inline bool IsTau() const {return j_LeptonFlavour == TAU;}

    void SetPtCone(float f){j_ptcone = f;}
    inline bool IsPtConeAvailable() const {return j_ptcone > 0;}
    inline float PtCone() const {
        if (!IsPtConeAvailable()) {
            cerr << "[Lepton::PtCone] pt-cone not set" << endl;
            exit(EXIT_FAILURE);
        }
        return j_ptcone;
    }
    // UPDATE: modified to use both relIso and miniIso
    float CalcPtCone(float thisIso, float tightIso);

    virtual void Print();

private:
    float j_dxy, j_dxyErr;
    float j_dz, j_dzErr;
    float j_ip3d, j_sip3d;
    float j_tkRelIso, j_pfRelIso03_all, j_pfRelIso04_all, j_miniPFRelIso_all;
    float j_ptcone;
    Flavour j_LeptonFlavour;

    ClassDef(Lepton,1)
    
};

#endif
