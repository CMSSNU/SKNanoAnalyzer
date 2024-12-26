#include "Vcb_FH.h"
//#include "MLHelper.h"

Vcb_FH::Vcb_FH() {}

void Vcb_FH::CreateTrainingTree(){
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
    if (!ev.PassTrigger(FH_Trigger_DoubleBTag[DataEra.Data()])) return false;
    if(!PassJetVetoMap(AllJets, AllMuons)) return false; 
    if(!PassMetFilter(AllJets, ev)) return false;

    //Set Objects
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
    HT = GetHT(Jets);
    n_jets = Jets.size();

    if(n_jets < 6) return false;

    for (const auto &jet : Jets)
    {
        if (jet.GetBTaggerResult(FlavTagger[DataEra.Data()]) > myCorr->GetBTaggingWP())
            n_b_tagged_jets++;
        if (jet.GetCTaggerResult(FlavTagger[DataEra.Data()]).first > myCorr->GetCTaggingWP().first &&
            jet.GetCTaggerResult(FlavTagger[DataEra.Data()]).second > myCorr->GetCTaggingWP().second)
            n_c_tagged_jets++;
        if(!IsDATA){
            if(abs(jet.partonFlavour()) == 5) n_partonFlav_b_jets++;
            if(abs(jet.partonFlavour()) == 4) n_partonFlav_c_jets++;
        }
    }
    if (n_b_tagged_jets < 2 && !remove_flavtagging_cut) return false;
    if(Muons_Veto.size() != 0 || Electrons_Veto.size() != 0) return false;
    if (HT < FH_HT_cut[DataEra.Data()]) return false;
    
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

void Vcb_FH::FillTrainingTree(){

    SetBranch("Training_Tree", "MET", MET.Pt());
    SetBranch("Training_Tree", "HT", HT);
    SetBranch("Training_Tree", "n_jets", n_jets);
    SetBranch("Training_Tree", "n_b_tagged_jets", n_b_tagged_jets);
    SetBranch("Training_Tree", "n_c_tagged_jets", n_c_tagged_jets);
    SetBranch("Training_Tree", "find_all_jets", find_all_jets);
    if(find_all_jets){
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
    else{
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
            
    //         auto it = find(ttbar_jet_indices.begin(), ttbar_jet_indices.end(), i);
    //         if(it != ttbar_jet_indices.end()){
    //             SetBranch("Training_Tree", "Jet_isTTbarJet_" + std::to_string(i), int(1));
    //             SetBranch("Training_Tree", "Jet_ttbarJet_idx_" + std::to_string(i), int(it - ttbar_jet_indices.begin()));
    //         }
    //         else{
    //             SetBranch("Training_Tree", "Jet_isTTbarJet_" + std::to_string(i), int(0));
    //             SetBranch("Training_Tree", "Jet_ttbarJet_idx_" + std::to_string(i), int(-999));
    //         }

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
    if(Jets.size() < max_jets) max_jets = Jets.size();


    for(size_t i = 0; i < max_jets; i++){
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
        if(it != ttbar_jet_indices.end()){
            Jet_isTTbarJet.push_back(1);
            Jet_ttbarJet_idx.push_back(it - ttbar_jet_indices.begin());
        }
        else{
            Jet_isTTbarJet.push_back(0);
            Jet_ttbarJet_idx.push_back(-999);
        }
    }

    for(int i = 0; i < max_jets; i++){
        for(int j = i + 1; j < max_jets; j++){
            edge_index0.push_back(i);
            edge_index1.push_back(j);
            //undirected graph
            float this_deltaR = Jets[i].DeltaR(Jets[j]);
            float this_invM = (Jets[i] + Jets[j]).M();
            TVector3 v1 = Jets[i].Vect();
            TVector3 v2 = Jets[j].Vect();
            float this_cosTheta = TMath::Cos(v1.Angle(v2));
            for(size_t k = 0; k < 2; k++){
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

RVec<int> Vcb_FH::FindTTbarJetIndices(){
    RVec<int> ttbar_jet_indices = {-1, -1, -1, -1, -1, -1};
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
    for(int i = AllGens.size() - 1; i >=0; i--){

        if(AllGens[i].PID() == 24 && !find_w_plus){
            w_plus_idx = i;
            find_w_plus = true;
        }
        if(AllGens[i].PID() == -24 && !find_w_minus){
            w_minus_idx = i;
            find_w_minus = true;
        }
        if(AllGens[i].PID() == 6 && !find_t_plus){
            t_plus_idx = i;
            find_t_plus = true;
        }
        if(AllGens[i].PID() == -6 && !find_t_minus){
            t_minus_idx = i;
            find_t_minus = true;
        }
        if(find_w_plus && find_w_minus && find_t_plus && find_t_minus) break;
    }

    int w_plus_up_idx = -1;
    int w_plus_down_idx = -1;
    int w_minus_up_idx = -1;
    int w_minus_down_idx = -1;

    // find W+/- daughters
    for(int i = AllGens.size() - 1; i >=0; i--){

        if (AllGens[i].MotherIndex() == w_plus_idx && isPIDUpTypeQuark(AllGens[i].PID()) && AllGens[i].PID() > 0 && !find_w_plus_up)
        {
            w_plus_up_idx = i;
            find_w_plus_up = true;
        }
        if (AllGens[i].MotherIndex() == w_plus_idx && isPIDDownTypeQuark(AllGens[i].PID()) && AllGens[i].PID() < 0 && !find_w_plus_down)
        {
            w_plus_down_idx = i;
            find_w_plus_down = true;
        }
        if (AllGens[i].MotherIndex() == w_minus_idx && isPIDUpTypeQuark(AllGens[i].PID()) && AllGens[i].PID() < 0 && !find_w_minus_up)
        {
            w_minus_up_idx = i;
            find_w_minus_up = true;
        }
        if (AllGens[i].MotherIndex() == w_minus_idx && isPIDDownTypeQuark(AllGens[i].PID()) && AllGens[i].PID() > 0 && !find_w_minus_down)
        {
            w_minus_down_idx = i;
            find_w_minus_down = true;
        }
    }

    int b_from_t_plus_idx = -1;
    int b_from_t_minus_idx = -1;

    for(int i = AllGens.size() - 1; i >=0; i--){

        if(AllGens[i].MotherIndex() == t_plus_idx && AllGens[i].PID() == 5 && !find_b_from_t_plus){
            b_from_t_plus_idx = i;
            find_b_from_t_plus = true;
        }
        if(AllGens[i].MotherIndex() == t_minus_idx && AllGens[i].PID() == -5 && !find_b_from_t_minus){
            b_from_t_minus_idx = i;
            find_b_from_t_minus = true;
        }
        if(find_b_from_t_plus && find_b_from_t_minus) break;
    }
    //print all gen that we found
    cout << "w plus up PID:" << AllGens[w_plus_up_idx].PID() << " w plus down PID:" << AllGens[w_plus_down_idx].PID() << " w minus up PID:" << AllGens[w_minus_up_idx].PID() << " w minus down PID:" << AllGens[w_minus_down_idx].PID() << " b from t plus PID:" << AllGens[b_from_t_plus_idx].PID() << " b from t minus PID:" << AllGens[b_from_t_minus_idx].PID() << endl; 

    bool findall_gen = (w_plus_up_idx >= 0 && w_plus_down_idx >= 0 && w_minus_up_idx >= 0 && w_minus_down_idx >= 0 && b_from_t_plus_idx >= 0 && b_from_t_minus_idx >= 0);
    if(!findall_gen) return ttbar_jet_indices;
    tt_decay_code = 1000 * abs(AllGens[w_plus_up_idx].PID()) + 100 * abs(AllGens[w_plus_down_idx].PID()) + 10 * abs(AllGens[w_minus_up_idx].PID()) + abs(AllGens[w_minus_down_idx].PID());
    RVec<Gen> this_gens = {AllGens[b_from_t_plus_idx], AllGens[b_from_t_minus_idx], AllGens[w_plus_up_idx], AllGens[w_plus_down_idx], AllGens[w_minus_up_idx], AllGens[w_minus_down_idx]};
    unordered_map<int,RVec<pair<size_t, Gen>>> gen_by_PID;

    for(size_t i = 0; i < this_gens.size(); i++){
        gen_by_PID[this_gens[i].PID()].push_back({i, this_gens[i]});
    }

    RVec<int> this_gens_PID = {this_gens[0].PID(), this_gens[1].PID(), this_gens[2].PID(), this_gens[3].PID(), this_gens[4].PID(), this_gens[5].PID()};
    RVec<int> this_genjet_PID;

    for(size_t i = 0; i < AllGenJets.size(); i++){
        this_genjet_PID.push_back(AllGenJets[i].partonFlavour());
    }

    unordered_map<int, int> gen_genjet_matching_result;

    for(const auto &PID_group : gen_by_PID){
        int current_PID = PID_group.first;
        const auto &gen_idx_Gen_pair = PID_group.second;

        RVec<GenJet> filtered_genjets;
        RVec<int> filtered_genjet_indices;

        for(size_t genjet_idx = 0; genjet_idx < this_genjet_PID.size(); genjet_idx++)
        {
            if (this_genjet_PID[genjet_idx] == current_PID)
            {
                filtered_genjets.push_back(AllGenJets[genjet_idx]);
                filtered_genjet_indices.push_back(genjet_idx);
            }
        }
        //if no genjets that matches to this PID found, this PID group is failed to match
        if(filtered_genjets.size() == 0){
            for(const auto &gen_idx_Gen_pair : gen_idx_Gen_pair){
                gen_genjet_matching_result[gen_idx_Gen_pair.first] = -999;
            }
        }
        //do deltaRMatching if we have matching genjets
        else{
            RVec<Gen> current_gen;
            RVec<int> current_gen_indices;
            for(const auto &gen_idx_Gen_pair : gen_idx_Gen_pair){
                current_gen.push_back(gen_idx_Gen_pair.second);
                current_gen_indices.push_back(gen_idx_Gen_pair.first);
            }
            unordered_map<int, int> temp_match = deltaRMatching(current_gen, filtered_genjets, 0.5);
            for(const auto &temp_match_pair : temp_match){
                if(temp_match_pair.second >= 0){
                    gen_genjet_matching_result[current_gen_indices[temp_match_pair.first]] = filtered_genjet_indices[temp_match_pair.second];
                }
                else{
                    gen_genjet_matching_result[current_gen_indices[temp_match_pair.first]] = -999;
                }
            }
        }
    }

    //we get gen-genjet matching. now do genjet-jet matching
    RVec<GenJet> gen_matched_genjet;
    RVec<size_t> gen_matched_genjet_indices;
    for(size_t i = 0; i < gen_genjet_matching_result.size(); i++){
        if (gen_genjet_matching_result[i] >= 0)
        {
            gen_matched_genjet.push_back(AllGenJets[gen_genjet_matching_result[i]]);
            gen_matched_genjet_indices.push_back(i);
        }
    }
    unordered_map<int, int> jet_genjet_matching_results = GenJetMatching(Jets, gen_matched_genjet, ev.GetRho());
    unordered_map<int, int> matching_result;

    //now store parton-jet matching result
    for(const auto jet_genjet_matching_result : jet_genjet_matching_results){
        if(jet_genjet_matching_result.second >= 0){
            matching_result[gen_matched_genjet_indices[jet_genjet_matching_result.second]] = jet_genjet_matching_result.first;
        }
        else{
            matching_result[gen_matched_genjet_indices[jet_genjet_matching_result.second]] = -999;
        }
    }

    for(size_t i = 0; i < 6; i++){
        if(matching_result.find(i) == matching_result.end()){
            matching_result[i] = -999;
        }
    }

    ttbar_jet_indices = {matching_result[0], matching_result[1], matching_result[2], matching_result[3], matching_result[4], matching_result[5]};
    for(size_t i = 0; i < ttbar_jet_indices.size(); i++){
        if(ttbar_jet_indices[i] == -999){
            find_all_jets = false;
            break;
        }
        else{
            find_all_jets = true;
        }
    }
    if(matching_result[0] >= 0 && matching_result[1] >= 0){
        //sort top system as pt order
        if(Jets[matching_result[0]].Pt() < Jets[matching_result[1]].Pt()){
            swap(ttbar_jet_indices[0], ttbar_jet_indices[1]);
            swap(ttbar_jet_indices[2], ttbar_jet_indices[4]);
            swap(ttbar_jet_indices[3], ttbar_jet_indices[5]);
        }
    }
    return ttbar_jet_indices;
}

void Vcb_FH::GetKineMaticFitterResult(const RVec<Jet> &jets){
    RVec<RVec<unsigned int>> possible_permutations = GetPermutations(jets);
    best_KF_result.chi2 = 9999.;
    for(const auto &permutation: possible_permutations){
        auto result = FitKinFitter(jets, permutation);
        int status = std::get<0>(result);
        float chi2 = std::get<1>(result);
        RVec<unsigned int> this_permutation = std::get<2>(result);
        RVec<TLorentzVector> fitted_result = std::get<3>(result);
        if(chi2 < best_KF_result.chi2 && status == 0){
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
    //auto mW1 = std::make_unique<TFitConstraintMGaus>("MW1", "MW1", nullptr, nullptr, W_MASS, W_WIDTH);
    auto mW1 = std::make_unique<TFitConstraintM>("MW1", "MW1", nullptr, nullptr, W_MASS);
    mW1->addParticle1(w11.get());
    mW1->addParticle1(w12.get());

    //auto mW2 = std::make_unique<TFitConstraintMGaus>("MW2", "MW2", nullptr, nullptr, W_MASS, W_WIDTH);
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

    //extra jets
    for(size_t i = 0; i < jets.size(); i++){
        if(std::find(permutation.begin(), permutation.end(), i) == permutation.end()){
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
    return std::make_tuple(status, chi2,permutation, fitted_result);
}
