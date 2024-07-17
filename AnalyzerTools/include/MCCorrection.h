#ifndef MCCorrection_h
#define MCCorrection_h

#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
using namespace std;

#include "TString.h"
#include "correction.h"
#include "Jet.h"
#include "JetTaggingParameter.h"
using correction::CorrectionSet;

class MCCorrection{
public:
    MCCorrection();
    MCCorrection(const TString &era, const string &btagging_eff_file = "btaggingEff.json", const string &ctagging_eff_file = "ctaggingEff.json");
    ~MCCorrection();

    inline TString GetEra() const { return DataEra; }
    inline void SetEra(TString era) { DataEra = era; }
    
    //Muon
    inline float GetMuonISOSF(const TString &Muon_ISO_SF_Key, const float abseta, const float pt, const TString &sys = "nominal"){return GetMuonIDSF(Muon_ISO_SF_Key, abseta, pt, sys);}
    inline float GetMuonTriggerSF(const TString &Muon_Trigger_SF_Key, const float abseta, const float pt, const TString &sys = "nominal"){return GetMuonIDSF(Muon_Trigger_SF_Key, abseta, pt, sys);}
    float GetMuonIDSF(const TString &Muon_ID_SF_Key, const float abseta, const float pt, const TString &sys = "nominal") const;
    //PUWeights
    float GetPUWeight(const float nTrueInt, const TString &sys = "nominal") const;
    //taggingparam
    void SetTaggingParam(JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerWP wp);
    // btaging
    float GetBTaggingWP() const;
    float GetBTaggingWP(JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerWP wp) const;
    float GetBTaggingEff(const float eta, const float pt, JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerWP wp, const TString &sys = "central");
    float GetBTaggingSF(const RVec<Jet> &jets, const JetTagging::JetFlavTagger tagger, const JetTagging::JetFlavTaggerWP wp, const TString &method = "mujets",const TString &sys = "central");
    inline float GetBTaggingSF(const RVec<Jet> &jets, const TString &method = "mujets", const TString &sys = "central"){return GetBTaggingSF(jets, global_tagger, global_wp, method, sys);}
    // ctagging
    pair<float,float> GetCTaggingWP() const;
    pair<float,float> GetCTaggingWP(JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerWP wp) const;
    float GetCTaggingEff(const float eta, const float pt, JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerWP wp, const TString &sys = "central");
    float GetCTaggingSF(const RVec<Jet> &jets, const JetTagging::JetFlavTagger tagger, const JetTagging::JetFlavTaggerWP wp, const TString &method = "mujets", const TString &sys = "central");
    inline float GetCTaggingSF(const RVec<Jet> &jets, const TString &method = "mujets", const TString &sys = "central") { return GetBTaggingSF(jets, global_tagger, global_wp, method, sys); }
    // electron
    float GetElectronRECOSF(const float abseta, const float pt, const TString &sys = "nominal") const;
    float GetElectronIDSF(const TString &Electron_ID_SF_Key, const float abseta, const float pt, const TString &sys = "nominal") const;
    float GetElectronTriggerSF(const TString &Electron_Trigger_SF_Key, const float abseta, const float pt, const TString &sys = "nominal") const;
    //photon
    //jerc
    //jerc_fatjet
    //jetvetomap
    bool IsJetVetoZone(const float eta, const float phi, TString mapCategory) const;

private:
    JetTagging::JetFlavTagger global_tagger;
    JetTagging::JetFlavTaggerWP global_wp;
    string global_wpStr;
    string global_taggerStr;
    TString DataEra;
    unique_ptr<CorrectionSet> cset_muon;
    unique_ptr<CorrectionSet> cset_puWeights;
    unique_ptr<CorrectionSet> cset_btagging;
    unique_ptr<CorrectionSet> cset_ctagging;
    unique_ptr<CorrectionSet> cset_btagging_eff;
    unique_ptr<CorrectionSet> cset_ctagging_eff;
    unique_ptr<CorrectionSet> cset_electron;
    unique_ptr<CorrectionSet> cset_photon;
    unique_ptr<CorrectionSet> cset_jerc;
    unique_ptr<CorrectionSet> cset_jerc_fatjet;
    unique_ptr<CorrectionSet> cset_jetvetomap;
};

#endif
