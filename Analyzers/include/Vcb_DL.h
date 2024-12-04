#ifndef Vcb_DL_h
#define Vcb_DL_h

#include "AnalyzerCore.h"
#include "VcbParameters.h"
#include <future>
#include <tuple>
#include <memory>
#include <set>
#include "correction.h"
#include <variant>
using correction::CorrectionSet;

class Vcb_DL : public AnalyzerCore
{
public:
    void initializeAnalyzer();
    void executeEvent();
    void executeEventFromParameter(TString syst);
    bool PassBaseLineSelection(bool remove_flavtagging_cut = true, TString syst = "nominal");
    void FillHistogramsAtThisPoint(const TString &histPrefix, float weight = 1.f);
    int Unroller(RVec<Jet> &jets);
    float CalculateEventWeight(bool include_tagging_weight=true);
    short GetPassedBTaggingWP(const Jet &jet);
    short GetPassedCTaggingWP(const Jet &jet);

    Vcb_DL();
    ~Vcb_DL();

    unique_ptr<CorrectionSet> cset_TriggerSF;

    enum class Channel
    {
        EE,
        ME,
        MM,
    };

    Channel channel;

    // Objects
    RVec<Muon> AllMuons;
    RVec<Electron> AllElectrons;
    RVec<Jet> AllJets;
    RVec<Gen> AllGens;
    RVec<LHE> AllLHEs;
    RVec<GenJet> AllGenJets;
    // Selected Objects
    RVec<Jet> Jets;
    RVec<Electron> Electrons_Veto;
    RVec<Electron> Electrons;
    RVec<Muon> Muons_Veto;
    RVec<Muon> Muons;
    RVec<Lepton> Leptons;
    Event ev;
    Particle MET;
    RVec<int> ttbar_jet_indices;
    // event info
    float HT;
    short n_jets;
    short n_b_tagged_jets;
    short n_c_tagged_jets;
    short n_partonFlav_b_jets;
    short n_partonFlav_c_jets;
    bool find_all_jets;
    int tt_decay_code;
};

#endif
