#include "Vcb_SL.h"

Vcb_SL::Vcb_SL() {}
Vcb_SL::~Vcb_SL() {}

void Vcb_SL::initializeAnalyzer()
{
    myCorr = new Correction(DataEra, IsDATA ? DataStream : MCSample, IsDATA);
    myCorr->SetTaggingParam(FlavTagger[DataEra.Data()], SL_BTag_WP);
    // cset_TriggerSF = CorrectionSet::from_file("/data6/Users/yeonjoon/SKNanoOutput/FullHadronicTriggerTnP/2022EE/SF.json");
}

RVec<RVec<unsigned int>> Vcb_SL::GetPermutations(const RVec<Jet> &jets)
{
    RVec<RVec<unsigned int>> permutations;
    std::vector<unsigned int> b_tagged_idx;
    int max_jet = 8;
    if(jets.size() < 8) max_jet = jets.size();
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
            if (i == j) continue;
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

float Vcb_SL::CalculateEventWeight(bool include_tagging_weight)
{
    float weight = 1.;
    if (!IsDATA)
    {
        float puWeight = myCorr->GetPUWeight(ev.nTrueInt());
        float mcWeight = MCweight();
        float BTagShapeWeight = myCorr->GetBTaggingSF(Jets, JetTagging::JetTaggingSFMethod::shape);
        float LumiWeight = ev.GetTriggerLumi(El_Trigger[DataEra.Data()]); //Hardcoded because all triggers are unprescaled
        float LeptonWeight = 1.;
        float TopPtWeight = myCorr->GetTopPtReweight(AllGens);
        if (channel == Vcb_SL::Channel::Mu)
        {
            LeptonWeight *= myCorr->GetMuonIDSF(Mu_ID_SF_key[DataEra.Data()], Muons[0].Eta(), Muons[0].Pt());
            LeptonWeight *= myCorr->GetMuonISOSF(Mu_Iso_SF_key[DataEra.Data()], Muons[0].Eta(), Muons[0].Pt());
            LeptonWeight *= myCorr->GetMuonTriggerSF(Mu_Trigger_SF_Key[DataEra.Data()], Muons[0].Eta(), Muons[0].Pt());
        }
        else if(channel == Vcb_SL::Channel::El)
        {
            LeptonWeight *= myCorr->GetElectronIDSF(El_ID_SF_Key[DataEra.Data()], Electrons[0].Eta(), Electrons[0].Pt());
            LeptonWeight *= myCorr->GetElectronRECOSF(Electrons[0].Eta(), Electrons[0].Pt());
            //LeptonWeight *= myCorr->GetElectronTriggerSF(Electrons[0]);
        }
        weight *= puWeight * mcWeight * LumiWeight * LeptonWeight * TopPtWeight;
        if (include_tagging_weight) weight *= BTagShapeWeight;
    }
    return weight;
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
    else{
        float pz1 = (-b + TMath::Sqrt(determinant)) / (2.f * a);
        float pz2 = (-b - TMath::Sqrt(determinant)) / (2.f * a);
        return std::make_pair(pz1, pz2);
    }
}

bool Vcb_SL::PassBaseLineSelection(bool remove_flavtagging_cut, TString syst)
{
    FillCutFlow(0);
    if (channel == Vcb_SL::Channel::Mu) if (!ev.PassTrigger(Mu_Trigger[DataEra.Data()]))
        return false;
    if (channel == Vcb_SL::Channel::El) if (!ev.PassTrigger(El_Trigger[DataEra.Data()]))
        return false;
    FillCutFlow(1);
     if (!PassJetVetoMap(AllJets, AllMuons)) return false;
    FillCutFlow(2);
    if (!PassMetFilter(AllJets, ev))
        return false;
    FillCutFlow(3);
    Jets = SelectJets(AllJets, Jet_ID, SL_Jet_Pt_cut, Jet_Eta_cut);
    std::sort(Jets.begin(), Jets.end(), PtComparing);
    Muons_Veto = SelectMuons(AllMuons, Muon_Veto_ID, Muon_Veto_Pt, Muon_Veto_Eta);
    Muons_Veto = SelectMuons(Muons_Veto, Muon_Veto_Iso, Muon_Veto_Pt, Muon_Veto_Eta);
    Muons = SelectMuons(AllMuons, Muon_Tight_ID, Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
    Muons = SelectMuons(Muons, Muon_Tight_Iso, Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
    Electrons_Veto = SelectElectrons(AllElectrons, Electron_Veto_ID, Electron_Veto_Pt, Electron_Veto_Eta);
    Electrons = SelectElectrons(AllElectrons, Electron_Tight_ID, Electron_Tight_Pt[DataEra.Data()], Electron_Tight_Eta);


    HT = GetHT(Jets);
    n_jets = Jets.size();

    if (n_jets < 4) return false;
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
    if (channel == Vcb_SL::Channel::Mu)
    {
        if (!(Muons.size() == 1 && Electrons_Veto.size() == 0 && Muons_Veto.size() == 1))
            return false;
        lepton = Muons[0];
    }
    else if (channel == Vcb_SL::Channel::El)
    {
        if (!(Electrons.size() == 1 && Muons_Veto.size() == 0 && Electrons_Veto.size() == 1))
            return false;
        lepton = Electrons[0];
    }
    FillCutFlow(5);
    if(!IsDATA){
        ttbj = (genTtbarId%100==51 || genTtbarId%100==52);
        ttbb = (genTtbarId%100>=53 && genTtbarId%100<=55);
        ttcc = (genTtbarId%100>=41 && genTtbarId%100<=45);
        ttLF = !(ttbj || ttbb || ttcc);
    }
    if (n_b_tagged_jets < 2 && remove_flavtagging_cut)
        return false;
    FillCutFlow(6);
    return true;
}

void Vcb_SL::FillHistogramsAtThisPoint(const TString &histPrefix, float weight)
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
    }
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
    RVec<TString> keeps = {};
    RVec<TString> drops = {"*"};
    NewTree("Training_Tree", keeps, drops);
    SetBranch("Training_Tree", "MET", MET.Pt());
    SetBranch("Training_Tree", "HT", HT);
    SetBranch("Training_Tree", "n_jets", n_jets);
    SetBranch("Training_Tree", "n_b_tagged_jets", n_b_tagged_jets);
    SetBranch("Training_Tree", "n_c_tagged_jets", n_c_tagged_jets);
    SetBranch("Training_Tree", "weight", CalculateEventWeight());
    SetBranch("Training_Tree", "find_all_jets", find_all_jets); 

    for (size_t i = 0; i <= 8; i++)
    {
        if (i < n_jets)
        {
            SetBranch("Training_Tree", "Jet_Pt_" + std::to_string(i), Jets[i].Pt());
            SetBranch("Training_Tree", "Jet_Eta_" + std::to_string(i), Jets[i].Eta());
            SetBranch("Training_Tree", "Jet_Phi_" + std::to_string(i), Jets[i].Phi());
            SetBranch("Training_Tree", "Jet_M_" + std::to_string(i), Jets[i].M());
            SetBranch("Training_Tree", "Jet_BvsC_" + std::to_string(i), Jets[i].GetBTaggerResult(FlavTagger[DataEra.Data()]));
            SetBranch("Training_Tree", "Jet_CvsB_" + std::to_string(i), Jets[i].GetCTaggerResult(FlavTagger[DataEra.Data()]).first);
            SetBranch("Training_Tree", "Jet_CvsL_" + std::to_string(i), Jets[i].GetCTaggerResult(FlavTagger[DataEra.Data()]).second);
            // Tagging WP
            SetBranch("Training_Tree", "Jet_B_WP_" + std::to_string(i), GetPassedBTaggingWP(Jets[i]));
            SetBranch("Training_Tree", "Jet_C_WP_" + std::to_string(i), GetPassedCTaggingWP(Jets[i]));

            auto it = find(ttbar_jet_indices.begin(), ttbar_jet_indices.end(), i);
            if (it != ttbar_jet_indices.end())
            {
                SetBranch("Training_Tree", "Jet_isTTbarJet_" + std::to_string(i), int(1));
                SetBranch("Training_Tree", "Jet_ttbarJet_idx_" + std::to_string(i), int(it - ttbar_jet_indices.begin()));
            }
            else
            {
                SetBranch("Training_Tree", "Jet_isTTbarJet_" + std::to_string(i), int(0));
                SetBranch("Training_Tree", "Jet_ttbarJet_idx_" + std::to_string(i), int(-999));
            }
        }
        else
        {
            SetBranch("Training_Tree", "Jet_Pt_" + std::to_string(i), -999.);
            SetBranch("Training_Tree", "Jet_Eta_" + std::to_string(i), -999.);
            SetBranch("Training_Tree", "Jet_Phi_" + std::to_string(i), -999.);
            SetBranch("Training_Tree", "Jet_M_" + std::to_string(i), -999.);
            SetBranch("Training_Tree", "Jet_BvsC_" + std::to_string(i), -999.);
            SetBranch("Training_Tree", "Jet_CvsB_" + std::to_string(i), -999.);
            SetBranch("Training_Tree", "Jet_CvsL_" + std::to_string(i), -999.);
            SetBranch("Training_Tree", "Jet_isTTbarJet_" + std::to_string(i), int(-999));
            SetBranch("Training_Tree", "Jet_ttbarJet_idx_" + std::to_string(i), int(-999));
            SetBranch("Training_Tree", "Jet_B_WP_" + std::to_string(i), -999);
            SetBranch("Training_Tree", "Jet_C_WP_" + std::to_string(i), -999);
        }
    }
    SetBranch("Training_Tree", "lepton_Pt", lepton.Pt());
    SetBranch("Training_Tree", "lepton_Eta", lepton.Eta());
    SetBranch("Training_Tree", "lepton_Phi", lepton.Phi());
    SetBranch("Training_Tree", "lepton_M", lepton.M());
    SetBranch("Training_Tree", "lepton_Charge", lepton.Charge());
    SetBranch("Training_Tree", "lepton_isMuon", int(channel == Vcb_SL::Channel::Mu));
    SetBranch("Training_Tree", "lepton_isElectron", int(channel == Vcb_SL::Channel::El));

    
    TLorentzVector neutrino_p4;
    neutrino_p4.SetXYZM(MET.Px(), MET.Py(), best_KF_result.best_neu_pz, 0.);
    Particle neutrino(neutrino_p4);
    SetBranch("Training_Tree", "neutrino_Pt", neutrino.Pt());
    SetBranch("Training_Tree", "neutrino_Eta", neutrino.Eta());
    SetBranch("Training_Tree", "neutrino_Phi", neutrino.Phi());

    int answer;
    if(IsDATA) answer = -999;
    // check TT is in MCSample string
    if(MCSample.Contains("TT"))
    {
        if(MCSample.Contains("Vcb")) answer = category_for_training_SL["Vcb"];
        else if(ttLF) answer = category_for_training_SL["TT"];
        else answer = category_for_training_SL["TT_tthf"];
    }
    else answer = category_for_training_SL["Others"];
    SetBranch("Training_Tree", "y", answer);

    SetBranch("Training_Tree", "KF_chi2", best_KF_result.chi2);

    FillTrees();
}

short Vcb_SL::GetPassedBTaggingWP(const Jet &jet)
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

short Vcb_SL::GetPassedCTaggingWP(const Jet &jet)
{
    float CvsB_TightWP = myCorr->GetCTaggingWP(FlavTagger[DataEra.Data()], JetTagging::JetFlavTaggerWP::Tight).first;
    float CvsL_TightWP = myCorr->GetCTaggingWP(FlavTagger[DataEra.Data()], JetTagging::JetFlavTaggerWP::Tight).second;
    float CvsB_MediumWP = myCorr->GetCTaggingWP(FlavTagger[DataEra.Data()], JetTagging::JetFlavTaggerWP::Medium).first;
    float CvsL_MediumWP = myCorr->GetCTaggingWP(FlavTagger[DataEra.Data()], JetTagging::JetFlavTaggerWP::Medium).second;
    float CvsB_LooseWP = myCorr->GetCTaggingWP(FlavTagger[DataEra.Data()], JetTagging::JetFlavTaggerWP::Loose).first;
    float CvsL_LooseWP = myCorr->GetCTaggingWP(FlavTagger[DataEra.Data()], JetTagging::JetFlavTaggerWP::Loose).second;

    if (jet.GetCTaggerResult(FlavTagger[DataEra.Data()]).first > CvsB_TightWP && jet.GetCTaggerResult(FlavTagger[DataEra.Data()]).second > CvsL_TightWP)
        return 4;
    else if (jet.GetCTaggerResult(FlavTagger[DataEra.Data()]).first > CvsB_MediumWP && jet.GetCTaggerResult(FlavTagger[DataEra.Data()]).second > CvsL_MediumWP)
        return 2;
    else if (jet.GetCTaggerResult(FlavTagger[DataEra.Data()]).first > CvsB_LooseWP && jet.GetCTaggerResult(FlavTagger[DataEra.Data()]).second > CvsL_LooseWP)
        return 1;
    else
        return 0;
}

RVec<int> Vcb_SL::FindTTbarJetIndices(const RVec<Gen> &gens, const RVec<GenJet> &genjets)
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
    bool find_w_plus_lepton = false;
    bool find_w_minus_lepton = false;
    tt_decay_code = -9999;
    find_all_jets = false;
    // find last t+/- and W+/- in gens
    for (int i = gens.size() - 1; i >= 0; i--)
    {

        if (gens[i].PID() == 24 && !find_w_plus)
        {
            w_plus_idx = i;
            find_w_plus = true;
        }
        if (gens[i].PID() == -24 && !find_w_minus)
        {
            w_minus_idx = i;
            find_w_minus = true;
        }
        if (gens[i].PID() == 6 && !find_t_plus)
        {
            t_plus_idx = i;
            find_t_plus = true;
        }
        if (gens[i].PID() == -6 && !find_t_minus)
        {
            t_minus_idx = i;
            find_t_minus = true;
        }
        if (find_w_plus && find_w_minus && find_t_plus && find_t_minus)
            break;
    }

    int w_plus_up_idx = -1;
    int w_plus_down_idx = -1;
    int w_minus_up_idx = -1;
    int w_minus_down_idx = -1;
    int w_plus_lepton_idx = -1;
    int w_minus_lepton_idx = -1;

    // find W+/- daughters
    for (int i = gens.size() - 1; i >= 0; i--)
    {

        if (gens[i].MotherIndex() == w_plus_idx && isPIDUpTypeQuark(gens[i].PID()) && gens[i].PID() > 0 && !find_w_plus_up)
        {
            w_plus_up_idx = i;
            find_w_plus_up = true;
            continue;
        }
        if (gens[i].MotherIndex() == w_plus_idx && isPIDDownTypeQuark(gens[i].PID()) && gens[i].PID() < 0 && !find_w_plus_down)
        {
            w_plus_down_idx = i;
            find_w_plus_down = true;
            continue;
        }
        if (gens[i].MotherIndex() == w_minus_idx && isPIDUpTypeQuark(gens[i].PID()) && gens[i].PID() < 0 && !find_w_minus_up)
        {
            w_minus_up_idx = i;
            find_w_minus_up = true;
            continue;
        }
        if (gens[i].MotherIndex() == w_minus_idx && isPIDDownTypeQuark(gens[i].PID()) && gens[i].PID() > 0 && !find_w_minus_down)
        {
            w_minus_down_idx = i;
            find_w_minus_down = true;
            continue;
        }
        if (gens[i].MotherIndex() == w_plus_idx && isPIDLepton(gens[i].PID()) && gens[i].PID() < 0 && !find_w_plus_lepton)
        {
            w_plus_lepton_idx = i;
            find_w_plus_lepton = true;
            continue;
        }
        if (gens[i].MotherIndex() == w_minus_idx && isPIDLepton(gens[i].PID()) && gens[i].PID() > 0 && !find_w_minus_lepton)
        {
            w_minus_lepton_idx = i;
            find_w_minus_lepton = true;
            continue;
        } 
        if(find_w_plus_up && find_w_plus_down && find_w_minus_up && find_w_minus_down && find_w_plus_lepton && find_w_minus_lepton)
            break;
    }

    bool w_plus_leptonic = (find_w_plus_lepton && find_w_minus_up && find_w_minus_down);
    bool w_minus_leptonic = (find_w_minus_lepton && find_w_plus_up && find_w_plus_down);

    int b_from_t_plus_idx = -1;
    int b_from_t_minus_idx = -1;

    for (int i = gens.size() - 1; i >= 0; i--)
    {

        if (gens[i].MotherIndex() == t_plus_idx && gens[i].PID() == 5 && !find_b_from_t_plus)
        {
            b_from_t_plus_idx = i;
            find_b_from_t_plus = true;
        }
        if (gens[i].MotherIndex() == t_minus_idx && gens[i].PID() == -5 && !find_b_from_t_minus)
        {
            b_from_t_minus_idx = i;
            find_b_from_t_minus = true;
        }
        if (find_b_from_t_plus && find_b_from_t_minus)
            break;
    }

    bool findall_gen;
    if (w_plus_leptonic) findall_gen = (find_b_from_t_plus && find_b_from_t_minus && find_w_minus_up && find_w_minus_down && find_w_plus_lepton);
    else if (w_minus_leptonic) findall_gen = (find_b_from_t_plus && find_b_from_t_minus && find_w_plus_up && find_w_plus_down && find_w_minus_lepton);
    else findall_gen = false;
    if (!findall_gen)
        return ttbar_jet_indices;
    if (w_plus_leptonic) tt_decay_code = 100 * abs(gens[w_plus_lepton_idx].PID()) + 10 * abs(gens[w_minus_up_idx].PID()) + abs(gens[w_minus_down_idx].PID());
    else if (w_minus_leptonic) tt_decay_code = 1000 * abs(gens[w_plus_up_idx].PID()) + 100 * abs(gens[w_plus_down_idx].PID()) + abs(gens[w_minus_lepton_idx].PID());

    RVec<Gen> this_gens;
    if(w_plus_leptonic) this_gens = {gens[b_from_t_plus_idx], gens[b_from_t_minus_idx], gens[w_plus_up_idx], gens[w_plus_down_idx]};
    else if(w_minus_leptonic) this_gens = {gens[b_from_t_plus_idx], gens[b_from_t_minus_idx], gens[w_minus_up_idx], gens[w_minus_down_idx]};

    unordered_map<int, RVec<pair<size_t, Gen>>> gen_by_PID;

    for (size_t i = 0; i < this_gens.size(); i++)
    {
        gen_by_PID[this_gens[i].PID()].push_back({i, this_gens[i]});
    }

    RVec<int> this_gens_PID = {this_gens[0].PID(), this_gens[1].PID(), this_gens[2].PID(), this_gens[3].PID()};
    RVec<int> this_genjet_PID;

    for (size_t i = 0; i < Jets.size(); i++)
    {
        this_genjet_PID.push_back(genjets[i].partonFlavour());
    }

    unordered_map<int, int> gen_genjet_matching_result;

    for (const auto &PID_group : gen_by_PID)
    {
        int current_PID = PID_group.first;
        const auto &gen_idx_Gen_pair = PID_group.second;

        RVec<GenJet> filtered_genjets;
        RVec<int> filtered_genjet_indices;

        for (size_t genjet_idx = 0; genjet_idx < this_genjet_PID.size(); genjet_idx++)
        {
            if (this_genjet_PID[genjet_idx] == current_PID)
            {
                filtered_genjets.push_back(genjets[genjet_idx]);
                filtered_genjet_indices.push_back(genjet_idx);
            }
        }
        // if no genjets that matches to this PID found, this PID group is failed to match
        if (filtered_genjets.size() == 0)
        {
            for (const auto &gen_idx_Gen_pair : gen_idx_Gen_pair)
            {
                gen_genjet_matching_result[gen_idx_Gen_pair.first] = -999;
            }
        }
        // do deltaRMatching if we have matching genjets
        else
        {
            RVec<Gen> current_gen;
            RVec<int> current_gen_indices;
            for (const auto &gen_idx_Gen_pair : gen_idx_Gen_pair)
            {
                current_gen.push_back(gen_idx_Gen_pair.second);
                current_gen_indices.push_back(gen_idx_Gen_pair.first);
            }
            unordered_map<int, int> temp_match = deltaRMatching(current_gen, filtered_genjets, 0.5);
            for (const auto &temp_match_pair : temp_match)
            {
                if (temp_match_pair.second >= 0)
                {
                    gen_genjet_matching_result[current_gen_indices[temp_match_pair.first]] = filtered_genjet_indices[temp_match_pair.second];
                }
                else
                {
                    gen_genjet_matching_result[current_gen_indices[temp_match_pair.first]] = -999;
                }
            }
        }
    }

    // we get gen-genjet matching. now do genjet-jet matching
    RVec<GenJet> gen_matched_genjet;
    RVec<size_t> gen_matched_genjet_indices;
    for (size_t i = 0; i < gen_genjet_matching_result.size(); i++)
    {
        if (gen_genjet_matching_result[i] >= 0)
        {
            gen_matched_genjet.push_back(genjets[gen_genjet_matching_result[i]]);
            gen_matched_genjet_indices.push_back(i);
        }
    }
    unordered_map<int, int> jet_genjet_matching_results = GenJetMatching(Jets, gen_matched_genjet, ev.GetRho());
    unordered_map<int, int> matching_result;

    // now store parton-jet matching result
    for (const auto jet_genjet_matching_result : jet_genjet_matching_results)
    {
        if (jet_genjet_matching_result.second >= 0)
        {
            matching_result[gen_matched_genjet_indices[jet_genjet_matching_result.second]] = jet_genjet_matching_result.first;
        }
        else
        {
            matching_result[gen_matched_genjet_indices[jet_genjet_matching_result.second]] = -999;
        }
    }

    for (size_t i = 0; i < 4; i++)
    {
        if (matching_result.find(i) == matching_result.end())
        {
            matching_result[i] = -999;
        }
    }

    ttbar_jet_indices = {matching_result[0], matching_result[1], matching_result[2], matching_result[3]};
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
    if (matching_result[0] >= 0 && matching_result[1] >= 0)
    {
        // sort first jet to be b jet from hadronic top
        if(w_minus_leptonic) swap(ttbar_jet_indices[0], ttbar_jet_indices[1]);
    }
    return ttbar_jet_indices;
}

void Vcb_SL::executeEvent()
{
    if(HasFlag("El")) channel = Vcb_SL::Channel::El;
    else if(HasFlag("Mu")) channel = Vcb_SL::Channel::Mu;
    else channel = Vcb_SL::Channel::Mu; // default channel is Muon
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

void Vcb_SL::executeEventFromParameter(TString syst)
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
    GetKineMaticFitterResult(Jets, MET, lepton);
    FillKinematicFitterResult("Nominal", weight);
    if (!IsDATA)
        ttbar_jet_indices = FindTTbarJetIndices(AllGens, AllGenJets);
    else
        ttbar_jet_indices = {-999, -999, -999, -999, -999, -999};
    if (find_all_jets)
    {
        Particle genRecoHadW = Jets[ttbar_jet_indices[2]] + Jets[ttbar_jet_indices[3]];
        Particle genRecoHadTop = Jets[ttbar_jet_indices[0]] + genRecoHadW;
        FillHist("GenRecoHadTopMass", genRecoHadTop.M(), weight, 100, 0., 500.);
        FillHist("GenRecoHadWMass", genRecoHadW.M(), weight, 100, 50., 150.);
    }
    FillTrainingTree();
}

void Vcb_SL::GetKineMaticFitterResult(const RVec<Jet> &jets, Particle &MET, Lepton &lepton)
{
    RVec<RVec<unsigned int>> possible_permutations = GetPermutations(jets);

    best_KF_result.chi2 = 9999.;
    std:variant<float, std::pair<float, float>> neutrino_pz = SolveNeutrinoPz(lepton, MET);
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
        else{
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
    lepton_Cov(0, 0) = TMath::Power((lepton.Pt() * 0.0001),2);
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

    //pt balance constraint
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
    for(size_t i = 0; i < extra_jets.size(); i++){
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

    //add extra jets
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
