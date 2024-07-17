#include "MCCorrection.h"
MCCorrection::MCCorrection() {}

MCCorrection::MCCorrection(const TString &era, const string &btagging_eff_file, const string &ctagging_eff_file) {
    cout << "[MCCorrection::MCCorrection] MCorrection created for " << era << endl;
    SetEra(era);
    string json_muon = string(getenv("JSONPOG_INTEGRATION_PATH")) + "/POG/MUO";
    string json_puWeights = string(getenv("JSONPOG_INTEGRATION_PATH")) + "/POG/LUM";
    string json_btagging = string(getenv("JSONPOG_INTEGRATION_PATH")) + "/POG/BTV";
    string json_ctagging = string(getenv("JSONPOG_INTEGRATION_PATH")) + "/POG/BTV";
    string json_btagging_eff = string(getenv("SKNANO_DATA"));
    string json_ctagging_eff = string(getenv("SKNANO_DATA"));
    string json_electron = string(getenv("JSONPOG_INTEGRATION_PATH")) + "/POG/EGM";
    string json_photon = string(getenv("JSONPOG_INTEGRATION_PATH")) + "/POG/EGM";
    string json_jerc = string(getenv("JSONPOG_INTEGRATION_PATH")) + "/POG/JME";
    string json_jerc_fatjet = string(getenv("JSONPOG_INTEGRATION_PATH")) + "/POG/JME";
    string json_jetvetomap = string(getenv("JSONPOG_INTEGRATION_PATH")) + "/POG/JME";

    if (era == "2022") {
        json_muon += "/2022_27Jun2023/muon_Z.json.gz";
        json_puWeights += "/2022_Summer22/puWeights.json.gz";
        json_btagging += "/2022_Summer22/btagging.json.gz";
        json_ctagging += "/2022_Summer22/ctagging.json.gz";
        json_btagging_eff += "/2022/BTV/"+btagging_eff_file;
        json_ctagging_eff += "/2022/BTV/"+ctagging_eff_file;
        json_electron += "/2022_Summer22/electron.json.gz";
        json_photon += "/2022_Summer22/photon.json.gz";
        json_jerc += "/2022_Summer22/jet_jerc.json.gz";
        json_jerc_fatjet += "/2022_Summer22/fatJet_jerc.json.gz";
        json_jetvetomap += "/2022_Summer22/jetvetomaps.json.gz";
    }
    else if (era == "2022EE") {
        json_muon += "/2022EE_27Jun2023/muon_Z.json.gz";
        json_puWeights += "/2022_Summer22EE/puWeights.json.gz";
        json_btagging += "/2022_Summer22EE/btagging.json.gz";
        json_ctagging += "/2022_Summer22EE/ctagging.json.gz";
        json_btagging_eff += "/2022EE/BTV/"+btagging_eff_file;
        json_ctagging_eff += "/2022EE/BTV/"+ctagging_eff_file;
        json_electron += "/2022_Summer22EE/electron.json.gz";
        json_photon += "/2022_Summer22EE/photon.json.gz";
        json_jerc += "/2022_Summer22EE/jet_jerc.json.gz";
        json_jerc_fatjet += "/2022_Summer22EE/fatJet_jerc.json.gz";
        json_jetvetomap += "/2022_Summer22EE/jetvetomaps.json.gz";
    }
    else {
        cerr << "[MCCorrection::MCCorrection] no correction file for era " << era << endl;
    }
    cout << "[MCCorrection::MCCorrection] using muon SF file: " << json_muon << endl;
    cset_muon = CorrectionSet::from_file(json_muon);
    cout << "[MCCorrection::MCCorrection] using puWeights file: " << json_puWeights << endl;
    cset_puWeights = CorrectionSet::from_file(json_puWeights);
    cout << "[MCCorrection::MCCorrection] using btagging file: " << json_btagging << endl;
    cset_btagging = CorrectionSet::from_file(json_btagging);
    cout << "[MCCorrection::MCCorrection] using ctagging file: " << json_ctagging << endl;
    cset_ctagging = CorrectionSet::from_file(json_ctagging);
    cout << "[MCCorrection::MCCorrection] using btagging eff file: " << json_btagging_eff << endl;
    cset_btagging_eff = CorrectionSet::from_file(json_btagging_eff);
    cout << "[MCCorrection::MCCorrection] using ctagging eff file: " << json_ctagging_eff << endl;
    cset_ctagging_eff = CorrectionSet::from_file(json_ctagging_eff);
    cout << "[MCCorrection::MCCorrection] using electron file: " << json_electron << endl;
    cset_electron = CorrectionSet::from_file(json_electron);
    cout << "[MCCorrection::MCCorrection] using photon file: " << json_photon << endl;
    cset_photon = CorrectionSet::from_file(json_photon);
    cout << "[MCCorrection::MCCorrection] using jerc file: " << json_jerc << endl;
    cset_jerc = CorrectionSet::from_file(json_jerc);
    cout << "[MCCorrection::MCCorrection] using jerc_fatjet file: " << json_jerc_fatjet << endl;
    cset_jerc_fatjet = CorrectionSet::from_file(json_jerc_fatjet);
    cout << "[MCCorrection::MCCorrection] using jetvetomap file: " << json_jetvetomap << endl;
    cset_jetvetomap = CorrectionSet::from_file(json_jetvetomap);
}

MCCorrection::~MCCorrection() {}

float MCCorrection::GetMuonIDSF(const TString &Muon_ID_SF_Key, const float eta, const float pt, const TString &sys) const {
    auto cset = cset_muon->at(string(Muon_ID_SF_Key));
    try{return cset->evaluate({fabs(eta), pt, sys.Data()});}
    catch (exception &e) {
        cerr << "[MCCorrection::GetMuonIDSF] " << e.what() << endl;
        return 1.;
    }

}

float MCCorrection::GetElectronIDSF(const TString &Electron_ID_SF_Key, const float eta, const float pt, const TString &sys) const {
    string syst = "sf";

    if (sys == "nominal") syst = "sf";
    else if (sys == "up") syst = "sf_up";
    else if (sys == "down") syst = "sf_down";
    else cout << "[MCCorrection::GetElectronIDSF] no sys " << sys << endl;

    auto cset = cset_electron->at("Electron-ID-SF");
    string erastr = "";
    if (DataEra == "2022") erastr = "2022Re-recoBCD";
    else if (DataEra == "2022EE") erastr = "2022Re-recoE+PromptFG";
    else cout << "[MCCorrection::GetElectronIDSF] no ElectronIDSF for era " << GetEra() << endl;
    try{return cset->evaluate({erastr,syst,string(Electron_ID_SF_Key),eta,pt});}
    catch (exception &e) {
        cerr << "[MCCorrection::GetElectronIDSF] " << e.what() << endl;
        return 1.;
    }
}

float MCCorrection::GetElectronTriggerSF(const TString &Electron_Trigger_SF_Key, const float eta, const float pt, const TString &sys) const {
    //Since EGM POG does not provide the trigger SF, I just keep this part as a dummy function.
    return 1.;
}

float MCCorrection::GetElectronRECOSF(const float eta, const float pt, const TString &sys) const {
    if(pt < 20.) return GetElectronIDSF("RecoBelow20", eta, pt, sys);
    else if(pt >= 20. && pt <= 75) return GetElectronIDSF("Reco20to75", eta, pt, sys);
    else if(pt > 75.) return GetElectronIDSF("RecoAbove75", eta, pt, sys);
    else return 1.;
}

float MCCorrection::GetPUWeight(const float nTrueInt, const TString &sys) const{
    //nota bene: Input should be nTrueInt, not nPileUp
    correction::Correction::Ref cset = nullptr;
    if (DataEra == "2022") cset = cset_puWeights->at("Collisions2022_355100_357900_eraBCD_GoldenJson");
    else if (DataEra == "2022EE") cset = cset_puWeights->at("Collisions2022_359022_362760_eraEFG_GoldenJson");
    else cout << "[MCCorrection::GetPUWeight] no PUWeight for era " << GetEra() << endl;
    try{return cset->evaluate({nTrueInt, sys.Data()});}
    catch (exception &e) {
        cerr << "[MCCorrection::GetPUWeight] " << e.what() << endl;
        return 1.;
    }
}
void MCCorrection::SetTaggingParam(JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerWP wp){
    global_tagger = tagger;
    global_wp = wp;
    global_taggerStr = JetTagging::GetTaggerCorrectionLibStr(tagger);
    global_wpStr = JetTagging::GetTaggerCorrectionWPStr(wp);
}

float MCCorrection::GetBTaggingWP() const{
    correction::Correction::Ref cset = cset_btagging->at(global_taggerStr + "_wp_values");
    return cset->evaluate({global_wpStr});
}

float MCCorrection::GetBTaggingWP(JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerWP wp) const{
    string this_taggerStr = JetTagging::GetTaggerCorrectionLibStr(tagger).Data();
    string this_wpStr = JetTagging::GetTaggerCorrectionWPStr(wp).Data();
    correction::Correction::Ref cset = cset_btagging->at(this_taggerStr+"_wp_values");
    return cset->evaluate({this_wpStr});
}

float MCCorrection::GetBTaggingEff(const float eta, const float pt, JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerWP wp, const TString &sys){
    string this_taggerStr = JetTagging::GetTaggerCorrectionLibStr(tagger).Data();
    string this_wpStr = JetTagging::GetTaggerCorrectionWPStr(wp).Data();
    correction::Correction::Ref cset = cset_btagging_eff->at(this_taggerStr);
    return cset->evaluate({sys.Data() ,this_wpStr, });
}

float MCCorrection::GetBTaggingSF(const RVec<Jet> &jets, const JetTagging::JetFlavTagger tagger, const JetTagging::JetFlavTaggerWP wp, const TString &method, const TString &sys){
    string this_taggerStr = JetTagging::GetTaggerCorrectionLibStr(tagger).Data();
    string this_wpStr = JetTagging::GetTaggerCorrectionWPStr(wp).Data();
    float weight = 1.;
        // method is comb, mujets, or shape
        if (method == "shape"){
            auto cset = cset_btagging->at(this_taggerStr+"_shape");
            for(auto &jet : jets){
                weight *= cset->evaluate({
                    sys.Data(),
                    this_wpStr,
                    jet.hadronFlavour(),
                    jet.Pt(),
                    jet.Eta()
                });
            } // TODO implement R correction
            return weight;
        }
        else if(method == "comb" or method == "mujets"){
            auto cset = cset_btagging->at(this_taggerStr+"_"+method.Data());
            auto cset_light = cset_btagging->at(this_taggerStr+"_light");
            float this_cut = GetBTaggingWP(tagger, wp);
            for(auto &jet : jets){
                float eff = GetBTaggingEff(jet.Eta(), jet.Pt(), tagger, wp, sys);
                float sf = cset->evaluate({sys.Data(), this_wpStr, jet.hadronFlavour(), jet.Pt(), jet.Eta()});
                float sf_light = cset_light->evaluate({sys.Data(), this_wpStr, 0, jet.Pt(), jet.Eta()});
                if (jet.GetBTaggerResult(tagger) > this_cut){
                    weight *= sf;
                }
                else{
                    weight *= (1 - eff * sf) / (1 - eff);
                }
            }
            return weight;
        }

        else{
            cout << "[MCCorrection::GetBTaggingSF] no method " << method << endl;
            return 1.;
        }
}

pair<float,float> MCCorrection::GetCTaggingWP() const{
    if ((int)global_wp >= 3){
        cout << "[MCCorrection::GetCTaggingWP] Workingpoint VeryTight and SuperTight are not available for C-Tagging" << endl;
        exit(ENODATA);
    }
    correction::Correction::Ref cset = cset_ctagging->at(global_taggerStr + "_wp_values");
    return make_pair(cset->evaluate({global_wpStr, "CvB"}), cset->evaluate({global_wpStr, "CvL"}));
}

pair<float,float> MCCorrection::GetCTaggingWP(JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerWP wp) const{
    string this_taggerStr = JetTagging::GetTaggerCorrectionLibStr(tagger).Data();
    string this_wpStr = JetTagging::GetTaggerCorrectionWPStr(wp).Data();
    if((int)wp >= 3){
        cout << "[MCCorrection::GetCTaggingWP] Workingpoint VeryTight and SuperTight are not available for C-Tagging" << endl;
        exit(ENODATA);
    }
    correction::Correction::Ref cset = cset_ctagging->at(this_taggerStr + "_wp_values");
    return make_pair(cset->evaluate({this_wpStr ,"CvB"}), cset->evaluate({this_wpStr ,"CvL"}));
}

float MCCorrection::GetCTaggingEff(const float eta, const float pt, JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerWP wp, const TString &sys)
{
    return 1.;
    string this_taggerStr = JetTagging::GetTaggerCorrectionLibStr(tagger).Data();
    string this_wpStr = JetTagging::GetTaggerCorrectionWPStr(wp).Data();
    correction::Correction::Ref cset = cset_btagging_eff->at(this_taggerStr);
    return cset->evaluate({
        sys.Data(),
        this_wpStr,
    });
}

float MCCorrection::GetCTaggingSF(const RVec<Jet> &jets, const JetTagging::JetFlavTagger tagger, const JetTagging::JetFlavTaggerWP wp, const TString &method, const TString &sys)
{
    return 1.;
    string this_taggerStr = JetTagging::GetTaggerCorrectionLibStr(tagger).Data();
    string this_wpStr = JetTagging::GetTaggerCorrectionWPStr(wp).Data();
    float weight = 1.;
    // method is comb, mujets, or shape
    if (method == "shape")
    {
        auto cset = cset_btagging->at(this_taggerStr + "_shape");
        for (auto &jet : jets)
        {
            weight *= cset->evaluate({sys.Data(),
                                      this_wpStr,
                                      jet.hadronFlavour(),
                                      jet.Pt(),
                                      jet.Eta()});
        } // TODO implement R correction
        return weight;
    }
    else if (method == "comb" or method == "mujets")
    {
        auto cset = cset_btagging->at(this_taggerStr + "_" + method.Data());
        auto cset_light = cset_btagging->at(this_taggerStr + "_light");
        float this_cut = GetBTaggingWP(tagger, wp);
        for (auto &jet : jets)
        {
            float eff = GetBTaggingEff(jet.Eta(), jet.Pt(), tagger, wp, sys);
            float sf = cset->evaluate({sys.Data(), this_wpStr, jet.hadronFlavour(), jet.Pt(), jet.Eta()});
            float sf_light = cset_light->evaluate({sys.Data(), this_wpStr, 0, jet.Pt(), jet.Eta()});
            if (jet.GetBTaggerResult(tagger) > this_cut)
            {
                weight *= sf;
            }
            else
            {
                weight *= (1 - eff * sf) / (1 - eff);
            }
        }
        return weight;
    }

    else
    {
        cout << "[MCCorrection::GetBTaggingSF] no method " << method << endl;
        return 1.;
    }
}

bool MCCorrection::IsJetVetoZone(const float eta, const float phi, TString mapCategory) const{
    correction::Correction::Ref cset = nullptr;
    if(DataEra == "2022") cset = cset_jetvetomap->at("Summer22_23Sep2023_RunCD_V1");
    else if(DataEra == "2022EE") cset = cset_jetvetomap->at("Summer22EE_23Sep2023_RunEFG_V1");
    else cout << "[MCCorrection::IsJetVetoZone] no JetVetoMap for era " << GetEra() << endl;

    if (cset->evaluate({mapCategory.Data(), eta, phi}) > 0) return true;

    return false;
}