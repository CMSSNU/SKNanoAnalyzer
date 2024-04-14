#include "AnalyzerCore.h"

AnalyzerCore::AnalyzerCore() {
    outfile = nullptr;
    pdfReweight = new PDFReweight();
}

AnalyzerCore::~AnalyzerCore() {
    for (const auto &pair: histmap1d) delete pair.second; histmap1d.clear();
    for (const auto &pair: histmap2d) delete pair.second; histmap2d.clear();
    for (const auto &pair: histmap3d) delete pair.second; histmap3d.clear();
    if (outfile) delete outfile;
    if (pdfReweight) delete pdfReweight;
    if (mcCorr) delete mcCorr;
}

void AnalyzerCore::SetOutfilePath(TString outpath) {
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
// TODO: See SKFlat
float AnalyzerCore::MCweight(bool usesign, bool norm_1invpb) const {
    if (IsDATA) return 1.;
    else return genWeight;
}

// Not implemented yets
float AnalyzerCore::GetPileUpWeight(int sys) {
    if (IsDATA) return 1.;
    else return 1.;
}


// Objects
Event AnalyzerCore::GetEvent() {
    Event ev;
    ev.SetnPileUp(Pileup_nPU);
    ev.SetEra(GetEra());
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
        if (! muon.PassID(ID)) continue;
        selected_muons.push_back(muon);
    }
    return selected_muons;
}

RVec<Electron> AnalyzerCore::GetAllElectrons() {
    RVec<Electron> electrons;
    for (int i = 0; i < nElectron; i++) {
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

RVec<Gen> AnalyzerCore::GetGens() {

    RVec<Gen> Gens;

    for (int i = 0; i < nGenPart; i++) {

        Gen gen;

        gen.SetIsEmpty(false);
        gen.SetPtEtaPhiM(GenPart_pt[i], GenPart_eta[i], GenPart_phi[i], GenPart_mass[i]);
        gen.SetIndexPIDStatus(i, GenPart_pdgId[i], GenPart_status[i]);
        gen.SetMother(GenPart_genPartIdxMother[i]);
        gen.SetGenStatusFlags(GenPart_statusFlags[i]);

        Gens.push_back(gen);
    }

    return Gens;
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

void AnalyzerCore::WriteHist() {
    for (const auto &pair: histmap1d) {
        const string &histname = pair.first;
        TH1F *hist = pair.second;
       
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
    for (const auto &pair: histmap2d) {
        const string &histname = pair.first;
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
    for (const auto &pair: histmap3d) {
        const string &histname = pair.first;
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
    outfile->Close();
}
