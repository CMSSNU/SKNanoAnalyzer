#ifndef Lepton_h
#define Lepton_h

#include "Particle.h"

class Lepton: public Particle {
public:
    Lepton();
    ~Lepton();

    void SetdXY(float dXY, float dXYerr) { j_dXY = dXY; j_dXYerr = dXYerr;}
    inline float dXY() const {return j_dXY;}
    inline float dXYerr() const {return j_dXYerr;}

    void SetdZ(float dZ, float dZerr) { j_dZ = dZ; j_dZerr = dZerr;}
    inline float dZ() const {return j_dZ;}
    inline float dZerr() const {return j_dZerr;}

    void SetIP3D(float IP3D, double IP3Derr) { j_IP3D = IP3D; j_IP3Derr = IP3Derr;}
    inline float IP3D() const {return j_IP3D;}
    inline float IP3Derr() const {return j_IP3Derr;}

    //==== AbsIso will be set in Muon/Electron,
    //==== and use SetRelIso to save calculated RelIso
    void SetRelIso(float r) {j_RelIso = r;}
    inline float RelIso() const {return j_RelIso;}

    // TODO: MiniIso already calculated in NANOAOD -> need fix
    void SetMiniIso(float ch, float nh, float ph, float pu, float rho, float EA) {
        float r_mini = ConeSizeMiniIso();
        float correction = rho * EA * (r_mini/0.3) * (r_mini/0.3);
        float correctedIso = ch + max(float(0.), nh + ph - correction);
        j_MiniRelIso = correctedIso/this->Pt();
    }
    inline float MiniRelIso() const {return j_MiniRelIso;}
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
    double j_dXY, j_dXYerr;
    double j_dZ, j_dZerr;
    double j_IP3D, j_IP3Derr;

    double j_RelIso, j_MiniRelIso;
    double j_ptcone;
    Flavour j_LeptonFlavour;

    ClassDef(Lepton,1)
    
};

#endif