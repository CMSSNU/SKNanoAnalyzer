#include "Vcb_FH.h"
// #include "MLHelper.h"

Vcb_FH::Vcb_FH() {}

void Vcb_FH::CreateTrainingTree()
{
    Clear();
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
    GetTree("Training_Tree")->Branch("edge_index_0", &edge_index0);
    GetTree("Training_Tree")->Branch("edge_index_1", &edge_index1);
    GetTree("Training_Tree")->Branch("deltaR", &deltaR);
    GetTree("Training_Tree")->Branch("invM", &invM);
    GetTree("Training_Tree")->Branch("cosTheta", &cosTheta);
}

RVec<RVec<unsigned int>> Vcb_FH::GetPermutations(const RVec<Jet> &jets)
{
    RVec<RVec<unsigned int>> permutations;
    std::vector<unsigned int> b_tagged_idx;
    for (unsigned int i = 0; i < jets.size(); i++)
    {
        if (jets[i].GetBTaggerResult(FlavTagger[DataEra.Data()]) > myCorr->GetBTaggingWP(FlavTagger[DataEra.Data()], JetTagging::JetFlavTaggerWP::Medium))
        {
            b_tagged_idx.push_back(i);
        }
    }
    // Permutations rules:
    // idx 0 and 1 are b jets from top decay. b-tagged jets only.
    // idx 2 and 3 are w jets from top decay. they are interchangeable.
    // idx 4 and 5 are w jets from top decay. they are interchangeable.
    std::vector<std::pair<unsigned int, unsigned int>> b_tagged_pairs;

    for (unsigned int i = 0; i < b_tagged_idx.size(); i++)
    {
        for (unsigned int j = i + 1; j < b_tagged_idx.size(); j++)
        {
            b_tagged_pairs.push_back(std::make_pair(b_tagged_idx[i], b_tagged_idx[j]));
        }
    }

    for (auto &pair : b_tagged_pairs)
    {

        std::vector<unsigned int> remaining_idx;
        std::vector<std::vector<unsigned int>> w_pairs;

        for (unsigned int i = 0; i < jets.size(); i++)
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
            w_pairs.push_back({remaining_idx[0], remaining_idx[1], remaining_idx[2], remaining_idx[3]});
        } while (std::next_permutation(remaining_idx.begin(), remaining_idx.end()));
        for (auto &w_pair : w_pairs)
        {
            // sort them
            // first w pair(first 2 elements)
            std::sort(w_pair.begin(), w_pair.begin() + 2);
            // second w pair(last 2 elements)
            std::sort(w_pair.begin() + 2, w_pair.end());
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

bool Vcb_FH::PassBaseLineSelection(bool remove_flavtagging_cut)
{
    Clear();
    if (!ev.PassTrigger(FH_Trigger_DoubleBTag[DataEra.Data()]))
        return false;
    if (!PassJetVetoMap(AllJets, AllMuons))
        return false;
    if (!PassMetFilter(AllJets, ev))
        return false;

    // Set Objects
    Jets = SelectJets(AllJets, Jet_ID, FH_Jet_Pt_cut[DataEra.Data()], Jet_Eta_cut);
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
    Jets = SelectJets(Jets, Jet_ID, FH_Jet_Pt_cut[DataEra.Data()], Jet_Eta_cut);

    MET = ev.GetMETVector(Event::MET_Type::PUPPI);
    Muons_Veto = SelectMuons(AllMuons, Muon_Veto_ID, Muon_Veto_Pt, Muon_Veto_Eta);
    Muons_Veto = SelectMuons(Muons_Veto, Muon_Veto_Iso, Muon_Veto_Pt, Muon_Veto_Eta);
    Electrons_Veto = SelectElectrons(AllElectrons, Electron_Veto_ID, Electron_Veto_Pt, Electron_Veto_Eta);
    Muons = SelectMuons(AllMuons, Muon_Tight_ID, Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
    Muons = SelectMuons(Muons, Muon_Tight_Iso, Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
    Electrons = SelectElectrons(AllElectrons, Electron_Tight_ID, Electron_Tight_Pt[DataEra.Data()], Electron_Tight_Eta);
    Jets = JetsVetoLeptonInside(Jets, Electrons_Veto, Muons_Veto, Jet_Veto_DR);
    Jets = SelectJets(Jets, Jet_PUID, FH_Jet_Pt_cut[DataEra.Data()], Jet_Eta_cut);
    HT = GetHT(Jets);
    n_jets = Jets.size();

    if (n_jets < 6)
        return false;

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
    if (n_b_tagged_jets < 2 && !remove_flavtagging_cut)
        return false;
    if (Muons_Veto.size() != 0 || Electrons_Veto.size() != 0)
        return false;
    if (HT < FH_HT_cut[DataEra.Data()])
        return false;

    SetSystematicLambda();
    SetTTbarId();

    return true;
}

void Vcb_FH::FillKinematicFitterResult(const TString &histPrefix, float weight)
{
    Particle fittedW1 = best_KF_result.fitted_w11 + best_KF_result.fitted_w12;
    Particle fittedW2 = best_KF_result.fitted_w21 + best_KF_result.fitted_w22;
    Particle fittedTop1 = best_KF_result.fitted_b1 + fittedW1;
    Particle fittedTop2 = best_KF_result.fitted_b2 + fittedW2;
    Particle W1 = Jets[best_KF_result.best_w11_idx] + Jets[best_KF_result.best_w12_idx];
    Particle W2 = Jets[best_KF_result.best_w21_idx] + Jets[best_KF_result.best_w22_idx];
    Particle Top1 = Jets[best_KF_result.best_b1_idx] + W1;
    Particle Top2 = Jets[best_KF_result.best_b2_idx] + W2;

    FillHist(histPrefix + "/" + "Top1Mass", Top1.M(), weight, 100, 0., 500.);
    FillHist(histPrefix + "/" + "Top2Mass", Top2.M(), weight, 100, 0., 500.);
    FillHist(histPrefix + "/" + "W1Mass", W1.M(), weight, 100, 50., 150.);
    FillHist(histPrefix + "/" + "W2Mass", W2.M(), weight, 100, 50., 150.);
    FillHist(histPrefix + "/" + "FittedTop1Mass", fittedTop1.M(), weight, 100, 100., 300.);
    FillHist(histPrefix + "/" + "FittedTop2Mass", fittedTop2.M(), weight, 100, 100., 300.);
    FillHist(histPrefix + "/" + "FittedW1Mass", fittedW1.M(), weight, 100, 60., 100.);
    FillHist(histPrefix + "/" + "FittedW2Mass", fittedW2.M(), weight, 100, 60., 100.);
    FillHist(histPrefix + "/" + "Top_bb_dR", Jets[best_KF_result.best_b1_idx].DeltaR(Jets[best_KF_result.best_b2_idx]), weight, 100, 0., 5.);
    FillHist(histPrefix + "/" + "Chi2", best_KF_result.chi2, weight, 600, 0., 300.);
}

void Vcb_FH::FillTrainingTree()
{
    ttbar_jet_indices = FindTTbarJetIndices();
    SetBranch("Training_Tree", "MET", MET.Pt());
    SetBranch("Training_Tree", "HT", HT);
    SetBranch("Training_Tree", "n_jets", n_jets);
    SetBranch("Training_Tree", "n_b_tagged_jets", n_b_tagged_jets);
    SetBranch("Training_Tree", "n_c_tagged_jets", n_c_tagged_jets);
    SetBranch("Training_Tree", "find_all_jets", find_all_jets);
    if (find_all_jets)
    {
        Particle W1Cand = Jets[ttbar_jet_indices[2]] + Jets[ttbar_jet_indices[3]];
        Particle W2Cand = Jets[ttbar_jet_indices[4]] + Jets[ttbar_jet_indices[5]];
        Particle Top1Cand = Jets[ttbar_jet_indices[0]] + W1Cand;
        Particle Top2Cand = Jets[ttbar_jet_indices[1]] + W2Cand;
        Particle ttCand = Top1Cand + Top2Cand;
        SetBranch("Training_Tree", "Top1Mass", Top1Cand.M());
        SetBranch("Training_Tree", "Top2Mass", Top2Cand.M());
        SetBranch("Training_Tree", "W1Mass", W1Cand.M());
        SetBranch("Training_Tree", "W2Mass", W2Cand.M());
        SetBranch("Training_Tree", "Top1Pt", Top1Cand.Pt());
        SetBranch("Training_Tree", "Top2Pt", Top2Cand.Pt());
        SetBranch("Training_Tree", "W1Pt", W1Cand.Pt());
        SetBranch("Training_Tree", "W2Pt", W2Cand.Pt());
        SetBranch("Training_Tree", "Top1Eta", Top1Cand.Eta());
        SetBranch("Training_Tree", "Top2Eta", Top2Cand.Eta());
        SetBranch("Training_Tree", "W1Eta", W1Cand.Eta());
        SetBranch("Training_Tree", "W2Eta", W2Cand.Eta());
        SetBranch("Training_Tree", "Top1Phi", Top1Cand.Phi());
        SetBranch("Training_Tree", "Top2Phi", Top2Cand.Phi());
        SetBranch("Training_Tree", "W1Phi", W1Cand.Phi());
        SetBranch("Training_Tree", "W2Phi", W2Cand.Phi());
        SetBranch("Training_Tree", "ttbarMass", ttCand.M());
        SetBranch("Training_Tree", "ttbarPt", ttCand.Pt());
        SetBranch("Training_Tree", "ttbarEta", ttCand.Eta());
        SetBranch("Training_Tree", "ttbarPhi", ttCand.Phi());
    }
    else
    {
        SetBranch("Training_Tree", "Top1Mass", -999.);
        SetBranch("Training_Tree", "Top2Mass", -999.);
        SetBranch("Training_Tree", "W1Mass", -999.);
        SetBranch("Training_Tree", "W2Mass", -999.);
        SetBranch("Training_Tree", "Top1Pt", -999.);
        SetBranch("Training_Tree", "Top2Pt", -999.);
        SetBranch("Training_Tree", "W1Pt", -999.);
        SetBranch("Training_Tree", "W2Pt", -999.);
        SetBranch("Training_Tree", "Top1Eta", -999.);
        SetBranch("Training_Tree", "Top2Eta", -999.);
        SetBranch("Training_Tree", "W1Eta", -999.);
        SetBranch("Training_Tree", "W2Eta", -999.);
        SetBranch("Training_Tree", "Top1Phi", -999.);
        SetBranch("Training_Tree", "Top2Phi", -999.);
        SetBranch("Training_Tree", "W1Phi", -999.);
        SetBranch("Training_Tree", "W2Phi", -999.);
        SetBranch("Training_Tree", "ttbarMass", -999.);
        SetBranch("Training_Tree", "ttbarPt", -999.);
        SetBranch("Training_Tree", "ttbarEta", -999.);
        SetBranch("Training_Tree", "ttbarPhi", -999.);
    }

    // for (size_t i = 0; i <= 10; i++)
    // {
    //     if(i < n_jets){
    //         SetBranch("Training_Tree", "Jet_Pt_" + std::to_string(i), Jets[i].Pt());
    //         SetBranch("Training_Tree", "Jet_Eta_" + std::to_string(i), Jets[i].Eta());
    //         SetBranch("Training_Tree", "Jet_Phi_" + std::to_string(i), Jets[i].Phi());
    //         SetBranch("Training_Tree", "Jet_M_" + std::to_string(i), Jets[i].M());
    //         SetBranch("Training_Tree", "Jet_BvsC_" + std::to_string(i), Jets[i].GetBTaggerResult(FlavTagger[DataEra.Data()]));
    //         SetBranch("Training_Tree", "Jet_CvsB_" + std::to_string(i), Jets[i].GetCTaggerResult(FlavTagger[DataEra.Data()]).first);
    //         SetBranch("Training_Tree", "Jet_CvsL_" + std::to_string(i), Jets[i].GetCTaggerResult(FlavTagger[DataEra.Data()]).second);
    //         SetBranch("Training_Tree", "Jet_QvsG_" + std::to_string(i), Jets[i].GetQvGTaggerResult(FlavTagger[DataEra.Data()]));
    //         //Tagging WP
    //         SetBranch("Training_Tree", "Jet_B_WP_" + std::to_string(i), GetPassedBTaggingWP(Jets[i]));
    //         SetBranch("Training_Tree", "Jet_C_WP_" + std::to_string(i), GetPassedCTaggingWP(Jets[i]));

    // auto it = find(ttbar_jet_indices.begin(), ttbar_jet_indices.end(), i);
    // if(it != ttbar_jet_indices.end()){
    //     SetBranch("Training_Tree", "Jet_isTTbarJet_" + std::to_string(i), int(1));
    //     SetBranch("Training_Tree", "Jet_ttbarJet_idx_" + std::to_string(i), int(it - ttbar_jet_indices.begin()));
    // }
    // else{
    //     SetBranch("Training_Tree", "Jet_isTTbarJet_" + std::to_string(i), int(0));
    //     SetBranch("Training_Tree", "Jet_ttbarJet_idx_" + std::to_string(i), int(-999));
    // }

    //     }
    //     else{
    //         SetBranch("Training_Tree", "Jet_Pt_" + std::to_string(i), -999.);
    //         SetBranch("Training_Tree", "Jet_Eta_" + std::to_string(i), -999.);
    //         SetBranch("Training_Tree", "Jet_Phi_" + std::to_string(i), -999.);
    //         SetBranch("Training_Tree", "Jet_M_" + std::to_string(i), -999.);
    //         SetBranch("Training_Tree", "Jet_BvsC_" + std::to_string(i), -999.);
    //         SetBranch("Training_Tree", "Jet_CvsB_" + std::to_string(i), -999.);
    //         SetBranch("Training_Tree", "Jet_CvsL_" + std::to_string(i), -999.);
    //         SetBranch("Training_Tree", "Jet_QvsG_" + std::to_string(i), -999.);
    //         SetBranch("Training_Tree", "Jet_isTTbarJet_" + std::to_string(i), int(-999));
    //         SetBranch("Training_Tree", "Jet_ttbarJet_idx_" + std::to_string(i), int(-999));
    //         SetBranch("Training_Tree", "Jet_B_WP_" + std::to_string(i), -999);
    //         SetBranch("Training_Tree", "Jet_C_WP_" + std::to_string(i), -999);
    //     }
    // }

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
    edge_index0.clear();
    edge_index1.clear();
    deltaR.clear();
    invM.clear();
    cosTheta.clear();

    float max_jets = 10;
    if (Jets.size() < max_jets)
        max_jets = Jets.size();

    for (size_t i = 0; i < max_jets; i++)
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

    for (int i = 0; i < max_jets; i++)
    {
        for (int j = i + 1; j < max_jets; j++)
        {
            edge_index0.push_back(i);
            edge_index1.push_back(j);
            // undirected graph
            float this_deltaR = Jets[i].DeltaR(Jets[j]);
            float this_invM = (Jets[i] + Jets[j]).M();
            TVector3 v1 = Jets[i].Vect();
            TVector3 v2 = Jets[j].Vect();
            float this_cosTheta = TMath::Cos(v1.Angle(v2));
            for (size_t k = 0; k < 2; k++)
            {
                deltaR.push_back(this_deltaR);
                invM.push_back(this_invM);
                cosTheta.push_back(this_cosTheta);
            }
        }
    }

    SetBranch("Training_Tree", "KF_b1_idx", best_KF_result.best_b1_idx);
    SetBranch("Training_Tree", "KF_b2_idx", best_KF_result.best_b2_idx);
    SetBranch("Training_Tree", "KF_w11_idx", best_KF_result.best_w11_idx);
    SetBranch("Training_Tree", "KF_w12_idx", best_KF_result.best_w12_idx);
    SetBranch("Training_Tree", "KF_w21_idx", best_KF_result.best_w21_idx);
    SetBranch("Training_Tree", "KF_w22_idx", best_KF_result.best_w22_idx);
    SetBranch("Training_Tree", "KF_chi2", best_KF_result.chi2);

    int answer;
    if (IsDATA)
        answer = -999;
    // check TT is in MCSample string
    if (MCSample.Contains("TT"))
    {
        if (MCSample.Contains("Vcb"))
            answer = category_for_training_FH["Vcb"];
        else if (ttLF)
            answer = category_for_training_FH["TT"];
        else
            answer = category_for_training_FH["TT_tthf"];
    }
    else
        answer = category_for_training_SL["Others"];
    SetBranch("Training_Tree", "y", answer);

    FillTrees();
}

RVec<int> Vcb_FH::FindTTbarJetIndices()
{
    // Return structure of size 6:
    //   {
    //     idx_b_t,        // b from top
    //     idx_b_tbar,     // b from tbar
    //     idx_Wplus_q1,   // W+ daughter quark #1
    //     idx_Wplus_q2,   // W+ daughter quark #2
    //     idx_Wminus_q1,  // W- daughter quark #1
    //     idx_Wminus_q2   // W- daughter quark #2
    //   }
    // Each entry is the matched Reco Jet index (or -1/-999 if not found).
    RVec<int> ttbar_jet_indices = {-1, -1, -1, -1, -1, -1};

    // For debugging or classification
    tt_decay_code = -9999; // optional for user-specific classification
    find_all_jets = false; // will set to true if all 6 jets are found

    // -----------------------------------------------------------------
    // 1) Find last copies of t, tbar, W+, W- in the Gen collection
    // ------------------------------------------------------------------
    int idx_t_plus = -1;
    int idx_t_minus = -1;
    int idx_w_plus = -1;
    int idx_w_minus = -1;
    bool found_t_plus = false;
    bool found_t_minus = false;
    bool found_w_plus = false;
    bool found_w_minus = false;

    // Traverse from the end so the first time we see them is the "last copy".
    for (int i = (int)AllGens.size() - 1; i >= 0; i--)
    {
        const int pid = AllGens[i].PID();
        if (!found_w_plus && pid == 24)
        {
            idx_w_plus = i;
            found_w_plus = true;
        }
        if (!found_w_minus && pid == -24)
        {
            idx_w_minus = i;
            found_w_minus = true;
        }
        if (!found_t_plus && pid == 6)
        {
            idx_t_plus = i;
            found_t_plus = true;
        }
        if (!found_t_minus && pid == -6)
        {
            idx_t_minus = i;
            found_t_minus = true;
        }

        if (found_w_plus && found_w_minus && found_t_plus && found_t_minus)
            break;
    }

    // If for some reason we fail to find them, exit early
    if (!found_w_plus || !found_w_minus || !found_t_plus || !found_t_minus)
    {
        std::cout << "[ERROR] Did not find last copies of t/tbar/W+/W- in full-hadronic routine.\n";
        return ttbar_jet_indices; // all -1
    }

    // ------------------------------------------------------------------
    // 2) Find final b-quarks from top and tbar, and final quark daughters of W+ and W-
    // ------------------------------------------------------------------
    bool found_b_from_t_plus = false;
    bool found_b_from_t_minus = false;

    int idx_b_from_t_plus = -1;
    int idx_b_from_t_minus = -1;

    // W+ daughters:
    int idx_wplus_dau1 = -1;
    int idx_wplus_dau2 = -1;

    // W- daughters:
    int idx_wminus_dau1 = -1;
    int idx_wminus_dau2 = -1;

    // Loop over Gens (backwards) to find first copies of quarks from t, tbar, W+, W-
    for (int i = (int)AllGens.size() - 1; i >= 0; i--)
    {
        const auto &g = AllGens[i];
        const int pid = g.PID();

        // We only care about final stable copies of quarks, so skip if not firstCopy
        if (!g.isFirstCopy())
            continue;

        // Also skip if not from the hard process / prompt
        // (this helps filter out quarks from hadron decays, QED FSR, etc.)
        if (!g.isPrompt() || !g.fromHardProcess())
            continue;

        // Identify b from top
        if (!found_b_from_t_plus && pid == 5 && isDaughterOf(i, idx_t_plus))
        {
            idx_b_from_t_plus = i;
            found_b_from_t_plus = true;
        }
        // Identify b-bar from tbar
        if (!found_b_from_t_minus && pid == -5 && isDaughterOf(i, idx_t_minus))
        {
            idx_b_from_t_minus = i;
            found_b_from_t_minus = true;
        }

        // W+ daughters
        if (isDaughterOf(i, idx_w_plus) && (isPIDUpTypeQuark(pid) || isPIDDownTypeQuark(pid)))
        {
            if (idx_wplus_dau1 < 0)
                idx_wplus_dau1 = i;
            else if (idx_wplus_dau2 < 0)
                idx_wplus_dau2 = i;
        }

        // W- daughters
        if (isDaughterOf(i, idx_w_minus) && (isPIDUpTypeQuark(pid) || isPIDDownTypeQuark(pid)))
        {
            if (idx_wminus_dau1 < 0)
                idx_wminus_dau1 = i;
            else if (idx_wminus_dau2 < 0)
                idx_wminus_dau2 = i;
        }

        // If we've found everything, we can break early
        if (found_b_from_t_plus && found_b_from_t_minus &&
            idx_wplus_dau1 >= 0 && idx_wplus_dau2 >= 0 &&
            idx_wminus_dau1 >= 0 && idx_wminus_dau2 >= 0)
        {
            break;
        }
    }

    // Now check that indeed we have all b’s and all quark daughters from each W
    bool found_all_gen = (found_b_from_t_plus && found_b_from_t_minus &&
                          idx_wplus_dau1 >= 0 && idx_wplus_dau2 >= 0 &&
                          idx_wminus_dau1 >= 0 && idx_wminus_dau2 >= 0);

    if (!found_all_gen)
    {
        // For debugging, you can print out info here if desired
        return ttbar_jet_indices; // all -1
    }

    // For possible debugging or classification—e.g., you can build a code from the quark PIDs
    int pid_wp1 = AllGens[idx_wplus_dau1].PID();
    int pid_wp2 = AllGens[idx_wplus_dau2].PID();
    int pid_wm1 = AllGens[idx_wminus_dau1].PID();
    int pid_wm2 = AllGens[idx_wminus_dau2].PID();
    tt_decay_code = abs(pid_wp1) * 1000 + abs(pid_wp2) * 100 + abs(pid_wm1) * 10 + abs(pid_wm2);

    // (Optional) Fill a histogram for passing these Gen-level selections
    FillHist("FindTT_FullHad_CutFlow", 0, 1.f, 10, 0., 10.);

    // ------------------------------------------------------------------
    // 3) Put these 6 quarks into a container in a known order
    // ------------------------------------------------------------------
    // Indices:
    //   0 => b_t,
    //   1 => b_tbar,
    //   2 => W+ q1,
    //   3 => W+ q2,
    //   4 => W- q1,
    //   5 => W- q2
    RVec<Gen> relevant_gens(6);
    relevant_gens[0] = AllGens[idx_b_from_t_plus];
    relevant_gens[1] = AllGens[idx_b_from_t_minus];
    relevant_gens[2] = AllGens[idx_wplus_dau1];
    relevant_gens[3] = AllGens[idx_wplus_dau2];
    relevant_gens[4] = AllGens[idx_wminus_dau1];
    relevant_gens[5] = AllGens[idx_wminus_dau2];

    // ------------------------------------------------------------------
    // 4) Match these 6 Gen objects to GenJets
    // ------------------------------------------------------------------
    // Build a map from PID -> vector of (indexInRelevantGens, Gen object)
    // so that each quark tries to match to GenJets of the same partonFlavour.
    std::unordered_map<int, RVec<std::pair<size_t, Gen>>> map_pid_to_genIdxObj;
    for (size_t iG = 0; iG < relevant_gens.size(); iG++)
    {
        int pid = relevant_gens[iG].PID();
        pid = GetSimplePID(pid, false);
        map_pid_to_genIdxObj[pid].push_back({iG, relevant_gens[iG]});
    }

    // PartonFlavour array for all GenJets
    RVec<int> genjet_flavours(AllGenJets.size());
    for (size_t i = 0; i < AllGenJets.size(); i++)
    {
        int genJet_flavour = AllGenJets[i].partonFlavour();
        genJet_flavour = GetSimplePID(genJet_flavour, false);
        genjet_flavours[i] = genJet_flavour;
    }

    // This map will store, for each final-state Gen quark, the index of the matched GenJet
    // Start all unmatched => -999
    std::unordered_map<size_t, int> map_genIndex_to_genJetIdx;
    for (size_t iG = 0; iG < relevant_gens.size(); iG++)
        map_genIndex_to_genJetIdx[iG] = -999;

    // Perform deltaR matching within each PID group
    for (auto &kv : map_pid_to_genIdxObj)
    {
        int target_pid = kv.first;
        auto &idxGenPairs = kv.second; // each element is { iG, GenObject }

        // Gather candidate GenJets that have the same partonFlavour as target_pid
        RVec<GenJet> candidateGenJets;
        RVec<int> candidateGJIndices;
        for (size_t j = 0; j < AllGenJets.size(); j++)
        {
            if (genjet_flavours[j] == target_pid)
            {
                candidateGenJets.push_back(AllGenJets[j]);
                candidateGJIndices.push_back(j);
            }
        }

        // If no GenJet found for this PID, mark as unmatched
        if (candidateGenJets.empty())
        {
            for (auto &p : idxGenPairs)
                map_genIndex_to_genJetIdx.at(p.first) = -1;
            continue;
        }

        // Prepare only the Gen objects that share that PID
        RVec<Gen> these_gens;
        these_gens.reserve(idxGenPairs.size());
        for (auto &p : idxGenPairs)
            these_gens.push_back(p.second);


        // Perform deltaR matching
        auto result_map = deltaRMatching(these_gens, candidateGenJets, 0.4);

        // Store the results
        // result_map[iGenInGroup] -> iJetInCandidate (or -1 if none matched)
        for (auto &matchPair : result_map)
        {
            size_t iGenInGroup = matchPair.first;
            int iGJetInGroup = matchPair.second;
            size_t relevantGIndex = idxGenPairs[iGenInGroup].first;

            if (iGJetInGroup >= 0)
                map_genIndex_to_genJetIdx.at(relevantGIndex) = candidateGJIndices[iGJetInGroup];
            else
                map_genIndex_to_genJetIdx.at(relevantGIndex) = -1;
        }
    }

    // Check how many are matched
    bool matched_all_genJets = true;
    for (size_t iG = 0; iG < relevant_gens.size(); iG++)
    {
        if (map_genIndex_to_genJetIdx[iG] < 0)
        {
            matched_all_genJets = false;
            break;
        }
    }
    if (matched_all_genJets)
        FillHist("FindTT_FullHad_CutFlow", 1, 1.f, 10, 0., 10.);

    // ------------------------------------------------------------------
    // 5) Match the GenJets to Reco Jets
    // ------------------------------------------------------------------
    // Build a subset of GenJets that *were* matched
    RVec<GenJet> matchedGenJets;
    RVec<size_t> matchedGenIndices; // which of the 6 quarks did it come from
    for (size_t iG = 0; iG < relevant_gens.size(); iG++)
    {
        int gjIdx = map_genIndex_to_genJetIdx[iG];
        if (gjIdx >= 0)
        {
            matchedGenJets.push_back(AllGenJets[gjIdx]);
            matchedGenIndices.push_back(iG);
        }
    }

    // Perform the GenJet -> RecoJet matching
    // e.g.  GenJetMatching(yourRecoJets, matchedGenJets, evt.GetRho(), dR=0.4, pTdiff=INFINITY)
    auto recoMatchMap = GenJetMatching(Jets, matchedGenJets, ev.GetRho(), 0.4, INFINITY);

    // Invert that map so we can see for the i-th GenJet in matchedGenJets which RecoJet was matched
    std::unordered_map<int, int> map_genJetIdx_inSubset_to_recoJetIdx;
    for (auto &kv : recoMatchMap)
    {
        int iRecoJet = kv.first;
        int iGenJetSubidx = kv.second; // index in matchedGenJets
        if (iGenJetSubidx >= 0)
            map_genJetIdx_inSubset_to_recoJetIdx[iGenJetSubidx] = iRecoJet;
    }

    // Fill the final 6-element array in the order:
    //   { b_t, b_tbar, W+q1, W+q2, W-q1, W-q2 }
    for (const auto &kv : map_genJetIdx_inSubset_to_recoJetIdx)
    {
        int iGenJetSub = kv.first;
        int iRecoJet = kv.second;
        int whichQuark = matchedGenIndices[iGenJetSub];
        ttbar_jet_indices[whichQuark] = iRecoJet;
    }

    // Check if all 6 are matched
    find_all_jets = true;
    for (auto idx : ttbar_jet_indices)
    {
        if (idx < 0)
        {
            find_all_jets = false;
            break;
        }
    }

    if (find_all_jets)
        FillHist("FindTT_FullHad_CutFlow", 2, 1.f, 10, 0., 10.);

    // ------------------------------------------------------------------
    // 6) (Optional) Fill some Gen-level/Reco-level histograms
    // ------------------------------------------------------------------
    if (find_all_jets)
    {
        // Reconstruct top from (b_t + W+ quarks)
        Particle Wplus_gen = AllGens[idx_wplus_dau1] + AllGens[idx_wplus_dau2];
        Particle TopPlus_gen = Wplus_gen + AllGens[idx_b_from_t_plus];

        // Reconstruct tbar from (b_tbar + W- quarks)
        Particle Wminus_gen = AllGens[idx_wminus_dau1] + AllGens[idx_wminus_dau2];
        Particle TopMinus_gen = Wminus_gen + AllGens[idx_b_from_t_minus];

        // Similarly, you can reconstruct from Reco-level Jets:
        Particle Wplus_reco = Jets[ttbar_jet_indices[2]] + Jets[ttbar_jet_indices[3]];
        Particle TopPlus_reco = Wplus_reco + Jets[ttbar_jet_indices[0]];

        Particle Wminus_reco = Jets[ttbar_jet_indices[4]] + Jets[ttbar_jet_indices[5]];
        Particle TopMinus_reco = Wminus_reco + Jets[ttbar_jet_indices[1]];

        // Example histograms
        FillHist("genLevel_TopPlusMass", TopPlus_gen.M(), 1.f, 100, 100., 300.);
        FillHist("genLevel_TopMinusMass", TopMinus_gen.M(), 1.f, 100, 100., 300.);
        FillHist("genLevel_WplusMass", Wplus_gen.M(), 1.f, 100, 50., 110.);
        FillHist("genLevel_WminusMass", Wminus_gen.M(), 1.f, 100, 50., 110.);

        FillHist("recoLevel_TopPlusMass", TopPlus_reco.M(), 1.f, 100, 100., 300.);
        FillHist("recoLevel_TopMinusMass", TopMinus_reco.M(), 1.f, 100, 100., 300.);
        FillHist("recoLevel_WplusMass", Wplus_reco.M(), 1.f, 100, 50., 110.);
        FillHist("recoLevel_WminusMass", Wminus_reco.M(), 1.f, 100, 50., 110.);
    }

    return ttbar_jet_indices;
}

void Vcb_FH::GetKineMaticFitterResult(const RVec<Jet> &jets)
{
    RVec<RVec<unsigned int>> possible_permutations = GetPermutations(jets);
    best_KF_result.chi2 = 9999.;
    for (const auto &permutation : possible_permutations)
    {
        auto result = FitKinFitter(jets, permutation);
        int status = std::get<0>(result);
        float chi2 = std::get<1>(result);
        RVec<unsigned int> this_permutation = std::get<2>(result);
        RVec<TLorentzVector> fitted_result = std::get<3>(result);
        if (chi2 < best_KF_result.chi2 && status == 0)
        {
            best_KF_result.status = status;
            best_KF_result.chi2 = chi2;
            best_KF_result.best_b1_idx = this_permutation[0];
            best_KF_result.best_b2_idx = this_permutation[1];
            best_KF_result.best_w11_idx = this_permutation[2];
            best_KF_result.best_w12_idx = this_permutation[3];
            best_KF_result.best_w21_idx = this_permutation[4];
            best_KF_result.best_w22_idx = this_permutation[5];
            best_KF_result.fitted_b1 = fitted_result[0];
            best_KF_result.fitted_b2 = fitted_result[1];
            best_KF_result.fitted_w11 = fitted_result[2];
            best_KF_result.fitted_w12 = fitted_result[3];
            best_KF_result.fitted_w21 = fitted_result[4];
            best_KF_result.fitted_w22 = fitted_result[5];
        }
    }
}

tuple<int, float, RVec<unsigned int>, RVec<TLorentzVector>> Vcb_FH::FitKinFitter(const RVec<Jet> &jets, const RVec<unsigned int> &permutation)
{
    // Initialize the fitter with smart pointers
    std::unique_ptr<TKinFitter> fitter = std::make_unique<TKinFitter>("fitter", "fitter");
    fitter->reset();
    fitter->setVerbosity(0);

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
    auto b1 = std::make_unique<TFitParticlePt>("b1", "b1", &(this_pts[permutation[0]]), &(this_JERs[permutation[0]]));
    auto b2 = std::make_unique<TFitParticlePt>("b2", "b2", &(this_pts[permutation[1]]), &(this_JERs[permutation[1]]));
    auto w11 = std::make_unique<TFitParticlePt>("w11", "w11", &(this_pts[permutation[2]]), &(this_JERs[permutation[2]]));
    auto w12 = std::make_unique<TFitParticlePt>("w12", "w12", &(this_pts[permutation[3]]), &(this_JERs[permutation[3]]));
    auto w21 = std::make_unique<TFitParticlePt>("w21", "w21", &(this_pts[permutation[4]]), &(this_JERs[permutation[4]]));
    auto w22 = std::make_unique<TFitParticlePt>("w22", "w22", &(this_pts[permutation[5]]), &(this_JERs[permutation[5]]));

    // Mass and energy-momentum constraints
    // auto mW1 = std::make_unique<TFitConstraintMGaus>("MW1", "MW1", nullptr, nullptr, W_MASS, W_WIDTH);
    auto mW1 = std::make_unique<TFitConstraintM>("MW1", "MW1", nullptr, nullptr, W_MASS);
    mW1->addParticle1(w11.get());
    mW1->addParticle1(w12.get());

    // auto mW2 = std::make_unique<TFitConstraintMGaus>("MW2", "MW2", nullptr, nullptr, W_MASS, W_WIDTH);
    auto mW2 = std::make_unique<TFitConstraintM>("MW2", "MW2", nullptr, nullptr, W_MASS);
    mW2->addParticle1(w21.get());
    mW2->addParticle1(w22.get());

    auto mT1 = std::make_unique<TFitConstraintM>("MT1", "MT1", nullptr, nullptr, T_MASS);
    mT1->addParticle1(b1.get());
    mT1->addParticle1(w11.get());
    mT1->addParticle1(w12.get());

    auto mT2 = std::make_unique<TFitConstraintM>("MT2", "MT2", nullptr, nullptr, T_MASS);
    mT2->addParticle1(b2.get());
    mT2->addParticle1(w21.get());
    mT2->addParticle1(w22.get());

    // Add particles and constraints to the fitter
    fitter->addMeasParticle(b1.get());
    fitter->addMeasParticle(b2.get());
    fitter->addMeasParticle(w11.get());
    fitter->addMeasParticle(w12.get());
    fitter->addMeasParticle(w21.get());
    fitter->addMeasParticle(w22.get());

    // extra jets
    for (size_t i = 0; i < jets.size(); i++)
    {
        if (std::find(permutation.begin(), permutation.end(), i) == permutation.end())
        {
            auto extra_jet = std::make_unique<TFitParticlePt>("extra_jet_" + std::to_string(i), "extra_jet_" + std::to_string(i), &(this_pts[i]), &(this_JERs[i]));
            fitter->addMeasParticle(extra_jet.get());
        }
    }

    fitter->addConstraint(mW1.get());
    fitter->addConstraint(mW2.get());
    fitter->addConstraint(mT1.get());
    fitter->addConstraint(mT2.get());

    // Perform the fit
    fitter->fit();

    int status = fitter->getStatus();
    float chi2 = fitter->getS();
    // Collect fitted particles
    RVec<TLorentzVector> fitted_result = {
        TLorentzVector(*b1->getCurr4Vec()),
        TLorentzVector(*b2->getCurr4Vec()),
        TLorentzVector(*w11->getCurr4Vec()),
        TLorentzVector(*w12->getCurr4Vec()),
        TLorentzVector(*w21->getCurr4Vec()),
        TLorentzVector(*w22->getCurr4Vec())};

    // Return the result as a tuple
    return std::make_tuple(status, chi2, permutation, fitted_result);
}

void Vcb_FH::InferONNX()
{
    // Prepare input tensor
    std::vector<uint8_t> MASK;
    std::vector<float> pt;
    std::vector<float> eta;
    std::vector<float> sin_phi;
    std::vector<float> cos_phi;
    std::vector<float> m;
    std::vector<float> btag;
    std::vector<float> Momenta_data;
    std::vector<uint8_t> Momenta_mask;

    int max_jet = 10;
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
        }
        else
        {
            pt.push_back(0.);
            eta.push_back(0.);
            sin_phi.push_back(0.);
            cos_phi.push_back(0.);
            m.push_back(0.);
            btag.push_back(0.);
        }
    }

    std::unordered_map<std::string, std::vector<int>> input_shape;
    input_shape["Momenta_data"] = {1, 10, 6};
    input_shape["Momenta_mask"] = {1, 10};

    // row-major order
    for (size_t i = 0; i < pt.size(); i++)
    {
        Momenta_data.push_back(pt[i]);
        Momenta_data.push_back(eta[i]);
        Momenta_data.push_back(sin_phi[i]);
        Momenta_data.push_back(cos_phi[i]);
        Momenta_data.push_back(m[i]);
        Momenta_data.push_back(btag[i]);
        Momenta_mask.push_back(MASK[i]);
    }

    std::unordered_map<std::string, VariousArray> input_data;
    input_data["Momenta_data"] = Momenta_data;
    input_data["Momenta_mask"] = Momenta_mask;

    std::unordered_map<std::string, FloatArray> output_data = myMLHelper->Run_ONNX_Model(input_data, input_shape);


    for(size_t i = 0; i < class_score.size(); i++)
    {
        
        class_score[i] = (std::exp((output_data.at("EVENT/signal").at(i))));
    }

    // find the assignment from output_data["t_assignment_log_probability"], output_data["ht_assignment_log_probability"]
    int t1b_assignment = -1;
    int t2b_assignment = -1;
    int t1q1_assignment = -1;
    int t1q2_assignment = -1;
    int t2q1_assignment = -1;
    int t2q2_assignment = -1;

    std::vector<int> t1_assignment_shape = {1, 10, 10, 10};
    std::vector<int> t2_assignment_shape = {1, 10, 10, 10};

    //first find the most lt probable assignment

    size_t max_idx = FindNthMaxIndex(output_data["t1_assignment_log_probability"], 0);
    std::vector<int> current_t1_assignment = UnravelIndex(max_idx, t1_assignment_shape);

    t1b_assignment = current_t1_assignment[1];
    t1q1_assignment = current_t1_assignment[2];
    t1q2_assignment = current_t1_assignment[3];

    bool checkUnique = false;
    int num_total_t2_assignments = output_data["t2_assignment_log_probability"].size();

    for (size_t i = 0; i < num_total_t2_assignments; i++)
    {
        size_t current_max_idx = FindNthMaxIndex(output_data["t2_assignment_log_probability"], i);
        std::vector<int> current_t2_assignment = UnravelIndex(current_max_idx, t2_assignment_shape);
        
        t2b_assignment = current_t2_assignment[1];
        t2q1_assignment = current_t2_assignment[2];
        t2q2_assignment = current_t2_assignment[3];

        std::set<int> unique_assignment = {t1b_assignment, t1q1_assignment, t1q2_assignment, t2b_assignment, t2q1_assignment, t2q2_assignment};
        if (unique_assignment.size() == 6)
        {
            checkUnique = true;
        }
    
        if (checkUnique)
        {
            break;
        }
    }

    Particle w1 = Jets[t1q1_assignment] + Jets[t1q2_assignment];
    Particle w2 = Jets[t2q1_assignment] + Jets[t2q2_assignment];
    Particle t1 = Jets[t1b_assignment] + w1;
    Particle t2 = Jets[t2b_assignment] + w2;

    assignment[0] = t1b_assignment;
    assignment[1] = t2b_assignment;
    assignment[2] = t1q1_assignment;
    assignment[3] = t1q2_assignment;
    assignment[4] = t2q1_assignment;
    assignment[5] = t2q2_assignment;


    // find which class has the highest score in class_score(find index)
    std::array<float, 3> class_score_temp = {class_score[0], class_score[1], class_score[2]};
    int max_class;
    if(class_score_temp[2] > 0.007) max_class = 2;
    else{
        if(class_score_temp[0] > 0.8) max_class = 0;
        else max_class = 1;
    }
    
    switch (max_class){
        case 0:
            class_label  = classCategory::Signal;
            break;
        case 1:
            class_label = classCategory::tt;
            break;
        case 2:
            class_label = classCategory::Disposal;
            break;
        default:
            break;
    }
}

void Vcb_FH::FillONNXRecoInfo(const TString &histPrefix, float weight)
{
    ttbar_jet_indices = FindTTbarJetIndices();
    // if (find_all_jets)
    // {
    //     FillHist(histPrefix + "/" + "CorrectAssignment_Tot", n_jets, n_b_tagged_jets, 1., 6, 4., 10., 4, 2, 6);
    //     bool isCorrect = true;
    //     //check if the assignment is correct. w1 and w2 can be swapped
    //     //if (assignment[0] != ttbar_jet_indices[0]) isCorrect = false;
    //     //if (assignment[1] != ttbar_jet_indices[1]) isCorrect = false;
    //     if ((assignment[2] != ttbar_jet_indices[2] || assignment[3] != ttbar_jet_indices[3]) && (assignment[2] != ttbar_jet_indices[3] || assignment[3] != ttbar_jet_indices[2])) isCorrect = false;
    //     if (isCorrect) FillHist(histPrefix + "/" + "CorrectAssignment", n_jets, n_b_tagged_jets, 1., 6, 4., 10., 4, 2, 6);
    //     else FillHist(histPrefix + "/" + "WrongAssignment", n_jets, n_b_tagged_jets, 1., 6, 4., 10., 4, 2, 6);
    // }
    if(Jets[assignment[2]].GetBTaggerResult(FlavTagger[DataEra.Data()]) > Jets[assignment[3]].GetBTaggerResult(FlavTagger[DataEra.Data()]))
    {
        //swap the assignment
        int temp = assignment[2];
        assignment[2] = assignment[3];
        assignment[3] = temp;
    }
    if(Jets[assignment[4]].GetBTaggerResult(FlavTagger[DataEra.Data()]) > Jets[assignment[5]].GetBTaggerResult(FlavTagger[DataEra.Data()]))
    {
        //swap the assignment
        int temp = assignment[4];
        assignment[4] = assignment[5];
        assignment[5] = temp;
    }
    if(Jets[assignment[3]].GetBTaggerResult(FlavTagger[DataEra.Data()]) > Jets[assignment[5]].GetBTaggerResult(FlavTagger[DataEra.Data()]))
    {
        //swap the assignment
        std::swap(assignment[3], assignment[5]);
        std::swap(assignment[2], assignment[4]);
        std::swap(assignment[0], assignment[1]);
    }
    Particle w1 = Jets[assignment[2]] + Jets[assignment[3]];
    Particle w2 = Jets[assignment[4]] + Jets[assignment[5]];
    Particle t1 = Jets[assignment[0]] + w1;
    Particle t2 = Jets[assignment[1]] + w2;
    float W11_BvsC = Jets[assignment[2]].GetBTaggerResult(FlavTagger[DataEra.Data()]);
    float W12_BvsC = Jets[assignment[3]].GetBTaggerResult(FlavTagger[DataEra.Data()]);
    float W21_BvsC = Jets[assignment[4]].GetBTaggerResult(FlavTagger[DataEra.Data()]);
    float W22_BvsC = Jets[assignment[5]].GetBTaggerResult(FlavTagger[DataEra.Data()]);

    FillHist(histPrefix + "/" + "Reco_HadWMass1", w1.M(), weight, 100, 30., 130.);
    FillHist(histPrefix + "/" + "Reco_HadTopMass1", t1.M(), weight, 100, 100., 300.);
    FillHist(histPrefix + "/" + "Reco_HadWMass2", w2.M(), weight, 100, 30., 130.);
    FillHist(histPrefix + "/" + "Reco_HadTopMass2", t2.M(), weight, 100, 100., 300.);
    FillHist(histPrefix + "/" + "Reco_W11JetPt", Jets[assignment[2]].Pt(), weight, 100, 0., 200.);
    FillHist(histPrefix + "/" + "Reco_W12JetPt", Jets[assignment[3]].Pt(), weight, 100, 0., 200.);
    FillHist(histPrefix + "/" + "Reco_W21JetPt", Jets[assignment[4]].Pt(), weight, 100, 0., 200.);
    FillHist(histPrefix + "/" + "Reco_W22JetPt", Jets[assignment[5]].Pt(), weight, 100, 0., 200.);
    FillHist(histPrefix + "/" + "Reco_W11BvsC", W11_BvsC, weight, 100, 0., 1.);
    FillHist(histPrefix + "/" + "Reco_W12BvsC", W12_BvsC, weight, 100, 0., 1.);
    FillHist(histPrefix + "/" + "Reco_W21BvsC", W21_BvsC, weight, 100, 0., 1.);
    FillHist(histPrefix + "/" + "Reco_W22BvsC", W22_BvsC, weight, 100, 0., 1.);
    FillHist(histPrefix + "/" + "Reco_t1bJetPt", Jets[assignment[0]].Pt(), weight, 100, 0., 200.);
    FillHist(histPrefix + "/" + "Reco_t2bJetPt", Jets[assignment[1]].Pt(), weight, 100, 0., 200.);
    FillHist(histPrefix + "/" + "Reco_t1bBvsC", Jets[assignment[0]].GetBTaggerResult(FlavTagger[DataEra.Data()]), weight, 100, 0., 1.);
    FillHist(histPrefix + "/" + "Reco_t2bBvsC", Jets[assignment[1]].GetBTaggerResult(FlavTagger[DataEra.Data()]), weight, 100, 0., 1.);

    FillHist(histPrefix + "/" + "Reco_BvsCAdded" , W21_BvsC + W22_BvsC, weight, 100, 0., 2.);
    int unrolledIdx = Unroller(Jets[assignment[4]], Jets[assignment[5]]);
    FillHist(histPrefix + "/" + "Reco_W1Bvsc_W2Bvsc_Unrolled", unrolledIdx, weight, 16, 0., 16.);
    std::vector<float> class_score_bin = {0.,0.5,0.6,0.7,0.8,0.85,0.9,0.95};
    FillHist(histPrefix + "/" + "Class_Category", static_cast<float>(class_label), weight, 3, 0., 3.);
    FillHist(histPrefix + "/" + "Class_Score0", static_cast<float>(class_score[0]), weight, class_score_bin.size() - 1 , class_score_bin.data());
    FillHist(histPrefix + "/" + "Class_Score1", static_cast<float>(class_score[1]), weight, class_score_bin.size() - 1 , class_score_bin.data());
    FillHist(histPrefix + "/" + "Class_Score2", static_cast<float>(class_score[2]), weight, class_score_bin.size() - 1 , class_score_bin.data());

}
