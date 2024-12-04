#ifndef TKinFitterDriver_h
#define TKinFitterDriver_h

#include "TString.h"
#include <iostream>
#include <numeric>

#include "TFitConstraintM.h"
#include "TFitConstraintM2Gaus.h"
#include "TFitConstraintMGaus.h"
#include "TAbsFitParticle.h"
#include "TFitParticlePt.h"
//#include "TFitParticlePz.h"
#include "TFitParticleMCCart.h"
#include "TFitParticleEtPhi.h"
#include "TKinFitter.h"
#include "TSCorrection.h"


using namespace std;

class TKinFitterDriver{

public:

  struct ResultContainer;

  TKinFitterDriver();
  TKinFitterDriver(int DataYear_);
  ~TKinFitterDriver();

  int DataYear;
  void SetDataYear(int i);

  void SetAllObjects(std::vector<TLorentzVector> jet_vector_,
                     std::vector<bool> btag_vector_,
                     TLorentzVector lepton_,
                     TLorentzVector met_);
  void SetAllObjects(std::vector<TLorentzVector> jet_vector_,
                     std::vector<double> btag_csv_vector_,
                     double btag_cut_,
                     TLorentzVector lepton_,
                     TLorentzVector met_);
  void SetHadronicTopBJets(TLorentzVector jet_); // it doesn't check tagging status
  void SetLeptonicTopBJets(TLorentzVector jet_); // it doesn't check tagging status
  void SetWCHUpTypeJets(TLorentzVector jet_); // u/c jet from W(H+)
  void SetWCHDownTypeJets(TLorentzVector jet_); // d/s/b jet from W(H+)
  void SetHadronicTopBJets(TLorentzVector jet_, double resolution_); // it doesn't check tagging status
  void SetLeptonicTopBJets(TLorentzVector jet_, double resolution_); // it doesn't check tagging status
  void SetWCHUpTypeJets(TLorentzVector jet_, double resolution_); // u/c jet from W(H+)
  void SetWCHDownTypeJets(TLorentzVector jet_, double resolution_); // d/s/b jet from W(H+)
  void SetJetPtResolution(std::vector<float> jetPtResolution_);
  void SetLepton(TLorentzVector lepton_);
  void SetMET(TLorentzVector met_);
  void SetMETShift(double met_pt_up, double met_pt_down, double met_phi_up, double met_phi_down);
  void SetMETShift(double met_shiftX, double met_shiftY);
  void SetNeutrino(TLorentzVector met_,int i); // i is related to neu. Pz
  void SetNeutrinoSmallerPz(TLorentzVector met_);

  void Fit();
  void FitCurrentPermutation();
  void FindBestChi2Fit(bool UseLeading4Jets=false, bool IsHighMassFitter=false);
  void FindBestSelTopFit(bool IsMaxHadTopPt, bool IsClosestHadTopM, bool IsMaxLepTopPt, bool IsClosestLepTopM, bool noAmbiguity);

  int GetStatus();
  double GetChi2();
  int GetBestDownTypeJetBTagged();
  double GetFittedDijetMass();
  double GetInitialDijetMass();
  double GetCorrectedDijetMass();

  int GetBestStatus();
  double GetBestChi2();
  double GetBestLambda();
  double GetBestFittedDijetMass();
  double GetBestFittedDijetMass_high();
  double GetBestInitialDijetMass();
  double GetBestInitialDijetMass_high();
  double GetBestCorrectedDijetMass();
  double GetBestCorrectedDijetMass_high();

  double GetBestHadronicTopMass();
  double GetBestHadronicTopPt();
  double GetBestLeptonicTopMass();
  double GetBestLeptonicWMass();
  bool GetBestIsRealNeuPz();

  int GetBestHadronicTopBJetIdx();
  int GetBestLeptonicTopBJetIdx();
  int GetBestHadronicWCHUpTypeJetIdx();
  int GetBestHadronicWCHDownTypeJetIdx();

  double GetBestHadronicTopBJetPull();
  double GetBestLeptonicTopBJetPull();
  double GetBestHadronicWCHUptypeJetIdxPull();
  double GetBestHadronicWCHDowntypeJetIdxPull();

  double GetBestHadronicTopMassF();
  double GetBestLeptonicTopMassF();
  double GetBestLeptonicWMassF();
  double GetBestDeltaS();

  std::vector<double> GetHadronicTopMassVector(bool IsConverge=true);
  std::vector<double> GetHadronicTopBPtVector(bool IsConverge=true);
  std::vector<double> GetLeptonicTopBPtVector(bool IsConverge=true);
  std::vector<double> GetWCHDownTypePtVector(bool IsConverge=true);
  std::vector<double> GetWCHUpTypePtVector(bool IsConverge=true);

  const std::vector<TKinFitterDriver::ResultContainer>* GetResults();

  enum JET_ASSIGNMENT{
    HADRONIC_TOP_B,
    LEPTONIC_TOP_B,
    W_CH_UP_TYPE,
    W_CH_DOWN_TYPE,
    NONE
  };

  struct ResultContainer{
    ResultContainer(){}
    ~ResultContainer(){}
    int status; //fitter status
    double fitted_dijet_M;
    double initial_dijet_M;
    double corrected_dijet_M;

    double fitted_dijet_M_high;
    double initial_dijet_M_high;
    double corrected_dijet_M_high;

    double fitted_dijet_M_new1;
    double fitted_dijet_M_new2;

    double hadronic_top_M;
    double hadronic_top_pt;
    double leptonic_top_M;
    double leptonic_top_pt;
    double leptonic_W_M;
    bool IsRealNeuPz;

    double hadronic_top_b_pt;
    double leptonic_top_b_pt;
    double w_ch_up_type_pt;
    double w_ch_down_type_pt;

    //idx
    int hadronic_top_b_jet_idx;
    int leptonic_top_b_jet_idx;
    int w_ch_up_type_jet_idx;
    int w_ch_down_type_jet_idx;

    //pull
    double hadronic_top_b_jet_pull;
    double leptonic_top_b_jet_pull;
    double w_ch_up_type_jet_pull;
    double w_ch_down_type_jet_pull;

    //tagging
    int down_type_jet_b_tagged;

    // F from constraints
    double hadronic_top_mass_F;
    double leptonic_top_mass_F;
    double leptonic_w_mass_F;
    double currS;
    double deltaS;
    double chi2;
    double chi2_lep;
    double chi2_had;
    double lambda;
  };

private:

  void SetJetError(TMatrixD *matrix,  double Pt, double Eta, double Phi, TString flavour_key);
  void SetUnclError(TMatrixD *matrix, TLorentzVector &met);
  double JetErrorPt(double Pt, double Eta, TString flavour_key);
  double JetErrorEta(double Pt, double Eta, TString flavour_key);
  double JetErrorPhi(double Pt, double Eta, TString flavour_key);

  void SetConstraint();
  void SetFitter();
  void SaveResults();

  double ComparingInHadTopRestFrame(const TLorentzVector *jet1,const TLorentzVector *jet2, const TLorentzVector *had_top_b_jet);

  double CalcChi2(TString option);
  double CalcEachChi2(TAbsFitParticle* ptr);
  double CalcPull(TAbsFitParticle* ptr);
  double CalcEachChi2(TFitConstraintM* ptr, double width);
  double CalcEachChi2(TFitConstraintMGaus* ptr);

  void SetCurrentPermutationJets();
  bool Check_BJet_Assignment();
  bool Kinematic_Cut();
  bool Quality_Cut();
  bool NextPermutation(bool UseLeading4Jets=false);

  void Sol_Neutrino_Pz();
  void Resol_Neutrino_Pt();
  double neutrino_pz_sol[2];
  bool IsRealNeuPz;

  TKinFitter *fitter;
  TSCorrection *ts_correction;

  std::vector<TLorentzVector> jet_vector;
  std::vector<double> jet_pt_resolution_vector;
  std::vector<bool> btag_vector;
  std::vector<double> btag_csv_vector;
  TLorentzVector METv;
  double MET_pt_shift;
  double MET_phi_shift;
  TLorentzVector recal_METv;

  int njets;
  int nbtags;
  std::vector<TKinFitterDriver::JET_ASSIGNMENT> permutation_vector;

  TLorentzVector hadronic_top_b_jet; // b jet comes from hadronic top 
  TLorentzVector leptonic_top_b_jet; // b jet comes from leptonic top
  TLorentzVector hadronic_w_ch_jet1; // u/c jet comes from W(H+)
  TLorentzVector hadronic_w_ch_jet2; // d/s/b jet comes from W(H+)
  TLorentzVector corr_hadronic_top_b_jet; // applied TS Correction
  TLorentzVector corr_leptonic_top_b_jet; 
  TLorentzVector corr_hadronic_w_ch_jet1;
  TLorentzVector corr_hadronic_w_ch_jet2;
  TLorentzVector corr_extra_jet;
  TLorentzVector lepton; // lepton comes from leptonic W
  TLorentzVector neutrino_pxpypz; // neutrino_pxpypz comes from leptonic W
  TVector3       neutrino_vec3; 
  TLorentzVector neutrino_pz; // neutrino_pz

  TFitParticlePt *fit_hadronic_top_b_jet;
  TFitParticlePt *fit_leptonic_top_b_jet;
  TFitParticlePt *fit_hadronic_w_ch_jet1;
  TFitParticlePt *fit_hadronic_w_ch_jet2;
  TFitParticlePt *fit_extra_jet;
  //std::vector<TFitParticlePt*> fit_extra_jets;
  TFitParticlePt *fit_lepton;
  //TFitParticleEtPhi *fit_neutrino_etphi;
  TFitParticleMCCart *fit_neutrino_pxpypz;
  //TFitParticlePz *fit_neutrino_pz;

  int hadronic_top_b_jet_idx;
  int leptonic_top_b_jet_idx;
  int w_ch_up_type_jet_idx;
  int w_ch_down_type_jet_idx;

  TMatrixD error_hadronic_top_b_jet; 
  TMatrixD error_leptonic_top_b_jet;
  TMatrixD error_hadronic_w_ch_jet1;
  TMatrixD error_hadronic_w_ch_jet2;
  TMatrixD error_lepton;
  TMatrixD error_neutrino_pxpypz;

  TFitConstraintM *constrain_hadronic_top_M;
  //TFitConstraintMGaus *constrain_hadronic_top_MGaus;
  TFitConstraintM *constrain_leptonic_top_M;
  //TFitConstraintMGaus *constrain_leptonic_top_MGaus;
  TFitConstraintM *constrain_leptonic_W_M;
  //TFitConstraintMGaus *constrain_leptonic_W_MGaus;

  TKinFitterDriver::ResultContainer fit_result;

  std::vector<TKinFitterDriver::ResultContainer> fit_result_vector;
  static bool Chi2Comparing(const TKinFitterDriver::ResultContainer& rc1, const TKinFitterDriver::ResultContainer& rc2);
  static bool HadTopMComparing(const TKinFitterDriver::ResultContainer& rc1, const TKinFitterDriver::ResultContainer& rc2);
  static bool HighMassFitter(const TKinFitterDriver::ResultContainer& rc1, const TKinFitterDriver::ResultContainer& rc2);
  static bool HadTopPtComparing(const TKinFitterDriver::ResultContainer& rc1, const TKinFitterDriver::ResultContainer& rc2);
  static bool LepTopPtComparing(const TKinFitterDriver::ResultContainer& rc1, const TKinFitterDriver::ResultContainer& rc2);
  static bool TopPtComparing(const TKinFitterDriver::ResultContainer& rc1, const TKinFitterDriver::ResultContainer& rc2);

};

#endif
