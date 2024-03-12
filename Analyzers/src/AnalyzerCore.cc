#include "AnalyzerCore.h"

AnalyzerCore::AnalyzerCore() {
    outfile = nullptr;
}

AnalyzerCore::~AnalyzerCore() {
    if (outfile != nullptr) {
        outfile->Close();
        delete outfile;
    }
}

void AnalyzerCore::SetOutfilePath(TString outpath) {
    outfile = new TFile(outpath, "RECREATE");
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

        muons.push_back(muon);
    }

    return muons;
}

TH1F* AnalyzerCore::GetHist1D(TString histname) {
    auto mapit = histmap.find(histname);
    return (mapit != histmap.end()) ? mapit->second : nullptr;
}

void AnalyzerCore::FillHist(TString histname, float value, float weight, int n_bin, float x_min, float x_max) {
    TH1F *this_hist = GetHist1D(histname);
    if ( !this_hist) {
        this_hist = new TH1F(histname, "", n_bin, x_min, x_max);
        this_hist->SetDirectory(nullptr);
        histmap[histname] = this_hist;
    }
    this_hist->Fill(value, weight);
}

void AnalyzerCore::WriteHist() {
    outfile->cd();
    for (const auto &hist: histmap) {
        // Extract the histogram name
        TString this_fullname = hist.second->GetName();
        TString this_name = this_fullname(this_fullname.Last('/') + 1, this_fullname.Length());
        TString this_suffix = this_fullname(0, this_fullname.Last('/'));

        // Get or create the directory for this histogram
        TDirectory *dir = outfile->GetDirectory(this_suffix);
        if (!dir) {
            dir = outfile->mkdir(this_suffix);
        }

        // change to the appropriate directory and write the histogram
        dir->cd();
        hist.second->Write(this_name);
    }
}
