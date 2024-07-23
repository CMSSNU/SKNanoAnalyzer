#ifndef Photon_h
#define Photon_h

#include "Particle.h"
#include <cmath>
class Photon: public Particle {
public:

    Photon();
    ~Photon();
  
    enum class ETAREGION {IB, OB, GAP, EC};
    inline ETAREGION etaRegion() const {
        if (fabs(scEta()) < 0.8)        return ETAREGION::IB;
        else if (fabs(scEta()) < 1.444) return ETAREGION::OB;
        else if (fabs(scEta()) < 1.566) return ETAREGION::GAP;
        else return ETAREGION::EC;
    }

    inline float scEta() const { return j_scEta; }


    void SetEnergy(float energy) { j_energy = energy; }
    inline float energy() const { return j_energy; }
    void SetSigmaIetaIeta(float sieie) { j_sieie = sieie; }
    inline float sieie() const { return j_sieie; }
    void SetHoe(float hoe) { j_hoe = hoe; }
    inline float hoe() const { return j_hoe; }
    void SetEnergyRaw(float energyRaw) { j_energyRaw = energyRaw; }
    inline float energyRaw() const { return j_energyRaw; }
    void SetPixelSeed(bool pixelSeed ) { j_pixelSeed = pixelSeed; }
    inline bool pixelSeed() const { return j_pixelSeed; }

    void SetisScEtaEB(bool isScEtaEB ) { j_isScEtaEB = isScEtaEB; }
    inline bool isScEtaEB() const { return j_isScEtaEB; }
    void SetisScEtaEE(bool isScEtaEE ) { j_isScEtaEE = isScEtaEE; }
    inline bool isScEtaEE() const { return j_isScEtaEE; }

    void SetSCEta(float eta, float phi, float PV_x, float PV_y, float PV_z,bool isScEtaEB, bool isScEtaEE); 


    // Boolean IDs
    enum class BooleanID {NONE, MVAIDWP80, MVAIDWP90};
    void SetBIDBit(BooleanID id, bool idbit);
    inline bool isMVAIDWP80() const { return j_mvaID_WP80; }
    inline bool isMVAIDWP90() const { return j_mvaID_WP90; }


    // cut-based ID
    enum class CutBasedID {NONE, CUTBASED};
    enum class WORKINGPOINT {NONE, VETO, LOOSE, MEDIUM, TIGHT};
    void SetCBIDBit(CutBasedID id, unsigned int value);
    inline WORKINGPOINT CutBased() const {return (WORKINGPOINT)j_cutBased; }
    bool PassID(const TString ID) const;

    // MVA scores
    enum class MVATYPE {NONE, MVAID};
    void SetMVA(MVATYPE type, float score);
    inline float MvaID() const { return j_mvaID; }

    // ID helper functions

private:
    float j_scEta;
    float j_energyRaw;
    float j_sieie;
    float j_hoe;
    unsigned char j_cutBased;
    bool j_mvaID_WP80;
    bool j_mvaID_WP90;
    float j_mvaID;
    float j_energy;
    float j_pixelSeed;
    bool j_isScEtaEB;
    bool j_isScEtaEE;
    ClassDef(Photon,1)

};

#endif
