#ifndef MeasTrigEff_h
#define MeasTrigEff_h

#include "AnalyzerCore.h"

class MeasTrigEff : public AnalyzerCore {
public:
    MeasTrigEff();
    ~MeasTrigEff();
    void initializeAnalyzer();
    void executeEvent();
    void measEMuTrigEff_ElLegs(RVec<Muon> &tightMuons,
                              RVec<Muon> &vetoMuons,
                              RVec<Electron> &tightElectrons,
                              RVec<Electron> &vetoElectrons,
                              RVec<TrigObj> &trigObjs,
                              float &weight);
    void measEMuTrigEff_MuLegs(RVec<Muon> &tightMuons,
                               RVec<Muon> &vetoMuons,
                               RVec<Electron> &tightElectrons,
                               RVec<Electron> &vetoElectrons,
                               RVec<TrigObj> &trigObjs,
                               float &weight);
    void measEMuTrigEff_DZ(Event &ev,
                           RVec<Muon> &tightMuons,
                           RVec<Muon> &vetoMuons,
                           RVec<Electron> &tightElectrons,
                           RVec<Electron> &vetoElectrons,
                           RVec<TrigObj> &trigObjs,
                           float &weight);
    void measDblMuTrigEff_DZ(Event &ev,
                             RVec<Muon> &tightMuons,
                             RVec<Muon> &vetoMuons,
                             RVec<Electron> &tightElectrons,
                             RVec<Electron> &vetoElectrons,
                             RVec<TrigObj> &trigObjs,
                             float &weight);
    bool PassSLT(const Electron &el, const RVec<TrigObj> &trigObjs);
    bool PassSLT(const Muon &mu, const RVec<TrigObj> &trigObjs);
    bool PassDLT(const Muon &mu, const RVec<TrigObj> &trigObjs, float pt_cut);
    bool PassEMT_ElLeg(const Electron &el, const RVec<TrigObj> &trigObjs, float pt_cut);
    bool PassEMT_MuLeg(const Muon &mu, const RVec<TrigObj> &trigObjs, float pt_cut);

    // Userflags
    bool MeasElLegs, MeasMuLegs;
    bool MeasDblMuPairwise, MeasEMuPairwise;
    // MeasElLegs -> SingleMuon,Muon,Muon0,Muon1 dataset
    // MeasMuLegs -> EGamma,EGamma0,Egamma1 dataset
    // MeasDblMuPairwise -> DoubleMuon/Muon dataset
    // MeasEMuPairwise -> MuonEG dataset

    // ID and Trigger definitions
    // Filters are defined per object level
    IDContainer *MuonIDs, *ElectronIDs;
    RVec<TString> SglMuTrigs, SglElTrigs;
    RVec<TString> DblMuTrigs, DblMuDZTrigs, DblMuDZMTrigs;
    RVec<TString> EMuTrigs, EMuDZTrigs;
};

#endif