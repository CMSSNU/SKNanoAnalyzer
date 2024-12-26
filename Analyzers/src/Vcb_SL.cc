#include "Vcb_SL.h"

Vcb_SL::Vcb_SL() {}

void Vcb_SL::CreateTrainingTree()
{
    RVec<TString> keeps = {};
    RVec<TString> drops = {"*"};
    NewTree("Training_Tree", keeps, drops);
    GetTree("Training_Tree")->Branch("Jet_Px", &Jet_Px);
    GetTree("Training_Tree")->Branch("Jet_Py", &Jet_Py);
    GetTree("Training_Tree")->Branch("Jet_Pz", &Jet_Pz);
    GetTree("Training_Tree")->Branch("Jet_E", &Jet_E);
    GetTree("Training_Tree")->Branch("Jet_M", &Jet_M);
    GetTree("Training_Tree")->Branch("Jet_BvsC", &Jet_BvsC);
    GetTree("Training_Tree")->Branch("Jet_CvsB", &Jet_CvsB);
    GetTree("Training_Tree")->Branch("Jet_CvsL", &Jet_CvsL);
    GetTree("Training_Tree")->Branch("Jet_QvsG", &Jet_QvsG);
    GetTree("Training_Tree")->Branch("Jet_B_WP", &Jet_B_WP);
    GetTree("Training_Tree")->Branch("Jet_C_WP", &Jet_C_WP);
    GetTree("Training_Tree")->Branch("Jet_isTTbarJet", &Jet_isTTbarJet);
    GetTree("Training_Tree")->Branch("Jet_ttbarJet_idx", &Jet_ttbarJet_idx);
    GetTree("Training_Tree")->Branch("Jet_Pt", &Jet_Pt);
    GetTree("Training_Tree")->Branch("Jet_Eta", &Jet_Eta);
    GetTree("Training_Tree")->Branch("Jet_Phi", &Jet_Phi);
    GetTree("Training_Tree")->Branch("edge_index_jet_jet0", &edge_index_jet_jet0);
    GetTree("Training_Tree")->Branch("edge_index_jet_jet1", &edge_index_jet_jet1);
    GetTree("Training_Tree")->Branch("deltaR_jet_jet", &deltaR_jet_jet);
    GetTree("Training_Tree")->Branch("invM_jet_jet", &invM_jet_jet);
    GetTree("Training_Tree")->Branch("cosTheta_jet_jet", &cosTheta_jet_jet);
    GetTree("Training_Tree")->Branch("edge_index_jet_lepton0", &edge_index_jet_lepton0);
    GetTree("Training_Tree")->Branch("edge_index_jet_lepton1", &edge_index_jet_lepton1);
    GetTree("Training_Tree")->Branch("deltaR_jet_lepton", &deltaR_jet_lepton);
    GetTree("Training_Tree")->Branch("invM_jet_lepton", &invM_jet_lepton);
    GetTree("Training_Tree")->Branch("cosTheta_jet_lepton", &cosTheta_jet_lepton);
    GetTree("Training_Tree")->Branch("edge_index_jet_neutrino0", &edge_index_jet_neutrino0);
    GetTree("Training_Tree")->Branch("edge_index_jet_neutrino1", &edge_index_jet_neutrino1);
    GetTree("Training_Tree")->Branch("deltaR_jet_neutrino", &deltaR_jet_neutrino);
    GetTree("Training_Tree")->Branch("invM_jet_neutrino", &invM_jet_neutrino);
    GetTree("Training_Tree")->Branch("cosTheta_jet_neutrino", &cosTheta_jet_neutrino);
    GetTree("Training_Tree")->Branch("edge_index_lepton_neutrino0", &edge_index_lepton_neutrino0);
    GetTree("Training_Tree")->Branch("edge_index_lepton_neutrino1", &edge_index_lepton_neutrino1);
    GetTree("Training_Tree")->Branch("deltaR_lepton_neutrino", &deltaR_lepton_neutrino);
    GetTree("Training_Tree")->Branch("invM_lepton_neutrino", &invM_lepton_neutrino);
    GetTree("Training_Tree")->Branch("cosTheta_lepton_neutrino", &cosTheta_lepton_neutrino);
    GetTree("Training_Tree")->Branch("parton_jet_assignment", &parton_jet_assignment);
}

RVec<RVec<unsigned int>> Vcb_SL::GetPermutations(const RVec<Jet> &jets)
{
    RVec<RVec<unsigned int>> permutations;
    std::vector<unsigned int> b_tagged_idx;
    int max_jet = 8;
    if (jets.size() < 8)
        max_jet = jets.size();
    for (unsigned int i = 0; i < max_jet; i++)
    {
        if (jets[i].GetBTaggerResult(FlavTagger[DataEra.Data()]) > myCorr->GetBTaggingWP(FlavTagger[DataEra.Data()], JetTagging::JetFlavTaggerWP::Medium))
        {
            b_tagged_idx.push_back(i);
        }
    }
    // Permutations rules:
    // idx 0 and 1 are b jets from top decay. b-tagged jets only.
    // idx 2 and 3 are w jets from hadronic top decay. they are interchangeable.

    std::vector<std::pair<unsigned int, unsigned int>> b_tagged_pairs;

    for (unsigned int i = 0; i < b_tagged_idx.size(); i++)
    {
        for (unsigned int j = 0; j < b_tagged_idx.size(); j++) // there is ordering of b-jet in Semileptonic channel
        {
            if (i == j)
                continue;
            b_tagged_pairs.push_back(std::make_pair(b_tagged_idx[i], b_tagged_idx[j]));
        }
    }

    for (auto &pair : b_tagged_pairs)
    {

        std::vector<unsigned int> remaining_idx;
        std::vector<std::vector<unsigned int>> w_pairs;

        for (unsigned int i = 0; i < max_jet; i++)
        {
            if (i != pair.first && i != pair.second)
            {
                remaining_idx.push_back(i);
            }
        }
        // make all permutations of remaining_idx using next_permutation
        do
        {
            // push back first 4 elements
            w_pairs.push_back({remaining_idx[0], remaining_idx[1]});
        } while (std::next_permutation(remaining_idx.begin(), remaining_idx.end()));
        for (auto &w_pair : w_pairs)
        {
            std::sort(w_pair.begin(), w_pair.end());
        }
        // remove duplicates
        std::sort(w_pairs.begin(), w_pairs.end());
        w_pairs.erase(std::unique(w_pairs.begin(), w_pairs.end()), w_pairs.end());
        for (auto &w_pair : w_pairs)
        {
            // push back the b jet pair and w jet pairs
            std::vector<unsigned int> permutation;
            permutation.push_back(pair.first);
            permutation.push_back(pair.second);
            permutation.insert(permutation.end(), w_pair.begin(), w_pair.end());
            permutations.push_back(permutation);
        }
    }
    return permutations;
}

std::variant<float, std::pair<float, float>> Vcb_SL::SolveNeutrinoPz(const Lepton &lepton, const Particle &met)
{
    float Ptl_dot_Ptnu = lepton.Px() * met.Px() + lepton.Py() * met.Py();
    float lepton_mass = lepton.M();

    // solve a*x^2 + b*x + c = 0, where x = pz of neutrino
    float k = TMath::Power(W_MASS, 2.) / 2.0 - lepton_mass * lepton_mass / 2.0 + Ptl_dot_Ptnu;
    float a = TMath::Power(lepton.Pt(), 2.0);
    float b = -2 * k * lepton.Pz();
    float c = TMath::Power(lepton.Pt(), 2.0) * TMath::Power(met.Pt(), 2.0) - TMath::Power(k, 2.0);

    float determinant = TMath::Power(b, 2.f) - 4.f * a * c;
    if (determinant < 0)
    {
        float real_pz = -b / (2.f * a);
        return real_pz;
    }
    else
    {
        float pz1 = (-b + TMath::Sqrt(determinant)) / (2.f * a);
        float pz2 = (-b - TMath::Sqrt(determinant)) / (2.f * a);
        return std::make_pair(pz1, pz2);
    }
}

bool Vcb_SL::PassBaseLineSelection(bool remove_flavtagging_cut)
{
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
    Clear();
    FillCutFlow(0);
    if (channel == Channel::Mu)
        if (!ev.PassTrigger(Mu_Trigger[DataEra.Data()]))
            return false;
    if (channel == Channel::El)
        if (!ev.PassTrigger(El_Trigger[DataEra.Data()]))
            return false;
    FillCutFlow(1);
    if (!PassJetVetoMap(AllJets, AllMuons))
        return false;
    FillCutFlow(2);
    if (!PassMetFilter(AllJets, ev))
        return false;
    FillCutFlow(3);

    Jets = SelectJets(AllJets, Jet_ID, SL_Jet_Pt_cut, Jet_Eta_cut);
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
    Jets = SelectJets(Jets, Jet_ID, SL_Jet_Pt_cut, Jet_Eta_cut);

    MET = ev.GetMETVector(Event::MET_Type::PUPPI);
    std::sort(Jets.begin(), Jets.end(), PtComparing);
    Muons_Veto = SelectMuons(AllMuons, Muon_Veto_ID, Muon_Veto_Pt, Muon_Veto_Eta);
    Muons_Veto = SelectMuons(Muons_Veto, Muon_Veto_Iso, Muon_Veto_Pt, Muon_Veto_Eta);
    Muons = SelectMuons(AllMuons, Muon_Tight_ID, Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
    Muons = SelectMuons(Muons, Muon_Tight_Iso, Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
    Electrons_Veto = SelectElectrons(AllElectrons, Electron_Veto_ID, Electron_Veto_Pt, Electron_Veto_Eta);
    Electrons = SelectElectrons(Electrons_Veto, Electron_Tight_ID, Electron_Tight_Pt[DataEra.Data()], Electron_Tight_Eta);
    Jets = JetsVetoLeptonInside(Jets, Electrons_Veto, Muons_Veto, Jet_Veto_DR);

    HT = GetHT(Jets);
    n_jets = Jets.size();

    if (n_jets < 4)
        return false;
    FillCutFlow(4);
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
    if (channel == Channel::Mu)
    {
        if (!(Muons.size() == 1 && Electrons_Veto.size() == 0 && Muons_Veto.size() == 1))
            return false;
        lepton = Muons[0];
        leptons.push_back(lepton);
    }
    else if (channel == Channel::El)
    {
        if (!(Electrons.size() == 1 && Muons_Veto.size() == 0 && Electrons_Veto.size() == 1))
            return false;
        lepton = Electrons[0];
        leptons.push_back(lepton);
    }
    FillCutFlow(5);
    if (n_b_tagged_jets < 2 && !remove_flavtagging_cut)
        return false;
    FillCutFlow(6);
    SetTTbarId();
    SetSystematicLambda();
    RVec<int> ttbar_jet_indices = FindTTbarJetIndices();
    return true;
}

void Vcb_SL::FillKinematicFitterResult(const TString &histPrefix, float weight)
{
    Particle fitted_lep_w = best_KF_result.fitted_lep + best_KF_result.fitted_neu;
    Particle fitted_had_w = best_KF_result.fitted_had_w1 + best_KF_result.fitted_had_w2;
    Particle fitted_lep_top = fitted_lep_w + best_KF_result.fitted_lep_t_b;
    Particle fitted_had_top = fitted_had_w + best_KF_result.fitted_had_t_b;
    Particle had_w = Jets[best_KF_result.best_had_w1_idx] + Jets[best_KF_result.best_had_w2_idx];
    TLorentzVector neutrino_p4;
    neutrino_p4.SetXYZM(MET.Px(), MET.Py(), best_KF_result.best_neu_pz, 0.);
    Particle neutrino(neutrino_p4);
    Particle lep_w = lepton + neutrino;
    Particle lep_top = lep_w + Jets[best_KF_result.best_lep_t_b_idx];
    Particle had_top = had_w + Jets[best_KF_result.best_had_t_b_idx];

    FillHist(histPrefix + "/" + "had_top_mass", had_top.M(), weight, 100, 100., 300.);
    FillHist(histPrefix + "/" + "lep_top_mass", lep_top.M(), weight, 100, 100., 300.);
    FillHist(histPrefix + "/" + "had_w_mass", had_w.M(), weight, 200, 50., 200.);
    FillHist(histPrefix + "/" + "lep_w_mass", lep_w.M(), weight, 100, 50., 100.);
    FillHist(histPrefix + "/" + "Fitted_had_top_mass", fitted_had_top.M(), weight, 100, 100., 300.);
    FillHist(histPrefix + "/" + "Fitted_lep_top_mass", fitted_lep_top.M(), weight, 100, 100., 300.);
    FillHist(histPrefix + "/" + "Fitted_had_w_mass", fitted_had_w.M(), weight, 100, 50., 100.);
    FillHist(histPrefix + "/" + "Fitted_lep_w_mass", fitted_lep_w.M(), weight, 100, 50., 100.);
    FillHist(histPrefix + "/" + "Chi2", best_KF_result.chi2, weight, 600, 0., 300.);
}

void Vcb_SL::FillTrainingTree()
{
    ttbar_jet_indices = FindTTbarJetIndices();

    // GetKineMaticFitterResult(Jets, MET, lepton);
    float weight = MCNormalization();
    weight *= systHelper->calculateWeight()["Central"];

    SetBranch("Training_Tree", "MET", MET.Pt());
    SetBranch("Training_Tree", "HT", HT);
    SetBranch("Training_Tree", "n_jets", n_jets);
    SetBranch("Training_Tree", "n_b_tagged_jets", n_b_tagged_jets);
    SetBranch("Training_Tree", "n_c_tagged_jets", n_c_tagged_jets);
    SetBranch("Training_Tree", "find_all_jets", find_all_jets);

    // for (size_t i = 0; i <= 8; i++)
    // {
    //     if (i < n_jets)
    //     {
    //         SetBranch("Training_Tree", "Jet_Pt_" + std::to_string(i), Jets[i].Pt());
    //         SetBranch("Training_Tree", "Jet_Eta_" + std::to_string(i), Jets[i].Eta());
    //         SetBranch("Training_Tree", "Jet_Phi_" + std::to_string(i), Jets[i].Phi());
    //         SetBranch("Training_Tree", "Jet_M_" + std::to_string(i), Jets[i].M());
    //         SetBranch("Training_Tree", "Jet_BvsC_" + std::to_string(i), Jets[i].GetBTaggerResult(FlavTagger[DataEra.Data()]));
    //         SetBranch("Training_Tree", "Jet_CvsB_" + std::to_string(i), Jets[i].GetCTaggerResult(FlavTagger[DataEra.Data()]).first);
    //         SetBranch("Training_Tree", "Jet_CvsL_" + std::to_string(i), Jets[i].GetCTaggerResult(FlavTagger[DataEra.Data()]).second);
    //         // Tagging WP
    //         SetBranch("Training_Tree", "Jet_B_WP_" + std::to_string(i), GetPassedBTaggingWP(Jets[i]));
    //         SetBranch("Training_Tree", "Jet_C_WP_" + std::to_string(i), GetPassedCTaggingWP(Jets[i]));

    //         auto it = find(ttbar_jet_indices.begin(), ttbar_jet_indices.end(), i);
    //         if (it != ttbar_jet_indices.end())
    //         {
    //             SetBranch("Training_Tree", "Jet_isTTbarJet_" + std::to_string(i), int(1));
    //             SetBranch("Training_Tree", "Jet_ttbarJet_idx_" + std::to_string(i), int(it - ttbar_jet_indices.begin()));
    //         }
    //         else
    //         {
    //             SetBranch("Training_Tree", "Jet_isTTbarJet_" + std::to_string(i), int(0));
    //             SetBranch("Training_Tree", "Jet_ttbarJet_idx_" + std::to_string(i), int(-999));
    //         }
    //     }
    //     else
    //     {
    //         SetBranch("Training_Tree", "Jet_Pt_" + std::to_string(i), -999.);
    //         SetBranch("Training_Tree", "Jet_Eta_" + std::to_string(i), -999.);
    //         SetBranch("Training_Tree", "Jet_Phi_" + std::to_string(i), -999.);
    //         SetBranch("Training_Tree", "Jet_M_" + std::to_string(i), -999.);
    //         SetBranch("Training_Tree", "Jet_BvsC_" + std::to_string(i), -999.);
    //         SetBranch("Training_Tree", "Jet_CvsB_" + std::to_string(i), -999.);
    //         SetBranch("Training_Tree", "Jet_CvsL_" + std::to_string(i), -999.);
    //         SetBranch("Training_Tree", "Jet_isTTbarJet_" + std::to_string(i), int(-999));
    //         SetBranch("Training_Tree", "Jet_ttbarJet_idx_" + std::to_string(i), int(-999));
    //         SetBranch("Training_Tree", "Jet_B_WP_" + std::to_string(i), -999);
    //         SetBranch("Training_Tree", "Jet_C_WP_" + std::to_string(i), -999);
    //     }
    // }
    // clear vectors
    Jet_Px.clear();
    Jet_Py.clear();
    Jet_Pz.clear();
    Jet_E.clear();
    Jet_M.clear();
    Jet_BvsC.clear();
    Jet_CvsB.clear();
    Jet_CvsL.clear();
    Jet_QvsG.clear();
    Jet_B_WP.clear();
    Jet_C_WP.clear();
    Jet_isTTbarJet.clear();
    Jet_ttbarJet_idx.clear();
    Jet_Pt.clear();
    Jet_Eta.clear();
    Jet_Phi.clear();
    edge_index_jet_jet0.clear();
    edge_index_jet_jet1.clear();
    deltaR_jet_jet.clear();
    invM_jet_jet.clear();
    cosTheta_jet_jet.clear();
    edge_index_jet_lepton0.clear();
    edge_index_jet_lepton1.clear();
    deltaR_jet_lepton.clear();
    invM_jet_lepton.clear();
    cosTheta_jet_lepton.clear();
    edge_index_jet_neutrino0.clear();
    edge_index_jet_neutrino1.clear();
    deltaR_jet_neutrino.clear();
    invM_jet_neutrino.clear();
    cosTheta_jet_neutrino.clear();
    edge_index_lepton_neutrino0.clear();
    edge_index_lepton_neutrino1.clear();
    deltaR_lepton_neutrino.clear();
    invM_lepton_neutrino.clear();
    cosTheta_lepton_neutrino.clear();

    int max_jet = 8;
    if (Jets.size() < 8)
        max_jet = Jets.size();

    TLorentzVector neutrino_p4;
    neutrino_p4.SetXYZM(MET.Px(), MET.Py(), best_KF_result.best_neu_pz, 0.);
    Particle neutrino(neutrino_p4);
    SetBranch("Training_Tree", "neutrino_Pt", neutrino.Pt());
    SetBranch("Training_Tree", "neutrino_Eta", neutrino.Eta());
    SetBranch("Training_Tree", "neutrino_Phi", neutrino.Phi());
    SetBranch("Training_Tree", "neutrino_M", neutrino.M());
    SetBranch("Training_Tree", "neutrino_Px", neutrino.Px());
    SetBranch("Training_Tree", "neutrino_Py", neutrino.Py());
    SetBranch("Training_Tree", "neutrino_Pz", neutrino.Pz());
    SetBranch("Training_Tree", "neutrino_E", neutrino.E());
    SetBranch("Training_Tree", "gen_neutrino_Pt", gen_neutrino.Pt());
    SetBranch("Training_Tree", "gen_neutrino_Eta", gen_neutrino.Eta());
    SetBranch("Training_Tree", "gen_neutrino_Phi", gen_neutrino.Phi());
    SetBranch("Training_Tree", "gen_neutrino_Px", gen_neutrino.Px());
    SetBranch("Training_Tree", "gen_neutrino_Py", gen_neutrino.Py());
    SetBranch("Training_Tree", "gen_neutrino_Pz", gen_neutrino.Pz());
    SetBranch("Training_Tree", "gen_neutrino_E", gen_neutrino.E());

    for (size_t i = 0; i < max_jet; i++)
    {

        Jet_Px.push_back(Jets[i].Px());
        Jet_Py.push_back(Jets[i].Py());
        Jet_Pz.push_back(Jets[i].Pz());
        Jet_E.push_back(Jets[i].E());
        Jet_M.push_back(Jets[i].M());
        Jet_BvsC.push_back(Jets[i].GetBTaggerResult(FlavTagger[DataEra.Data()]));
        Jet_CvsB.push_back(Jets[i].GetCTaggerResult(FlavTagger[DataEra.Data()]).first);
        Jet_CvsL.push_back(Jets[i].GetCTaggerResult(FlavTagger[DataEra.Data()]).second);
        Jet_QvsG.push_back(Jets[i].GetQvGTaggerResult(FlavTagger[DataEra.Data()]));
        Jet_B_WP.push_back(GetPassedBTaggingWP(Jets[i]));
        Jet_C_WP.push_back(GetPassedCTaggingWP(Jets[i]));
        Jet_Pt.push_back(Jets[i].Pt());
        Jet_Eta.push_back(Jets[i].Eta());
        Jet_Phi.push_back(Jets[i].Phi());
        auto it = find(ttbar_jet_indices.begin(), ttbar_jet_indices.end(), i);
        if (it != ttbar_jet_indices.end())
        {
            Jet_isTTbarJet.push_back(1);
            Jet_ttbarJet_idx.push_back(it - ttbar_jet_indices.begin());
        }
        else
        {
            Jet_isTTbarJet.push_back(0);
            Jet_ttbarJet_idx.push_back(-999);
        }
    }

    for (int i = 0; i < max_jet; i++)
    {
        for (int j = i + 1; j < max_jet; j++)
        {
            edge_index_jet_jet0.push_back(i);
            edge_index_jet_jet1.push_back(j);
            float this_deltaR = Jets[i].DeltaR(Jets[j]);
            float this_invM = (Jets[i] + Jets[j]).M();
            TVector3 v1 = Jets[i].Vect();
            TVector3 v2 = Jets[j].Vect();
            float this_cosTheta = TMath::Cos(v1.Angle(v2));

            deltaR_jet_jet.push_back(this_deltaR);
            invM_jet_jet.push_back(this_invM);
            cosTheta_jet_jet.push_back(this_cosTheta);
        }
    }

    for (int i = 0; i < max_jet; i++)
    {
        edge_index_jet_lepton0.push_back(i);
        edge_index_jet_lepton1.push_back(0);
        edge_index_jet_neutrino0.push_back(i);
        edge_index_jet_neutrino1.push_back(0);
        deltaR_jet_lepton.push_back(Jets[i].DeltaR(lepton));
        invM_jet_lepton.push_back((Jets[i] + lepton).M());
        deltaR_jet_neutrino.push_back(Jets[i].DeltaR(neutrino));
        invM_jet_neutrino.push_back((Jets[i] + neutrino).M());

        TVector3 vjet = Jets[i].Vect();
        TVector3 vlepton = lepton.Vect();
        TVector3 vneutrino = neutrino.Vect();
        cosTheta_jet_lepton.push_back(TMath::Cos(vjet.Angle(vlepton)));
        cosTheta_jet_neutrino.push_back(TMath::Cos(vjet.Angle(vneutrino)));
    }

    edge_index_lepton_neutrino0.push_back(0);
    edge_index_lepton_neutrino1.push_back(0);
    TVector3 vlepton = lepton.Vect();
    TVector3 vneutrino = neutrino.Vect();
    deltaR_lepton_neutrino.push_back(lepton.DeltaR(neutrino));
    invM_lepton_neutrino.push_back((lepton + neutrino).M());
    cosTheta_lepton_neutrino.push_back(TMath::Cos(vlepton.Angle(vneutrino)));

    SetBranch("Training_Tree", "lepton_Pt", lepton.Pt());
    SetBranch("Training_Tree", "lepton_Eta", lepton.Eta());
    SetBranch("Training_Tree", "lepton_Phi", lepton.Phi());
    SetBranch("Training_Tree", "lepton_M", lepton.M());
    SetBranch("Training_Tree", "lepton_Px", lepton.Px());
    SetBranch("Training_Tree", "lepton_Py", lepton.Py());
    SetBranch("Training_Tree", "lepton_Pz", lepton.Pz());
    SetBranch("Training_Tree", "lepton_E", lepton.E());
    SetBranch("Training_Tree", "lepton_Charge", lepton.Charge());
    SetBranch("Training_Tree", "lepton_isMuon", int(channel == Channel::Mu));
    SetBranch("Training_Tree", "lepton_isElectron", int(channel == Channel::El));

    SetBranch("Training_Tree", "KF_had_t_b_idx", best_KF_result.best_had_t_b_idx);
    SetBranch("Training_Tree", "KF_lep_t_b_idx", best_KF_result.best_lep_t_b_idx);
    SetBranch("Training_Tree", "KF_had_w1_idx", best_KF_result.best_had_w1_idx);
    SetBranch("Training_Tree", "KF_had_w2_idx", best_KF_result.best_had_w2_idx);
    SetBranch("Training_Tree", "KF_chi2", best_KF_result.chi2);

    int answer;
    if (IsDATA)
        answer = -999;
    // check TT is in MCSample string
    if (MCSample.Contains("TT"))
    {
        if (MCSample.Contains("Vcb"))
            answer = category_for_training_SL["Vcb"];
        else if (ttLF)
            answer = category_for_training_SL["TT"];
        else
            answer = category_for_training_SL["TT_tthf"];
    }
    else
        answer = category_for_training_SL["Others"];
    SetBranch("Training_Tree", "y", answer);

    SetBranch("Training_Tree", "KF_chi2", best_KF_result.chi2);

    FillTrees();
}

RVec<int> Vcb_SL::FindTTbarJetIndices()
{
    RVec<int> ttbar_jet_indices = {-1, -1, -1, -1};
    int w_plus_idx = -1;
    int w_minus_idx = -1;
    int t_plus_idx = -1;
    int t_minus_idx = -1;
    bool find_w_plus = false;
    bool find_w_minus = false;
    bool find_t_plus = false;
    bool find_t_minus = false;
    bool find_w_plus_up = false;
    bool find_w_plus_down = false;
    bool find_w_minus_up = false;
    bool find_w_minus_down = false;
    bool find_b_from_t_plus = false;
    bool find_b_from_t_minus = false;

    tt_decay_code = -9999;
    find_all_jets = false;
    // find last t+/- and W+/- in gens
    for (int i = AllGens.size() - 1; i >= 0; i--)
    {
        if (AllGens[i].PID() == 24 && !find_w_plus)
        {
            w_plus_idx = i;
            find_w_plus = true;
        }
        if (AllGens[i].PID() == -24 && !find_w_minus)
        {
            w_minus_idx = i;
            find_w_minus = true;
        }
        if (AllGens[i].PID() == 6 && !find_t_plus)
        {
            t_plus_idx = i;
            find_t_plus = true;
        }
        if (AllGens[i].PID() == -6 && !find_t_minus)
        {
            t_minus_idx = i;
            find_t_minus = true;
        }
        if (find_w_plus && find_w_minus && find_t_plus && find_t_minus)
            break;
    }

    int b_from_t_plus_idx = -1;
    int b_from_t_minus_idx = -1;
    int w_plus_up_idx = -1;
    int w_plus_down_idx = -1;
    int w_minus_up_idx = -1; // if leptonic decay, up is lepton
    int w_minus_down_idx = -1;

    // Loop through AllGens to find last-copy W decay products
    for (int i = (int)AllGens.size() - 1; i >= 0; i--)
    {
        const auto &gen = AllGens[i];
        int pid = gen.PID();

        // We only want last-copy particles
        if (!gen.isFirstCopy())
            continue;

        // Handle W+ decays
        if (!find_w_plus_up &&
            ((isPIDUpTypeQuark(pid) && pid > 0) || (isPIDLepton(pid) && pid < 0 && gen.isPrompt())) &&
            isDaughterOf(i, w_plus_idx))
        {
            w_plus_up_idx = i;
            find_w_plus_up = true;
            continue;
        }

        if (!find_w_plus_down &&
            ((isPIDDownTypeQuark(pid) && pid < 0) || (isPIDNeutrino(pid) && pid > 0 && gen.isPrompt())) &&
            isDaughterOf(i, w_plus_idx))
        {
            w_plus_down_idx = i;
            if (isPIDNeutrino(pid))
            {
                gen_neutrino = gen;
            }
            find_w_plus_down = true;
            continue;
        }

        // Handle W- decays
        if (!find_w_minus_up &&
            ((isPIDUpTypeQuark(pid) && pid < 0) || (isPIDLepton(pid) && pid > 0 && gen.isPrompt())) &&
            isDaughterOf(i, w_minus_idx))
        {
            w_minus_up_idx = i;
            find_w_minus_up = true;
            continue;
        }

        if (!find_w_minus_down &&
            ((isPIDDownTypeQuark(pid) && pid > 0) || (isPIDNeutrino(pid) && pid < 0 && gen.isPrompt())) &&
            isDaughterOf(i, w_minus_idx))
        {
            w_minus_down_idx = i;
            if (isPIDNeutrino(pid))
            {
                gen_neutrino = gen;
            }
            find_w_minus_down = true;
            continue;
        }

        // If we have found all W decay products, we can break early
        if (find_w_plus_up && find_w_plus_down && find_w_minus_up && find_w_minus_down)
            break;
    }
    bool w_plus_hadronic = false;
    FillHist("FindTT_CutFlow", 0, 1.f, 10, 0., 10.);

    if (isPIDLepton(AllGens[w_plus_up_idx].PID()))
        w_plus_hadronic = false;
    else
        w_plus_hadronic = true;

    bool w_minus_hadronic = !w_plus_hadronic;

    for (int i = AllGens.size() - 1; i >= 0; i--)
    {
        if (!AllGens[i].isFirstCopy())
            continue;
        if (AllGens[i].PID() == 5 &&
            !find_b_from_t_plus &&
            isDaughterOf(i, t_plus_idx) &&
            !isDaughterOf(i, w_plus_idx))
        {
            b_from_t_plus_idx = i;
            find_b_from_t_plus = true;
        }
        if (AllGens[i].PID() == -5 &&
            !find_b_from_t_minus &&
            isDaughterOf(i, t_minus_idx) &&
            !isDaughterOf(i, w_minus_idx))
        {
            b_from_t_minus_idx = i;
            find_b_from_t_minus = true;
        }
        if (find_b_from_t_plus && find_b_from_t_minus)
            break;
    }

    bool findall_gen = false;
    findall_gen = find_w_plus_up && find_w_plus_down && find_w_minus_up && find_w_minus_down && find_b_from_t_plus && find_b_from_t_minus;
    if (!findall_gen)
    {
        cout << "[ERROR] Cannot find all necessary particles in gen level" << endl;
        return ttbar_jet_indices;
    }
    FillHist("FindTT_CutFlow", 1, 1.f, 10, 0., 10.);

    if (w_plus_hadronic)
        tt_decay_code = 1000 * abs(AllGens[w_plus_up_idx].PID()) + 100 * abs(AllGens[w_plus_down_idx].PID()) + abs(AllGens[w_minus_up_idx].PID());
    else if (w_minus_hadronic)
        tt_decay_code = 100 * abs(AllGens[w_plus_up_idx].PID()) + 10 * abs(AllGens[w_minus_up_idx].PID()) + abs(AllGens[w_minus_down_idx].PID());

    RVec<Gen> this_gens;
    if (w_plus_hadronic)
    {
        this_gens.push_back(AllGens[b_from_t_plus_idx]);
        this_gens.push_back(AllGens[b_from_t_minus_idx]);
        this_gens.push_back(AllGens[w_plus_up_idx]);
        this_gens.push_back(AllGens[w_plus_down_idx]);
    }
    else if (w_minus_hadronic)
    {
        this_gens.push_back(AllGens[b_from_t_minus_idx]);
        this_gens.push_back(AllGens[b_from_t_plus_idx]);
        this_gens.push_back(AllGens[w_minus_up_idx]);
        this_gens.push_back(AllGens[w_minus_down_idx]);
    }

    unordered_map<int, RVec<pair<size_t, Gen>>> map_key_pid_val_vector_of_tuple_gen_index_gen;

    for (size_t i = 0; i < this_gens.size(); i++)
    {
        map_key_pid_val_vector_of_tuple_gen_index_gen[this_gens[i].PID()].push_back({i, this_gens[i]});
    }

    RVec<int> gens_PID = {this_gens[0].PID(), this_gens[1].PID(), this_gens[2].PID(), this_gens[3].PID()};
    RVec<int> all_genjet_PID;

    for (size_t i = 0; i < AllGenJets.size(); i++)
    {
        all_genjet_PID.push_back(AllGenJets[i].partonFlavour());
    }

    unordered_map<int, int> map_key_4gen_idx_val_genjet_idx_deltaRMatched;

    for (const auto &PID_group : map_key_pid_val_vector_of_tuple_gen_index_gen)
    {
        int current_target_PID = PID_group.first;
        const auto &vector_gen_idx_Gen_pairs = PID_group.second;

        RVec<GenJet> genjets_same_PID_as_current_gen_group;
        RVec<int> genjets_same_PID_as_current_gen_group_indices;
        unordered_map<int, int> map_key_gen_idx_of_current_gens_val_genjet_idx_of_pid_group;
        for (size_t genjet_idx = 0; genjet_idx < all_genjet_PID.size(); genjet_idx++)
        {
            if (all_genjet_PID[genjet_idx] == current_target_PID)
            {
                genjets_same_PID_as_current_gen_group.push_back(AllGenJets[genjet_idx]);
                genjets_same_PID_as_current_gen_group_indices.push_back(genjet_idx);
            }
        }
        // if no genjets that matches to this PID found, this PID group is failed to match
        if (genjets_same_PID_as_current_gen_group_indices.size() == 0)
        {
            for (const auto &gen_idx_Gen_pair : vector_gen_idx_Gen_pairs)
            {
                map_key_4gen_idx_val_genjet_idx_deltaRMatched[gen_idx_Gen_pair.first] = -999;
            }
        }
        // do deltaRMatching if we have matching genjets
        else
        {
            RVec<Gen> current_gens;
            RVec<int> current_gen_indices;
            for (const auto &gen_idx_Gen_pair : vector_gen_idx_Gen_pairs)
            {
                current_gens.push_back(gen_idx_Gen_pair.second);
                current_gen_indices.push_back(gen_idx_Gen_pair.first);
            }
            map_key_gen_idx_of_current_gens_val_genjet_idx_of_pid_group = deltaRMatching(current_gens, genjets_same_PID_as_current_gen_group, 0.5);

            for (const auto &temp_match_pair : map_key_gen_idx_of_current_gens_val_genjet_idx_of_pid_group)
            {
                if (temp_match_pair.second >= 0)
                {
                    map_key_4gen_idx_val_genjet_idx_deltaRMatched[current_gen_indices[temp_match_pair.first]] = genjets_same_PID_as_current_gen_group_indices[temp_match_pair.second];
                }
                else
                {
                    map_key_4gen_idx_val_genjet_idx_deltaRMatched[current_gen_indices[temp_match_pair.first]] = -999;
                }
            }
        }
    }

    // check all gen particles are matched to genjets
    bool find_all_genjets = true;
    for (size_t i = 0; i < 4; i++)
    {
        if (map_key_4gen_idx_val_genjet_idx_deltaRMatched[i] < 0.f)
        {
            find_all_genjets = false;
            break;
        }
    }
    if (find_all_genjets)
        FillHist("FindTT_CutFlow", 2, 1.f, 10, 0., 10.);

    // we get gen-genjet matching. now do genjet-jet matching
    RVec<GenJet> gen_matched_genjets;
    RVec<size_t> idx_4gens_of_gen_match_to_genjet;
    for (size_t i = 0; i < 4; i++)
    {
        if (map_key_4gen_idx_val_genjet_idx_deltaRMatched[i] >= 0)
        {
            gen_matched_genjets.push_back(AllGenJets[map_key_4gen_idx_val_genjet_idx_deltaRMatched[i]]);
            idx_4gens_of_gen_match_to_genjet.push_back(i);
        }
    }

    unordered_map<int, int> map_key_Jet_idx_val_gen_matched_genjets_idx = GenJetMatching(Jets, gen_matched_genjets, ev.GetRho(), 0.4, 999999);
    unordered_map<int, int> map_key_idx_4gen_val_jet_idx;

    // now store parton-jet matching result
    for (const auto jet_genjet_matching_result : map_key_Jet_idx_val_gen_matched_genjets_idx)
    {
        if (jet_genjet_matching_result.second >= 0)
        {
            map_key_idx_4gen_val_jet_idx[idx_4gens_of_gen_match_to_genjet[jet_genjet_matching_result.second]] = jet_genjet_matching_result.first;
        }
        else
        {
            map_key_idx_4gen_val_jet_idx[idx_4gens_of_gen_match_to_genjet[jet_genjet_matching_result.second]] = -999;
        }
    }

    for (size_t i = 0; i < 4; i++)
    {
        if (map_key_idx_4gen_val_jet_idx.find(i) == map_key_idx_4gen_val_jet_idx.end())
        {
            map_key_idx_4gen_val_jet_idx[i] = -999;
        }
    }

    ttbar_jet_indices = {map_key_idx_4gen_val_jet_idx[0], map_key_idx_4gen_val_jet_idx[1], map_key_idx_4gen_val_jet_idx[2], map_key_idx_4gen_val_jet_idx[3]};
    for (size_t i = 0; i < ttbar_jet_indices.size(); i++)
    {
        if (ttbar_jet_indices[i] == -999)
        {
            find_all_jets = false;
            break;
        }
        else
        {
            find_all_jets = true;
        }
    }

    Particle LepW;
    Particle HadW;
    Particle LepTop;
    Particle HadTop;

    if (w_minus_hadronic)
    {
        LepW = AllGens[w_plus_up_idx] + AllGens[w_plus_down_idx];
        HadW = AllGens[w_minus_up_idx] + AllGens[w_minus_down_idx];
        LepTop = LepW + AllGens[b_from_t_plus_idx];
        HadTop = HadW + AllGens[b_from_t_minus_idx];
    }
    else if (w_plus_hadronic)
    {
        LepW = AllGens[w_minus_up_idx] + AllGens[w_minus_down_idx];
        HadW = AllGens[w_plus_up_idx] + AllGens[w_plus_down_idx];
        LepTop = LepW + AllGens[b_from_t_minus_idx];
        HadTop = HadW + AllGens[b_from_t_plus_idx];
    }
    if (find_all_jets)
        FillHist("FindTT_CutFlow", 3, 1.f, 10, 0., 10.);
    if (find_all_genjets)
    {
        Particle GenJetHadW = AllGenJets[map_key_4gen_idx_val_genjet_idx_deltaRMatched[2]] + AllGenJets[map_key_4gen_idx_val_genjet_idx_deltaRMatched[3]];
        Particle GenJetHadTop = GenJetHadW + AllGenJets[map_key_4gen_idx_val_genjet_idx_deltaRMatched[0]];
        FillHist("genLevel_LepTopMass", LepTop.M(), 1.f, 100, 100., 300.);
        FillHist("genLevel_HadTopMass", HadTop.M(), 1.f, 100, 100., 300.);
        FillHist("genLevel_LepWMass", LepW.M(), 1.f, 100, 50., 100.);
        FillHist("genLevel_HadWMass", HadW.M(), 1.f, 100, 50., 100.);
        FillHist("genLevel_GenJetHadTopMass", GenJetHadTop.M(), 1.f, 100, 100., 300.);
        FillHist("genLevel_GenJetHadWMass", GenJetHadW.M(), 1.f, 100, 50., 100.);
    }

    return ttbar_jet_indices;
}

void Vcb_SL::GetKineMaticFitterResult(const RVec<Jet> &jets, Particle &MET, Lepton &lepton)
{
    RVec<RVec<unsigned int>> possible_permutations = GetPermutations(jets);

    best_KF_result.chi2 = 9999.;
std:
    variant<float, std::pair<float, float>> neutrino_pz = SolveNeutrinoPz(lepton, MET);
    for (const auto &permutation : possible_permutations)
    {
        int status = -999;
        float chi2 = 9999.;
        RVec<unsigned int> this_permutation;
        RVec<TLorentzVector> fitted_result;
        // check second element of neutrino_pz is nullptr or not
        float this_neutrino_pz;
        if (std::holds_alternative<float>(neutrino_pz))
        {
            Particle neutrino = Particle();
            neutrino.SetXYZM(MET.Px(), MET.Py(), std::get<float>(neutrino_pz), 0.);
            auto result = FitKinFitter(jets, permutation, neutrino, lepton);
            status = std::get<0>(result);
            chi2 = std::get<1>(result);
            this_permutation = std::get<2>(result);
            fitted_result = std::get<3>(result);
            this_neutrino_pz = std::get<float>(neutrino_pz);
        }
        else
        {
            float pz1 = std::get<std::pair<float, float>>(neutrino_pz).first;
            float pz2 = std::get<std::pair<float, float>>(neutrino_pz).second;
            Particle neutrino1 = Particle();
            neutrino1.SetXYZM(MET.Px(), MET.Py(), pz1, 0.);
            Particle neutrino2 = Particle();
            neutrino2.SetXYZM(MET.Px(), MET.Py(), pz2, 0.);
            auto result1 = FitKinFitter(jets, permutation, neutrino1, lepton);
            auto result2 = FitKinFitter(jets, permutation, neutrino2, lepton);
            auto result = (std::get<1>(result1) < std::get<1>(result2)) ? result1 : result2;
            status = std::get<0>(result);
            chi2 = std::get<1>(result);
            this_permutation = std::get<2>(result);
            fitted_result = std::get<3>(result);
            this_neutrino_pz = (std::get<1>(result1) < std::get<1>(result2)) ? pz1 : pz2;
        }

        if (chi2 < best_KF_result.chi2 && status == 0)
        {
            best_KF_result.status = status;
            best_KF_result.chi2 = chi2;
            best_KF_result.best_had_t_b_idx = this_permutation[0];
            best_KF_result.best_lep_t_b_idx = this_permutation[1];
            best_KF_result.best_had_w1_idx = this_permutation[2];
            best_KF_result.best_had_w2_idx = this_permutation[3];
            best_KF_result.fitted_had_t_b = fitted_result[0];
            best_KF_result.fitted_lep_t_b = fitted_result[1];
            best_KF_result.fitted_had_w1 = fitted_result[2];
            best_KF_result.fitted_had_w2 = fitted_result[3];
            best_KF_result.fitted_lep = fitted_result[4];
            best_KF_result.fitted_neu = fitted_result[5];
            best_KF_result.best_neu_pz = this_neutrino_pz;
        }
    }
}

tuple<int, float, RVec<unsigned int>, RVec<TLorentzVector>> Vcb_SL::FitKinFitter(const RVec<Jet> &jets, const RVec<unsigned int> &permutation, Particle &neutrino, Lepton &lepton)
{
    // Initialize the fitter with smart pointers
    std::unique_ptr<TKinFitter> fitter = std::make_unique<TKinFitter>("fitter", "fitter");
    fitter->reset();
    fitter->setVerbosity(0);
    fitter->setMaxNbIter(500);
    fitter->setMaxDeltaS(1e-2);
    fitter->setMaxF(1e-2);

    // Prepare particle and covariance matrices
    RVec<TLorentzVector> this_pts;
    RVec<TMatrixD> this_JERs;
    for (const auto &jet : jets)
    {
        TLorentzVector this_part = static_cast<TLorentzVector>(jet);
        this_pts.push_back(this_part);
        TMatrixD this_Cov(1, 1);
        float this_JER = jet.Pt();
        this_JER *= myCorr->GetJER(jet.Eta(), jet.Pt(), ev.GetRho());
        this_JER *= myCorr->GetJERSF(jet.Eta(), jet.Pt());
        this_Cov(0, 0) = this_JER * this_JER;
        this_JERs.push_back(this_Cov);
    }

    // Initialize the TFitParticlePt objects with smart pointers
    auto had_t_b = std::make_unique<TFitParticlePt>("had_t_b", "had_t_b", &(this_pts[permutation[0]]), &(this_JERs[permutation[0]]));
    auto lep_t_b = std::make_unique<TFitParticlePt>("lep_t_b", "lep_t_b", &(this_pts[permutation[1]]), &(this_JERs[permutation[1]]));
    auto had_w1 = std::make_unique<TFitParticlePt>("w11", "w11", &(this_pts[permutation[2]]), &(this_JERs[permutation[2]]));
    auto had_w2 = std::make_unique<TFitParticlePt>("w12", "w12", &(this_pts[permutation[3]]), &(this_JERs[permutation[3]]));

    TMatrixD lepton_Cov(1, 1);
    lepton_Cov(0, 0) = TMath::Power((lepton.Pt() * 0.0001), 2);
    auto lep = std::make_unique<TFitParticlePt>("lep", "lep", &(lepton), &(lepton_Cov));

    TVector3 neutrino_p3 = neutrino.Vect();
    auto neu = std::make_unique<TFitParticleMCCart>("neu", "neu", &(neutrino_p3), 0., nullptr);

    RVec<std::unique_ptr<TFitParticlePt>> extra_jets;
    for (size_t i = 0; i < jets.size(); i++)
    {
        if (std::find(permutation.begin(), permutation.end(), i) == permutation.end())
        {
            auto extra_jet = std::make_unique<TFitParticlePt>("extra_jet_" + std::to_string(i), "extra_jet_" + std::to_string(i), &(this_pts[i]), &(this_JERs[i]));
            extra_jets.push_back(std::move(extra_jet));
        }
    }

    // Mass and energy-momentum constraints
    // auto mW1 = std::make_unique<TFitConstraintMGaus>("MW1", "MW1", nullptr, nullptr, W_MASS, W_WIDTH);
    auto mHadW = std::make_unique<TFitConstraintMGaus>("MW1", "MW1", nullptr, nullptr, W_MASS, W_WIDTH);
    mHadW->addParticle1(had_w1.get());
    mHadW->addParticle1(had_w2.get());

    // auto mW2 = std::make_unique<TFitConstraintMGaus>("MW2", "MW2", nullptr, nullptr, W_MASS, W_WIDTH);
    auto mLepW = std::make_unique<TFitConstraintMGaus>("MW2", "MW2", nullptr, nullptr, W_MASS, W_WIDTH);
    mLepW->addParticle1(lep.get());
    mLepW->addParticle1(neu.get());

    auto mHadT = std::make_unique<TFitConstraintMGaus>("MT1", "MT1", nullptr, nullptr, T_MASS, T_WIDTH);
    mHadT->addParticle1(had_t_b.get());
    mHadT->addParticle1(had_w1.get());
    mHadT->addParticle1(had_w2.get());

    auto mLepT = std::make_unique<TFitConstraintMGaus>("MT2", "MT2", nullptr, nullptr, T_MASS, T_WIDTH);
    mLepT->addParticle1(lep_t_b.get());
    mLepT->addParticle1(lep.get());
    mLepT->addParticle1(neu.get());

    // pt balance constraint
    auto px_balance = std::make_unique<TFitConstraintEp>("px", "px", TFitConstraintEp::component::pX, 0.);
    auto py_balance = std::make_unique<TFitConstraintEp>("py", "py", TFitConstraintEp::component::pY, 0.);
    px_balance->addParticle(had_t_b.get());
    px_balance->addParticle(lep_t_b.get());
    px_balance->addParticle(had_w1.get());
    px_balance->addParticle(had_w2.get());
    px_balance->addParticle(lep.get());
    px_balance->addParticle(neu.get());
    py_balance->addParticle(had_t_b.get());
    py_balance->addParticle(lep_t_b.get());
    py_balance->addParticle(had_w1.get());
    py_balance->addParticle(had_w2.get());
    py_balance->addParticle(lep.get());
    py_balance->addParticle(neu.get());
    for (size_t i = 0; i < extra_jets.size(); i++)
    {
        px_balance->addParticle(extra_jets[i].get());
        py_balance->addParticle(extra_jets[i].get());
    }

    // Add particles and constraints to the fitter
    fitter->addMeasParticle(had_t_b.get());
    fitter->addMeasParticle(lep_t_b.get());
    fitter->addMeasParticle(had_w1.get());
    fitter->addMeasParticle(had_w2.get());
    fitter->addMeasParticle(lep.get());
    fitter->addUnmeasParticle(neu.get());

    // add extra jets
    for (size_t i = 0; i < extra_jets.size(); i++)
    {
        fitter->addMeasParticle(extra_jets[i].get());
    }

    fitter->addConstraint(mHadW.get());
    fitter->addConstraint(mLepW.get());
    fitter->addConstraint(mHadT.get());
    fitter->addConstraint(mLepT.get());
    fitter->addConstraint(px_balance.get());
    fitter->addConstraint(py_balance.get());

    // Perform the fit
    fitter->fit();

    int status = fitter->getStatus();
    float chi2 = fitter->getS();
    // Collect fitted particles
    RVec<TLorentzVector> fitted_result = {
        TLorentzVector(*had_t_b->getCurr4Vec()),
        TLorentzVector(*lep_t_b->getCurr4Vec()),
        TLorentzVector(*had_w1->getCurr4Vec()),
        TLorentzVector(*had_w2->getCurr4Vec()),
        TLorentzVector(*lep->getCurr4Vec()),
        TLorentzVector(*neu->getCurr4Vec())};

    // Return the result as a tuple
    return std::make_tuple(status, chi2, permutation, fitted_result);
}

void Vcb_SL::InferONNX()
{
    // Prepare input tensor
    std::vector<uint8_t> MASK;
    std::vector<float> pt;
    std::vector<float> eta;
    std::vector<float> sin_phi;
    std::vector<float> cos_phi;
    std::vector<float> m;
    std::vector<float> btag;
    std::vector<float> etag;
    std::vector<float> utag;

    int max_jet = 8;
    for (size_t i = 0; i < max_jet; i++)
    {
        if (i < Jets.size())
            MASK.push_back(1);
        else
            MASK.push_back(0);

        if (i < Jets.size())
        {
            pt.push_back(Jets[i].Pt());
            eta.push_back(Jets[i].Eta());
            sin_phi.push_back(static_cast<float>(TMath::Sin(Jets[i].Phi())));
            cos_phi.push_back(static_cast<float>(TMath::Cos(Jets[i].Phi())));
            m.push_back(Jets[i].M());
            btag.push_back(Jets[i].GetBTaggerResult(FlavTagger[DataEra.Data()]));
            etag.push_back(0.);
            utag.push_back(0.);
        }
        else
        {
            pt.push_back(0.);
            eta.push_back(0.);
            sin_phi.push_back(0.);
            cos_phi.push_back(0.);
            m.push_back(0.);
            btag.push_back(0.);
            etag.push_back(0.);
            utag.push_back(0.);
        }
    }
    pt.push_back(lepton.Pt());
    eta.push_back(lepton.Eta());
    sin_phi.push_back(static_cast<float>(TMath::Sin(lepton.Phi())));
    cos_phi.push_back(static_cast<float>(TMath::Cos(lepton.Phi())));
    m.push_back(lepton.M());
    btag.push_back(0.);
    if (channel == Channel::Mu)
    {
        utag.push_back(1.);
        etag.push_back(0.);
    }
    else if (channel == Channel::El)
    {
        utag.push_back(0.);
        etag.push_back(1.);
    }
    MASK.push_back(1);

    std::vector<float> met_pt = {static_cast<float>(MET.Pt())};
    std::vector<float> met_sin_phi = {static_cast<float>(TMath::Sin(MET.Phi()))};
    std::vector<float> met_cos_phi = {static_cast<float>(TMath::Cos(MET.Phi()))};
    std::vector<float> Momenta_data;
    std::vector<uint8_t> Momenta_mask;
    std::vector<float> Met_data;
    std::vector<uint8_t> Met_mask;
    // Flatteing the input tensor

    std::unordered_map<std::string, std::vector<int>> input_shape;
    input_shape["Momenta_data"] = {1, 9, 8};
    input_shape["Momenta_mask"] = {1, 9};
    input_shape["Met_data"] = {1, 1, 3};
    input_shape["Met_mask"] = {1, 1};

    // row-major order
    for (size_t i = 0; i < pt.size(); i++)
    {
        Momenta_data.push_back(pt[i]);
        Momenta_data.push_back(eta[i]);
        Momenta_data.push_back(sin_phi[i]);
        Momenta_data.push_back(cos_phi[i]);
        Momenta_data.push_back(m[i]);
        Momenta_data.push_back(btag[i]);
        Momenta_data.push_back(etag[i]);
        Momenta_data.push_back(utag[i]);
        Momenta_mask.push_back(MASK[i]);
    }

    Met_data.push_back(met_pt[0]);
    Met_data.push_back(met_sin_phi[0]);
    Met_data.push_back(met_cos_phi[0]);
    Met_mask.push_back(1);

    std::unordered_map<std::string, VariousArray> input_data;
    input_data["Momenta_data"] = Momenta_data;
    input_data["Momenta_mask"] = Momenta_mask;
    input_data["Met_data"] = Met_data;
    input_data["Met_mask"] = Met_mask; 

    std::unordered_map<std::string, FloatArray> output_data = myMLHelper->Run_ONNX_Model(input_data, input_shape);


    for(size_t i = 0; i < class_score.size(); i++)
    {
        
        class_score[i] = (std::exp((output_data.at("EVENT/signal").at(i))));
    }

    // find the assignment from output_data["t_assignment_log_probability"], output_data["ht_assignment_log_probability"]
    int l_assignment = -999;
    int lb_assignment = -999;
    int hb_assignment = -999;
    int w1_assignment = -999;
    int w2_assignment = -999;
    Particle lt; 
    Particle ht;
    Particle hw;

    std::vector<int> lt_assignment_shape = {1, 9, 9};
    std::vector<int> ht_assignment_shape = {1, 9, 9, 9};

    //first find the most lt probable assignment
    for(size_t i = 0; i < n_jets+1; i++){
        size_t max_idx = FindNthMaxIndex(output_data["lt_assignment_log_probability"], i);
        std::vector<int> assignment = UnravelIndex(max_idx, lt_assignment_shape);
        if(assignment[1] != 8) cout << "Assignment failed" << endl;
        else{
            l_assignment = assignment[1];
            lb_assignment = assignment[2];
            break;
        }
    }

    bool checkUnique = false;
    int num_total_ht_assignments = output_data["ht_assignment_log_probability"].size();

    for (size_t i = 0; i < num_total_ht_assignments; i++)
    {
        size_t max_idx = FindNthMaxIndex(output_data["ht_assignment_log_probability"], i);
        std::vector<int> assignment = UnravelIndex(max_idx, ht_assignment_shape);
        hb_assignment = assignment[1];
        w1_assignment = assignment[2];
        w2_assignment = assignment[3];

        checkUnique = true; 
        for (size_t j = 1; j < assignment.size(); j++)
        {
            if (assignment[j] == l_assignment || assignment[j] == lb_assignment)
            {
                checkUnique = false; // Found a conflict
                break;               // Exit the inner loop early
            }
        }

        if (checkUnique)
        {
            break;
        }
    }

    Particle regressed_neutrino;
    regressed_neutrino.SetXYZM(MET.Px(), MET.Py(), output_data.at("EVENT/neutrino_pz")[0], 0.);

    lt = Jets[lb_assignment] + lepton + regressed_neutrino;
    hw = Jets[w1_assignment] + Jets[w2_assignment];
    ht = Jets[hb_assignment] + hw;

    assignment[0] = hb_assignment;
    assignment[1] = lb_assignment;
    assignment[2] = w1_assignment;
    assignment[3] = w2_assignment;

    if(class_score[0] > 0.6)
        class_label = Vcb_SL::classCategory::Signal;
    else if(class_score[0] <= 0.6 && class_score[0] > 0.4)
        class_label = Vcb_SL::classCategory::Control0;
    else
        class_label = Vcb_SL::classCategory::Disposal;
}

void Vcb_SL::FillONNXRecoInfo(const TString &histPrefix, float weight)
{
    if(Jets[assignment[2]].GetBTaggerResult(FlavTagger[DataEra.Data()]) > Jets[assignment[3]].GetBTaggerResult(FlavTagger[DataEra.Data()]))
    {
        //swap the assignment
        int temp = assignment[2];
        assignment[2] = assignment[3];
        assignment[3] = temp;
    }
    Particle hw = Jets[assignment[2]] + Jets[assignment[3]];
    Particle ht = Jets[assignment[0]] + hw;
    float W1_BvsC = Jets[assignment[2]].GetBTaggerResult(FlavTagger[DataEra.Data()]);
    float W2_BvsC = Jets[assignment[3]].GetBTaggerResult(FlavTagger[DataEra.Data()]);
    FillHist(histPrefix + "/" + "Reco_HadWMass", hw.M(), weight, 100, 30., 130.);
    FillHist(histPrefix + "/" + "Reco_HadTopMass", ht.M(), weight, 100, 100., 300.);
    FillHist(histPrefix + "/" + "Reco_W1JetPt", Jets[assignment[2]].Pt(), weight, 100, 0., 200.);
    FillHist(histPrefix + "/" + "Reco_W2JetPt", Jets[assignment[3]].Pt(), weight, 100, 0., 200.);
    FillHist(histPrefix + "/" + "Reco_W1BvsC", W1_BvsC, weight, 100, 0., 1.);
    FillHist(histPrefix + "/" + "Reco_W2BvsC", W2_BvsC, weight, 100, 0., 1.);
    FillHist(histPrefix + "/" + "Reco_BvsCAdded" , W1_BvsC + W2_BvsC, weight, 100, 0., 2.);
    int unrolledIdx = Unroller(Jets[assignment[2]], Jets[assignment[3]]);
    FillHist(histPrefix + "/" + "Reco_W1Bvsc_W2Bvsc_Unrolled", unrolledIdx, weight, 16, 0., 16.);
    FillHist(histPrefix + "/" + "Class_Score0", static_cast<float>(class_score[0]), weight, 100, 0., 1.);
    FillHist(histPrefix + "/" + "Class_Score1", static_cast<float>(class_score[1]), weight, 100, 0., 1.);
    FillHist(histPrefix + "/" + "Class_Score2", static_cast<float>(class_score[2]), weight, 100, 0., 1.);
    FillHist(histPrefix + "/" + "EnergyFrac0p5VsBvsC", W2_BvsC, GetJetEnergyFractionWithRadius(Jets[assignment[3]], 0.5), weight, 10, 0., 1., 50, 0. ,1.);
    FillHist(histPrefix + "/" + "EnergyFrac0p8VsBvsC", W2_BvsC, GetJetEnergyFractionWithRadius(Jets[assignment[3]], 0.8), weight, 10, 0., 1., 50, 0. ,1.);
    FillHist(histPrefix + "/" + "EnergyFrac1p2VsBvsC", W2_BvsC, GetJetEnergyFractionWithRadius(Jets[assignment[3]], 1.2), weight, 10, 0., 1., 50, 0. ,1.);
    FillHist(histPrefix + "/" + "nConstituentsVsBvsC", W2_BvsC, Jets[assignment[3]].nConstituents(), weight, 10, 0., 1., 50, 0., 50.);
}