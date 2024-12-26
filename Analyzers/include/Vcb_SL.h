#ifndef Vcb_SL_h
#define Vcb_SL_h

#include "Vcb.h"

class Vcb_SL : public Vcb 
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

    inline bool CheckChannel() override
    {
        if (channel!=Channel::El && channel!=Channel::Mu) return false;
        return true;
    };
    inline float MCNormalization() override { 
        if(channel == Channel::El) return MCweight()*ev.GetTriggerLumi(El_Trigger[DataEra.Data()]);
        else if(channel == Channel::Mu) return MCweight()*ev.GetTriggerLumi(Mu_Trigger[DataEra.Data()]);
        else throw std::runtime_error("Invalid channel");
    }
    bool PassBaseLineSelection(bool remove_flavtagging_cut = false) override;
    void FillKinematicFitterResult(const TString &histPrefix, float weight) override;
    std::variant<float, std::pair<float, float>> SolveNeutrinoPz(const Lepton &lepton, const Particle &met);
    void GetKineMaticFitterResult(const RVec<Jet> &jets, Particle &MET, Lepton &lepton);
    RVec<int> FindTTbarJetIndices() override;
    tuple<int, float, RVec<unsigned int>, RVec<TLorentzVector>> FitKinFitter(const RVec<Jet> &jets, const RVec<unsigned int> &permutation, Particle &neutrino, Lepton &lepton);
    void FillTrainingTree() override;
    void virtual CreateTrainingTree() override;
    RVec<RVec<unsigned int>> GetPermutations(const RVec<Jet> &jets) override;
    void virtual InferONNX() override;
    void virtual FillONNXRecoInfo(const TString &histPrefix, float weight) override;

    inline std::string GetRegionString() override
    {
        if(class_label == classCategory::Signal) return "SR";
        else if(class_label == classCategory::Control0) return "CR";
        else if(class_label == classCategory::Disposal) return "Disposal";
        else throw std::runtime_error("Invalid class label");
    }

    Vcb_SL();
    ~Vcb_SL() override = default;


    std::unordered_map<std::string, std::string> SL_Trigger;

    KinematicFitterResult best_KF_result;

    //Training Tree
    std::vector<float> Jet_Px;
    std::vector<float> Jet_Py;
    std::vector<float> Jet_Pz;
    std::vector<float> Jet_E;
    std::vector<float> Jet_M;
    std::vector<float> Jet_BvsC;
    std::vector<float> Jet_CvsB;
    std::vector<float> Jet_CvsL;
    std::vector<float> Jet_QvsG;
    std::vector<int> Jet_B_WP;
    std::vector<int> Jet_C_WP;
    std::vector<int> Jet_isTTbarJet;
    std::vector<int> Jet_ttbarJet_idx;
    std::vector<float> Jet_Pt;
    std::vector<float> Jet_Eta;
    std::vector<float> Jet_Phi;

    std::vector<int> edge_index_jet_jet0;
    std::vector<int> edge_index_jet_jet1;
    std::vector<float> deltaR_jet_jet;
    std::vector<float> invM_jet_jet;
    std::vector<float> cosTheta_jet_jet;

    std::vector<int> edge_index_jet_lepton0;
    std::vector<int> edge_index_jet_lepton1;
    std::vector<float> deltaR_jet_lepton;
    std::vector<float> invM_jet_lepton;
    std::vector<float> cosTheta_jet_lepton;

    std::vector<int> edge_index_jet_neutrino0;
    std::vector<int> edge_index_jet_neutrino1;
    std::vector<float> deltaR_jet_neutrino;
    std::vector<float> invM_jet_neutrino;
    std::vector<float> cosTheta_jet_neutrino;

    std::vector<int> edge_index_lepton_neutrino0;
    std::vector<int> edge_index_lepton_neutrino1;
    std::vector<float> deltaR_lepton_neutrino;
    std::vector<float> invM_lepton_neutrino;
    std::vector<float> cosTheta_lepton_neutrino;

    std::vector<int> parton_jet_assignment;

    Particle gen_neutrino;

    //infering ONNX
    enum class classCategory
    {
        Signal,
        Control0,
        Disposal
    };

    std::array<int,4> assignment;
    std::array<float, 3> class_score;
    classCategory class_label;
};

#endif
