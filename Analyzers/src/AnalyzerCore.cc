#include "AnalyzerCore.h"

AnalyzerCore::AnalyzerCore() {
    outfile = nullptr;
    // pdfReweight = new PDFReweight();
}

AnalyzerCore::~AnalyzerCore() {
    for (const auto &pair: histmap1d) delete pair.second; histmap1d.clear();
    for (const auto &pair: histmap2d) delete pair.second; histmap2d.clear();
    for (const auto &pair: histmap3d) delete pair.second; histmap3d.clear();
    if (outfile) delete outfile;
    if (mcCorr) delete mcCorr;
    // if (pdfReweight) delete pdfReweight;
}

bool AnalyzerCore::PassMetFilter(const RVec<Jet> &Alljets, const Event &ev) {
    bool MetFilter = true;
    MetFilter = Flag_goodVertices && Flag_globalSuperTightHalo2016Filter && Flag_BadPFMuonFilter && Flag_BadPFMuonDzFilter && Flag_hfNoisyHitsFilter && Flag_ecalBadCalibFilter && Flag_eeBadScFilter; // && !Flag_ecalBadCalibFilter;
    //&& Flag_ECalDeadCellTriggerPrimitiveFilter documente as this existed in NanoAOD, but it isn't
    if(!MetFilter) return false;

    // Temporarily remove the ecalBadCalibFilter, Instead,
    if(!(Run <= 367144 && Run >= 362433)) return MetFilter;
    if (ev.GetMETVector().Pt() <= 100.) return MetFilter;
    RVec<Jet> this_jet = SelectJets(Alljets, "NOCUT", 50., 5.0);
    for(const auto &jet: this_jet){
        bool badEcal = (jet.Pt() > 50.);
        badEcal = badEcal && (jet.neutralEMFraction() > 0.9 || jet.chargedEMFraction() > 0.9) ;
        badEcal = badEcal && jet.DeltaR(ev.GetMETVector()) < 0.3;
        badEcal = badEcal && jet.Eta() > -0.5 && jet.Eta() < -0.1;
        badEcal = badEcal && jet.Phi() > -2.1 && jet.Phi() < -1.8;
        if(badEcal) return false;
    }
    return true;
}

float AnalyzerCore::GetScaleVariation(const int &muF_syst, const int &muR_syst) {
    if(muF_syst == -1 && muR_syst == -1) return LHEScaleWeight[0];
    else if(muF_syst == 0 && muR_syst == -1) return LHEScaleWeight[1];
    else if(muF_syst == 1 && muR_syst == -1) return LHEScaleWeight[2];
    else if(muF_syst == -1 && muR_syst == 0) return LHEScaleWeight[3];
    else if(muF_syst == 1 && muR_syst == 0) return LHEScaleWeight[4];
    else if(muF_syst == -1 && muR_syst == 1) return LHEScaleWeight[5];
    else if(muF_syst == 0 && muR_syst == 1) return LHEScaleWeight[6];
    else if(muF_syst == 1 && muR_syst == 1) return LHEScaleWeight[7];
    else if(muF_syst == 0 && muR_syst == 0) return 1.;
    else{
        cout << "[AnalyzerCore::GetScaleVariation] muF_syst = " << muF_syst << " and muR_syst = " << muR_syst << " is not implemented" << endl;
        exit(ENODATA);
    }
}

unordered_map<int, int> AnalyzerCore::GenJetMatching(const RVec<Jet> &jets, const RVec<GenJet> &genjets, const float &rho, const float dR, const float pTJerCut){
    unordered_map<int, int> matched_genjet_idx;
    RVec<tuple<int,int,float,float>> possible_matches;

    //All possible matches that pass the dR and pTJerCut
    for(size_t i = 0; i < jets.size(); i++){
        for(size_t j = 0; j < genjets.size(); j++){
            float this_DeltaR = jets[i].DeltaR(genjets[j]);
            float this_pt_diff = fabs(jets[i].Pt() - genjets[j].Pt());
            float this_jer = mcCorr->GetJER(jets[i].Eta(), jets[i].Pt(), rho);
            if(jets[i].DeltaR(genjets[j]) < dR && this_pt_diff < pTJerCut*this_jer){
                possible_matches.emplace_back(i, j, this_DeltaR, this_pt_diff);
            }
        }
    }
    //sort them by dR and then pt_diff
    sort(possible_matches.begin(), possible_matches.end(), [](const tuple<int,int,float,float> &a, const tuple<int,int,float,float> &b){
        if(get<2>(a) == get<2>(b)) return get<3>(a) < get<3>(b);
        return get<2>(a) < get<2>(b);
    });
    //greeedy matching
    RVec<bool> used_jet(jets.size(), false);
    RVec<bool> used_genjet(genjets.size(), false);
    for(const auto &match: possible_matches){
        int jet_idx = get<0>(match);
        int genjet_idx = get<1>(match);
        if(used_jet[jet_idx] || used_genjet[genjet_idx]) continue;
        matched_genjet_idx[jet_idx] = genjet_idx;
        used_jet[jet_idx] = true;
        used_genjet[genjet_idx] = true;
    }
    // assign -999 to unmatched jet
    for(size_t i = 0; i < jets.size(); i++){
        if(used_jet[i]) continue;
        matched_genjet_idx[i] = -999;
    }
    return matched_genjet_idx;
}

RVec<Jet> AnalyzerCore::SmearJets(const RVec<Jet> &jets, const RVec<GenJet> &genjets){
    unordered_map<int, int> matched_idx = GenJetMatching(jets, genjets, fixedGridRhoFastjetAll);
    RVec<Jet> smeared_jets;
    for(size_t i = 0; i < jets.size(); i++){
        Jet this_jet = jets.at(i);
        float this_corr = 1.;
        float this_jer = mcCorr->GetJER(jets[i].Eta(), jets[i].Pt(), fixedGridRhoFastjetAll);
        float this_sf = mcCorr->GetJERSF(jets[i].Eta(), jets[i].Pt());

        if(matched_idx[i] < 0){
            // if the jet is not matched to any genjet, do stochastic smearing
            this_corr += (gRandom -> Gaus(0,this_jer))*sqrt(max(this_sf * this_sf - 1., 0.0));
            this_jet *= this_corr;
            smeared_jets.push_back(this_jet);
            continue;
        }

        float this_genjet_pt = genjets[matched_idx[i]].Pt();
        this_corr += (this_sf-1.)+(this_genjet_pt - jets[i].Pt()) / jets[i].Pt();
        this_jet *= this_corr;
        smeared_jets.push_back(this_jet);
    }
    return smeared_jets;
}

RVec<Jet> AnalyzerCore::ScaleJets(const RVec<Jet> &jets, const int &syst, const TString &source)
{
    RVec<Jet> scaled_jets;
    RVec<TString> syst_sources = { "AbsoluteMPFBias",
                                   "AbsoluteScale",
                                   "AbsoluteStat",
                                   "FlavorQCD",
                                   "Fragmentation",
                                   //"PileUpDataMC",
                                   //"PileUpPtBB",
                                   //"PileUpPtEC1",
                                   //"PileUpPtEC2",
                                   //"PileUpPtHF",
                                   //"PileUpPtRef",
                                   "PileUpEnvelope",
                                   "RelativeFSR",
                                   "RelativeJEREC1",
                                   "RelativeJEREC2",
                                   "RelativeJERHF",
                                   "RelativePtBB",
                                   "RelativePtEC1",
                                   "RelativePtEC2",
                                   "RelativePtHF",
                                   "RelativeBal",
                                   "RelativeSample",
                                   "RelativeStatEC",
                                   "RelativeStatFSR",
                                   "RelativeStatHF",
                                   "SinglePionECAL",
                                   "SinglePionHCAL",
                                   "TimePtEta" };
    switch (syst)
    {
    case 0:
        return jets;
    case 1:
        break;
    case -1:
        break;
    default:
        cout << "[AnalyzerCore::ScaleJets] scale = " << syst << " is not implemented" << endl;
        exit(ENODATA);
    }
    for(const auto &jet: jets){
        Jet this_jet = jet;
        if(source == ""){
            for(const auto &it: syst_sources){
                this_jet *= mcCorr->GetJESUncertainty(jet.Eta(), jet.Pt(), it, syst);
            }
            scaled_jets.push_back(this_jet);
        }
        else{
            this_jet *= mcCorr->GetJESUncertainty(jet.Eta(), jet.Pt(), source, syst);
            scaled_jets.push_back(this_jet);
        }
    }
    return scaled_jets;
}

void AnalyzerCore::SetOutfilePath(TString outpath)
{
    outfile = new TFile(outpath, "RECREATE");
}

// pdfs
float AnalyzerCore::GetPDFWeight(LHAPDF::PDF *pdf) {
    float pdf1 = pdf->xfxQ2(Generator_id1, Generator_x1, Generator_scalePDF);
    float pdf2 = pdf->xfxQ2(Generator_id2, Generator_x2, Generator_scalePDF);
    return pdf1 * pdf2;
}

float AnalyzerCore::GetPDFReweight() {
    return GetPDFWeight(pdfReweight->NewPDF) / GetPDFWeight(pdfReweight->ProdPDF);
}

float AnalyzerCore::GetPDFReweight(int member) {
    return GetPDFWeight(pdfReweight->PDFErrorSet.at(member)) / GetPDFWeight(pdfReweight->ProdPDF);
}

// MC weights
// TODO: Treat MiNNLO and Sherpa weights 
float AnalyzerCore::MCweight(bool usesign, bool norm_1invpb) const {
    if (IsDATA) return 1.;
    float weight = genWeight;
      if (usesign){
        if (weight > 0)
        weight = 1.0;
        else if (weight < 0)
        weight = -1.0;
        else
        weight = 0.0;
    }

    if (norm_1invpb)
    {
        if (usesign)
        weight *= xsec / sumSign;
        else
        weight *= xsec / sumW;
    }
    return weight;

}

// Objects
Event AnalyzerCore::GetEvent(RVec<TString> HLT_List)
{
    Event ev;
    ev.SetnPileUp(Pileup_nPU);
    ev.SetnTrueInt(Pileup_nTrueInt);
    ev.SetnPVsGood(PV_npvsGood);
    ev.SetEra(GetEra());
    ev.SetTrigger(HLT_List, TriggerMap);
    ev.SetMET(PuppiMET_pt, PuppiMET_phi);
    ev.setRho(fixedGridRhoFastjetAll);
    return ev;
}

RVec<Muon> AnalyzerCore::GetAllMuons() {
    RVec<Muon> muons;
    for (int i = 0; i < nMuon; i++) {
        Muon muon;
        muon.SetPtEtaPhiM(Muon_pt[i], Muon_eta[i], Muon_phi[i], Muon_mass[i]);
        muon.SetCharge(Muon_charge[i]);
        muon.SetTkRelIso(Muon_tkRelIso[i]);
        muon.SetPfRelIso03(Muon_pfRelIso03_all[i]);
        muon.SetPfRelIso04(Muon_pfRelIso04_all[i]);
        muon.SetMiniPFRelIso(Muon_miniPFRelIso_all[i]);
        muon.SetdXY(Muon_dxy[i], Muon_dxyErr[i]);
        muon.SetdZ(Muon_dz[i], Muon_dzErr[i]);
        muon.SetIP3D(Muon_ip3d[i], Muon_sip3d[i]);
        muon.SetBIDBit(Muon::BooleanID::LOOSE, Muon_looseId[i]);
        muon.SetBIDBit(Muon::BooleanID::MEDIUM, Muon_mediumId[i]);
        muon.SetBIDBit(Muon::BooleanID::MEDIUMPROMPT, Muon_mediumPromptId[i]);
        muon.SetBIDBit(Muon::BooleanID::TIGHT, Muon_tightId[i]);
        muon.SetBIDBit(Muon::BooleanID::SOFT, Muon_softId[i]);
        muon.SetBIDBit(Muon::BooleanID::SOFTMVA, Muon_softMvaId[i]);
        muon.SetBIDBit(Muon::BooleanID::TRIGGERLOOSE, Muon_triggerIdLoose[i]);
        muon.SetWIDBit(Muon::WorkingPointID::HIGHPT, Muon_highPtId[i]);
        muon.SetWIDBit(Muon::WorkingPointID::MINIISO, Muon_miniIsoId[i]);
        muon.SetWIDBit(Muon::WorkingPointID::MULTIISO, Muon_multiIsoId[i]);
        muon.SetWIDBit(Muon::WorkingPointID::MVAMU, Muon_mvaMuId[i]);
        //muon.SetWIDBit(Muon::WorkingPointID::MVALOWPT, Muon_mvaLowPtId[i]);
        muon.SetWIDBit(Muon::WorkingPointID::PFISO, Muon_pfIsoId[i]);
        muon.SetWIDBit(Muon::WorkingPointID::PUPPIISO, Muon_puppiIsoId[i]);
        muon.SetWIDBit(Muon::WorkingPointID::TKISO, Muon_tkIsoId[i]);
        muon.SetMVAID(Muon::MVAID::SOFTMVA, Muon_softMva[i]);
        muon.SetMVAID(Muon::MVAID::MVALOWPT, Muon_mvaLowPt[i]);
        muon.SetMVAID(Muon::MVAID::MVATTH, Muon_mvaTTH[i]);

        muons.push_back(muon);
    }

    return muons;
}

RVec<Muon> AnalyzerCore::GetMuons(const TString ID, const float ptmin, const float fetamax) {
    RVec<Muon> muons = GetAllMuons();
    RVec<Muon> selected_muons;
    for (const auto &muon: muons) {
        if (! (muon.Pt() > ptmin)) continue;
        if (! (fabs(muon.Eta()) < fetamax)) continue;
        if (! muon.PassID(ID)) continue;
        selected_muons.push_back(muon);
    }
    return selected_muons;
}

RVec<Muon> AnalyzerCore::SelectMuons(const RVec<Muon> &muons, const TString ID, const float ptmin, const float fetamax) {
    RVec<Muon> selected_muons;

    for (const auto &muon: muons) {
        if (! (muon.Pt() > ptmin)) continue;
        if (! (fabs(muon.Eta()) < fetamax)) continue;
        if (ID.Contains("&&"))
        {
            TObjArray *IDs = ID.Tokenize("&&");
            bool pass = true;
            for (int i = 0; i < IDs->GetEntries(); i++)
            {
                TString thisID = ((TObjString *)IDs->At(i))->GetString();
                if (!muon.PassID(thisID))
                {
                    pass = false;
                    break;
                }
            }
            delete IDs;
            if (!pass)
                continue;
        }
        else
        {
            if (!muon.PassID(ID))
                continue;
        }
        selected_muons.push_back(muon);
    }
    return selected_muons;
}

RVec<Electron> AnalyzerCore::GetAllElectrons(){
    RVec<Electron> electrons;
    for (int i = 0; i < nElectron; i++){
        Electron electron;
        electron.SetPtEtaPhiM(Electron_pt[i], Electron_eta[i], Electron_phi[i], Electron_mass[i]);
        electron.SetCharge(Electron_charge[i]);
        electron.SetDeltaEtaSC(Electron_deltaEtaSC[i]);
        electron.SetPfRelIso03(Electron_pfRelIso03_all[i]);
        electron.SetMiniPFRelIso(Electron_miniPFRelIso_all[i]);
        electron.SetdXY(Electron_dxy[i], Electron_dxyErr[i]);
        electron.SetdZ(Electron_dz[i], Electron_dzErr[i]);
        electron.SetIP3D(Electron_ip3d[i], Electron_sip3d[i]);
        electron.SetConvVeto(Electron_convVeto[i]);
        electron.SetLostHits(Electron_lostHits[i]);
        electron.SetSeedGain(Electron_seedGain[i]);
        electron.SetTightCharge(Electron_tightCharge[i]);
        electron.SetSieie(Electron_sieie[i]);
        electron.SetHoe(Electron_hoe[i]);
        electron.SetEInvMinusPInv(Electron_eInvMinusPInv[i]);
        electron.SetDr03EcalRecHitSumEt(Electron_dr03EcalRecHitSumEt[i]);
        electron.SetDr03HcalDepth1TowerSumEt(Electron_dr03HcalDepth1TowerSumEt[i]);
        electron.SetDr03TkSumPt(Electron_dr03TkSumPt[i]);
        electron.SetDr03TkSumPtHEEP(Electron_dr03TkSumPtHEEP[i]);
        electron.SetR9(Electron_r9[i]);
        electron.SetBIDBit(Electron::BooleanID::MVAISOWP80, Electron_mvaIso_WP80[i]);
        electron.SetBIDBit(Electron::BooleanID::MVAISOWP90, Electron_mvaIso_WP90[i]);
        electron.SetBIDBit(Electron::BooleanID::MVANOISOWP80, Electron_mvaNoIso_WP80[i]);
        electron.SetBIDBit(Electron::BooleanID::MVANOISOWP90, Electron_mvaNoIso_WP90[i]);
        electron.SetBIDBit(Electron::BooleanID::CUTBASEDHEEP, Electron_cutBased_HEEP[i]);
        electron.SetCBIDBit(Electron::CutBasedID::CUTBASED, Electron_cutBased[i]);
        electron.SetMVA(Electron::MVATYPE::MVAISO, Electron_mvaIso[i]);
        electron.SetMVA(Electron::MVATYPE::MVANOISO, Electron_mvaNoIso[i]);
        electron.SetMVA(Electron::MVATYPE::MVATTH, Electron_mvaTTH[i]);
        electron.SetGenPartFlav(Electron_genPartFlav[i]);
        electron.SetGenPartIdx(Electron_genPartIdx[i]);

        electrons.push_back(electron);
    }

    return electrons;
}

RVec<Electron> AnalyzerCore::GetElectrons(const TString ID, const float ptmin, const float fetamax) {
    RVec<Electron> electrons = GetAllElectrons();
    RVec<Electron> selected_electrons;
    for (const auto &electron: electrons) {
        if (! (electron.Pt() > ptmin)) continue;
        if (! (fabs(electron.Eta()) < fetamax)) continue;
        if (! electron.PassID(ID)) continue;
        selected_electrons.push_back(electron);
    }
    return selected_electrons;
}

RVec<Electron> AnalyzerCore::SelectElectrons(const RVec<Electron> &electrons, const TString ID, const float ptmin, const float fetamax) {
    RVec<Electron> selected_electrons;
    for (const auto &electron: electrons) {
        if (! (electron.Pt() > ptmin)) continue;
        if (! (fabs(electron.Eta()) < fetamax)) continue;
        if (! electron.PassID(ID)) continue;
        selected_electrons.push_back(electron);
    }
    return selected_electrons;
}

RVec<Gen> AnalyzerCore::GetAllGens() {
    RVec<Gen> gens;
    for (int i = 0; i < nGenPart; i++) {
        Gen gen;
        gen.SetPtEtaPhiM(GenPart_pt[i], GenPart_eta[i], GenPart_phi[i], GenPart_mass[i]);
        gen.SetPdgId(GenPart_pdgId[i]);
        gen.SetMotherIdx(GenPart_genPartIdxMother[i]);
        gen.SetStatus(GenPart_status[i]);
        gen.SetStatusFlags(GenPart_statusFlags[i]);
        gens.push_back(gen);
    }
    return gens;
}

RVec<LHE> AnalyzerCore::GetAllLHEs() {
    RVec<LHE> lhes;
    for (int i = 0; i < nLHEPart; i++) {
        LHE lhe;
        lhe.SetPtEtaPhiM(LHEPart_pt[i], LHEPart_eta[i], LHEPart_phi[i], LHEPart_mass[i]);
        lhe.SetStatus(LHEPart_status[i]);
        lhe.SetSpin(LHEPart_spin[i]);
        lhe.SetIncomingPz(LHEPart_incomingpz[i]);
        lhe.SetPdgId(LHEPart_pdgId[i]);
        lhes.push_back(lhe);
    }
    return lhes;
}

RVec<Tau> AnalyzerCore::GetAllTaus(){

    RVec<Tau> taus;
    
    for (int i = 0; i < nTau; i++) {

        Tau tau;
        tau.SetPtEtaPhiM(Tau_pt[i], Tau_eta[i], Tau_phi[i], Tau_mass[i]);
        tau.SetCharge(Tau_charge[i]);
        tau.SetDecayMode(Tau_decayMode[i]);
        tau.SetdXY(Tau_dxy[i]);
        tau.SetdZ(Tau_dz[i]);
        tau.SetGenPartFlav(Tau_genPartFlav[i]);
        tau.SetGenPartIdx(Tau_genPartIdx[i]);
        tau.SetIdDeepTau2018v2p5VSjet(Tau_idDeepTau2018v2p5VSjet[i]);
        tau.SetIdDeepTau2018v2p5VSmu(Tau_idDeepTau2018v2p5VSmu[i]);
        tau.SetIdDeepTau2018v2p5VSe(Tau_idDeepTau2018v2p5VSe[i]);
        tau.SetIdDecayModeNewDMs(Tau_idDecayModeNewDMs[i]);
        taus.push_back(tau);

    }

    return taus;

}

RVec<Tau> AnalyzerCore::SelectTaus(const RVec<Tau> &taus, const TString ID, const float ptmin, const float absetamax){
    
    RVec<Tau> selected_taus;
    
    for (const auto &tau: taus) {

        if (! (tau.Pt() > ptmin)) continue;
        if (! (fabs(tau.Eta()) < absetamax)) continue;
        if (! tau.PassID(ID)) continue;
        selected_taus.push_back(tau);

    }

    return selected_taus;

}

RVec<Jet> AnalyzerCore::GetAllJets() {
    RVec<Jet> Jets;
    for (int i = 0; i < nJet; i++) {
        Jet jet;
        jet.SetPtEtaPhiM(Jet_pt[i], Jet_eta[i], Jet_phi[i], Jet_mass[i]);
        jet.SetArea(Jet_area[i]);
        jet.SetGenFlavours(Jet_hadronFlavour[i], Jet_partonFlavour[i]);
        RVec<float> tvs = {Jet_btagDeepFlavB[i], Jet_btagDeepFlavCvB[i], Jet_btagDeepFlavCvL[i], Jet_btagDeepFlavQG[i],
                           Jet_btagPNetB[i], Jet_btagPNetCvB[i], Jet_btagPNetCvL[i], Jet_btagPNetQvG[i],
                           Jet_btagPNetTauVJet[i], Jet_btagRobustParTAK4B[i], Jet_btagRobustParTAK4CvB[i], Jet_btagRobustParTAK4CvL[i], Jet_btagRobustParTAK4QG[i]};
        jet.SetTaggerResults(tvs);
        jet.SetEnergyFractions(Jet_chHEF[i], Jet_neHEF[i], Jet_neEmEF[i], Jet_chEmEF[i], Jet_muEF[i]);
        jet.SetMultiplicities(Jet_nConstituents[i], Jet_nElectrons[i], Jet_nMuons[i], Jet_nSVs[i]);
        jet.SetMatchingIndices(Jet_electronIdx1[i], Jet_electronIdx2[i], Jet_muonIdx1[i], Jet_muonIdx2[i], Jet_svIdx1[i], Jet_svIdx2[i], Jet_genJetIdx[i]);
        jet.SetJetID(Jet_jetId[i]);
        RVec<float> tvs2 = {Jet_PNetRegPtRawCorr[i], Jet_PNetRegPtRawCorrNeutrino[i], Jet_PNetRegPtRawRes[i]};
        jet.SetCorrections(tvs2);

        Jets.push_back(jet);
    }

    return Jets;
}

RVec<Jet> AnalyzerCore::GetJets(const TString ID, const float ptmin, const float fetamax) {
    RVec<Jet> jets = GetAllJets();
    RVec<Jet> selected_jets;
    for (const auto &jet: jets) {
        if (jet.Pt() < ptmin) continue;
        if (fabs(jet.Eta()) > fetamax) continue;
        if (! jet.PassID(ID)) continue;
        selected_jets.push_back(jet);
    }
    return selected_jets;
}

RVec<Jet> AnalyzerCore::SelectJets(const RVec<Jet> &jets, const TString ID, const float ptmin, const float fetamax){
    RVec<Jet> selected_jets;
    for (const auto &jet: jets) {
        if (jet.Pt() < ptmin) continue;
        if (fabs(jet.Eta()) > fetamax) continue;
        if (! jet.PassID(ID)) continue;
        selected_jets.push_back(jet);
    }
    return selected_jets;
}

RVec<Jet> AnalyzerCore::JetsVetoLeptonInside(const RVec<Jet> &jets, const RVec<Electron> &electrons, const RVec<Muon> &muons, const float dR){
    RVec<Jet> selected_jets;
    for(const auto &jet: jets){
        bool isLeptonInside = false;
        for(const auto &electron: electrons){
            if(jet.DeltaR(electron) < dR){
                isLeptonInside = true;
                break;
            }
        }
        if(isLeptonInside) continue;
        for(const auto &muon: muons){
            if(jet.DeltaR(muon) < dR){
                isLeptonInside = true;
                break;
            }
        }
        if(isLeptonInside) continue;
        selected_jets.push_back(jet);
    }
    return selected_jets;
}

bool AnalyzerCore::PassJetVetoMap(const RVec<Jet> &AllJets, const RVec<Muon> &AllMuons, const TString mapCategory){
    RVec<Jet> this_jet = SelectJets(AllJets, "tight", 15., 5.0);
    RVec<Jet> selected_jets;
    RVec<Electron> empty_electrons;
    this_jet = JetsVetoLeptonInside(this_jet, empty_electrons, AllMuons, 0.2);
    for(const auto &jet: this_jet){
        if(jet.EMFraction() < 0.9) selected_jets.push_back(jet);
    }
    for(const auto &jet: selected_jets){
        if(mcCorr->IsJetVetoZone(jet.Eta(), jet.Phi(), mapCategory)) return false;
    }
    return true;
}

RVec<FatJet> AnalyzerCore::GetAllFatJets() {
    
    RVec<FatJet> FatJets;

    for (int i = 0; i < nFatJet; i++) {

        FatJet fatjet;

        RVec<float> pnet_m = { FatJet_particleNetWithMass_H4qvsQCD[i], FatJet_particleNetWithMass_HccvsQCD[i],
                                      FatJet_particleNetWithMass_HbbvsQCD[i], FatJet_particleNetWithMass_QCD[i], 
                                      FatJet_particleNetWithMass_TvsQCD[i]  , FatJet_particleNetWithMass_WvsQCD[i],
                                      FatJet_particleNetWithMass_ZvsQCD[i] };

        RVec<float> pnet   = { FatJet_particleNet_QCD[i], FatJet_particleNet_QCD0HF[i],
                                      FatJet_particleNet_QCD1HF[i], FatJet_particleNet_QCD2HF[i],
                                      FatJet_particleNet_XbbVsQCD[i], FatJet_particleNet_XccVsQCD[i],
                                      FatJet_particleNet_XqqVsQCD[i], FatJet_particleNet_XggVsQCD[i],
                                      FatJet_particleNet_XteVsQCD[i], FatJet_particleNet_XtmVsQCD[i],
                                      FatJet_particleNet_XttVsQCD[i], FatJet_particleNet_massCorr[i] };

        fatjet.SetPtEtaPhiM(FatJet_pt[i], FatJet_eta[i], FatJet_phi[i], FatJet_mass[i]);
        fatjet.SetArea(FatJet_area[i]);
        fatjet.SetSDMass(FatJet_msoftdrop[i]);
        fatjet.SetLSF3(FatJet_lsf3[i]);
        fatjet.SetGenMatchIDs(FatJet_genJetAK8Idx[i], FatJet_subJetIdx1[i], FatJet_subJetIdx2[i]);
        fatjet.SetConstituents(FatJet_nBHadrons[i], FatJet_nCHadrons[i], FatJet_nConstituents[i]);
        fatjet.SetBTaggingInfo(FatJet_btagDDBvLV2[i], FatJet_btagDDCvBV2[i], FatJet_btagDDCvLV2[i], FatJet_btagDeepB[i], FatJet_btagHbb[i]);
        fatjet.SetPNetwithMassResults(pnet_m);
        fatjet.SetPNetResults(pnet);
        fatjet.SetSubjettiness(FatJet_tau1[i], FatJet_tau2[i], FatJet_tau3[i], FatJet_tau4[i]);

        FatJets.push_back(fatjet);
    }

    return FatJets;
}

RVec<GenJet> AnalyzerCore::GetAllGenJets() {
    
    RVec<GenJet> GenJets;

    for (int i = 0; i < nGenJet; i++) {

        GenJet genjet;

        genjet.SetPtEtaPhiM(GenJet_pt[i], GenJet_eta[i], GenJet_phi[i], GenJet_mass[i]);
        genjet.SetGenFlavours(GenJet_partonFlavour[i], GenJet_hadronFlavour[i]);
        GenJets.push_back(genjet);
    }

    return GenJets;
}
void AnalyzerCore::FillHist(const TString &histname, float value, float weight, int n_bin, float x_min, float x_max) {
    auto histkey = string(histname);
    auto it = histmap1d.find(histkey);
    if (it == histmap1d.end()) {
        TH1F *this_hist = new TH1F(histkey.c_str(), "", n_bin, x_min, x_max);
        this_hist->SetDirectory(nullptr);
        histmap1d[histkey] = this_hist;
        this_hist->Fill(value, weight);
    }
    else {
        it->second->Fill(value, weight);
    }
}

void AnalyzerCore::FillHist(const TString &histname, float value, float weight, int n_bin, float *xbins) {
    auto histkey = string(histname.Data());
    auto it = histmap1d.find(histkey);
    if (it == histmap1d.end()) {
        TH1F *this_hist = new TH1F(histkey.c_str(), "", n_bin, xbins);
        this_hist->SetDirectory(nullptr);
        histmap1d[histkey] = this_hist;
        this_hist->Fill(value, weight);
    }
    else {
        it->second->Fill(value, weight);
    }
}

void AnalyzerCore::FillHist(const TString &histname, float value_x, float value_y, float weight,
                                                   int n_binx, float x_min, float x_max,
                                                   int n_biny, float y_min, float y_max) {
    auto histkey = string(histname);
    auto it = histmap2d.find(histkey);
    if (it == histmap2d.end()) {
        TH2F *this_hist = new TH2F(histkey.c_str(), "", n_binx, x_min, x_max, n_biny, y_min, y_max);
        this_hist->SetDirectory(nullptr);
        histmap2d[histkey] = this_hist;
        this_hist->Fill(value_x, value_y, weight);
    }
    else {
        it->second->Fill(value_x, value_y, weight);
    }
}

void AnalyzerCore::FillHist(const TString &histname, float value_x, float value_y, float weight,
                                                    int n_binx, float *xbins,
                                                    int n_biny, float *ybins) {
    auto histkey = string(histname);
    auto it = histmap2d.find(histkey);
    if (it == histmap2d.end()) {
        TH2F *this_hist = new TH2F(histkey.c_str(), "", n_binx, xbins, n_biny, ybins);
        this_hist->SetDirectory(nullptr);
        histmap2d[histkey] = this_hist;
        this_hist->Fill(value_x, value_y, weight);
    }
    else {
        it->second->Fill(value_x, value_y, weight);
    }
}

void AnalyzerCore::FillHist(const TString &histname, float value_x, float value_y, float value_z, 
                                      float weight, int n_binx, float x_min, float x_max,
                                                    int n_biny, float y_min, float y_max,
                                                    int n_binz, float z_min, float z_max) {
    auto histkey = string(histname);
    auto it = histmap3d.find(histkey);
    if (it == histmap3d.end()) {
        TH3F *this_hist = new TH3F(histkey.c_str(), "", n_binx, x_min, x_max, n_biny, y_min, y_max, n_binz, z_min, z_max);
        this_hist->SetDirectory(nullptr);
        histmap3d[histkey] = this_hist;
        this_hist->Fill(value_x, value_y, value_z, weight);
    }
    else {
        it->second->Fill(value_x, value_y, value_z, weight);
    }
}

void AnalyzerCore::FillHist(const TString &histname, float value_x, float value_y, float value_z, 
                                      float weight, int n_binx, float *xbins,
                                                    int n_biny, float *ybins,
                                                    int n_binz, float *zbins) {
    auto histkey = string(histname);
    auto it = histmap3d.find(histkey);
    if (it == histmap3d.end()) {
        TH3F *this_hist = new TH3F(histkey.c_str(), "", n_binx, xbins, n_biny, ybins, n_binz, zbins);
        this_hist->SetDirectory(nullptr);
        histmap3d[histkey] = this_hist;
        this_hist->Fill(value_x, value_y, value_z, weight);
    }
    else {
        it->second->Fill(value_x, value_y, value_z, weight);
    }
}
TTree* AnalyzerCore::NewTree(const TString &treename, const RVec<TString> &keeps, const RVec<TString> &drops){
    auto treekey = string(treename);
    auto it = treemap.find(treekey);
    if (it == treemap.end()){
        //if keeps and drops are empty, make new tree
        if(keeps.size() == 0 && drops.size() == 0){
            TTree *newtree = new TTree(treekey.c_str(), "");
            treemap[treekey] = newtree;
            return newtree;
        }
        else{
            TTree *newtree = fChain->CloneTree(0);
            newtree->SetName(treekey.c_str());
            for (const auto &drop : drops)
            {
                newtree->SetBranchStatus(drop, 0);
            }
            for (const auto &keep : keeps)
            {
                newtree->SetBranchStatus(keep, 1);
            }
            treemap[treekey] = newtree;
            unordered_map<string, TBranch*> this_branchmap;
            branchmaps[newtree] = this_branchmap;
            return newtree;
        }
    }
    else{
        return it->second;
    }
}

TTree* AnalyzerCore::GetTree(const TString &treename){
    auto treekey = string(treename);
    auto it = treemap.find(treekey);
    if (it == treemap.end()){
        cout << "[AnalyzerCore::GetTree] Tree " << treename << " not found" << endl;
        exit(ENODATA);
    }
    return it->second;
}

void AnalyzerCore::SetBranch(const TString &treename, const TString &branchname, void *address, const TString &leaflist){
    try{
       void* this_address = address;
        TTree *tree = GetTree(treename);
        
        unordered_map<string, TBranch*>* this_branchmap = &branchmaps[tree];
        auto it = this_branchmap->find(string(branchname));
        if (it == this_branchmap->end()){
            auto br = tree->Branch(branchname, this_address, leaflist);
            this_branchmap->insert({string(branchname), br});
        }
        else{
            it->second->SetAddress(this_address);
        }
    }
    catch(int e){
        cout << "[AnalyzerCore::SetBranch] Error get tree: " << treename.Data() << endl;
        exit(e);
    }
}

void AnalyzerCore::FillTrees(){
    for (const auto &pair : treemap)
    {
        const string &treename = pair.first;
        TTree *tree = pair.second;
        tree->Fill();
        RVec<float>().swap(this_floats);
        RVec<int>().swap(this_ints);
        RVec<char>().swap(this_bools);
    }
}
void AnalyzerCore::WriteHist() {
    cout << "[AnalyzerCore::WriteHist] Writing histograms to " << outfile->GetName() << endl;
    std::vector<std::pair<std::string, TH1F *>> sorted_histograms1d(histmap1d.begin(), histmap1d.end());
    std::vector<std::pair<std::string, TH2F *>> sorted_histograms2d(histmap2d.begin(), histmap2d.end());
    std::vector<std::pair<std::string, TH3F *>> sorted_histograms3d(histmap3d.begin(), histmap3d.end());
    std::sort(sorted_histograms1d.begin(), sorted_histograms1d.end(),
              [](const std::pair<std::string, TH1F *> &a, const std::pair<std::string, TH1F *> &b)
              {
                  return a.first < b.first;
              });
    std::sort(sorted_histograms2d.begin(), sorted_histograms2d.end(),
              [](const std::pair<std::string, TH2F *> &a, const std::pair<std::string, TH2F *> &b)
              {
                  return a.first < b.first;
              });
    std::sort(sorted_histograms3d.begin(), sorted_histograms3d.end(),
              [](const std::pair<std::string, TH3F *> &a, const std::pair<std::string, TH3F *> &b)
              {
                  return a.first < b.first;
              });
    for (const auto &pair: sorted_histograms1d) {
        const string &histname = pair.first;
        TH1F *hist = pair.second;
        cout << "[AnalyzerCore::WriteHist] Writing 1D histogram" << histname << endl;
        // Split the directory and name
        // e.g. "dir1/dir2/histname" -> "dir1/dir2", "histname"
        // e.g. "histname" -> "", "histname"
        size_t last_slash = histname.find_last_of('/');
        string this_prefix, this_name;
        last_slash == string::npos ? this_prefix = "" : this_prefix = histname.substr(0, last_slash);
        last_slash == string::npos ? this_name = histname : this_name = histname.substr(last_slash + 1);

        TDirectory *this_dir = outfile->GetDirectory(this_prefix.c_str());
        if (!this_dir) outfile->mkdir(this_prefix.c_str());
        outfile->cd(this_prefix.c_str());
        hist->Write(this_name.c_str());
    }
    for (const auto &pair: sorted_histograms2d) {
        const string &histname = pair.first;
        cout << "[AnalyzerCore::WriteHist] Writing 2D histogram" << histname << endl;
        TH2F *hist = pair.second;
        // Split the directory and name
        // e.g. "dir1/dir2/histname" -> "dir1/dir2", "histname"
        // e.g. "histname" -> "", "histname"
        size_t last_slash = histname.find_last_of('/');
        string this_prefix, this_name;
        last_slash == string::npos ? this_prefix = "" : this_prefix = histname.substr(0, last_slash);
        last_slash == string::npos ? this_name = histname : this_name = histname.substr(last_slash + 1);

        TDirectory *this_dir = outfile->GetDirectory(this_prefix.c_str());
        if (!this_dir) outfile->mkdir(this_prefix.c_str());
        outfile->cd(this_prefix.c_str());
        hist->Write(this_name.c_str());
    }
    for (const auto &pair: sorted_histograms3d) {
        const string &histname = pair.first;
        cout << "[AnalyzerCore::WriteHist] Writing 3D histogram" << histname << endl;
        TH3F *hist = pair.second;
        // Split the directory and name
        // e.g. "dir1/dir2/histname" -> "dir1/dir2", "histname"
        // e.g. "histname" -> "", "histname"
        size_t last_slash = histname.find_last_of('/');
        string this_prefix, this_name;
        last_slash == string::npos ? this_prefix = "" : this_prefix = histname.substr(0, last_slash);
        last_slash == string::npos ? this_name = histname : this_name = histname.substr(last_slash + 1);

        TDirectory *this_dir = outfile->GetDirectory(this_prefix.c_str());
        if (!this_dir) outfile->mkdir(this_prefix.c_str());
        outfile->cd(this_prefix.c_str());
        hist->Write(this_name.c_str());
    }
    for (const auto &pair: treemap) {
        const string &treename = pair.first;
        cout << "[AnalyzerCore::WriteHist] Writing tree" << treename << endl;
        TTree *tree = pair.second;

        size_t last_slash = treename.find_last_of('/');
        string this_prefix, this_name;
        last_slash == string::npos ? this_prefix = "" : this_prefix = treename.substr(0, last_slash);
        last_slash == string::npos ? this_name = treename : this_name = treename.substr(last_slash + 1);

        TDirectory *this_dir = outfile->GetDirectory(this_prefix.c_str());
        if (!this_dir) outfile->mkdir(this_prefix.c_str());
        outfile->cd(this_prefix.c_str());
        TTree* temptree = tree->CloneTree(-1);//this is because the tree contains lot of empty disabled branch. I don't know better way to handle this, so just keep memory-consuming way for now.
        temptree->Write();
        delete temptree;
        delete tree;
    }
    cout << "[AnalyzerCore::WriteHist] Writing histograms done" << endl;
    outfile->Close();
}

