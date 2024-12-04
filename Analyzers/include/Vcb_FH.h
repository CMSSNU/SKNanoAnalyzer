#ifndef Vcb_FH_h
#define Vcb_FH_h

#include "AnalyzerCore.h"
#include "TKinFitter.h"
#include "TFitParticlePt.h"
#include "TFitParticleEtEtaPhi.h"
#include "TFitConstraintMGaus.h"
#include "TFitConstraintM.h"
#include "TFitConstraintEp.h"
#include "TMatrixD.h"
#include "VcbParameters.h"
#include <future>
#include <tuple>
#include <memory>
#include <set>
#include "correction.h"

using correction::CorrectionSet;


class Vcb_FH : public AnalyzerCore
{
    public:
        struct KinematicFitterResult
        {
            int status;
            float chi2;
            int best_b1_idx;
            int best_b2_idx;
            int best_w11_idx;
            int best_w12_idx;
            int best_w21_idx;
            int best_w22_idx;
            TLorentzVector fitted_b1;
            TLorentzVector fitted_b2;
            TLorentzVector fitted_w11;
            TLorentzVector fitted_w12;
            TLorentzVector fitted_w21;
            TLorentzVector fitted_w22;
        };
    inline bool isPIDUpTypeQuark(int pdg) { return (abs(pdg) == 2 || abs(pdg) == 4 || abs(pdg) == 6); }
    inline bool isPIDDownTypeQuark(int pdg) { return (abs(pdg) == 1 || abs(pdg) == 3 || abs(pdg) == 5); }
    void initializeAnalyzer();
    void executeEvent();
    void executeEventFromParameter(TString syst);
    bool PassBaseLineSelection(bool remove_flavtagging_cut = true, TString syst = "nominal");
    void FillHistogramsAtThisPoint(const TString &histPrefix, float weight = 1.f);
    void FillKinematicFitterResult(const TString &histPrefix, float weight);
    float CalculateEventWeight(bool include_tagging_weight=true);
    void GetKineMaticFitterResult(const RVec<Jet> &jets);
    short GetPassedBTaggingWP(const Jet &jet);
    short GetPassedCTaggingWP(const Jet &jet);
    RVec<int> FindTTbarJetIndices(const RVec<Gen> &gens, const RVec<GenJet> &genjets);
    tuple<int, float, RVec<unsigned int>, RVec<TLorentzVector>> FitKinFitter(const RVec<Jet> &jets, const RVec<unsigned int> &permutation);
    void FillTrainingTree();
    float gofFromChi2(float chi2, int ndf);
    RVec<RVec<unsigned int>> GetPermutations(const RVec<Jet> &jets);

    Vcb_FH();
    ~Vcb_FH();

    unique_ptr<CorrectionSet> cset_TriggerSF;
    

    //Objects
    RVec<Muon> AllMuons;
    RVec<Electron> AllElectrons;
    RVec<Jet> AllJets;
    RVec<Gen> AllGens;
    RVec<LHE> AllLHEs;
    RVec<GenJet> AllGenJets;
    //Selected Objects
    RVec<Jet> Jets;
    RVec<Muon> Muons_Veto;
    RVec<Electron> Electrons_Veto;
    RVec<Muon> Muons;
    RVec<Electron> Electrons;
    Event ev;
    Particle MET;
    RVec<int> ttbar_jet_indices;
    //event info
    float HT;
    short n_jets;
    short n_b_tagged_jets;
    short n_c_tagged_jets;
    short n_partonFlav_b_jets;
    short n_partonFlav_c_jets;
    bool find_all_jets;
    int tt_decay_code;
    KinematicFitterResult best_KF_result;
    bool ttLF;
    bool ttcc;
    bool ttbb;
    bool ttbj;


};

#endif
