#ifndef FatJet_h
#define FatJet_h

#include "Particle.h"

class FatJet : public Particle {

public:

    FatJet();
    ~FatJet();

    void SetArea(float area) { j_area = area; };
    inline float Area() const { return j_area; };

    void SetSDMass(float msoftdrop) { j_msoftdrop = msoftdrop; };
    inline float SDMass() const { return j_msoftdrop; };

    void SetLSF3(float lsf3) { j_lsf3 = lsf3; };
    inline float LSF3() const { return j_lsf3; };

    void SetGenMatchIDs(short gj, short sj1, short sj2) {
        j_genJetAK8Idx = gj; j_subJetIdx1 = sj1; j_subJetIdx2 = sj2;
    };
    inline short GenJetAK8Idx() const { return j_genJetAK8Idx; };
    inline short SubJetIdx1() const { return j_subJetIdx1; };
    inline short SubJetIdx2() const { return j_subJetIdx2; };

    enum Selector {
        Loose        = 1 << 0,
        Tight        = 1 << 1,
        TightLepVeto = 1 << 2
    };
    inline bool PassSelector( unsigned int s ) const { return (j_jetId & s)==s; }
    inline bool PassLoose() const { return PassSelector(Loose); }
    inline bool PassTight() const { return PassSelector(Tight); }
    inline bool PassTightLepVeto() const { return PassSelector(TightLepVeto); }

    void SetConstituents(unsigned char nB, unsigned char nC, unsigned char nConst) {
        j_nBHadrons = nB; j_nCHadrons = nC; j_nConstituents = nConst;
    };
    inline unsigned char NConstituents() const { return j_nConstituents; };
    inline unsigned char NBHadrons() const { return j_nBHadrons; };
    inline unsigned char NCHadrons() const { return j_nCHadrons; };

    void SetBTaggingInfo(float ddbv2, float ddcv2, float ddclv2, float db, float hbb) {
        j_btagDDBvLV2 = ddbv2; j_btagDDCvBV2 = ddcv2; j_btagDDCvLV2 = ddclv2; j_btagDeepB = db; j_btagHbb = hbb;
    };

    void SetPNetwithMassResults(RVec<float> ds){
        j_particleNetWithMass_H4qvsQCD = ds[0];
        j_particleNetWithMass_HccvsQCD = ds[1];
        j_particleNetWithMass_HbbvsQCD = ds[2];
        j_particleNetWithMass_QCD      = ds[3];
        j_particleNetWithMass_TvsQCD   = ds[4];
        j_particleNetWithMass_WvsQCD   = ds[5];
        j_particleNetWithMass_ZvsQCD   = ds[6];
    };
    
    void SetPNetResults(RVec<float> ds){
        j_particleNet_QCD      = ds[0];
        j_particleNet_QCD0HF   = ds[1];
        j_particleNet_QCD1HF   = ds[2];
        j_particleNet_QCD2HF   = ds[3];
        j_particleNet_XbbVsQCD = ds[4];
        j_particleNet_XccVsQCD = ds[5];
        j_particleNet_XqqVsQCD = ds[6];
        j_particleNet_XggVsQCD = ds[7];
        j_particleNet_XteVsQCD = ds[8];
        j_particleNet_XtmVsQCD = ds[9];
        j_particleNet_XttVsQCD = ds[10];
        j_particleNet_massCorr = ds[11];
    }

    void SetSubjettiness(float tau1, float tau2, float tau3, float tau4) {
        j_tau1 = tau1; j_tau2 = tau2; j_tau3 = tau3; j_tau4 = tau4;
    };

    bool PassID(TString ID) const;
    // TODO  double GetTaggerResult(JetTagging::Tagger tg) const;

private:

    float j_msoftdrop;
    float j_area;
    float j_lsf3;
    short j_genJetAK8Idx, j_subJetIdx1, j_subJetIdx2;
    unsigned char j_jetId;
    unsigned char j_nBHadrons, j_nCHadrons, j_nConstituents;
    float j_btagDDBvLV2, j_btagDDCvBV2, j_btagDDCvLV2, j_btagDeepB, j_btagHbb;
    float j_particleNetWithMass_H4qvsQCD;
    float j_particleNetWithMass_HccvsQCD;
    float j_particleNetWithMass_HbbvsQCD;
    float j_particleNetWithMass_QCD;
    float j_particleNetWithMass_TvsQCD;
    float j_particleNetWithMass_WvsQCD;
    float j_particleNetWithMass_ZvsQCD;
    float j_particleNet_QCD, j_particleNet_QCD0HF, j_particleNet_QCD1HF, j_particleNet_QCD2HF;
    float j_particleNet_XbbVsQCD, j_particleNet_XccVsQCD, j_particleNet_XqqVsQCD, j_particleNet_XggVsQCD, j_particleNet_XteVsQCD;
    float j_particleNet_XtmVsQCD, j_particleNet_XttVsQCD, j_particleNet_massCorr;
    float j_tau1, j_tau2, j_tau3, j_tau4;
    ClassDef(FatJet, 1)

};

#endif
