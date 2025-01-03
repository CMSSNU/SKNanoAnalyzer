#ifndef Vcb_FH_h
#define Vcb_FH_h

#include "Vcb.h"


class Vcb_FH : public Vcb
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
    inline bool CheckChannel() override
    {
        if (channel != Channel::FH) return false;
        return true;
    };
    inline float MCNormalization() override { 
        return MCweight()*ev.GetTriggerLumi(FH_Trigger_DoubleBTag[DataEra.Data()]);
    }
    bool PassBaseLineSelection(bool remove_flavtagging_cut = false) override;
    void FillKinematicFitterResult(const TString &histPrefix, float weight) override;
    void GetKineMaticFitterResult(const RVec<Jet> &jets);
    RVec<int> FindTTbarJetIndices() override;
    tuple<int, float, RVec<unsigned int>, RVec<TLorentzVector>> FitKinFitter(const RVec<Jet> &jets, const RVec<unsigned int> &permutation);
    void FillTrainingTree() override;
    RVec<RVec<unsigned int>> GetPermutations(const RVec<Jet> &jets) override;
    void CreateTrainingTree() override;
    void virtual InferONNX() override;
    void virtual FillONNXRecoInfo(const TString &histPrefix, float weight) override;
    inline std::string GetRegionString() override
    {
        if(class_label == classCategory::Signal) return "SR_FH";
        else if(class_label == classCategory::tt) return "tt_FH";
        else if(class_label == classCategory::Disposal) return "Disposal_FH";
        else throw std::runtime_error("Invalid class label");
    }

    
    Vcb_FH();
    ~Vcb_FH() override = default;

    KinematicFitterResult best_KF_result;

    // Training Tree
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

    std::vector<int> edge_index0;
    std::vector<int> edge_index1;
    std::vector<float> deltaR;
    std::vector<float> invM;
    std::vector<float> cosTheta;

    //infering ONNX
    enum class classCategory
    {
        Signal,
        tt,
        Disposal,
    };

    std::array<int,6> assignment;
    std::array<float, 3> class_score;
    classCategory class_label;
};

#endif
