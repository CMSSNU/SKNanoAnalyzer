#ifndef FatJet_h
#define FatJet_h

#include "Particle.h"
#include "JetTaggingParameter.h"

class FatJet : public Particle
{

public:
    FatJet();
    ~FatJet();

    // inline void SetJetID(unsigned char b)
    // {
    //     // bit 0 is loose, bit 1 is tight, bit 2 is tightLepVeto
    //     j_looseJetId = (b & 1);
    //     j_tightJetID = (b & 2);
    //     j_tightLepVetoJetID = (b & 4);
    // };
    void SetArea(float area) { j_area = area; };
    inline float Area() const { return j_area; };

    void SetSDMass(float msoftdrop) { j_msoftdrop = msoftdrop; };
    inline float SDMass() const { return j_msoftdrop; };

    void SetRawFactor(float raw) { j_rawFactor = raw; };

    void SetLSF3(float lsf3) { j_lsf3 = lsf3; };
    inline float LSF3() const { return j_lsf3; };

    void SetHadronFlavour(unsigned char hf) { j_hadronFlavour = hf; };
    inline float hadronFlavour() const { return j_hadronFlavour; };

    void SetGenMatchIDs(short gj, short sj1, short sj2)
    {
        j_genJetAK8Idx = gj;
        j_subJetIdx1 = sj1;
        j_subJetIdx2 = sj2;
    };
    inline short GenJetAK8Idx() const { return j_genJetAK8Idx; };
    inline short SubJetIdx1() const { return j_subJetIdx1; };
    inline short SubJetIdx2() const { return j_subJetIdx2; };

    enum Selector
    {
        Loose = 1 << 0,
        Tight = 1 << 1,
        TightLepVeto = 1 << 2
    };
    //inline bool PassSelector(unsigned int s) const { return (j_jetId & s) == s; }
    //TODO JetID need to be set
    inline bool PassSelector(unsigned int s) const { return true; }
    inline bool PassLoose() const { return PassSelector(Loose); }
    inline bool PassTight() const { return PassSelector(Tight); }
    inline bool PassTightLepVeto() const { return PassSelector(TightLepVeto); }

    void SetConstituents(short nNeu, short nCh, unsigned char nConst)
    {
        j_chMultiplicity = nCh;
        j_neMultiplicity = nNeu;
        j_nConstituents = nConst;
    };
    inline unsigned char NConstituents() const { return j_nConstituents; };
    inline unsigned char NCh() const { return j_chMultiplicity; };
    inline unsigned char NNeu() const { return j_neMultiplicity; };

    void SetParticleNetResult(RVec<float> pnet)
    {
        j_particleNetWithMass[JetTagging::FatJetTaggerScoreType::H4qvsQCD] = pnet[0];
        j_particleNetWithMass[JetTagging::FatJetTaggerScoreType::HbbvsQCD] = pnet[1];
        j_particleNetWithMass[JetTagging::FatJetTaggerScoreType::HccvsQCD] = pnet[2];
        j_particleNetWithMass[JetTagging::FatJetTaggerScoreType::QCD] = pnet[3];
        j_particleNetWithMass[JetTagging::FatJetTaggerScoreType::TopvsQCD] = pnet[4];
        j_particleNetWithMass[JetTagging::FatJetTaggerScoreType::WvsQCD] = pnet[5];
        j_particleNetWithMass[JetTagging::FatJetTaggerScoreType::ZvsQCD] = pnet[6];
    };

    void SetParticleNetNoMassResult(RVec<float> pnet_nomass)
    {
        j_particleNetWithMass[JetTagging::FatJetTaggerScoreType::QCD] = pnet_nomass[0];
        j_particleNetWithMass[JetTagging::FatJetTaggerScoreType::QCD0HF] = pnet_nomass[1];
        j_particleNetWithMass[JetTagging::FatJetTaggerScoreType::QCD1HF] = pnet_nomass[2];
        j_particleNetWithMass[JetTagging::FatJetTaggerScoreType::QCD2HF] = pnet_nomass[3];
        j_particleNetWithMass[JetTagging::FatJetTaggerScoreType::WvsQCD] = pnet_nomass[4];
        j_particleNetWithMass[JetTagging::FatJetTaggerScoreType::XbbVsQCD] = pnet_nomass[5];
        j_particleNetWithMass[JetTagging::FatJetTaggerScoreType::XccVsQCD] = pnet_nomass[6];
        j_particleNetWithMass[JetTagging::FatJetTaggerScoreType::XggVsQCD] = pnet_nomass[7];
        j_particleNetWithMass[JetTagging::FatJetTaggerScoreType::XqqVsQCD] = pnet_nomass[8];
        j_particleNetWithMass[JetTagging::FatJetTaggerScoreType::XteVsQCD] = pnet_nomass[9];
        j_particleNetWithMass[JetTagging::FatJetTaggerScoreType::XtmVsQCD] = pnet_nomass[10];
        j_particleNetWithMass[JetTagging::FatJetTaggerScoreType::XttVsQCD] = pnet_nomass[11];
    };

    void SetGloParTResult(RVec<float> gloParT)
    {
        j_globalParT3[JetTagging::FatJetTaggerScoreType::QCD] = gloParT[0];
        j_globalParT3[JetTagging::FatJetTaggerScoreType::TopbWev] = gloParT[1];
        j_globalParT3[JetTagging::FatJetTaggerScoreType::TopbWmv] = gloParT[2];
        j_globalParT3[JetTagging::FatJetTaggerScoreType::TopbWq] = gloParT[3];
        j_globalParT3[JetTagging::FatJetTaggerScoreType::TopbWqq] = gloParT[4];
        j_globalParT3[JetTagging::FatJetTaggerScoreType::TopbWtauhv] = gloParT[5];
        j_globalParT3[JetTagging::FatJetTaggerScoreType::WvsQCD] = gloParT[6];
        j_globalParT3[JetTagging::FatJetTaggerScoreType::XWW3q] = gloParT[7];
        j_globalParT3[JetTagging::FatJetTaggerScoreType::XWW4q] = gloParT[8];
        j_globalParT3[JetTagging::FatJetTaggerScoreType::XWWqqev] = gloParT[9];
        j_globalParT3[JetTagging::FatJetTaggerScoreType::XWWqqmv] = gloParT[10];
        j_globalParT3[JetTagging::FatJetTaggerScoreType::Xbb] = gloParT[11];
        j_globalParT3[JetTagging::FatJetTaggerScoreType::Xcc] = gloParT[12];
        j_globalParT3[JetTagging::FatJetTaggerScoreType::Xcs] = gloParT[13];
        j_globalParT3[JetTagging::FatJetTaggerScoreType::Xqq] = gloParT[14];
        j_globalParT3[JetTagging::FatJetTaggerScoreType::Xtauhtaue] = gloParT[15];
        j_globalParT3[JetTagging::FatJetTaggerScoreType::Xtauhtauh] = gloParT[16];
        j_globalParT3[JetTagging::FatJetTaggerScoreType::Xtauhtaum] = gloParT[17];
        j_globalParT3WithMass[JetTagging::FatJetTaggerScoreType::TopvsQCD] = gloParT[18];
        j_globalParT3WithMass[JetTagging::FatJetTaggerScoreType::WvsQCD] = gloParT[19];
        j_globalParT3WithMass[JetTagging::FatJetTaggerScoreType::ZvsQCD] = gloParT[20];
    };

    void SetSubjettiness(float tau1, float tau2, float tau3, float n2b1, float n3b1)
    {
        j_tau1 = tau1;
        j_tau2 = tau2;
        j_tau3 = tau3;
        j_n2b1 = n2b1;
        j_n3b1 = n3b1;
    };
    inline float Tau1() const { return j_tau1; };
    inline float Tau2() const { return j_tau2; };
    inline float Tau3() const { return j_tau3; };
    inline float N2b1() const { return j_n2b1; };
    inline float N3b1() const { return j_n3b1; };

    void SetEnergyFraction(float chEmEF, float chHEF, float hfEmEF, float hfHEF, float muEF, float neEmEF, float neHEF)
    {
        j_chEmEF = chEmEF;
        j_chHEF = chHEF;
        j_hfEmEF = hfEmEF;
        j_hfHEF = hfHEF;
        j_muEF = muEF;
        j_neEmEF = neEmEF;
        j_neHEF = neHEF;
    };

    bool PassID(TString ID) const;
    float GetTaggerResult(JetTagging::FatJetTagger tagger, JetTagging::FatJetTaggerScoreType scoreType) const;

private:
    // ───── Basic Jet Properties ─────────────
    float j_pt, j_eta, j_phi, j_mass, j_rawFactor, j_area;

    // ───── Energy Fractions ────────────────
    float j_chEmEF, j_chHEF, j_hfEmEF, j_hfHEF, j_muEF, j_neEmEF, j_neHEF;

    // ───── Multiplicities ──────────────────
    short j_chMultiplicity, j_neMultiplicity;
    unsigned char j_nConstituents;

    // ───── Indices and Flavors ─────────────
    short j_genJetAK8Idx, j_subJetIdx1, j_subJetIdx2;
    unsigned char j_hadronFlavour;

    // ───── Jet Substructure ────────────────
    float j_msoftdrop, j_n2b1, j_n3b1, j_lsf3;
    float j_tau1, j_tau2, j_tau3, j_tau4;

    // ───── ParticleNet WithMass ────────────
    float j_particleNetWithMass_H4qvsQCD, j_particleNetWithMass_HbbvsQCD, j_particleNetWithMass_HccvsQCD;
    float j_particleNetWithMass_QCD, j_particleNetWithMass_TvsQCD, j_particleNetWithMass_WvsQCD, j_particleNetWithMass_ZvsQCD;
    unordered_map<JetTagging::FatJetTaggerScoreType, float> j_particleNetWithMass;

    // ───── ParticleNet W/O mass ───────────
    float j_particleNet_QCD, j_particleNet_QCD0HF, j_particleNet_QCD1HF, j_particleNet_QCD2HF;
    float j_particleNet_WVsQCD, j_particleNet_XbbVsQCD, j_particleNet_XccVsQCD;
    float j_particleNet_XggVsQCD, j_particleNet_XqqVsQCD, j_particleNet_XteVsQCD, j_particleNet_XtmVsQCD, j_particleNet_XttVsQCD;
    float j_particleNet_massCorr;
    unordered_map<JetTagging::FatJetTaggerScoreType, float> j_particleNet;

    // ───── GlobalParT3 Taggers ─────────────
    float j_globalParT3_QCD, j_globalParT3_TopbWev, j_globalParT3_TopbWmv, j_globalParT3_TopbWq, j_globalParT3_TopbWqq, j_globalParT3_TopbWtauhv;
    float j_globalParT3_WvsQCD;
    float j_globalParT3_XWW3q, j_globalParT3_XWW4q, j_globalParT3_XWWqqev, j_globalParT3_XWWqqmv;
    float j_globalParT3_Xbb, j_globalParT3_Xcc, j_globalParT3_Xcs, j_globalParT3_Xqq;
    float j_globalParT3_Xtauhtaue, j_globalParT3_Xtauhtauh, j_globalParT3_Xtauhtaum;
    float j_globalParT3_massCorrGeneric, j_globalParT3_massCorrX2p;
    float j_globalParT3_withMassTopvsQCD, j_globalParT3_withMassWvsQCD, j_globalParT3_withMassZvsQCD;
    unordered_map<JetTagging::FatJetTaggerScoreType, float> j_globalParT3;
    unordered_map<JetTagging::FatJetTaggerScoreType, float> j_globalParT3WithMass;

    ClassDef(FatJet, 1)
};

#endif
