#ifndef Jet_h
#define Jet_h

#include "Particle.h"
// #include "JetTaggingParameters.h"

class Jet : public Particle
{
public:
  Jet();
  ~Jet();
  // setting functions
  inline void SetArea(double area) { j_area = area; };
  inline void SetGenFlavours(int pf, int hf)
  {
    j_partonFlavour = short(pf);
    j_hadronFlavour = short(hf);
  };

  inline void SetTaggerResults(RVec<float> ds)
  {
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
  inline void SetMultiplicities(unsigned char nC, unsigned char nEl, unsigned char nM, unsigned char nSV)
  {
    // typecast before assign
    j_nConstituents = short(nC);
    j_nElectrons = short(nEl);
    j_nMuons = short(nM);
    j_nSVs = short(nSV);
  };

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

  inline void SetJetID(unsigned char b)
  {
    // bit 0 is loose, bit 1 is tight, bit 2 is tightLepVeto
    j_looseJetId = (b & 1);
    j_tightJetID = (b & 2);
    j_tightLepVetoJetID = (b & 4);
  };

  inline void SetCorrections(RVec<float> corrs)
  {
    j_PNetRegPtRawCorr = corrs[0];
    j_PNetRegPtRawCorrNeutrino = corrs[1];
    j_PNetRegPtRawRes = corrs[2];
    j_rawFactor = corrs[3];
  };

  inline void SetM(double jet_m)
  {
    j_m = jet_m;
  };
  inline double GetM() { return j_m; }
  inline int partonFlavour() const { return j_partonFlavour; };
  inline int hadronFlavour() const { return j_hadronFlavour; };
  inline bool Pass_tightJetID() const { return j_tightJetID; }
  inline bool Pass_tightLepVetoJetID() const { return j_tightLepVetoJetID; }

  bool PassID(TString ID) const;

  // double GetTaggerResult(JetTagging::Tagger tg) const;

private:
  // jetID
  bool j_looseJetId;
  bool j_tightJetID;
  bool j_tightLepVetoJetID;
  // corrections
  float j_PNetRegPtRawCorr;
  float j_PNetRegPtRawCorrNeutrino;
  float j_PNetRegPtRawRes;
  float j_rawFactor;
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

  ClassDef(Jet, 1)
};

#endif
