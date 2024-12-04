#include "Vcb_DL.h"

Vcb_DL::Vcb_DL() {}
Vcb_DL::~Vcb_DL() {}

void Vcb_DL::initializeAnalyzer()
{
    myCorr = new Correction(DataEra, IsDATA ? DataStream : MCSample, IsDATA);
    myCorr->SetTaggingParam(FlavTagger[DataEra.Data()], DL_BTag_WP);
    // cset_TriggerSF = CorrectionSet::from_file("/data6/Users/yeonjoon/SKNanoOutput/FullHadronicTriggerTnP/2022EE/SF.json");
}

int Vcb_DL::Unroller(RVec<Jet> &jets){
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

float Vcb_DL::CalculateEventWeight(bool include_tagging_weight)
{
    float weight = 1.;
    if (!IsDATA)
    {
        float puWeight = myCorr->GetPUWeight(ev.nTrueInt());
        float mcWeight = MCweight();
        float BTagShapeWeight = myCorr->GetBTaggingSF(Jets, JetTagging::JetTaggingSFMethod::shape);
        float LumiWeight = ev.GetTriggerLumi(Mu_Trigger[DataEra.Data()]); // hardcoded due to the all trigger are unprescaled
        float TopPtWeight = myCorr->GetTopPtReweight(AllGens);
        float LeptonWeight = 1.;
        if(channel == Vcb_DL::Channel::MM){
           for(auto &muon : Muons){
               LeptonWeight *= myCorr->GetMuonIDSF(Mu_ID_SF_key[DataEra.Data()], muon.Eta(), muon.Pt());
               LeptonWeight *= myCorr->GetMuonISOSF(Mu_Iso_SF_key[DataEra.Data()], muon.Eta(), muon.Pt());
               LeptonWeight *= myCorr->GetMuonTriggerSF(Mu_Trigger_SF_Key[DataEra.Data()], muon.Eta(), muon.Pt());
           }
        }
        else if(channel == Vcb_DL::Channel::EE){
            for(auto &electron : Electrons){
                LeptonWeight *= myCorr->GetElectronIDSF(El_ID_SF_Key[DataEra.Data()], electron.Eta(), electron.Pt());
                LeptonWeight *= myCorr->GetElectronRECOSF(electron.Eta(), electron.Pt());
                //LeptonWeight *= myCorr->GetElectronTriggerSF(electron);
            }
        }
        else{
            LeptonWeight *= myCorr->GetMuonIDSF(Mu_ID_SF_key[DataEra.Data()], Muons[0].Eta(), Muons[0].Pt());
            LeptonWeight *= myCorr->GetMuonISOSF(Mu_Iso_SF_key[DataEra.Data()], Muons[0].Eta(), Muons[0].Pt());
            LeptonWeight *= myCorr->GetMuonTriggerSF(Mu_Trigger_SF_Key[DataEra.Data()], Muons[0].Eta(), Muons[0].Pt());
            LeptonWeight *= myCorr->GetElectronIDSF(El_ID_SF_Key[DataEra.Data()], Electrons[0].Eta(), Electrons[0].Pt());
            LeptonWeight *= myCorr->GetElectronRECOSF(Electrons[0].Eta(), Electrons[0].Pt());
            //LeptonWeight *= myCorr->GetElectronTriggerSF(Electrons[0]);
        }
        weight *= puWeight * mcWeight * LumiWeight * LeptonWeight * TopPtWeight;
        if(include_tagging_weight) weight *= BTagShapeWeight;
    }
    return weight;
}

bool Vcb_DL::PassBaseLineSelection(bool remove_flavtagging_cut, TString syst)
{
    FillCutFlow(0);
    switch (channel)
    {
    case Vcb_DL::Channel::MM:
        if (!ev.PassTrigger(Mu_Trigger[DataEra.Data()]))
            return false;
        break;
    case Vcb_DL::Channel::EE:
        if (!ev.PassTrigger(El_Trigger[DataEra.Data()]))
            return false;
        break;
    case Vcb_DL::Channel::ME:
        if (!ev.PassTrigger(Mu_Trigger[DataEra.Data()]) && !ev.PassTrigger(El_Trigger[DataEra.Data()]))
            return false;
        break;
    default:
        break;
    } 
    FillCutFlow(1);
    if (!PassJetVetoMap(AllJets, AllMuons))
        return false;
    FillCutFlow(2);
    if (!PassMetFilter(AllJets, ev))
        return false;
    FillCutFlow(3);

    Jets = SelectJets(AllJets, Jet_ID, DL_Jet_Pt_cut, Jet_Eta_cut);
    Muons_Veto = SelectMuons(AllMuons, Muon_Veto_ID, Muon_Veto_Pt, Muon_Veto_Eta);
    Muons_Veto = SelectMuons(Muons_Veto, Muon_Veto_Iso, Muon_Veto_Pt, Muon_Veto_Eta);
    Muons = SelectMuons(AllMuons, Muon_Tight_ID, Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
    Muons = SelectMuons(Muons, Muon_Tight_Iso, Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
    Electrons_Veto = SelectElectrons(AllElectrons, Electron_Veto_ID, Electron_Veto_Pt, Electron_Veto_Eta);
    Electrons = SelectElectrons(AllElectrons, Electron_Tight_ID, Electron_Tight_Pt[DataEra.Data()], Electron_Tight_Eta);

    HT = GetHT(Jets);
    n_jets = Jets.size();

    if (n_jets < 4)
        return false;
    FillCutFlow(4);
    if (MET.Pt() < 40.) return false;
    FillCutFlow(5);
    for (const auto &jet : Jets)
    {
        if (jet.GetBTaggerResult(FlavTagger[DataEra.Data()]) > myCorr->GetBTaggingWP())
            n_b_tagged_jets++;
        if (jet.GetCTaggerResult(FlavTagger[DataEra.Data()]).first > myCorr->GetCTaggingWP().first &&
            jet.GetCTaggerResult(FlavTagger[DataEra.Data()]).second > myCorr->GetCTaggingWP().second)
            n_c_tagged_jets++;
        if (!IsDATA)
        {
            if (abs(jet.partonFlavour()) == 5)
                n_partonFlav_b_jets++;
            if (abs(jet.partonFlavour()) == 4)
                n_partonFlav_c_jets++;
        }
    }


    if (channel == Vcb_DL::Channel::MM)
    {
        if (!(Muons.size() == 2 && Electrons_Veto.size() == 0 && Muons_Veto.size() == 2)) return false;
        Leptons.push_back(Muons[0]);
        Leptons.push_back(Muons[1]);
    }
    else if (channel == Vcb_DL::Channel::ME)
    {
        if (!(Electrons.size() == 1 && Muons_Veto.size() == 1 && Electrons_Veto.size() == 1 && Muons.size() == 1)) return false;
        Leptons.push_back(Muons[0]);
        Leptons.push_back(Electrons[0]);
    }
    else if (channel == Vcb_DL::Channel::EE)
    {
        if (!(Electrons.size() == 2 && Muons_Veto.size() == 0 && Electrons_Veto.size() == 2)) return false;
        Leptons.push_back(Electrons[0]);
        Leptons.push_back(Electrons[1]);
    }
    Particle ZCand = Leptons[0] + Leptons[1];
    if (channel == Vcb_DL::Channel::MM || channel == Vcb_DL::Channel::EE)
    {
        if (fabs(ZCand.M() - Z_MASS) < 15.) return false;
        if (fabs(ZCand.M()) < 15.) return false;
        if (Leptons[0].Charge() * Leptons[1].Charge() > 0) return false;
    }
    FillCutFlow(6);
    if (n_b_tagged_jets < 2 && remove_flavtagging_cut) return false;
    FillCutFlow(7);
    return true;
}

void Vcb_DL::FillHistogramsAtThisPoint(const TString &histPrefix, float weight)
{
    FillHist(histPrefix + "/" + "MET", MET.Pt(), weight, 100, 0., 200.);
    FillHist(histPrefix + "/" + "HT", HT, weight, 200, 450., 2000.);
    FillHist(histPrefix + "/" + "n_jets", n_jets, weight, 10, 6., 16.);
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
    for(size_t i = 0; i < Leptons.size(); i++){
        FillHist(histPrefix + "/" + "Lepton_Pt_" + std::to_string(i), Leptons[i].Pt(), weight, 100, 0., 500.);
        FillHist(histPrefix + "/" + "Lepton_Eta_" + std::to_string(i), Leptons[i].Eta(), weight, 100, -2.5, 2.5);
        FillHist(histPrefix + "/" + "Lepton_Phi_" + std::to_string(i), Leptons[i].Phi(), weight, 100, -3.14, 3.14);
    }
    Particle ZCand = Leptons[0] + Leptons[1];
    FillHist(histPrefix + "/" + "ZCand_Mass", ZCand.M(), weight, 100, 0., 200.);
    FillHist(histPrefix + "/" + "ZCand_Pt", ZCand.Pt(), weight, 100, 0., 200.);
    FillHist(histPrefix + "/" + "ZCand_Eta", ZCand.Eta(), weight, 100, -2.5, 2.5);
    FillHist(histPrefix + "/" + "BvsC_Unrolled", Unroller(Jets), weight, 16, 0., 16.);
}

short Vcb_DL::GetPassedBTaggingWP(const Jet &jet)
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

short Vcb_DL::GetPassedCTaggingWP(const Jet &jet)
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

void Vcb_DL::executeEvent()
{
    if(HasFlag("MM")) channel = Vcb_DL::Channel::MM;
    if(HasFlag("ME")) channel = Vcb_DL::Channel::ME;
    if(HasFlag("EE")) channel = Vcb_DL::Channel::EE;

    AllMuons = GetAllMuons();
    AllElectrons = GetAllElectrons();
    AllJets = GetAllJets();
    AllGens = GetAllGens();
    AllGenJets = GetAllGenJets();
    ev = GetEvent();
    MET = ev.GetMETVector();
    executeEventFromParameter("nominal");
    n_b_tagged_jets = 0;
    n_c_tagged_jets = 0;
    n_partonFlav_b_jets = 0;
    n_partonFlav_c_jets = 0;
}

void Vcb_DL::executeEventFromParameter(TString syst)
{
    if (!IsDATA)
    {
        try
        {
            myCorr->METXYCorrection(MET, RunNumber, ev.nPVsGood(), Correction::XYCorrection_MetType::Type1PuppiMET);
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
    }

    if (!PassBaseLineSelection()) return;
    float weight = CalculateEventWeight();
    FillHistogramsAtThisPoint("BaseLine", weight);
}
