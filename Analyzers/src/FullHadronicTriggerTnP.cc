#include "FullHadronicTriggerTnP.h"

FullHadronicTriggerTnP::FullHadronicTriggerTnP() {}
FullHadronicTriggerTnP::~FullHadronicTriggerTnP() {}

void FullHadronicTriggerTnP::initializeAnalyzer()
{
    myCorr = new MyCorrection(DataEra, IsDATA?DataStream:MCSample ,IsDATA);
}


void FullHadronicTriggerTnP::executeEvent()
{
    AllMuons = GetAllMuons();
    AllElectrons = GetAllElectrons();
    AllJets = GetAllJets();
    AllGens = GetAllGens();
    AllGenJets = GetAllGenJets();
    FillCutFlow(0, 10);
    ControlTrigger = "HLT_PFHT350";
    SingleBTagTrigger = "HLT_PFHT450_SixPFJet36_PFBTagDeepJet_1p59";
    DoubleBTagTrigger = "HLT_PFHT400_SixPFJet32_DoublePFBTagDeepJet_2p94";
    ev = GetEvent();
    // Met Filter
    if (!PassMetFilter(AllJets, ev))
    {
        return;
    }
    FillCutFlow(1, 10);
    // jet vetomap
    if (!PassJetVetoMap(AllJets, AllMuons))
    {
        return;
    }
    FillCutFlow(2, 10);
    RVec<TString> systs = {"nominal", "JESUp", "JESDown","Unsmeared"};
    if (IsDATA)
        systs = {"nominal"};

    for (const auto &syst : systs)
    {
        executeEventFromParameter(syst);
    }
}
void FullHadronicTriggerTnP::executeEventFromParameter(TString syst)
{
    RVec<Muon> this_muon = SelectMuons(AllMuons, "POGLoose", 10., 2.4);
    RVec<Electron> this_electron = SelectElectrons(AllElectrons, "POGVeto", 10., 2.4);

    RVec<Jet> this_systjet;
    RVec<Jet> this_jet = JetsVetoLeptonInside(AllJets, this_electron, this_muon, 0.3);
    if (!IsDATA)
    {
        this_systjet = SmearJets(this_jet, AllGenJets);
    }
    else
        this_systjet = this_jet;
    if (syst == "nominal")
    {
    }
    else if (syst == "Unsmeared")
    {
        this_systjet = this_jet;
    }
    else if (syst == "JESUp")
    {
        this_systjet = ScaleJets(this_jet, MyCorrection::variation::up);
    }
    else if (syst == "JESDown")
    {
        this_systjet = ScaleJets(this_jet, MyCorrection::variation::down);
    }
    else
    {
        return;
    }
    this_systjet = SelectJets(this_systjet, "tight", 0., 2.4);
    float HT = 0.f;
    for (const auto &jet : this_systjet)
    {
        HT += jet.Pt();
    }
    sort(this_systjet.begin(), this_systjet.end(), PtComparing);
    if (this_systjet.size() < 6)
    {
        return;
    }
    RVec<JetTagging::JetFlavTaggerWP> WPs = {JetTagging::JetFlavTaggerWP::Loose, JetTagging::JetFlavTaggerWP::Medium, JetTagging::JetFlavTaggerWP::Tight, JetTagging::JetFlavTaggerWP::VeryTight, JetTagging::JetFlavTaggerWP::SuperTight};
    for(const auto &wp : WPs)
    {
        passOneBTag[wp] = false;
        passTwoBTag[wp] = false;
    }
    for(const auto &wp : WPs)
    {
        int nBTag = 0;
        for (const auto &jet : this_systjet)
        {
            if (jet.GetBTaggerResult(JetTagging::JetFlavTagger::DeepJet) > myCorr->GetBTaggingWP(JetTagging::JetFlavTagger::DeepJet, wp))
            {
                nBTag++;
            }
        }
        if (nBTag >= 1)
        {
            passOneBTag[wp] = true;
        }
        if (nBTag >= 2)
        {
            passTwoBTag[wp] = true;
        }
    }
    if (this_muon.size() != 0 && this_electron.size() != 0)
    {
        return;
    }
    float weight = 1.f;

    if (!IsDATA){
        weight *= myCorr->GetPUWeight(ev.nTrueInt());
        weight *= MCweight();
    }

    for(const auto &wp : WPs)
    {
        if (passOneBTag[wp])
        {
            FillHist(syst + "/" + JetTagging::GetTaggerCorrectionWPStr(wp) + "/" + SingleBTagTrigger + "/" + "BaseLine_SixthJetPt", this_systjet[5].Pt(), weight, 100, 0.f, 200.f);
            FillHist(syst + "/" + JetTagging::GetTaggerCorrectionWPStr(wp) + "/" + SingleBTagTrigger + "/" + "BaseLine_HT", HT, weight, 100, 0.f, 2000.f);
            FillHist(syst + "/" + JetTagging::GetTaggerCorrectionWPStr(wp) + "/" + SingleBTagTrigger + "/" + "BaseLine_SixthJetPt_VS_HT", this_systjet[5].Pt(), HT, weight, 100, 0.f, 200.f, 100, 0.f, 2000.f);
        }
        if (passTwoBTag[wp])
        {
            FillHist(syst + "/" + JetTagging::GetTaggerCorrectionWPStr(wp) + "/" + DoubleBTagTrigger + "/" + "BaseLine_SixthJetPt", this_systjet[5].Pt(), weight, 100, 0.f, 200.f);
            FillHist(syst + "/" + JetTagging::GetTaggerCorrectionWPStr(wp) + "/" + DoubleBTagTrigger + "/" + "BaseLine_HT", HT, weight, 100, 0.f, 2000.f);
            FillHist(syst + "/" + JetTagging::GetTaggerCorrectionWPStr(wp) + "/" + DoubleBTagTrigger + "/" + "BaseLine_SixthJetPt_VS_HT", this_systjet[5].Pt(), HT, weight, 100, 0.f, 200.f, 100, 0.f, 2000.f);
        }
    }


    if(!ev.PassTrigger(ControlTrigger)){
        return;
    }

    for(const auto &wp : WPs)
    {
        if (passOneBTag[wp])
        {
            FillHist(syst + "/" + JetTagging::GetTaggerCorrectionWPStr(wp) + "/" + SingleBTagTrigger + "/" + "Control_SixthJetPt", this_systjet[5].Pt(), weight, 100, 0.f, 200.f);
            FillHist(syst + "/" + JetTagging::GetTaggerCorrectionWPStr(wp) + "/" + SingleBTagTrigger + "/" + "Control_HT", HT, weight, 100, 0.f, 2000.f);
            FillHist(syst + "/" + JetTagging::GetTaggerCorrectionWPStr(wp) + "/" + SingleBTagTrigger + "/" + "Control_SixthJetPt_VS_HT", this_systjet[5].Pt(), HT, weight, 100, 0.f, 200.f, 100, 0.f, 2000.f);
        }
        if (passTwoBTag[wp])
        {
            FillHist(syst + "/" + JetTagging::GetTaggerCorrectionWPStr(wp) + "/" + DoubleBTagTrigger + "/" + "Control_SixthJetPt", this_systjet[5].Pt(), weight, 100, 0.f, 200.f);
            FillHist(syst + "/" + JetTagging::GetTaggerCorrectionWPStr(wp) + "/" + DoubleBTagTrigger + "/" + "Control_HT", HT, weight, 100, 0.f, 2000.f);
            FillHist(syst + "/" + JetTagging::GetTaggerCorrectionWPStr(wp) + "/" + DoubleBTagTrigger + "/" + "Control_SixthJetPt_VS_HT", this_systjet[5].Pt(), HT, weight, 100, 0.f, 200.f, 100, 0.f, 2000.f);
        }
    }

    if(!ev.PassTrigger(SingleBTagTrigger) && !ev.PassTrigger(DoubleBTagTrigger)){
        return;
    }

    for(const auto &wp : WPs)
    {
        if (passOneBTag[wp] && ev.PassTrigger(SingleBTagTrigger))
        {
            FillHist(syst + "/" + JetTagging::GetTaggerCorrectionWPStr(wp) + "/" + SingleBTagTrigger + "/" + "Signal_SixthJetPt", this_systjet[5].Pt(), weight, 100, 0.f, 200.f);
            FillHist(syst + "/" + JetTagging::GetTaggerCorrectionWPStr(wp) + "/" + SingleBTagTrigger + "/" + "Signal_HT", HT, weight, 100, 0.f, 2000.f);
            FillHist(syst + "/" + JetTagging::GetTaggerCorrectionWPStr(wp) + "/" + SingleBTagTrigger + "/" + "Signal_SixthJetPt_VS_HT", this_systjet[5].Pt(), HT, weight, 100, 0.f, 200.f, 100, 0.f, 2000.f);
        }
        if (passTwoBTag[wp] && ev.PassTrigger(DoubleBTagTrigger))
        {
            FillHist(syst + "/" + JetTagging::GetTaggerCorrectionWPStr(wp) + "/" + DoubleBTagTrigger + "/" + "Signal_SixthJetPt", this_systjet[5].Pt(), weight, 100, 0.f, 200.f);
            FillHist(syst + "/" + JetTagging::GetTaggerCorrectionWPStr(wp) + "/" + DoubleBTagTrigger + "/" + "Signal_HT", HT, weight, 100, 0.f, 2000.f);
            FillHist(syst + "/" + JetTagging::GetTaggerCorrectionWPStr(wp) + "/" + DoubleBTagTrigger + "/" + "Signal_SixthJetPt_VS_HT", this_systjet[5].Pt(), HT, weight, 100, 0.f, 200.f, 100, 0.f, 2000.f);
        }
    }
}
