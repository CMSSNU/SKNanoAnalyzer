#ifndef Jet_h
#define Jet_h

#include "Particle.h"
#include "JetTaggingParameter.h"

class Jet : public Particle
{
public:
  Jet();
  ~Jet();

  enum class JetID
  {
    NOCUT,
    LOOSE,
    TIGHT,
    TIGHTLEPVETO,
    PUID_LOOSE,
    PUID_MEDIUM,
    PUID_TIGHT,
  };
  // setting functions
  inline void SetArea(double area) { j_area = area; };
  inline void SetJetFlavours(short pf, unsigned char hf)
  {
    j_partonFlavour = static_cast<short>(pf);
    j_hadronFlavour = static_cast<short>(hf);
  };

  inline void SetTaggerResults(RVec<float> ds)
  {
    //check nan. change to -1
    for (auto &d : ds)
    {
      if (std::isnan(d))
        d = -1;
    }
    j_btagDeepFlavB = ds[0];
    j_btagDeepFlavCvB = ds[1];
    j_btagDeepFlavCvL = ds[2];
    j_btagDeepFlavQG = ds[3];
    j_btagPNetB = ds[4];
    j_btagPNetCvB = ds[5];
    j_btagPNetCvL = ds[6];
    j_btagPNetQvG = ds[7];
    j_btagPNetTauVJet = ds[8];
    j_btagRobustParTAK4B = ds[9];
    j_btagRobustParTAK4CvB = ds[10];
    j_btagRobustParTAK4CvL = ds[11];
    j_btagRobustParTAK4QG = ds[12];
  };
  inline void SetEnergyFractions(float cH, float nH, float nEM, float cEM, float muE)
  {
    j_chHEF = cH;
    j_neHEF = nH;
    j_neEmEF = nEM;
    j_chEmEF = cEM;
    j_muEF = muE;
  };
  inline float chHEF() const { return j_chHEF; }
  inline float neHEF() const { return j_neHEF; }
  inline float neEmEF() const { return j_neEmEF; }
  inline float chEmEF() const { return j_chEmEF; }
  inline float muEF() const { return j_muEF; }

  inline void SetMultiplicities(unsigned char nC, unsigned char nEl, unsigned char nM, unsigned char nSV) {
    // typecast before assign
    j_nConstituents = short(nC);
    j_nElectrons = short(nEl);
    j_nMuons = short(nM);
    j_nSVs = short(nSV);
  };
  inline short nConstituents() const { return j_nConstituents; }
  inline short nElectrons() const { return j_nElectrons; }
  inline short nMuons() const { return j_nMuons; }
  inline short nSVs() const { return j_nSVs; }

  // For NanoAODv13
  inline void SetHadronMultiplicities(unsigned char chMult, unsigned char neMult){
    j_chMultiplicity = chMult;
    j_neMultiplicity = neMult;
  };
  inline unsigned char chMultiplicity() const { return j_chMultiplicity; }
  inline unsigned char neMultiplicity() const { return j_neMultiplicity; }

  inline void SetMatchingIndices(short e1, short e2, short m1, short m2, short sv1, short sv2, short gj)
  {
    j_electronIdx1 = e1;
    j_electronIdx2 = e2;
    j_muonIdx1 = m1;
    j_muonIdx2 = m2;
    j_svIdx1 = sv1;
    j_svIdx2 = sv2;
    j_genJetIdx = gj;
  };

  inline void SetMatchingIndices(short e1, short e2, short m1, short m2, short sv1, short sv2)
  {
    j_electronIdx1 = e1;
    j_electronIdx2 = e2;
    j_muonIdx1 = m1;
    j_muonIdx2 = m2;
    j_svIdx1 = sv1;
    j_svIdx2 = sv2;
  };

  void SetJetID(unsigned char IDBit, int Run);
  inline void SetOriginalIndex(int idx) { j_originalIndex = idx; };
  inline int OriginalIndex() const { return j_originalIndex; };

  inline void SetJetPuID(int puIDBit) {
    j_loosePuId = (puIDBit & 1);
    j_mediumPuId = (puIDBit & 2);
    j_tightPuId = (puIDBit & 4);
  };

  inline void SetCorrections(RVec<float> corrs) {
    j_PNetRegPtRawCorr = corrs[0];
    j_PNetRegPtRawCorrNeutrino = corrs[1];
    j_PNetRegPtRawRes = corrs[2];
    j_rawFactor = corrs[3];
    j_bRegCorr = corrs[4];
    j_bRegRes = corrs[5];
    j_cRegCorr = corrs[6];
    j_cRegRes = corrs[7];
  };

  inline void SetM(double jet_m) {
    j_m = jet_m;
  };
  
  inline void SetUnsmearedP4(Jet jet) {
    j_unsmearedP4 = jet;
  };
  inline double GetM() { return j_m; }
  inline int partonFlavour() const { return j_partonFlavour; };
  inline int hadronFlavour() const { return j_hadronFlavour; };
  inline int genJetIdx() const { return j_genJetIdx; };
  inline bool Pass_tightJetID() const { return j_tightJetID; }
  inline bool Pass_tightLepVetoJetID() const { return j_tightLepVetoJetID; }

  float GetBTaggerResult(JetTagging::JetFlavTagger tagger) const;
  pair<float,float> GetCTaggerResult(JetTagging::JetFlavTagger tagger) const;
  float GetQvGTaggerResult(JetTagging::JetFlavTagger tagger) const;
  TLorentzVector GetUnsmearedP4() const;

  bool PassID(TString ID) const;
  bool PassID(JetID id) const; 
private:
  // For matching indices in leptons
  int j_originalIndex;  
  // jetID
  bool j_looseJetId;
  bool j_tightJetID;
  bool j_tightLepVetoJetID;
  // jetPuID
  bool j_loosePuId;
  bool j_mediumPuId;
  bool j_tightPuId;
  // corrections
  float j_PNetRegPtRawCorr;
  float j_PNetRegPtRawCorrNeutrino;
  float j_PNetRegPtRawRes;
  float j_rawFactor;
  float j_bRegCorr;
  float j_bRegRes;
  float j_cRegCorr;
  float j_cRegRes;
  // flav. tagging scores
  float j_btagDeepFlavB;
  float j_btagDeepFlavCvB;
  float j_btagDeepFlavCvL;
  float j_btagDeepFlavQG;
  float j_btagPNetB;
  float j_btagPNetCvB;
  float j_btagPNetCvL;
  float j_btagPNetQvG;
  float j_btagPNetTauVJet;
  float j_btagRobustParTAK4B;
  float j_btagRobustParTAK4CvB;
  float j_btagRobustParTAK4CvL;
  float j_btagRobustParTAK4QG;
  // jet substructure
  float j_chEmEF;
  float j_chHEF;
  float j_neEmEF;
  float j_neHEF;
  float j_muEF;
  // float j_muonSubtrFactor;
  short j_nConstituents;
  short j_nElectrons;
  short j_nMuons;
  short j_nSVs;
  short j_chMultiplicity;
  short j_neMultiplicity;

  // matching information
  short j_electronIdx1;
  short j_electronIdx2;
  short j_muonIdx1;
  short j_muonIdx2;
  short j_svIdx1;
  short j_svIdx2;
  short j_genJetIdx;
  // jet Flav.
  short j_hadronFlavour;
  short j_partonFlavour;
  // others
  float j_area;
  // int j_hfadjacentEtaStripsSize;
  // int j_hfcentralEtaStripSize;
  // float j_hfsigmaEtaEta;
  // float j_hfsigmaPhiPhi;
  float j_m; // jet mass
  TLorentzVector j_unsmearedP4;
  ClassDef(Jet, 1)
};

#endif
