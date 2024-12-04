#include "MeasureJetTaggingR.h"
#include "JetTaggingParameter.h"
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

    //set channel
    if (std::holds_alternative<Vcb_DL *>(analyzer))
    {
        if(current_channel == "MM") std::get<Vcb_DL *>(analyzer)->channel = Vcb_DL::Channel::MM;
        else if(current_channel == "ME") std::get<Vcb_DL *>(analyzer)->channel = Vcb_DL::Channel::ME;
        else if(current_channel == "EE") std::get<Vcb_DL *>(analyzer)->channel = Vcb_DL::Channel::EE;
    }
    else if (std::holds_alternative<Vcb_SL *>(analyzer))
    {
        if(current_channel == "Mu") std::get<Vcb_SL *>(analyzer)->channel = Vcb_SL::Channel::Mu;
        else if(current_channel == "El") std::get<Vcb_SL *>(analyzer)->channel = Vcb_SL::Channel::El;
    }
    else if (std::holds_alternative<Vcb_FH *>(analyzer))
    {
        
    }
    else
    {
        cout << "Invalid channel" << endl;
        exit(0);
    }
} 

void MeasureJetTaggingR::initializeAnalyzer()
{

    myCorr = new Correction(DataEra, IsDATA ? DataStream : MCSample, IsDATA);
    myCorr->SetTaggingParam(JetTagging::JetFlavTagger::DeepJet, JetTagging::JetFlavTaggerWP::Medium);//Temporary Initialisation
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

    n_vec_njetbin = vec_njetbin.size() - 1;
    n_vec_htbins = vec_htbins.size() - 1;
    n_vec_nTrueIntbin = vec_nTrueIntbin.size() - 1;

    if(HasFlag("El")) current_channel = "El";
    else if(HasFlag("Mu")) current_channel = "Mu";
    else if(HasFlag("MM")) current_channel = "MM";
    else if(HasFlag("EE")) current_channel = "EE";
    else if(HasFlag("ME")) current_channel = "ME";
    else current_channel = "FH";
    cout << "Will run on " << current_channel << " channel" << endl;
    if(current_channel == "FH") SetAnalyzer(new Vcb_FH());
    else if(current_channel == "Mu" || current_channel == "El") SetAnalyzer(new Vcb_SL());
    else if (current_channel == "MM" || current_channel == "ME" || current_channel == "EE") SetAnalyzer(new Vcb_DL());
    else{
        cout << "Invalid channel" << endl;
        exit(0);
    }

} // void MeasureJetTaggingR::initializeAnalyzer

//////////
void MeasureJetTaggingR::executeEvent()
{
    //Since We passed the fCain to the analyzer, our fChain is set one's address to analyzer's variable
    //therfore from this analyzer we can not access everyting from fChain
    std::visit([&](auto *analyzer)
               {
    if (analyzer) {
        analyzer->ev = analyzer->GetEvent();
        analyzer->AllJets = analyzer->GetAllJets();
        analyzer->AllMuons = analyzer->GetAllMuons();
        analyzer->AllElectrons = analyzer->GetAllElectrons();
        analyzer->AllGenJets = analyzer->GetAllGenJets();
        analyzer->AllGens = analyzer->GetAllGens();
        analyzer->MET = (analyzer)->ev.GetMETVector();
    } }, current_analyzer);

    executeEventFromParameter("nominal");
}

void MeasureJetTaggingR::executeEventFromParameter(TString syst){
    string category = "total";
    float weight = 1.;
    float HT = 1.;
    bool passBaseline;
    float nTrueInt = 1.;
    RVec<Jet> vec_sel_jet;
    std::visit([&](auto *analyzer)
               {
    if (analyzer) {
        std::cout << "Analyzer type: " << typeid(*analyzer).name() << std::endl;
        passBaseline = analyzer->PassBaseLineSelection();
        if (!passBaseline) return;
        weight = analyzer->CalculateEventWeight(false);
        vec_sel_jet = analyzer->Jets;
        HT = analyzer->HT;
        nTrueInt = analyzer->ev.nTrueInt();
    } }, current_analyzer);
    if(!passBaseline) return;

    std::sort(vec_sel_jet.begin(), vec_sel_jet.end(), PtComparing);


    

    for (unsigned int i_tagger = 0; i_tagger < Taggers.size(); i_tagger++)
    {
        float BTagWeight = 1.;
        float CTagWeight = 1.;
        myCorr->SetTaggingParam(Taggers[i_tagger], JetTagging::JetFlavTaggerWP::Medium); // We will use shape method, so WP is not important
        BTagWeight = myCorr->GetBTaggingSF(vec_sel_jet, JetTagging::JetTaggingSFMethod::shape);
        CTagWeight = myCorr->GetCTaggingSF(vec_sel_jet, JetTagging::JetTaggingSFMethod::shape);
        FillHist(string("tagging#b") + "##era#" + DataEra.Data() + "##systematic#central##Hist#1D##tagger#" + JetTagging::GetTaggerCorrectionLibStr(Taggers[i_tagger]).Data() + "##num##category#" + category, vec_sel_jet.size(), weight, vec_njetbin);
        FillHist(string("tagging#c") + "##era#" + DataEra.Data() + "##systematic#central##Hist#1D##tagger#" + JetTagging::GetTaggerCorrectionLibStr(Taggers[i_tagger]).Data() + "##num##category#" + category, vec_sel_jet.size(), weight, vec_njetbin);
        FillHist(string("tagging#b") + "##era#" + DataEra.Data() + "##systematic#central##Hist#2D##tagger#" + JetTagging::GetTaggerCorrectionLibStr(Taggers[i_tagger]).Data() + "##num##category#" + category, vec_sel_jet.size(), HT, weight, vec_njetbin, vec_htbins);
        FillHist(string("tagging#c") + "##era#" + DataEra.Data() + "##systematic#central##Hist#2D##tagger#" + JetTagging::GetTaggerCorrectionLibStr(Taggers[i_tagger]).Data() + "##num##category#" + category, nTrueInt, HT, weight, vec_nTrueIntbin, vec_htbins);
        FillHist(string("tagging#b") + "##era#" + DataEra.Data() + "##systematic#central##Hist#1D##tagger#" + JetTagging::GetTaggerCorrectionLibStr(Taggers[i_tagger]).Data() + "##den##category#" + category, vec_sel_jet.size(), weight * BTagWeight, vec_njetbin);
        FillHist(string("tagging#c") + "##era#" + DataEra.Data() + "##systematic#central##Hist#1D##tagger#" + JetTagging::GetTaggerCorrectionLibStr(Taggers[i_tagger]).Data() + "##den##category#" + category, vec_sel_jet.size(), weight * CTagWeight, vec_njetbin);
        FillHist(string("tagging#b") + "##era#" + DataEra.Data() + "##systematic#central##Hist#2D##tagger#" + JetTagging::GetTaggerCorrectionLibStr(Taggers[i_tagger]).Data() + "##den##category#" + category, vec_sel_jet.size(), HT, weight * BTagWeight, vec_njetbin, vec_htbins);
        FillHist(string("tagging#c") + "##era#" + DataEra.Data() + "##systematic#central##Hist#2D##tagger#" + JetTagging::GetTaggerCorrectionLibStr(Taggers[i_tagger]).Data() + "##den##category#" + category, nTrueInt, HT, weight * CTagWeight, vec_nTrueIntbin, vec_htbins);
        FillHist(JetTagging::GetTaggerCorrectionLibStr(Taggers[i_tagger]) + "_BTaggingWeight", BTagWeight, 1.f, 100, 0.f, 3.f);
        FillHist(JetTagging::GetTaggerCorrectionLibStr(Taggers[i_tagger]) + "_CTaggingWeight", CTagWeight, 1.f, 100, 0.f, 3.f);
        FillHist(JetTagging::GetTaggerCorrectionLibStr(Taggers[i_tagger]) + "_LeadingJet_BTaggingScore_BvsC", vec_sel_jet[0].GetBTaggerResult(Taggers[i_tagger]), weight, 100, 0.f, 1.f);
        FillHist(JetTagging::GetTaggerCorrectionLibStr(Taggers[i_tagger]) + "_LeadingJet_CTaggingScore_CvsB", vec_sel_jet[0].GetCTaggerResult(Taggers[i_tagger]).first, weight, 100, 0.f, 1.f);
        FillHist(JetTagging::GetTaggerCorrectionLibStr(Taggers[i_tagger]) + "_LeadingJet_CTaggingScore_CvsL", vec_sel_jet[0].GetCTaggerResult(Taggers[i_tagger]).second, weight, 100, 0.f, 1.f);
    }
}
