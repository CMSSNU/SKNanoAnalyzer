#ifndef Correction_h
#define Correction_h

#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
using namespace std;

#include "TString.h"
#include "correction.h"
#include "Jet.h"
#include "JetTaggingParameter.h"
#include "Gen.h"
#include "Muon.h"
using correction::CorrectionSet;

class Correction
{
public:
    Correction();
    Correction(const TString &era, const TString &sample, const bool IsData, const string &btagging_eff_file = "btaggingEff.json", const string &ctagging_eff_file = "ctaggingEff.json", const string &btagging_R_file = "btaggingR.json", const string &ctagging_R_file = "ctaggingR.json");
    ~Correction();

    enum class variation
    {
        nom,
        up,
        down
    };

    enum class POG
    {
        MUO,
        LUM,
        BTV,
        EGM,
        JME
    };

    enum class XYCorrection_MetType{
        Type1PFMET,
        Type1PuppiMET
    };

    inline TString GetEra() const { return DataEra; }
    inline void SetEra(TString era) { 
        DataEra = era; 
        if(era == "2016preVFP" || era == "2016postVFP" || era == "2017" || era == "2018" || era == "2018UL" ) Run = 2;
        else Run = 3;
    }
    inline void SetSample(TString sample) { Sample = sample; }
    inline void setIsData(bool isData) { IsDATA = isData; }



    // Muon
    inline float GetMuonISOSF(const TString &Muon_ISO_SF_Key, const float abseta, const float pt, const variation syst = variation::nom, const TString &source = "") { return GetMuonIDSF(Muon_ISO_SF_Key, abseta, pt, syst, source); }
    inline float GetMuonTriggerSF(const TString &Muon_Trigger_SF_Key, const float abseta, const float pt, const variation syst = variation::nom, const TString &source = "") { return GetMuonIDSF(Muon_Trigger_SF_Key, abseta, pt, syst, source); };
    float GetMuonIDSF(const TString &Muon_ID_SF_Key, const float abseta, const float pt, const variation syst = variation::nom, const TString &source = "") const;
    float GetMuonTriggerEff(const TString &Muon_Trigger_Eff_Key, const float abseta, const float pt, const bool eff_for_data, const variation syst = variation::nom, const TString &source = "") const;
    float GetMuonTriggerWeight(const TString &Muon_Trigger_Eff_Key, const RVec<Muon> &muons, const variation syst = variation::nom, const TString &source = "") const;
    // PUWeights
    float GetPUWeight(const float nTrueInt, const variation syst = variation::nom, const TString &source = "") const;
    // taggingparam
    void SetTaggingParam(JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerWP wp);
    // btaging
    float GetBTaggingWP() const;
    float GetBTaggingWP(JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerWP wp) const;
    float GetBTaggingEff(const float eta, const float pt, const int flav, JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerWP wp, const variation syst = variation::nom);
    float GetBTaggingSF(const RVec<Jet> &jets, const JetTagging::JetFlavTagger tagger, const JetTagging::JetFlavTaggerWP wp, const JetTagging::JetTaggingSFMethod &method = JetTagging::JetTaggingSFMethod::mujets, const variation syst = variation::nom, const TString &source = "total");
    float GetBTaggingR(const int njets, const float HT, const JetTagging::JetFlavTagger tagger, const TString &processName = "", const TString &ttBarCategory = "total", const variation syst = variation::nom, const TString &source = "total") const;
    inline float GetBTaggingSF(const RVec<Jet> &jets, const JetTagging::JetTaggingSFMethod &method = JetTagging::JetTaggingSFMethod::mujets, const variation syst = variation::nom, const TString &source = "total") { return GetBTaggingSF(jets, global_tagger, global_wp, method, syst, source); }
    // ctagging
    pair<float, float> GetCTaggingWP() const;
    pair<float, float> GetCTaggingWP(JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerWP wp) const;
    float GetCTaggingEff(const float eta, const float pt, const int flav, JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerWP wp, const variation syst = variation::nom);
    float GetCTaggingSF(const RVec<Jet> &jets, const JetTagging::JetFlavTagger tagger, const JetTagging::JetFlavTaggerWP wp, const JetTagging::JetTaggingSFMethod &method, const variation syst, const TString &source = "total");
    float GetCTaggingR(const float npvs, const float HT, const JetTagging::JetFlavTagger tagger, const TString &processName = "", const TString &ttBarCategory = "total", const variation syst = variation::nom, const TString &source = "total") const;
    inline float GetCTaggingSF(const RVec<Jet> &jets, const JetTagging::JetTaggingSFMethod &method = JetTagging::JetTaggingSFMethod::mujets, const variation syst = variation::nom, const TString &source = "total") { return GetCTaggingSF(jets, global_tagger, global_wp, method, syst, source); }
    // electron
    float GetElectronRECOSF(const float abseta, const float pt, const variation syst = variation::nom, const TString &source = "total") const;
    float GetElectronIDSF(const TString &Electron_ID_SF_Key, const float abseta, const float pt, const variation syst = variation::nom, const TString &source = "total") const;
    float GetElectronTriggerSF(const TString &Electron_Trigger_SF_Key, const float abseta, const float pt, const variation syst = variation::nom, const TString &source = "total") const;
    // photon
    // jerc
    float GetJER(const float eta, const float pt, const float rho) const;
    float GetJERSF(const float eta, const float pt, const variation syst = variation::nom, const TString &source = "total") const;
    float GetJESUncertainty(const float eta, const float pt, const variation syst = variation::nom, const TString &source = "total") const;
    // jerc_fatjet
    // jetvetomap
    bool IsJetVetoZone(const float eta, const float phi, TString mapCategory) const;
    //MET
    void METXYCorrection(Particle &Met, const int RunNumber, const int npvs, const XYCorrection_MetType MetType);
    //reweighting
    float GetTopPtReweight(const RVec<Gen> &gens) const;

private: 
    JetTagging::JetFlavTaggerWP global_wp;
    JetTagging::JetFlavTagger global_tagger;
    string global_wpStr;
    string global_taggerStr;
    TString DataEra;
    int Run;
    TString Sample;
    bool IsDATA;
    unique_ptr<CorrectionSet> cset_muon;
    unique_ptr<CorrectionSet> cset_muon_trig_eff;
    unique_ptr<CorrectionSet> cset_puWeights;
    unique_ptr<CorrectionSet> cset_btagging;
    unique_ptr<CorrectionSet> cset_ctagging;
    unique_ptr<CorrectionSet> cset_btagging_eff;
    unique_ptr<CorrectionSet> cset_ctagging_eff;
    unique_ptr<CorrectionSet> cset_btagging_R;
    unique_ptr<CorrectionSet> cset_ctagging_R;
    unique_ptr<CorrectionSet> cset_electron;
    unique_ptr<CorrectionSet> cset_photon;
    unique_ptr<CorrectionSet> cset_jerc;
    unique_ptr<CorrectionSet> cset_jerc_fatjet;
    unique_ptr<CorrectionSet> cset_jetvetomap;
    unique_ptr<CorrectionSet> cset_met;
    unordered_map<string, string> MUO_keys;
    unordered_map<string, string> LUM_keys;
    unordered_map<string, string> BTV_keys;
    unordered_map<string, string> EGM_keys;
    unordered_map<string, string> JME_keys;
    unordered_map<string, string> JME_vetomap_keys;
    unordered_map<string, string> JME_MET_keys;
    // All POG choose different string for the systematics, so we need to convert the enum to the string....
    // Here I implement every single function instead of a general one, because heavy use of switch-case might be slow.
    inline string
    getSystString_MUO(const variation syst) const
    {
        string sys_string = "nominal";
        switch (syst)
        {
        case variation::nom:
            sys_string = "nominal";
            break;
        case variation::up:
            sys_string = "systup";
            break;
        case variation::down:
            sys_string = "systdown";
            break;
        };
        return sys_string;
    };

    inline string getSystString_LUM(const variation syst) const
    {
        string sys_string = "nominal";
        switch (syst)
        {
        case variation::nom:
            sys_string = "nominal";
            break;
        case variation::up:
            sys_string = "up";
            break;
        case variation::down:
            sys_string = "down";
            break;
        };
        return sys_string;
    };

    inline string getSystString_BTV(const variation syst) const
    {
        string sys_string = "central";
        switch (syst)
        {
        case variation::nom:
            sys_string = "central";
            break;
        case variation::up:
            sys_string = "up";
            break;
        case variation::down:
            sys_string = "down";
            break;
        };
        return sys_string;
    };

    inline string getSystString_EGM(const variation syst) const
    {
        string sys_string = "sf";
        switch (syst)
        {
        case variation::nom:
            sys_string = "sf";
            break;
        case variation::up:
            sys_string = "sfup";
            break;
        case variation::down:
            sys_string = "sfdown";
            break;
        };
        return sys_string;
    };

    inline string getSystString_JME(const variation syst) const
    {
        string sys_string = "nom";
        switch (syst)
        {
        case variation::nom:
            sys_string = "nom";
            break;
        case variation::up:
            sys_string = "up";
            break;
        case variation::down:
            sys_string = "down";
            break;
        };
        return sys_string;
    };
};

#endif
