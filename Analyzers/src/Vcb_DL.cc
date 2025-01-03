#include "Vcb_DL.h"

Vcb_DL::Vcb_DL() {}

bool Vcb_DL::PassBaseLineSelection(bool remove_flavtagging_cut)
{
    FillCutFlow(0);
    // if (!IsDATA)
    // {
    //     try
    //     {
    //         myCorr->METXYCorrection(MET, RunNumber, ev.nPVsGood(), Correction::XYCorrection_MetType::Type1PuppiMET);
    //     }
    //     catch (const std::exception &e)
    //     {
    //         std::cerr << e.what() << '\n';
    //     }
    // }
    bool passMuTrig = ev.PassTrigger(Mu_Trigger[DataEra.Data()]);
    bool passElTrig = ev.PassTrigger(El_Trigger[DataEra.Data()]);

    switch (channel)
    {
    case Channel::MM:
        if (!(passMuTrig || passElTrig))
            return false;
        break;
    case Channel::EE:
        if (!(passElTrig || passMuTrig))
            return false;
        break;
    case Channel::ME:
        //subtract passMuTrig && passElTrig from electron datastream
        if (IsDATA && (DataStream.Contains("EG") || DataStream.Contains("Electron")) && (passMuTrig && passElTrig))
            return false;
        if (!(passMuTrig || passElTrig))
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
    if (systHelper->getCurrentIterSysSource() == "Jet_En")
    {
        Jets = ScaleJets(AllJets, systHelper->getCurrentIterVariation());
        MET = ev.GetMETVector(Event::MET_Type::PUPPI, systHelper->getCurrentIterVariation(), Event::MET_Syst::JES);
    }
    if (systHelper->getCurrentIterSysSource() == "Jet_Res")
    {
        Jets = SmearJets(AllJets, AllGenJets, systHelper->getCurrentIterVariation());
        MET = ev.GetMETVector(Event::MET_Type::PUPPI, systHelper->getCurrentIterVariation(), Event::MET_Syst::JER);
    }
    if (systHelper->getCurrentIterSysSource() == "UE")
    {
        MET = ev.GetMETVector(Event::MET_Type::PUPPI, systHelper->getCurrentIterVariation(), Event::MET_Syst::UE);
    }
    Jets = SelectJets(Jets, Jet_ID, DL_Jet_Pt_cut, Jet_Eta_cut);
    

    MET = ev.GetMETVector(Event::MET_Type::PUPPI);
    Muons_Veto = SelectMuons(AllMuons, Muon_Veto_ID, Muon_Veto_Pt, Muon_Veto_Eta);
    Muons_Veto = SelectMuons(Muons_Veto, Muon_Veto_Iso, Muon_Veto_Pt, Muon_Veto_Eta);
    Muons = SelectMuons(AllMuons, Muon_Tight_ID, Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
    Muons = SelectMuons(Muons, Muon_Tight_Iso, Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
    Electrons_Veto = SelectElectrons(AllElectrons, Electron_Veto_ID, Electron_Veto_Pt, Electron_Veto_Eta);
    Electrons = SelectElectrons(AllElectrons, Electron_Tight_ID, Electron_Tight_Pt[DataEra.Data()], Electron_Tight_Eta);
    Jets = JetsVetoLeptonInside(Jets, Electrons_Veto, Muons_Veto, Jet_Veto_DR);
    Jets = SelectJets(Jets, Jet_PUID, DL_Jet_Pt_cut, Jet_Eta_cut);

    HT = GetHT(Jets);
    n_jets = Jets.size();

    if (n_jets < 4)
        return false;
    FillCutFlow(4);
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


    if (channel == Channel::MM)
    {
        if (!(Muons.size() == 2 && Electrons_Veto.size() == 0 && Muons_Veto.size() == 2)) return false;
        leptons.push_back(Muons[0]);
        leptons.push_back(Muons[1]);
    }
    else if (channel == Channel::ME)
    {
        if (!(Electrons.size() == 1 && Muons_Veto.size() == 1 && Electrons_Veto.size() == 1 && Muons.size() == 1)) return false;
        leptons.push_back(Muons[0]);
        leptons.push_back(Electrons[0]);
    }
    else if (channel == Channel::EE)
    {
        if (!(Electrons.size() == 2 && Muons_Veto.size() == 0 && Electrons_Veto.size() == 2)) return false;
        leptons.push_back(Electrons[0]);
        leptons.push_back(Electrons[1]);
    }
    Particle ZCand = leptons[0] + leptons[1];
    if (channel == Channel::MM || channel == Channel::EE)
    {
        if (fabs(ZCand.M() - Z_MASS) < 15.) return false;
        if (fabs(ZCand.M()) < 15.) return false;
    }
    if (MET.Pt() < 40.)
        return false;
    if (leptons[0].Charge() * leptons[1].Charge() > 0)
        return false;
    FillCutFlow(6);
    if (n_b_tagged_jets < 2 && !remove_flavtagging_cut) return false;
    FillCutFlow(7);
    SetSystematicLambda();
    SetTTbarId();
    return true;
}