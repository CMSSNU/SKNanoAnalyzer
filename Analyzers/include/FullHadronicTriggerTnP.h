#ifndef FullHadronicTriggerTnP_h
#define FullHadronicTriggerTnP_h

#include "AnalyzerCore.h"
#include "TKinFitter.h"
#include "TFitParticlePt.h"
#include "TFitParticleEtEtaPhi.h"
#include "TFitConstraintMGaus.h"
#include "TFitConstraintM.h"
#include "TFitConstraintEp.h"
#include "TMatrixD.h"
#include <future>
#include <tuple>
#include <memory>
#include <set>
class FullHadronicTriggerTnP : public AnalyzerCore
{
public:
    void executeEvent();
    void executeEventFromParameter(TString syst);
    FullHadronicTriggerTnP();
    ~FullHadronicTriggerTnP();

    void initializeAnalyzer();
    TString ControlTrigger;
    TString SingleBTagTrigger;
    TString DoubleBTagTrigger;

    map<JetTagging::JetFlavTaggerWP,bool> passOneBTag;
    map<JetTagging::JetFlavTaggerWP,bool> passTwoBTag;

    float MuTriggerSafePtCut, ElTriggerSafePtCut;
    bool dataDrivenMode;

    RVec<TString> MuonIDs, MuonIDSFKeys, MuonISOs, MuonIsoSFKeys, MuonTriggerSFKeys;
    RVec<TString> ElectronIDs, ElectronIDSFKeys;

    RVec<Muon> AllMuons;
    RVec<Electron> AllElectrons;
    RVec<Jet> AllJets;
    RVec<Gen> AllGens;
    RVec<LHE> AllLHEs;
    RVec<GenJet> AllGenJets;
    Event ev;
};

#endif
