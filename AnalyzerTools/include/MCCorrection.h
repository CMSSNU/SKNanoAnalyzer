#ifndef MCCorrection_h
#define MCCorrection_h

#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
using namespace std;

#include "TString.h"
#include "correction.h"
#include "JetTaggingParameter.h"
using correction::CorrectionSet;

class MCCorrection {
public:
    MCCorrection();
    MCCorrection(const TString &era);
    ~MCCorrection();

    inline TString GetEra() const { return DataEra; }
    inline void SetEra(TString era) { DataEra = era; }
    
    //Muon
    float GetMuonIDSF(const TString &Muon_ID_SF_Key, const float abseta, const float pt, const TString &sys = "nominal") const;
    //PUWeights
    float GetPUWeight(const float nTrueInt, const TString &sys = "nominal") const;
    //taggingparam
    void SetTaggingParam(JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerWP wp);
    // btaging
    float GetBTaggingWP() const;
    float GetBTaggingWP(JetTagging::JetFlavTagger, JetTagging::JetFlavTaggerWP) const;
    //bool IsBTagged(Jet jet) const;
    //float GetBTaggingSF_WP(Jet jet, const TString &sys = "nominal") const;
    //float GetBTaggingSF_ShapeCorr(Jet jet, const TString &sys = "nominal") const;
    // ctagging
    pair<float,float> GetCTaggingWP() const;
    pair<float,float> GetCTaggingWP(JetTagging::JetFlavTagger, JetTagging::JetFlavTaggerWP) const;
    //bool IsCTagged(Jet jet) const;
    //float GetCTaggingSF_WP(Jet jet, const TString &sys = "nominal") const;
    //float GetCTaggingSF_ShapeCorr(Jet jet, const TString &sys = "nominal") const;
    // electron
    float GetElectronIDSF(const TString &Electron_ID_SF_Key, const float abseta, const float pt, const TString &sys = "nominal") const;
    float GetElectronTriggerSF(const TString &Electron_Trigger_SF_Key, const float abseta, const float pt, const TString &sys = "nominal") const;
    //photon
    //jerc
    //jerc_fatjet
    //jetvetomap
    bool IsJetVetoZone(const float eta, const float phi, TString mapCategory) const;

private:
    JetTagging::JetFlavTagger tagger;
    JetTagging::JetFlavTaggerWP wp;
    string wpStr;
    string taggerStr;
    TString DataEra;
    unique_ptr<CorrectionSet> cset_muon;
    unique_ptr<CorrectionSet> cset_puWeights;
    unique_ptr<CorrectionSet> cset_btagging;
    unique_ptr<CorrectionSet> cset_ctagging;
    unique_ptr<CorrectionSet> cset_electron;
    unique_ptr<CorrectionSet> cset_photon;
    unique_ptr<CorrectionSet> cset_jerc;
    unique_ptr<CorrectionSet> cset_jerc_fatjet;
    unique_ptr<CorrectionSet> cset_jetvetomap;
};

#endif
