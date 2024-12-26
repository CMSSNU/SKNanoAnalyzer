#include "Vcb.h"

Vcb::Vcb() {}

void Vcb::initializeAnalyzer()
{
    SetChannel();
    std::string btagging_eff_file = "btaggingEff.json";
    std::string ctagging_eff_file = "ctaggingEff.json";
    std::string btagging_R_file = "btaggingR.json";
    std::string ctagging_R_file = "ctaggingR.json";
    if(channel == Channel::FH){
        std::cout << "Initialize Correction for FH" << std::endl;
        btagging_R_file = "Vcb_FH_btaggingR.json";
        ctagging_R_file = "Vcb_FH_ctaggingR.json";
        myCorr = new Correction(DataEra, IsDATA ? DataStream : MCSample, IsDATA, btagging_eff_file, ctagging_eff_file, btagging_R_file, ctagging_R_file);
        myCorr->SetTaggingParam(FlavTagger[DataEra.Data()], FH_BTag_WP);
    }
    else if(channel == Channel::Mu || channel == Channel::El){
        std::cout << "Initialize Correction for SL" << std::endl;
        btagging_R_file = "Vcb_SL_btaggingR.json";
        ctagging_R_file = "Vcb_SL_ctaggingR.json";
        myCorr = new Correction(DataEra, IsDATA ? DataStream : MCSample, IsDATA, btagging_eff_file, ctagging_eff_file, btagging_R_file, ctagging_R_file);
        myCorr->SetTaggingParam(FlavTagger[DataEra.Data()], SL_BTag_WP);
        myMLHelper = std::make_unique<MLHelper>("/data6/Users/yeonjoon/SKNanoAnalyzer/data/Run3_v12_Run2_v9/2022EE/spanet_2022EE.onnx", MLHelper::ModelType::ONNX);
    }
    else if(channel == Channel::MM || channel == Channel::ME || channel == Channel::EE){
        std::cout << "Initialize Correction for DL" << std::endl;
        btagging_R_file = "Vcb_DL_btaggingR.json";
        ctagging_R_file = "Vcb_DL_ctaggingR.json";
        myCorr = new Correction(DataEra, IsDATA ? DataStream : MCSample, IsDATA, btagging_eff_file, ctagging_eff_file, btagging_R_file, ctagging_R_file);
        myCorr->SetTaggingParam(FlavTagger[DataEra.Data()], DL_BTag_WP);
    }
    else{ // because FH doesn't need to specify the channel
        std::cout << "Initialize Correction for FH" << std::endl;
        btagging_R_file = "Vcb_FH_btaggingR.json";
        ctagging_R_file = "Vcb_FH_ctaggingR.json";
        myCorr = new Correction(DataEra, IsDATA ? DataStream : MCSample, IsDATA, btagging_eff_file, ctagging_eff_file, btagging_R_file, ctagging_R_file);
        myCorr->SetTaggingParam(FlavTagger[DataEra.Data()], FH_BTag_WP);
    }

    
    
    systHelper = std::make_unique<SystematicHelper>("/data6/Users/yeonjoon/SKNanoAnalyzer/AnalyzerTools/VcbSystematic_BTag.yaml", MCSample);
    if (IsDATA)
    {
        systHelper = std::make_unique<SystematicHelper>("/data6/Users/yeonjoon/SKNanoAnalyzer/AnalyzerTools/noSyst.yaml", DataStream);
    }
    else
    {
        systHelper = std::make_unique<SystematicHelper>("/data6/Users/yeonjoon/SKNanoAnalyzer/AnalyzerTools/VcbSystematic_BTag.yaml", MCSample);
    }
    
    CreateTrainingTree();
}

void Vcb::SetChannel()
{
    if (HasFlag("Skim"))
        std::cout << "Skimming mode detected, will iterate over channel" << std::endl;
    else if (HasFlag("MM"))
        channel = Channel::MM;
    else if (HasFlag("ME"))
        channel = Channel::ME;
    else if (HasFlag("EE"))
        channel = Channel::EE;
    else if (HasFlag("Mu"))
        channel = Channel::Mu;
    else if (HasFlag("El"))
        channel = Channel::El;
    else if (HasFlag("FH"))
        channel = Channel::FH;
    else
        channel = Channel::FH;
}

int Vcb::Unroller(RVec<Jet> &jets)
{
    RVec<Jet> jets_BvsC_sorted = jets;
    JetTagging::JetFlavTagger tagger = FlavTagger[DataEra.Data()];
    std::sort(jets_BvsC_sorted.begin(), jets_BvsC_sorted.end(), [&tagger](const Jet &a, const Jet &b)
              { return a.GetBTaggerResult(tagger) > b.GetBTaggerResult(tagger); });
    Jet third_leading_BvsC_jet = jets_BvsC_sorted[2];
    Jet fourth_leading_BvsC_jet = jets_BvsC_sorted[3];

    int third_leading_BvsC_jet_PassedBTaggingWP = GetPassedBTaggingWP(third_leading_BvsC_jet);
    int fourth_leading_BvsC_jet_PassedBTaggingWP = GetPassedBTaggingWP(fourth_leading_BvsC_jet);
    third_leading_BvsC_jet_PassedBTaggingWP = third_leading_BvsC_jet_PassedBTaggingWP == 4 ? 3 : third_leading_BvsC_jet_PassedBTaggingWP;
    fourth_leading_BvsC_jet_PassedBTaggingWP = fourth_leading_BvsC_jet_PassedBTaggingWP == 4 ? 3 : fourth_leading_BvsC_jet_PassedBTaggingWP;
    int unrolled = third_leading_BvsC_jet_PassedBTaggingWP + fourth_leading_BvsC_jet_PassedBTaggingWP * 4;
    return unrolled;
}

int Vcb::Unroller(Jet &jet1, Jet &jet2){
    int jet1_PassedBTaggingWP = GetPassedBTaggingWP(jet1);
    int jet2_PassedBTaggingWP = GetPassedBTaggingWP(jet2);
    jet1_PassedBTaggingWP = jet1_PassedBTaggingWP == 4 ? 3 : jet1_PassedBTaggingWP;
    jet2_PassedBTaggingWP = jet2_PassedBTaggingWP == 4 ? 3 : jet2_PassedBTaggingWP;
    int unrolled = jet1_PassedBTaggingWP + jet2_PassedBTaggingWP * 4;
    return unrolled;
}

void Vcb::FillHistogramsAtThisPoint(const TString &histPrefix, float weight)
{
    FillHist(histPrefix + "/" + "MET", MET.Pt(), weight, 100, 0., 200.);
    FillHist(histPrefix + "/" + "HT", HT, weight, 200, 450., 2000.);
    FillHist(histPrefix + "/" + "n_jets", n_jets, weight, 10, 4., 14.);
    FillHist(histPrefix + "/" + "n_b_tagged_jets", n_b_tagged_jets, weight, 6, 2., 8.);
    FillHist(histPrefix + "/" + "n_c_tagged_jets", n_c_tagged_jets, weight, 6, 2., 8.);
    FillHist(histPrefix + "/" + "n_partonFlav_b_jets", n_partonFlav_b_jets, weight, 6, 2., 8.);
    FillHist(histPrefix + "/" + "n_partonFlav_c_jets", n_partonFlav_c_jets, weight, 8, 0., 8.);
    FillHist(histPrefix + "/" + "real_b_vs_tagged_b", n_partonFlav_b_jets, n_b_tagged_jets, weight, 8, 0., 8., 6, 2., 8.);
    FillHist(histPrefix + "/" + "real_c_vs_tagged_c", n_partonFlav_c_jets, n_c_tagged_jets, weight, 8, 0., 8., 6, 2., 8.);
    for (size_t i = 0; i < Jets.size(); i++)
    {
        FillHist(histPrefix + "/" + "Jet_Pt_" + std::to_string(i), Jets[i].Pt(), weight, 100, 0., 500.);
        FillHist(histPrefix + "/" + "Jet_Eta_" + std::to_string(i), Jets[i].Eta(), weight, 100, -2.5, 2.5);
        FillHist(histPrefix + "/" + "Jet_Phi_" + std::to_string(i), Jets[i].Phi(), weight, 100, -3.14, 3.14);
        FillHist(histPrefix + "/" + "Jet_BvsC_" + std::to_string(i), Jets[i].GetBTaggerResult(FlavTagger[DataEra.Data()]), weight, 100, 0., 1.);
        FillHist(histPrefix + "/" + "Jet_CvsB_" + std::to_string(i), Jets[i].GetCTaggerResult(FlavTagger[DataEra.Data()]).first, weight, 100, 0., 1.);
        FillHist(histPrefix + "/" + "Jet_CvsL_" + std::to_string(i), Jets[i].GetCTaggerResult(FlavTagger[DataEra.Data()]).second, weight, 100, 0., 1.);
        FillHist(histPrefix + "/" + "Jet_QvsG_" + std::to_string(i), Jets[i].GetQvGTaggerResult(FlavTagger[DataEra.Data()]), weight, 100, 0., 1.);
    }
    for (size_t i = 0; i < leptons.size(); i++)
    {
        FillHist(histPrefix + "/" + "Lepton_Pt_" + std::to_string(i), leptons[i].Pt(), weight, 100, 0., 500.);
        FillHist(histPrefix + "/" + "Lepton_Eta_" + std::to_string(i), leptons[i].Eta(), weight, 100, -2.5, 2.5);
        FillHist(histPrefix + "/" + "Lepton_Phi_" + std::to_string(i), leptons[i].Phi(), weight, 100, -3.14, 3.14);
    }
    if (leptons.size() == 2)
    {
        Particle ZCand = leptons[0] + leptons[1];
        FillHist(histPrefix + "/" + "ZCand_Mass", ZCand.M(), weight, 100, 0., 200.);
        FillHist(histPrefix + "/" + "ZCand_Pt", ZCand.Pt(), weight, 100, 0., 200.);
        FillHist(histPrefix + "/" + "ZCand_Eta", ZCand.Eta(), weight, 100, -2.5, 2.5);
        FillHist(histPrefix + "/" + "BvsC_Unrolled", Unroller(Jets), weight, 16, 0., 16.);
    }
}

short Vcb::GetPassedBTaggingWP(const Jet &jet)
{
    float TightWP = myCorr->GetBTaggingWP(FlavTagger[DataEra.Data()], JetTagging::JetFlavTaggerWP::Tight);
    float MediumWP = myCorr->GetBTaggingWP(FlavTagger[DataEra.Data()], JetTagging::JetFlavTaggerWP::Medium);
    float LooseWP = myCorr->GetBTaggingWP(FlavTagger[DataEra.Data()], JetTagging::JetFlavTaggerWP::Loose);
    if (jet.GetBTaggerResult(FlavTagger[DataEra.Data()]) > TightWP)
        return 4;
    else if (jet.GetBTaggerResult(FlavTagger[DataEra.Data()]) > MediumWP)
        return 2;
    else if (jet.GetBTaggerResult(FlavTagger[DataEra.Data()]) > LooseWP)
        return 1;
    else
        return 0;
}

short Vcb::GetPassedCTaggingWP(const Jet &jet)
{
    float CvsB_TightWP = myCorr->GetCTaggingWP(FlavTagger[DataEra.Data()], JetTagging::JetFlavTaggerWP::Tight).first;
    float CvDL_TightWP = myCorr->GetCTaggingWP(FlavTagger[DataEra.Data()], JetTagging::JetFlavTaggerWP::Tight).second;
    float CvsB_MediumWP = myCorr->GetCTaggingWP(FlavTagger[DataEra.Data()], JetTagging::JetFlavTaggerWP::Medium).first;
    float CvDL_MediumWP = myCorr->GetCTaggingWP(FlavTagger[DataEra.Data()], JetTagging::JetFlavTaggerWP::Medium).second;
    float CvsB_LooseWP = myCorr->GetCTaggingWP(FlavTagger[DataEra.Data()], JetTagging::JetFlavTaggerWP::Loose).first;
    float CvDL_LooseWP = myCorr->GetCTaggingWP(FlavTagger[DataEra.Data()], JetTagging::JetFlavTaggerWP::Loose).second;

    if (jet.GetCTaggerResult(FlavTagger[DataEra.Data()]).first > CvsB_TightWP && jet.GetCTaggerResult(FlavTagger[DataEra.Data()]).second > CvDL_TightWP)
        return 4;
    else if (jet.GetCTaggerResult(FlavTagger[DataEra.Data()]).first > CvsB_MediumWP && jet.GetCTaggerResult(FlavTagger[DataEra.Data()]).second > CvDL_MediumWP)
        return 2;
    else if (jet.GetCTaggerResult(FlavTagger[DataEra.Data()]).first > CvsB_LooseWP && jet.GetCTaggerResult(FlavTagger[DataEra.Data()]).second > CvDL_LooseWP)
        return 1;
    else
        return 0;
}

void Vcb::executeEvent()
{
    AllMuons = GetAllMuons();
    AllElectrons = GetAllElectrons();
    AllJets = GetAllJets();
    AllGens = GetAllGens();
    AllGenJets = GetAllGenJets();
    ev = GetEvent();

    if (HasFlag("Skim"))
    {
        Clear();
        SkimTree();
        return;
    }
    if (HasFlag("Training"))
    {
        Clear();
        if (!PassBaseLineSelection())
            return;
        FillTrainingTree();
        return;
    }
    if (!CheckChannel())
    {
        throw std::runtime_error("Invalid channel flag for this analyzer");
    }
    for (const auto &syst_dummy : *systHelper)
    {
        leptons.clear();
        executeEventFromParameter();
    }
}

void Vcb::SetSystematicLambda(bool remove_flavtagging_sf)
{
    std::unordered_map<std::string, std::variant<std::function<float(Correction::variation, TString)>, std::function<float()>>> weight_function_map;
    auto mu_id_lambda = [&](Correction::variation syst, TString source)
    { return myCorr->GetMuonIDSF(Mu_ID_SF_Key[DataEra.Data()], Muons, syst, source); };
    auto mu_iso_lambda = [&](Correction::variation syst, TString source)
    { return myCorr->GetMuonISOSF(Mu_Iso_SF_Key[DataEra.Data()], Muons, syst, source); };
    auto mu_trigger_lambda = [&](Correction::variation syst, TString source)
    { return LeptonTriggerWeight(false, syst, source); };
    auto el_id_lambda = [&](Correction::variation syst, TString source)
    { return myCorr->GetElectronIDSF(El_ID_SF_Key[DataEra.Data()], Electrons, syst, source); };
    auto el_recosf_lambda = [&](Correction::variation syst, TString source)
    { return myCorr->GetElectronRECOSF(Electrons, syst, source); };
    auto el_trigger_lambda = [&](Correction::variation syst, TString source)
    { return LeptonTriggerWeight(true, syst, source); };

    auto pileup_lambda = [&](Correction::variation syst, TString source)
    { return myCorr->GetPUWeight(ev.nTrueInt(), syst, source); };
    auto MuF_lambda = [&](Correction::variation syst, TString source)
    {
        switch (syst)
        {
        case Correction::variation::up:
            return GetScaleVariation(Correction::variation::up, Correction::variation::nom);
        case Correction::variation::down:
            return GetScaleVariation(Correction::variation::down, Correction::variation::nom);
        default:
            return 1.f;
        }
    };

    auto MuR_lambda = [&](Correction::variation syst, TString source)
    {
        switch (syst)
        {
        case Correction::variation::up:
            return GetScaleVariation(Correction::variation::nom, Correction::variation::up);
        case Correction::variation::down:
            return GetScaleVariation(Correction::variation::nom, Correction::variation::down);
        default:
            return 1.f;
        }
    };

    auto ISR_lambda = [&](Correction::variation syst, TString source)
    {
        switch (syst)
        {
        case Correction::variation::up:
            return GetPSWeight(Correction::variation::up, Correction::variation::nom);
        case Correction::variation::down:
            return GetPSWeight(Correction::variation::down, Correction::variation::nom);
        default:
            return 1.f;
        }
    };

    auto FSR_lambda = [&](Correction::variation syst, TString source)
    {
        switch (syst)
        {
        case Correction::variation::up:
            return GetPSWeight(Correction::variation::nom, Correction::variation::up);
        case Correction::variation::down:
            return GetPSWeight(Correction::variation::nom, Correction::variation::down);
        default:
            return 1.f;
        }
    };

    auto BTag_lambda = [&](Correction::variation syst, TString source)
    { float weight = 1.f;
        weight*=myCorr->GetBTaggingSF(Jets, JetTagging::JetTaggingSFMethod::shape, syst, source); 
        weight*=myCorr->GetBTaggingR(Jets, Sample_Shorthand[MCSample.Data()], syst, source);
        return weight;};

    auto dummy_lambda = [&](Correction::variation syst, TString source)
    { return 1.f; };

    weight_function_map["Mu_ID"] = mu_id_lambda;
    weight_function_map["Mu_Iso"] = mu_iso_lambda;
    weight_function_map["Mu_Trig"] = mu_trigger_lambda;
    weight_function_map["El_ID"] = el_id_lambda;
    weight_function_map["El_Reco"] = el_recosf_lambda;
    weight_function_map["El_Trig"] = el_trigger_lambda;
    weight_function_map["Pileup"] = pileup_lambda;
    weight_function_map["MuF"] = MuF_lambda;
    weight_function_map["MuR"] = MuR_lambda;
    weight_function_map["ISR"] = ISR_lambda;
    weight_function_map["FSR"] = FSR_lambda;
    if (remove_flavtagging_sf)
        weight_function_map["btag"] = dummy_lambda;
    else
        weight_function_map["btag"] = BTag_lambda;
    systHelper->assignWeightFunctionMap(weight_function_map);
}

void Vcb::Clear(){
    HT = 0;
    n_jets = 0;
    n_b_tagged_jets = 0;
    n_c_tagged_jets = 0;
    n_partonFlav_b_jets = 0;
    n_partonFlav_c_jets = 0;
    find_all_jets = false;
}

void Vcb::executeEventFromParameter()
{
    Clear();
    if (!PassBaseLineSelection())
        return;
    InferONNX();
    std::string region_string = GetRegionString();
    std::string channel_string = GetChannelString(channel).Data();
    if (IsDATA)
    {
        FillHistogramsAtThisPoint(channel_string + "/" + region_string + "/" + "Central/data_obs" , 1.f);
        FillONNXRecoInfo(channel_string + "/" + region_string + "/" + "Central/data_obs", 1.f);
        if(n_b_tagged_jets >=3){
            FillHistogramsAtThisPoint(channel_string + "/" + "ThreeB" + "/" + "Central/data_obs" , 1.f);
            FillONNXRecoInfo(channel_string + "/" + "ThreeB" + "/" + "Central/data_obs", 1.f);
        }
        else{
            FillHistogramsAtThisPoint(channel_string + "/" + "TwoB" + "/" + "Central/data_obs" , 1.f);
            FillONNXRecoInfo(channel_string + "/" + "TwoB" + "/" + "Central/data_obs", 1.f);
        }
        return;
    }

    std::string sample_postfix = Sample_Shorthand[MCSample.Data()];
    if(MCSample.Contains("TT") && !MCSample.Contains("Vcb")) sample_postfix = sample_postfix + GetTTHFPostFix();

    unordered_map<std::string, float> weight_map = systHelper->calculateWeight();
    float default_weight = 1.f;
    default_weight *= MCNormalization();
    for (const auto &weight : weight_map)
    {
        FillHistogramsAtThisPoint(channel_string + "/" + region_string + "/" + weight.first + "/" + sample_postfix, weight.second * default_weight);
        FillONNXRecoInfo(channel_string + "/" + region_string + "/" + weight.first + "/" + sample_postfix, weight.second * default_weight);
        if(n_b_tagged_jets >=3){
            FillHistogramsAtThisPoint(channel_string + "/" + "ThreeB" + "/" + weight.first + "/" + sample_postfix, weight.second * default_weight);
            FillONNXRecoInfo(channel_string + "/" + "ThreeB" + "/" + weight.first + "/" + sample_postfix, weight.second * default_weight);
        }
        else{
            FillHistogramsAtThisPoint(channel_string + "/" + "TwoB" + "/" + weight.first + "/" + sample_postfix, weight.second * default_weight);
            FillONNXRecoInfo(channel_string + "/" + "TwoB" + "/" + weight.first + "/" + sample_postfix, weight.second * default_weight);
        }
    }
}

void Vcb::SetTTbarId()
{
    if (!IsDATA)
    {
        ttbj = (genTtbarId % 100 == 51 || genTtbarId % 100 == 52);
        ttbb = (genTtbarId % 100 >= 53 && genTtbarId % 100 <= 55);
        ttcc = (genTtbarId % 100 >= 41 && genTtbarId % 100 <= 45);
        ttLF = !(ttbj || ttbb || ttcc);
    }
}

void Vcb::CreateTrainingTree()
{
}

void Vcb::FillTrainingTree()
{
}

RVec<int> Vcb::FindTTbarJetIndices()
{
    RVec<int> iamnothing;
    return iamnothing;
}

void Vcb::FillKinematicFitterResult(const TString &histPrefix, float weight)
{
}

RVec<RVec<unsigned int>> Vcb::GetPermutations(const RVec<Jet> &jets)
{
    RVec<RVec<unsigned int>> iamnothing;
    return iamnothing;
}

void Vcb::SkimTree()
{
    Clear();
    RVec<TString> keeps = {"*"};
    RVec<TString> drops = {};
    NewTree("Events", keeps, drops);
    bool fail_all = true;
    RVec<Channel> all_channels = {Channel::MM, Channel::ME, Channel::EE, Channel::Mu, Channel::El, Channel::FH};
    for (const auto &ch : all_channels)
    {
        this->channel = ch;
        if (!CheckChannel())
            continue;
        for (const auto &syst_dummy : *systHelper)
        {
            Clear();
            leptons.clear();
            if (PassBaseLineSelection(true)) // no btagging cut for measure R
            {
                FillTrees();
                return; // Exit immediately upon success
            }
        }
    }

    // If no channel passed for all systematic variations
    return;
}

float Vcb::LeptonTriggerWeight(bool isEle, const Correction::variation syst, const TString &source)
{

    Correction::variation electronVariation = Correction::variation::nom;
    Correction::variation muonVariation = Correction::variation::nom;
    TString electronSystSource = "total";
    TString muonSystSource = "total";

    if (syst != Correction::variation::nom)
    {
        if (isEle)
        {
            electronVariation = syst;
            electronSystSource = source;
        }
        else
        {
            muonVariation = syst;
            muonSystSource = source;
        }
    }

    switch (channel)
    {
    case Channel::FH:
    {
        return 1.f;
    }
    case Channel::Mu:
    {
        return myCorr->GetMuonTriggerSF(Mu_Trigger_SF_Key[DataEra.Data()], lepton.Eta(), lepton.Pt(), muonVariation, muonSystSource);
    }
    case Channel::El:
    {
        return myCorr->GetElectronTriggerSF(El_Trigger_SF_Key[DataEra.Data()], lepton.Eta(), lepton.Pt(), electronVariation, electronSystSource);
    }
    case Channel::MM:
    {
        float num = 1.f;
        float den = 1.f;
        for (const auto &mu : Muons)
        {
            num *= (1.f - myCorr->GetMuonTriggerSF(Mu_Trigger_SF_Key[DataEra.Data()], mu.Eta(), mu.Pt(), muonVariation, muonSystSource));
            den *= (1.f - 1.f); // hardcoded: let MCEff = 1, DataEff = SF, MUON POG PLEASE GIVE ME EFFICIENCY
        }
        return (1.f - num) / (1.f - den);
    }
    case Channel::ME:
    {
        float num = 1.f;
        float den = 1.f;
        num *= (1.f - myCorr->GetMuonTriggerSF(Mu_Trigger_SF_Key[DataEra.Data()], Muons[0].Eta(), Muons[0].Pt(), muonVariation, muonSystSource));
        den *= (1.f - 1.f); // hardcoded: let MCEff = 1, DataEff = SF, MUON POG PLEASE GIVE ME EFFICIENCY
        num *= (1.f - myCorr->GetElectronTriggerDataEff(El_Trigger_SF_Key[DataEra.Data()], Electrons[0].Eta(), Electrons[0].Pt(), electronVariation, electronSystSource));
        den *= (1.f - myCorr->GetElectronTriggerMCEff(El_Trigger_SF_Key[DataEra.Data()], Electrons[0].Eta(), Electrons[0].Pt(), electronVariation, electronSystSource));
        return (1.f - num) / (1.f - den);
    }
    case Channel::EE:
    {
        float num = 1.f;
        float den = 1.f;
        for (const auto &el : Electrons)
        {
            num *= (1.f - myCorr->GetElectronTriggerDataEff(El_Trigger_SF_Key[DataEra.Data()], el.Eta(), el.Pt(), electronVariation, electronSystSource));
            den *= (1.f - myCorr->GetElectronTriggerMCEff(El_Trigger_SF_Key[DataEra.Data()], el.Eta(), el.Pt(), electronVariation, electronSystSource));
        }
        return (1.f - num) / (1.f - den);
    }
    default:
    {
        throw std::runtime_error("[Vcb::LeptonTriggerWeight] Invalid channel");
    }
    }
}

void Vcb::InferONNX()
{
}

void Vcb::FillONNXRecoInfo(const TString &histPrefix, float weight)
{
    
}