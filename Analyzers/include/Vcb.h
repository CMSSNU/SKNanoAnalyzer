#ifndef Vcb_h
#define Vcb_h


#include <future>
#include <tuple>
#include <memory>
#include <set>
#include <correction.h>
#include <variant>
#include <functional>

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
using correction::CorrectionSet;
#include "SystematicHelper.h"
#include "MLHelper.h"

using FloatArray = std::vector<float>;
using FloatArrays = std::vector<FloatArray>;
using IntArray = std::vector<int>;
using BoolArray = std::vector<uint8_t>;
using VariousArray = std::variant<FloatArray, IntArray, BoolArray>;

class Vcb : public AnalyzerCore
{
public:

    inline bool isPIDUpTypeQuark(int pdg) { return (abs(pdg) == 2 || abs(pdg) == 4 || abs(pdg) == 6); }
    inline bool isPIDDownTypeQuark(int pdg) { return (abs(pdg) == 1 || abs(pdg) == 3 || abs(pdg) == 5); }
    inline bool isPIDLepton(int pdg) { return (abs(pdg) == 11 || abs(pdg) == 13 || abs(pdg) == 15); }
    inline bool isPIDNeutrino(int pdg) { return (abs(pdg) == 12 || abs(pdg) == 14 || abs(pdg) == 16); }
    inline bool isDaughterOf(int idx, int m_idx) {
    while (idx >= 0) {
        int midx = AllGens[idx].MotherIndex();
        if (midx < 0) break; // No valid mother
        if (midx == m_idx) return true; // Found the W in the chain
        idx = midx;
    }
    return false;
    };
    float LeptonTriggerWeight(bool isEle, const Correction::variation syst = Correction::variation::nom, const TString &source = "total");   
    void Clear();
    int Unroller(RVec<Jet> &jets);
    int Unroller(Jet &jet1, Jet &jet2);
    short GetPassedBTaggingWP(const Jet &jet);
    short GetPassedCTaggingWP(const Jet &jet);
    void SetChannel();
    bool virtual CheckChannel() = 0;
    void initializeAnalyzer();
    void executeEvent();
    void executeEventFromParameter();
    bool virtual PassBaseLineSelection(bool remove_flavtagging_cut = false) = 0;
    void virtual FillHistogramsAtThisPoint(const TString &histPrefix, float weight = 1.f);
    void virtual FillKinematicFitterResult(const TString &histPrefix, float weight);
    void virtual SkimTree();
    void SetTTbarId();
    void SetSystematicLambda(bool remove_flavtagging_sf = false);
    void virtual CreateTrainingTree();
    RVec<int> virtual FindTTbarJetIndices();
    void virtual FillTrainingTree();
    float virtual MCNormalization() = 0;
    void virtual InferONNX();
    std::string virtual GetRegionString() = 0;
    inline size_t FindNthMaxIndex(FloatArray &array, int ranking)
    {
        std::vector<size_t> indices(array.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::sort(indices.begin(), indices.end(), [&array](size_t i1, size_t i2) { return array[i1] > array[i2]; });
        return indices[ranking];
    }
    inline std::vector<int> UnravelIndex(int idx, std::vector<int> shape)
    {
        std::vector<int> indices(shape.size());
        for (int i = shape.size(); i-- > 0;)
        {
            indices[i] = idx % shape[i];
            idx /= shape[i];
        }
        return indices;
    }
    inline std::string GetTTHFPostFix(){
        if (ttbj || ttbb) return "+B";
        if (ttcc) return "+C";
        if (ttLF) return "+LF";
        return "";
    }

    void virtual FillONNXRecoInfo(const TString &histPrefix, float weight);
    inline float GetJetEnergyFractionWithRadius(Jet &jet, float radius)
    {
        float sum = 0;
        for (size_t i = 0; i < Jets.size(); i++)
        {
            if (jet.DeltaR(Jets[i]) < 1e-5) continue;
            if (jet.DeltaR(Jets[i]) < radius) sum += Jets[i].E();
        }
        return jet.E()/sum;

    }

    RVec<RVec<unsigned int>> virtual GetPermutations(const RVec<Jet> &jets);
    Vcb();
    virtual ~Vcb() = default;

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
    RVec<Lepton> leptons;
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
    std::unique_ptr<SystematicHelper> systHelper;

    enum class Channel
    {
        MM,
        ME,
        EE,
        Mu,
        El,
        FH
    };

    inline TString GetChannelString(Channel ch)
    {
        switch (ch)
        {
        case Channel::MM:
            return "MM";
        case Channel::ME:
            return "ME";
        case Channel::EE:
            return "EE";
        case Channel::Mu:
            return "Mu";
        case Channel::El:
            return "El";
        case Channel::FH:
            return "FH";
        default:
            return "";
        }
    }
    
    Channel channel;

    std::unique_ptr<MLHelper> myMLHelper;
};

#endif
