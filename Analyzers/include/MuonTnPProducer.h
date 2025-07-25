#ifndef MuonTnPProducer_h
#define MuonTnPProducer_h

#include "AnalyzerCore.h"

class MuonTnPProducer : public AnalyzerCore {

public:

  virtual void initializeAnalyzer();
  virtual void executeEvent();
  virtual void WriteHist();
  
  LHE lhe_p0, lhe_p1, lhe_l0, lhe_l1, lhe_j0;
  Gen gen_p0, gen_p1, gen_l0_bare, gen_l1_bare;
  void GetLHEParticles(const RVec<LHE> &lhes, LHE &p0, LHE &p1, LHE &l0, LHE &l1, LHE &j0);
  void GetGenParticles(const RVec<Gen> &gens, Gen &parton0, Gen &parton1, Gen &l0, Gen &l1, int mode);
  bool PassSLT(const Muon &muon, const RVec<TrigObj> &trigObjs, const float pt_cut);
  bool PassDLT(const Muon &muon, const RVec<TrigObj> &trigObjs, const float pt_cut);

  MuonTnPProducer();
  ~MuonTnPProducer();

  TTree* newtree;
  float genWeight;
  float PUweight,PUweight_up,PUweight_down;
  float prefireweight,prefireweight_up,prefireweight_down;
  float zptweight;
  float z0weight;

  bool probe_isTracker;
  bool probe_isGlobal;
  bool probe_isSA;
  bool probe_isSA_unique;
  bool probe_isTight;
  bool probe_isMedium;
  bool probe_isMedium2016a;
  bool probe_isLoose;
  bool probe_TkIsoLoose;
  bool probe_PFIsoTight;
  bool probe_IsoMu24;
  bool probe_IsoMu27;
  bool probe_Mu17Leg1;
  bool probe_Mu8Leg2;

  float probe_pt;
  float probe_pt_cor;
  float probe_eta;
  float probe_phi;
  int probe_q;
  float probe_pfRelIso03;
  float probe_pfRelIso04;
  float probe_tkRelIso;
  float probe_miniPFRelIso;
  float probe_sip3d;
  float probe_dz;
  float probe_dxy;

  bool tag_IsoMu24;
  bool tag_IsoMu27;
  bool tag_isTight;
  bool tag_isMedium;
  bool tag_isMedium2016a;
  bool tag_isLoose;
  bool tag_TkIsoLoose;
  bool tag_PFIsoTight;

  float tag_pt;
  float tag_pt_cor;
  float tag_eta;
  float tag_phi;
  float tag_q;
  float tag_pfRelIso03;
  float tag_pfRelIso04;
  float tag_tkRelIso;
  float tag_miniPFRelIso;
  float tag_sip3d;
  float tag_dz;
  float tag_dxy;

  unsigned int pair_probeMultiplicity;
  float pair_mass;
  float pair_mass_cor;
  float pair_pt;
  float pair_pt_cor;
  bool pair_EMTF;

  bool probe_gen_matched;
  bool tag_gen_matched;
  bool pair_gen_matched;

  float pair_gen_mass;
  float probe_gen_pt;
  float probe_gen_eta;
  float probe_gen_phi;
  float probe_gen_dR;
  float probe_gen_reldpt;
  float tag_gen_pt;
  float tag_gen_eta;
  float tag_gen_phi;
  float tag_gen_dR;
  float tag_gen_reldpt;
};



#endif