#include "MeasureJetTaggingR.h"
#include "JetTaggingParameter.h"
#include "VcbParameters.h"
//////////

MeasureJetTaggingR::MeasureJetTaggingR()
{
} // MeasureJetTaggingR::MeasureJetTaggingR

//////////

MeasureJetTaggingR::~MeasureJetTaggingR()
{
} // MeasureJetTaggingR::~MeasureJetTaggingR()

//////////

void MeasureJetTaggingR::SetAnalyzer(std::variant<Vcb_DL *, Vcb_FH *, Vcb_SL *> analyzer)
{

    // set channel
    if (std::holds_alternative<Vcb_DL *>(analyzer))
    {
        if (current_channel == "MM" || current_channel == "DL") // default channel is DL
            std::get<Vcb_DL *>(analyzer)->channel = Vcb::Channel::MM;
        else if (current_channel == "ME")
            std::get<Vcb_DL *>(analyzer)->channel = Vcb::Channel::ME;
        else if (current_channel == "EE")
            std::get<Vcb_DL *>(analyzer)->channel = Vcb::Channel::EE;
    }
    else if (std::holds_alternative<Vcb_SL *>(analyzer))
    {
        if (current_channel == "Mu" || current_channel == "SL") // default channel is SL
            std::get<Vcb_SL *>(analyzer)->channel = Vcb::Channel::Mu;
        else if (current_channel == "El")
            std::get<Vcb_SL *>(analyzer)->channel = Vcb::Channel::El;
    }
    else if (std::holds_alternative<Vcb_FH *>(analyzer))
    {
    }
    else
    {
        cout << "Invalid channel" << endl;
        exit(0);
    }

    current_analyzer = analyzer;
    std::visit([&](auto *analyzer)
               {
    if (analyzer) {
        analyzer->IsDATA = IsDATA;
        analyzer->DataEra = DataEra;
        analyzer->DataStream = DataStream;
        analyzer->SetEra(DataEra);
        analyzer->MCSample = MCSample;
        analyzer->xsec = xsec;
        analyzer->sumW = sumW;
        analyzer->sumSign = sumSign;
        analyzer->fChain = fChain;
        analyzer->Init();
        analyzer->initializeAnalyzer();
    } }, current_analyzer);
}

void MeasureJetTaggingR::initializeAnalyzer()
{

    myCorr = new Correction(DataEra, IsDATA ? DataStream : MCSample, IsDATA);
    myCorr->SetTaggingParam(JetTagging::JetFlavTagger::DeepJet, JetTagging::JetFlavTaggerWP::Medium); // Temporary Initialisation
    TString datapath = getenv("DATA_DIR");
    TString btagpath = datapath + "/" + DataEra + "/BTag/";
    if (IsDATA)
    {
        cout << "Why you run this code on DATA?" << endl;
        exit(0);
    }
    Taggers.clear();
    WPs.clear();

    Taggers.push_back(JetTagging::JetFlavTagger::DeepJet);
    Taggers.push_back(JetTagging::JetFlavTagger::ParticleNet);
    Taggers.push_back(JetTagging::JetFlavTagger::ParT);

    vec_njetbin = {4, 5, 6, 7, 8, 10, 12, 14, 20};
    vec_htbins = {80, 180, 230, 280, 330, 400, 500, 1000};
    vec_nTrueIntbin = {0, 20, 25, 30, 35, 40, 50, 70};
    vec_jet_ptbin = {20, 30, 50, 70, 90, 120, 150, 200, 300, 500};
    vec_jet_etabin = {0., 0.2, 0.4, 0.6, 0.8, 1.0, 1.2, 1.48, 1.80, 2.10, 2.40};

    n_vec_njetbin = vec_njetbin.size() - 1;
    n_vec_htbins = vec_htbins.size() - 1;
    n_vec_nTrueIntbin = vec_nTrueIntbin.size() - 1;

    if (HasFlag("El"))
        current_channel = "El";
    else if (HasFlag("Mu"))
        current_channel = "Mu";
    else if (HasFlag("MM"))
        current_channel = "MM";
    else if (HasFlag("EE"))
        current_channel = "EE";
    else if (HasFlag("ME"))
        current_channel = "ME";
    else if (HasFlag("DL"))
        current_channel = "DL";
    else if (HasFlag("SL"))
        current_channel = "SL";
    else
        current_channel = "FH";
    cout << "Will run on " << current_channel << " channel" << endl;

    if (current_channel == "FH")
        SetAnalyzer(new Vcb_FH());
    else if (current_channel == "Mu" || current_channel == "El" || current_channel == "SL")
        SetAnalyzer(new Vcb_SL());
    else if (current_channel == "MM" || current_channel == "ME" || current_channel == "EE" || current_channel == "DL")
        SetAnalyzer(new Vcb_DL());
    else
    {
        cout << "Invalid channel" << endl;
        exit(0);
    }

} // void MeasureJetTaggingR::initializeAnalyzer

//////////
void MeasureJetTaggingR::executeEvent()
{
    // Since We passed the fCain to the analyzer, our fChain is set one's address to analyzer's variable
    // therfore from this analyzer we can not access everyting from fChain
    std::visit([&](auto *analyzer)
               {
    if (analyzer) {
        analyzer->ev = analyzer->GetEvent();
        analyzer->AllJets = analyzer->GetAllJets();
        analyzer->AllMuons = analyzer->GetAllMuons();
        analyzer->AllElectrons = analyzer->GetAllElectrons();
        analyzer->AllGenJets = analyzer->GetAllGenJets();
        analyzer->AllGens = analyzer->GetAllGens();
        analyzer->MET = (analyzer)->ev.GetMETVector(Event::MET_Type::PUPPI);
        analyzer->Userflags = Userflags;
    } }, current_analyzer);
    makeIteratingChannels();
    executeEventFromParameter();
}

void MeasureJetTaggingR::makeIteratingChannels()
{
    if (current_channel == "FH")
        iterating_channels = {Vcb::Channel::FH};
    else if (current_channel == "Mu")
        iterating_channels = {Vcb::Channel::Mu};
    else if (current_channel == "El")
        iterating_channels = {Vcb::Channel::El};
    else if (current_channel == "MM")
        iterating_channels = {Vcb::Channel::MM};
    else if (current_channel == "ME")
        iterating_channels = {Vcb::Channel::ME};
    else if (current_channel == "EE")
        iterating_channels = {Vcb::Channel::EE};
    else if (current_channel == "DL")
        iterating_channels = {Vcb::Channel::MM, Vcb::Channel::ME, Vcb::Channel::EE};
    else if (current_channel == "SL")
        iterating_channels = {Vcb::Channel::Mu, Vcb::Channel::El};
    else
    {
        cout << "Invalid channel" << endl;
        exit(0);
    }
}

void MeasureJetTaggingR::executeEventFromParameter()
{
    string category = "total";
    unordered_map<string, float> weight_map;
    float HT = 1.;
    bool passBaseline;
    float nTrueInt = 1.;
    RVec<Jet> vec_sel_jet;

    for (const auto &ch : iterating_channels)
    {
        std::visit([&](auto *analyzer)
                   {
            if (analyzer) {
                for (const auto &syst_dummy : *analyzer->systHelper){
                    analyzer->channel = ch;
                    if(!analyzer->CheckChannel()) continue;
                    passBaseline = analyzer->PassBaseLineSelection(true); //true: remove_flavtagging_cut
                    if (!passBaseline) continue;
                    analyzer->SetSystematicLambda(true); //true: remove_flavtagging_sf
                    weight_map = analyzer->systHelper->calculateWeight();
                    vec_sel_jet = analyzer->Jets;

                    std::sort(vec_sel_jet.begin(), vec_sel_jet.end(), PtComparing);
                    
                    float default_weight = 1.;
                    default_weight = analyzer->MCNormalization(); 
                    for(const auto &weight_pair : weight_map){
                        for (unsigned int i_tagger = 0; i_tagger < Taggers.size(); i_tagger++)
                        {
                            float n_medium_btagged_jets = 0;
                            float n_medium_ctagged_jets = 0;

                            std::string this_syst = weight_pair.first;
                            float weight = weight_pair.second * default_weight;
                            myCorr->SetTaggingParam(Taggers[i_tagger], JetTagging::JetFlavTaggerWP::Medium); // We will use shape method, so WP is not important
                            //BTagWeight = myCorr->GetBTaggingSF(vec_sel_jet, JetTagging::JetTaggingSFMethod::shape);
                            //CTagWeight = myCorr->GetCTaggingSF(vec_sel_jet, JetTagging::JetTaggingSFMethod::shape);
             
                            std::vector<std::string> this_targets = analyzer->systHelper->get_targets_from_name(this_syst);
                            std::vector<std::string> this_sources = analyzer->systHelper->get_sources_from_name(this_syst);
                            Correction::variation this_variation = analyzer->systHelper->get_variation_from_name(this_syst);

                            std::string source_str = "central";
                            std::string variation_str = "";
                            bool is_btag_syst = false;
                            is_btag_syst = std::find(this_targets.begin(), this_targets.end(), "btag") != this_targets.end();

                            if(is_btag_syst){
                                // get index of "btag" in this_targets TODO: have to implementaion for ctag
                                int index = std::distance(this_targets.begin(), std::find(this_targets.begin(), this_targets.end(), "btag"));
                                
                                if(this_variation == Correction::variation::up){
                                    source_str = this_sources[index];
                                    variation_str = "up_";
                                }
                                else if (this_variation == Correction::variation::down){
                                    source_str = this_sources[index];
                                    variation_str = "down_";
                                }
                            }

                            for(const auto &this_jet : vec_sel_jet){
                                if(this_jet.GetBTaggerResult(Taggers[i_tagger]) > myCorr->GetBTaggingWP(Taggers[i_tagger], JetTagging::JetFlavTaggerWP::Medium))
                                    n_medium_btagged_jets++;
                                if(this_jet.GetCTaggerResult(Taggers[i_tagger]).first > myCorr->GetCTaggingWP(Taggers[i_tagger], JetTagging::JetFlavTaggerWP::Medium).first &&
                                    this_jet.GetCTaggerResult(Taggers[i_tagger]).second > myCorr->GetCTaggingWP(Taggers[i_tagger], JetTagging::JetFlavTaggerWP::Medium).second)
                                    n_medium_ctagged_jets++;

                                RVec<Jet> this_jet_vec = {this_jet};
                                float BTagWeight = 1.f;
                                float CTagWeight = 1.f;

                                if(is_btag_syst){
                                    BTagWeight = myCorr->GetBTaggingSF(this_jet_vec, JetTagging::JetTaggingSFMethod::shape, this_variation, source_str);
                                    CTagWeight = myCorr->GetCTaggingSF(this_jet_vec, JetTagging::JetTaggingSFMethod::shape, this_variation, source_str);
                                }
                                else{
                                    BTagWeight = myCorr->GetBTaggingSF(this_jet_vec, JetTagging::JetTaggingSFMethod::shape);
                                    CTagWeight = myCorr->GetCTaggingSF(this_jet_vec, JetTagging::JetTaggingSFMethod::shape);
                                }

                                int this_jet_partonFlavour = abs(this_jet.partonFlavour());

                                if(this_jet_partonFlavour == 5) this_jet_partonFlavour = 5;
                                else if(this_jet_partonFlavour == 4) this_jet_partonFlavour = 4;
                                else this_jet_partonFlavour = 0;

                                

                                FillHist("tagging#b##parton_flav#" + std::to_string(this_jet_partonFlavour) + "##era#" + DataEra.Data() + "##systematic#" + this_syst + "##Hist#2D##tagger#" + JetTagging::GetTaggerCorrectionLibStr(Taggers[i_tagger]).Data() + "##num##category#" + category + "##sample#" + Sample_Shorthand[MCSample.Data()] + "##variation#" + variation_str + source_str, this_jet.Pt(), abs(this_jet.Eta()), weight, vec_jet_ptbin, vec_jet_etabin);
                                FillHist("tagging#c##parton_flav#" + std::to_string(this_jet_partonFlavour) + "##era#" + DataEra.Data() + "##systematic#" + this_syst + "##Hist#2D##tagger#" + JetTagging::GetTaggerCorrectionLibStr(Taggers[i_tagger]).Data() + "##num##category#" + category + "##sample#" + Sample_Shorthand[MCSample.Data()] + "##variation#" + variation_str + source_str, this_jet.Pt(), abs(this_jet.Eta()), weight, vec_jet_ptbin, vec_jet_etabin);
                                FillHist("tagging#b##parton_flav#" + std::to_string(this_jet_partonFlavour) + "##era#" + DataEra.Data() + "##systematic#" + this_syst + "##Hist#2D##tagger#" + JetTagging::GetTaggerCorrectionLibStr(Taggers[i_tagger]).Data() + "##den##category#" + category + "##sample#" + Sample_Shorthand[MCSample.Data()] + "##variation#" + variation_str + source_str, this_jet.Pt(), abs(this_jet.Eta()), weight * BTagWeight, vec_jet_ptbin, vec_jet_etabin);
                                FillHist("tagging#c##parton_flav#" + std::to_string(this_jet_partonFlavour) + "##era#" + DataEra.Data() + "##systematic#" + this_syst + "##Hist#2D##tagger#" + JetTagging::GetTaggerCorrectionLibStr(Taggers[i_tagger]).Data() + "##den##category#" + category + "##sample#" + Sample_Shorthand[MCSample.Data()] + "##variation#" + variation_str + source_str, this_jet.Pt(), abs(this_jet.Eta()), weight * CTagWeight, vec_jet_ptbin, vec_jet_etabin);
                                
                                FillHist("parton_flav#" + std::to_string(this_jet_partonFlavour) + "##tagger#" + JetTagging::GetTaggerCorrectionLibStr(Taggers[i_tagger]).Data() +"_BvsC_" + this_syst, this_jet.GetBTaggerResult(Taggers[i_tagger]), weight, 100, 0.f, 1.f);
                                FillHist("parton_flav#" + std::to_string(this_jet_partonFlavour) + "##tagger#" + JetTagging::GetTaggerCorrectionLibStr(Taggers[i_tagger]).Data() +"_CvsB_" + this_syst, this_jet.GetCTaggerResult(Taggers[i_tagger]).first, weight, 100, 0.f, 1.f);
                                FillHist("parton_flav#" + std::to_string(this_jet_partonFlavour) + "##tagger#" + JetTagging::GetTaggerCorrectionLibStr(Taggers[i_tagger]).Data() +"_CvsL_" + this_syst, this_jet.GetCTaggerResult(Taggers[i_tagger]).second, weight, 100, 0.f, 1.f);

                                FillHist(JetTagging::GetTaggerCorrectionLibStr(Taggers[i_tagger]) + "/BTaggingWeight_" + this_syst, BTagWeight, 1.f, 100, 0.f, 3.f);
                                FillHist(JetTagging::GetTaggerCorrectionLibStr(Taggers[i_tagger]) + "/CTaggingWeight_" + this_syst, CTagWeight, 1.f, 100, 0.f, 3.f);
                                FillHist(JetTagging::GetTaggerCorrectionLibStr(Taggers[i_tagger]) + "/LeadingJet_BTaggingScore_BvsC_" + this_syst, vec_sel_jet[0].GetBTaggerResult(Taggers[i_tagger]), weight, 100, 0.f, 1.f);
                                FillHist(JetTagging::GetTaggerCorrectionLibStr(Taggers[i_tagger]) + "/LeadingJet_CTaggingScore_CvsB_" + this_syst, vec_sel_jet[0].GetCTaggerResult(Taggers[i_tagger]).first, weight, 100, 0.f, 1.f);
                                FillHist(JetTagging::GetTaggerCorrectionLibStr(Taggers[i_tagger]) + "/LeadingJet_CTaggingScore_CvsL_" + this_syst, vec_sel_jet[0].GetCTaggerResult(Taggers[i_tagger]).second, weight, 100, 0.f, 1.f);
                                
                            }
                        }
                    }
                }
            } }, current_analyzer);
    }
}
