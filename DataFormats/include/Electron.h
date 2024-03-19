#ifndef Electron_h
#define Electron_h

#include "TString.h"
#include "Lepton.h"

// Need update
// Missing variables (compared to SKFlat)
// energy / resolution corrections & errors
// ID variables: j_dEtaSeed, j_dPhiIn, j_e2x5OverE5x5, j_e1x5OverE5x5, j_calPFClusterIso, j_hcalPFClusterIso, j_dr03HcalTowerSumEt
// SuperCluster: phi, E
// IDs: j_mvaIso_Loose, j_mvaNoIso_Loose
// others: j_ea, j_rho

class Electron : public Lepton {
public:
    Electron();
    ~Electron();
    
    void SetConvVeto(bool convVeto) { j_convVeto = convVeto; }
    inline bool ConvVeto() const { return j_convVeto; }

    void SetLostHits(unsigned char lostHits) { j_lostHits = lostHits; }
    inline unsigned char LostHits() const { return j_lostHits; }

    void SetSeedGain(unsigned char seedGain) { j_seedGain = seedGain; }
    inline unsigned char SeedGain() const { return j_seedGain; }

    void SetTightCharge(unsigned char tightCharge) { j_tightCharge = tightCharge; }
    inline unsigned char TightCharge() const { return j_tightCharge; } // 0: None, 1: isGsfScPixChargeConsistent, 2: isGsfCtfScPixChargeConsistent

    void SetSieie(float sieie) { j_sieie = sieie; }
    inline float sieie() const { return j_sieie; }

    void SetHoe(float hoe) { j_hoe = hoe; }
    inline float hoe() const { return j_hoe; }

    void SetEInvMinusPInv(float eInvMinusPInv) { j_eInvMinusPInv = eInvMinusPInv; }
    inline float eInvMinusPInv() const { return j_eInvMinusPInv; }

    void SetDr03EcalRecHitSumEt(float dr03EcalRecHitSumEt) { j_dr03EcalRecHitSumEt = dr03EcalRecHitSumEt; }
    inline float dr03EcalRecHitSumEt() const { return j_dr03EcalRecHitSumEt; }

    void SetDr03HcalDepth1TowerSumEt(float dr03HcalDepth1TowerSumEt) { j_dr03HcalDepth1TowerSumEt = dr03HcalDepth1TowerSumEt; }
    inline float dr03HcalDepth1TowerSumEt() const { return j_dr03HcalDepth1TowerSumEt; }

    void SetDr03TkSumPt(float dr03TkSumPt) { j_dr03TkSumPt = dr03TkSumPt; }
    inline float dr03TkSumPt() const { return j_dr03TkSumPt; }

    void SetDr03TkSumPtHEEP(float dr03TkSumPtHEEP) { j_dr03TkSumPtHEEP = dr03TkSumPtHEEP; }
    inline float dr03TkSumPtHEEP() const { return j_dr03TkSumPtHEEP; }

    void SetR9(float r9) { j_r9 = r9; }
    inline float r9() const { return j_r9; }

    // Boolean IDs
    enum class BooleanID {NONE, MVAISOWP80, MVAISOWP90, MVANOISOWP80, MVANOISOWP90, CUTBASEDHEEP};
    void SetBIDBit(BooleanID id, bool idbit);
    inline bool isMVAIsoWP80() const { return j_mvaIso_WP80; }
    inline bool isMVAIsoWP90() const { return j_mvaIso_WP90; }
    inline bool isMVANoIsoWP80() const { return j_mvaNoIso_WP80; }
    inline bool isMVANoIsoWP90() const { return j_mvaNoIso_WP90; }
    inline bool isCutBasedHEEP() const { return j_cutBased_HEEP; }


    // cut-based ID
    enum class CutBasedID {NONE, CUTBASED};
    enum class WORKINGPOINT {NONE, VETO, LOOSE, MEDIUM, TIGHT};
    void SetCBIDBit(CutBasedID id, bool value);
    inline WORKINGPOINT CutBased() const {return (WORKINGPOINT)j_cutBased; }

    // MVA scores
    enum class MVATYPE {NONE, MVAISO, MVANOISO, MVATTH};
    void SetMVA(MVATYPE type, float score);
    inline float MvaIso() const { return j_mvaIso; }
    inline float MvaNoIso() const { return j_mvaNoIso; }
    inline float MvaTTH() const { return j_mvaTTH; }

private:
    // uncertainties
    float j_energyErr; // resErr?
    // ID variables
    bool j_convVeto;
    unsigned char j_lostHits, j_seedGain, j_tightCharge;
    float j_sieie, j_hoe, j_eInvMinusPInv; // no j_dEtaSeed, j_dPhiIn
    float j_dr03EcalRecHitSumEt, j_dr03HcalDepth1TowerSumEt, j_dr03TkSumPt, j_dr03TkSumPtHEEP; // no j_e2x5OverE5x5, j_e1x5OverE5x5, j_calPFClusterIso, j_hcalPFClusterIso, j_dr03HcalTowerSumEt

    // SuperCluster
    float j_deltaEtaSC; // missing phi and E?

    // IDs
    bool j_mvaIso_WP80, j_mvaIso_WP90, j_mvaNoIso_WP80, j_mvaNoIso_WP90, j_cutBased_HEEP;
    unsigned char j_cutBased;
    
    float j_mvaIso, j_mvaNoIso, j_mvaTTH;

    // others
    float j_r9;
    ClassDef(Electron, 1);
};

#endif

