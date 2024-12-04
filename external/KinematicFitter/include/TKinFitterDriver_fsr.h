#ifndef TKinFitterDriver_h
#define TKinFitterDriver_h

#include "TString.h"
#include <iostream>
#include <numeric>

#include "TMath.h"

#include "TFitConstraintM.h"
#include "TFitConstraintMGaus.h"
#include "TFitConstraintEp.h"
#include "TAbsFitParticle.h"
#include "TFitParticlePt.h"
#include "TFitParticleEtEtaPhi.h"
#include "TFitParticleMCCart.h"
#include "TKinFitter.h"
#include "TSCorrection.h"


using namespace std;

class TKinFitterDriver{

public:

  struct ResultContainer;

  TKinFitterDriver();
  TKinFitterDriver(Int_t DataYear_);
  ~TKinFitterDriver();

  Int_t DataYear;
  void SetDataYear(Int_t i);

  void SetAllObjects(std::vector<TLorentzVector> jet_vector_,
                     std::vector<bool> btag_vector_,
                     TLorentzVector lepton_,
                     TLorentzVector met_);
  void SetAllObjects(std::vector<TLorentzVector> jet_vector_,
                     std::vector<Double_t> btag_csv_vector_,
                     Double_t btag_cut_,
                     TLorentzVector lepton_,
                     TLorentzVector met_);
  void SetHadronicTopBJets(TLorentzVector jet_); // it doesn't check tagging status
  void SetLeptonicTopBJets(TLorentzVector jet_); // it doesn't check tagging status
  void SetWCHUpTypeJets(TLorentzVector jet_); // u/c jet from W(H+)
  void SetWCHDownTypeJets(TLorentzVector jet_); // d/s/b jet from W(H+)
  void SetHadronicTopBJets(Int_t jet_, std::vector<Int_t> fsr_jet_); // it doesn't check tagging status
  void SetLeptonicTopBJets(Int_t jet_, std::vector<Int_t> fsr_jet_); // it doesn't check tagging status
  void SetWCHUpTypeJets(   Int_t jet_, std::vector<Int_t> fsr_jet_); // u/c jet from W(H+)
  void SetWCHDownTypeJets( Int_t jet_, std::vector<Int_t> fsr_jet_); // d/s/b jet from W(H+)

  void SetExtraJets(std::vector<Int_t> extra_jet_);
  void SetJetPtResolution(std::vector<Double_t> jetPtResolution_);
  void SetJetEtaResolution(std::vector<Double_t> jetEtaResolution_);
  void SetJetPhiResolution(std::vector<Double_t> jetPhiResolution_);
  void SetLepton(TLorentzVector lepton_);
  void SetMET(TLorentzVector met_);
  void SetMETShift(Double_t met_pt_up, Double_t met_pt_down, Double_t met_phi_up, Double_t met_phi_down);
  void SetMETShift(Double_t met_shiftX, Double_t met_shiftY);
  void SetNeutrino(TLorentzVector met_,Int_t i); // i is related to neu. Pz
  void SetNeutrinoSmallerPz(TLorentzVector met_);
  void SetGenJets(std::vector<Double_t> genjet_vector_);

  void Fit();
  void FitCurrentPermutation();
  void FindBestChi2Fit(bool UseLeading4Jets=false, bool IsHighMassFitter=false);
  void FindBestSelTopFit(bool IsMaxHadTopPt, bool IsClosestHadTopM, bool IsMaxLepTopPt, bool IsClosestLepTopM, bool noAmbiguity);

  Int_t GetStatus();
  Double_t GetChi2();
  Int_t GetBestDownTypeJetBTagged();
  Double_t GetFittedDijetMass();
  Double_t GetInitialDijetMass();
  Double_t GetCorrectedDijetMass();

  Int_t GetBestStatus();
  Double_t GetBestChi2();
  //Double_t GetBestLambda();
  Double_t GetBestFittedDijetMass();
  Double_t GetBestFittedDijetMass_high();
  Double_t GetBestInitialDijetMass();
  Double_t GetBestInitialDijetMass_high();
  Double_t GetBestCorrectedDijetMass();
  Double_t GetBestCorrectedDijetMass_high();

  Double_t GetBestHadronicTopMass();
  Double_t GetBestHadronicTopPt();
  Double_t GetBestLeptonicTopMass();
  Double_t GetBestLeptonicWMass();
  bool GetBestIsRealNeuPz();

  Int_t GetBestHadronicTopBJetIdx();
  Int_t GetBestLeptonicTopBJetIdx();
  Int_t GetBestHadronicWCHUpTypeJetIdx();
  Int_t GetBestHadronicWCHDownTypeJetIdx();

  Double_t GetBestHadronicTopBJetPull();
  Double_t GetBestLeptonicTopBJetPull();
  Double_t GetBestHadronicWCHUptypeJetIdxPull();
  Double_t GetBestHadronicWCHDowntypeJetIdxPull();

  //Double_t GetBestHadronicTopMassF();
  //Double_t GetBestLeptonicTopMassF();
  //Double_t GetBestLeptonicWMassF();
  //Double_t GetBestDeltaS();

  std::vector<Double_t> GetHadronicTopMassVector(bool IsConverge=true);
  std::vector<Double_t> GetHadronicTopBPtVector(bool IsConverge=true);
  std::vector<Double_t> GetLeptonicTopBPtVector(bool IsConverge=true);
  std::vector<Double_t> GetWCHDownTypePtVector(bool IsConverge=true);
  std::vector<Double_t> GetWCHUpTypePtVector(bool IsConverge=true);

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
    Int_t status; //fitter status
    Double_t fitted_dijet_M;
    Double_t initial_dijet_M;
    Double_t corrected_dijet_M;

    Double_t fitted_dijet_M_high;
    Double_t initial_dijet_M_high;
    Double_t corrected_dijet_M_high;

    Double_t hadronic_top_M;
    Double_t hadronic_top_pt;
    Double_t leptonic_top_M;
    Double_t leptonic_top_pt;
    Double_t leptonic_W_M;
    bool IsRealNeuPz;

    Double_t hadronic_top_b_pt;
    Double_t leptonic_top_b_pt;
    Double_t w_ch_up_type_pt;
    Double_t w_ch_down_type_pt;

    //idx
    Int_t hadronic_top_b_jet_idx;
    Int_t leptonic_top_b_jet_idx;
    Int_t w_ch_up_type_jet_idx;
    Int_t w_ch_down_type_jet_idx;

    //pull
    Double_t hadronic_top_b_jet_pull;
    Double_t leptonic_top_b_jet_pull;
    Double_t w_ch_up_type_jet_pull;
    Double_t w_ch_down_type_jet_pull;

    Double_t fitted_hadronic_top_b_jet_pull;
    Double_t fitted_leptonic_top_b_jet_pull;
    Double_t fitted_w_ch_up_type_jet_pull;
    Double_t fitted_w_ch_down_type_jet_pull;

    //tagging
    Int_t down_type_jet_b_tagged;

    // F from constraints
    //Double_t hadronic_top_mass_F;
    //Double_t leptonic_top_mass_F;
    //Double_t leptonic_w_mass_F;
    //Double_t currS;
    //Double_t deltaS;
    Double_t chi2;
    Double_t initChi2;
    Double_t chi2_lep;
    Double_t chi2_had;
    //Double_t lambda;

    // pX pY
    Double_t init_pX;
    Double_t init_pY;
    Double_t fitted_pX;
    Double_t fitted_pY;
  };

private:

  void SetJetError(TMatrixD *matrix,  Double_t Pt, Double_t Eta, Double_t Phi, TString flavour_key);
  void SetUnclError(TMatrixD *matrix, TLorentzVector &met);
  Double_t JetErrorPt(Double_t Pt, Double_t Eta, TString flavour_key);
  Double_t JetErrorEta(Double_t Pt, Double_t Eta, TString flavour_key);
  Double_t JetErrorPhi(Double_t Pt, Double_t Eta, TString flavour_key);

  void SetConstraint();
  void SetFitter();
  void SaveResults();

  Double_t ComparingInHadTopRestFrame(const TLorentzVector *jet1,const TLorentzVector *jet2, const TLorentzVector *had_top_b_jet);

  Double_t CalcChi2(TString option);
  Double_t CalcEachChi2(TAbsFitParticle* ptr);
  Double_t CalcEachChi2(std::vector<TAbsFitParticle*> ptrs);
  Double_t CalcPull(TAbsFitParticle* ptr);
  Double_t CalcPull(std::vector<TAbsFitParticle*> ptrs);
  Double_t CalcEachChi2(TAbsFitConstraint* ptr, Double_t width);

  void SetCurrentPermutationJets(bool fsr_recover);
  bool Check_BJet_Assignment();
  bool Kinematic_Cut();
  bool Quality_Cut();
  bool NextPermutation(bool UseLeading4Jets=false);

  void Sol_Neutrino_Pz();
  void Resol_Neutrino_Pt();
  Double_t neutrino_pz_sol[2];
  bool IsRealNeuPz;

  TKinFitter *fitter;
  TSCorrection *ts_correction;

  std::vector<TLorentzVector> jet_vector;
  std::vector<Double_t> genjet_pt_vector;
  std::vector<Double_t> jet_pt_resolution_vector;
  std::vector<Double_t> jet_eta_resolution_vector;
  std::vector<Double_t> jet_phi_resolution_vector;
  std::vector<bool> btag_vector;
  std::vector<Double_t> btag_csv_vector;
  TLorentzVector METv;
  Double_t MET_pt_shift;
  Double_t MET_phi_shift;
  TLorentzVector recal_METv;

  bool isMGaus;
  bool isMinimumChi2;
  bool isEtaPhiFit;
  bool isSameTopM;

  Int_t njets;
  Int_t nbtags;
  std::vector<TKinFitterDriver::JET_ASSIGNMENT> permutation_vector;

  std::vector<TLorentzVector> hadronic_top_b_jet; // b jet comes from hadronic top 
  std::vector<TLorentzVector> leptonic_top_b_jet; // b jet comes from leptonic top
  std::vector<TLorentzVector> hadronic_w_ch_jet1; // u/c jet comes from W(H+)
  std::vector<TLorentzVector> hadronic_w_ch_jet2; // d/s/b jet comes from W(H+)
  std::vector<TLorentzVector> corr_hadronic_top_b_jet; // applied TS Correction
  std::vector<TLorentzVector> corr_leptonic_top_b_jet; 
  std::vector<TLorentzVector> corr_hadronic_w_ch_jet1;
  std::vector<TLorentzVector> corr_hadronic_w_ch_jet2;
  std::vector<TLorentzVector> corr_extra_jets;
  TLorentzVector lepton; // lepton comes from leptonic W
  TLorentzVector neutrino_pxpypz; // neutrino_pxpypz comes from leptonic W
  TVector3       neutrino_vec3; 
  TLorentzVector neutrino_pz; // neutrino_pz

  std::vector<TAbsFitParticle*> fit_hadronic_top_b_jet;
  std::vector<TAbsFitParticle*> fit_leptonic_top_b_jet;
  std::vector<TAbsFitParticle*> fit_hadronic_w_ch_jet1;
  std::vector<TAbsFitParticle*> fit_hadronic_w_ch_jet2;
  std::vector<TAbsFitParticle*> fit_extra_jets;
  TFitParticlePt *fit_lepton;
  TFitParticleMCCart *fit_neutrino_pxpypz;

  Int_t hadronic_top_b_jet_idx;
  Int_t leptonic_top_b_jet_idx;
  Int_t w_ch_up_type_jet_idx;
  Int_t w_ch_down_type_jet_idx;

  std::vector<TMatrixD> error_hadronic_top_b_jet; 
  std::vector<TMatrixD> error_leptonic_top_b_jet;
  std::vector<TMatrixD> error_hadronic_w_ch_jet1;
  std::vector<TMatrixD> error_hadronic_w_ch_jet2;
  std::vector<TMatrixD> error_extra_jets;
  TMatrixD error_lepton;
  TMatrixD error_neutrino_pxpypz;

  TFitConstraintM *constrain_hadronic_top_M;
  TFitConstraintMGaus *constrain_hadronic_top_MGaus;
  TFitConstraintM *constrain_leptonic_top_M;
  TFitConstraintMGaus *constrain_leptonic_top_MGaus;
  TFitConstraintM *constrain_leptonic_W_M;
  TFitConstraintMGaus *constrain_leptonic_W_MGaus;

  TFitConstraintEp *constrain_pX;
  TFitConstraintEp *constrain_pY;

  TKinFitterDriver::ResultContainer fit_result;

  std::vector<TKinFitterDriver::ResultContainer> fit_result_vector;
  static bool Chi2Comparing(const TKinFitterDriver::ResultContainer& rc1, const TKinFitterDriver::ResultContainer& rc2);
  static bool InitChi2Comparing(const TKinFitterDriver::ResultContainer& rc1, const TKinFitterDriver::ResultContainer& rc2);
  static bool HadTopMComparing(const TKinFitterDriver::ResultContainer& rc1, const TKinFitterDriver::ResultContainer& rc2);
  static bool HighMassFitter(const TKinFitterDriver::ResultContainer& rc1, const TKinFitterDriver::ResultContainer& rc2);
  static bool HadTopPtComparing(const TKinFitterDriver::ResultContainer& rc1, const TKinFitterDriver::ResultContainer& rc2);
  static bool LepTopPtComparing(const TKinFitterDriver::ResultContainer& rc1, const TKinFitterDriver::ResultContainer& rc2);
  static bool TopPtComparing(const TKinFitterDriver::ResultContainer& rc1, const TKinFitterDriver::ResultContainer& rc2);

  Double_t GetChi2Dist(Double_t chi2_, Double_t lambda_);

};

#endif
