#ifndef Vcb_SL_h
#define Vcb_SL_h

#include "AnalyzerCore.h"
#include "TKinFitter.h"
#include "TFitParticlePt.h"
#include "TFitParticleEtEtaPhi.h"
#include "TFitParticleMCCart.h"
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
#include <variant>
using correction::CorrectionSet;

class Vcb_SL : public AnalyzerCore
{
public:
    struct KinematicFitterResult
    {
        int status;
        float chi2;
        int best_had_t_b_idx;
        int best_lep_t_b_idx;
        int best_had_w1_idx;
        int best_had_w2_idx;
        TLorentzVector fitted_had_t_b;
        TLorentzVector fitted_lep_t_b;
        TLorentzVector fitted_had_w1;
        TLorentzVector fitted_had_w2;
        TLorentzVector fitted_lep;
        TLorentzVector fitted_neu;
        float best_neu_pz;
    };
    inline bool isPIDUpTypeQuark(int pdg) { return (abs(pdg) == 2 || abs(pdg) == 4 || abs(pdg) == 6); }
    inline bool isPIDDownTypeQuark(int pdg) { return (abs(pdg) == 1 || abs(pdg) == 3 || abs(pdg) == 5); }
    inline bool isPIDLepton(int pdg) { return (abs(pdg) == 11 || abs(pdg) == 13 || abs(pdg) == 15); }
    void initializeAnalyzer();
    void executeEvent();
    void executeEventFromParameter(TString syst);
    bool PassBaseLineSelection(bool remove_flavtagging_cut = true, TString syst = "nominal");
    void FillHistogramsAtThisPoint(const TString &histPrefix, float weight = 1.f);
    void FillKinematicFitterResult(const TString &histPrefix, float weight);
    float CalculateEventWeight(bool include_tagging_weight=true);
    std::variant<float, std::pair<float, float>> SolveNeutrinoPz(const Lepton &lepton, const Particle &met);
    void GetKineMaticFitterResult(const RVec<Jet> &jets, Particle &MET, Lepton &lepton);
    short GetPassedBTaggingWP(const Jet &jet);
    short GetPassedCTaggingWP(const Jet &jet);
    RVec<int> FindTTbarJetIndices(const RVec<Gen> &gens, const RVec<GenJet> &genjets);
    tuple<int, float, RVec<unsigned int>, RVec<TLorentzVector>> FitKinFitter(const RVec<Jet> &jets, const RVec<unsigned int> &permutation, Particle &neutrino, Lepton &lepton);
    void FillTrainingTree();
    float gofFromChi2(float chi2, int ndf);
    RVec<RVec<unsigned int>> GetPermutations(const RVec<Jet> &jets);
    RVec<Jet> GetObjects;

    Vcb_SL();
    ~Vcb_SL();

    unique_ptr<CorrectionSet> cset_TriggerSF;

    enum class Channel{
        El, Mu
    };
    std::unordered_map<std::string, std::string> SL_Trigger;

    Channel channel;

    // Objects
    RVec<Muon> AllMuons;
    RVec<Electron> AllElectrons;
    RVec<Jet> AllJets;
    RVec<Gen> AllGens;
    RVec<LHE> AllLHEs;
    RVec<GenJet> AllGenJets;
    // Selected Objects
    RVec<Jet> Jets;
    RVec<Electron> Electrons_Veto;
    RVec<Electron> Electrons;
    RVec<Muon> Muons_Veto;
    RVec<Muon> Muons;
    Lepton lepton;
    Event ev;
    Particle MET;
    RVec<int> ttbar_jet_indices;
    // event info
    float HT;
    short n_jets;
    short n_b_tagged_jets;
    short n_c_tagged_jets;
    short n_partonFlav_b_jets;
    short n_partonFlav_c_jets;
    bool find_all_jets;
    int tt_decay_code;
    bool ttbj;
    bool ttbb;
    bool ttcc;
    bool ttLF;
    KinematicFitterResult best_KF_result;
};

#endif
