#ifndef MeasureJetTaggingR_h
#define MeasureJetTaggingR_h

#include "AnalyzerCore.h"
#include "Vcb_DL.h"
#include "Vcb_FH.h"
#include "Vcb_SL.h"
#include <variant>
class MeasureJetTaggingR : public AnalyzerCore
{

public:
    void initializeAnalyzer();
    void SetAnalyzer(std::variant<Vcb_DL*, Vcb_FH*, Vcb_SL*> analyzer);
    void executeEvent();
    void executeEventFromParameter();

    void makeIteratingChannels();
    RVec<Vcb::Channel> iterating_channels;

    MeasureJetTaggingR();
    ~MeasureJetTaggingR();

protected:
    RVec<Jet> AllJets;
    RVec<Muon> AllMuons;
    RVec<Electron> AllElectrons;
    RVec<GenJet> AllGenJets;
    RVec<Gen> AllGens;

    vector<float> vec_htbins;
    vector<float> vec_nTrueIntbin;
    vector<float> vec_njetbin;
    vector<float> vec_jet_ptbin;
    vector<float> vec_jet_etabin;
    
    int n_vec_htbins;
    int n_vec_nTrueIntbin;
    int n_vec_njetbin;

    float PtMax;

    int NEtaBin;
    int NPtBin;

    float *etabins;
    float *ptbins;

    RVec<JetTagging::JetFlavTagger> Taggers;
    RVec<JetTagging::JetFlavTaggerWP> WPs;

    std::variant<Vcb_DL*, Vcb_FH*, Vcb_SL*> current_analyzer;
    
    TString current_channel;
};

#endif