#include "MeasureJetTaggingEff.h"
#include "JetTaggingParameter.h"
//////////

MeasureJetTaggingEff::MeasureJetTaggingEff()
{
} // MeasureJetTaggingEff::MeasureJetTaggingEff

//////////

MeasureJetTaggingEff::~MeasureJetTaggingEff()
{
} // MeasureJetTaggingEff::~MeasureJetTaggingEff()

//////////

void MeasureJetTaggingEff::initializeAnalyzer()
{
    fChain->SetBranchStatus("*", 0);
    fChain->SetBranchStatus("Jet_*", 1);
    fChain->SetBranchStatus("nTrueInt", 1);
    fChain->SetBranchStatus("genWeight", 1);

    mcCorr = new MCCorrection(DataEra);
    TString datapath = getenv("DATA_DIR");
    TString btagpath = datapath + "/" + DataEra + "/BTag/";

    Taggers.clear();
    WPs.clear();

    Taggers.push_back(JetTagging::JetFlavTagger::DeepJet);
    Taggers.push_back(JetTagging::JetFlavTagger::ParticleNet);
    Taggers.push_back(JetTagging::JetFlavTagger::ParT);
    WPs.push_back(JetTagging::JetFlavTaggerWP::Loose);
    WPs.push_back(JetTagging::JetFlavTaggerWP::Medium);
    WPs.push_back(JetTagging::JetFlavTaggerWP::Tight);
    WPs.push_back(JetTagging::JetFlavTaggerWP::VeryTight);
    WPs.push_back(JetTagging::JetFlavTaggerWP::SuperTight);





    vec_etabins = {0.0, 0.8, 1.6, 2., 2.5};
    vec_ptbins = {20., 30., 50., 70., 100., 140., 200., 300., 600., 1000.}; // PT bins used in POG SF measurements
    // for average users, this binning will be sufficient.
    // but eta-dependence of efficiency can be larger for |eta|>~2, where track & muon detector information of jet constituents starts to get lost, which is critical in tagging.
    // precision analysis with high-eta b may use finer binnings there, but beware of small number of b-jets in high-eta, high-pt bins if you use ttbar sample; proper optimization of bin size should be studied.

    PtMax = vec_ptbins.at(vec_ptbins.size() - 1);
    NEtaBin = vec_etabins.size() - 1;
    NPtBin = vec_ptbins.size() - 1;

    etabins = new float[NEtaBin + 1];
    for (int i = 0; i < NEtaBin + 1; i++)
        etabins[i] = vec_etabins.at(i);
    ptbins = new float[NPtBin + 1];
    for (int i = 0; i < NPtBin + 1; i++)
        ptbins[i] = vec_ptbins.at(i);
} // void MeasureJetTaggingEff::initializeAnalyzer

//////////

void MeasureJetTaggingEff::executeEvent()
{

    Event ev = GetEvent();
    if(IsEventJetMapVetoed()) return;
    RVec<Jet> jets = GetJets("tightLepVeto", 20., 2.5);
    float weight = 1.;
    float w_Gen = MCweight();
    float w_Norm = ev.GetTriggerLumi("Full");
    float w_PU = mcCorr->GetPUWeight(ev.nTrueInt(), "nominal"); 
    weight *= w_Gen * w_Norm * w_PU;
    // tagging performance depends on PU, so it is better reweight to proper PU profile

    //==== code to measure btag efficiencies in TT MC
    //==== Reference : https://github.com/rappoccio/usercode/blob/Dev_53x/EDSHyFT/plugins/BTaggingEffAnalyzer.cc
    for (unsigned int ij = 0; ij < jets.size(); ij++)
    {
        TString flav = "B";
        if (fabs(jets.at(ij).hadronFlavour()) == 4)
            flav = "C";
        if (fabs(jets.at(ij).hadronFlavour()) == 0)
            flav = "Light";

        double this_Eta = fabs(jets.at(ij).Eta());                                // POG recommendation is to use |eta|
        double this_Pt = jets.at(ij).Pt() < PtMax ? jets.at(ij).Pt() : PtMax - 1; // put overflows in the last bin

        //==== First, fill the denominator
        FillHist("Jet_" + DataEra + "_eff_" + flav + "_denom", this_Eta, this_Pt, weight, NEtaBin, etabins, NPtBin, ptbins);

        for(unsigned int i_tag=0; i_tag < Taggers.size(); i_tag++){
            JetTagging::JetFlavTagger this_tagger = Taggers.at(i_tag);
            for(unsigned int i_wp=0; i_wp < WPs.size(); i_wp++){
                JetTagging::JetFlavTaggerWP this_wp = WPs.at(i_wp);
                mcCorr->SetTaggingParam(this_tagger, this_wp);
                float this_bTaggingCut = mcCorr->GetBTaggingWP();
                if (jets.at(ij).GetBTaggerResult(this_tagger) > this_bTaggingCut)
                    FillHist("Jet_B" + DataEra + "_" + JetTagging::GetTaggerCorrectionLibStr(this_tagger).Data() + "_" + JetTagging::GetTaggerCorrectionWPStr(this_wp).Data() + "_eff_" + flav + "_num", this_Eta, this_Pt, weight, NEtaBin, etabins, NPtBin, ptbins);
                //No XT and XXT for c-tagging
                if(this_wp == JetTagging::JetFlavTaggerWP::VeryTight) continue;
                if(this_wp == JetTagging::JetFlavTaggerWP::SuperTight) continue;
                float this_CvBCut = mcCorr->GetCTaggingWP().first;
                float this_CvLCut = mcCorr->GetCTaggingWP().second;
                if (jets.at(ij).GetCTaggerResult(this_tagger).first > this_CvBCut && jets.at(ij).GetCTaggerResult(this_tagger).second > this_CvLCut)
                    FillHist("Jet_C" + DataEra + "_" + JetTagging::GetTaggerCorrectionLibStr(this_tagger).Data() + "_" + JetTagging::GetTaggerCorrectionWPStr(this_wp).Data() + "_eff_" + flav + "_num", this_Eta, this_Pt, weight, NEtaBin, etabins, NPtBin, ptbins);
            }
        }
    }
} // void MeasureJetTaggingEff::executeEvent
