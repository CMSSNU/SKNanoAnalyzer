#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Event+;
#pragma link C++ class Particle+;
#pragma link C++ class Lepton+;
#pragma link C++ class MuMu+;

// Event-scoped input references are reflected for PyROOT but are never
// streamed. Their raw pointers and column providers are valid only while the
// loader remains on the originating event.
#pragma link C++ class MuonView-;
#pragma link C++ class ElectronView-;
#pragma link C++ class JetView-;
#pragma link C++ class SelectedJetView-;
#pragma link C++ class GenView-;
#pragma link C++ class GenJetView-;
#pragma link C++ class LHEView-;
#pragma link C++ class TauView-;
#pragma link C++ class PhotonView-;
#pragma link C++ class FatJetView-;
#pragma link C++ class SelectedFatJetView-;
#pragma link C++ class SVView-;
#pragma link C++ class TrigObjView-;
#pragma link C++ class GenDressedLeptonView-;
#pragma link C++ class GenIsolatedPhotonView-;
#pragma link C++ class GenVisTauView-;

#pragma link C++ class EventRange<MuonSoA,MuonView>-;
#pragma link C++ class ElectronViewCollection-;
#pragma link C++ class EventRange<JetSoA,JetView>-;
#pragma link C++ class SelectedJetViewCollection-;
#pragma link C++ class EventRange<GenSoA,GenView>-;
#pragma link C++ class EventRange<GenJetSoA,GenJetView>-;
#pragma link C++ class EventRange<LHESoA,LHEView>-;
#pragma link C++ class EventRange<TauSoA,TauView>-;
#pragma link C++ class EventRange<PhotonSoA,PhotonView>-;
#pragma link C++ class EventRange<FatJetSoA,FatJetView>-;
#pragma link C++ class SelectedFatJetViewCollection-;
#pragma link C++ class EventRange<SVSoA,SVView>-;
#pragma link C++ class EventRange<TrigObjSoA,TrigObjView>-;
#pragma link C++ class EventRange<GenDressedLeptonSoA,GenDressedLeptonView>-;
#pragma link C++ class EventRange<GenIsolatedPhotonSoA,GenIsolatedPhotonView>-;
#pragma link C++ class EventRange<GenVisTauSoA,GenVisTauView>-;

#pragma link C++ typedef MuonViewCollection;
#pragma link C++ typedef JetViewCollection;
#pragma link C++ typedef GenViewCollection;
#pragma link C++ typedef GenJetViewCollection;
#pragma link C++ typedef LHEViewCollection;
#pragma link C++ typedef TauViewCollection;
#pragma link C++ typedef PhotonViewCollection;
#pragma link C++ typedef FatJetViewCollection;
#pragma link C++ typedef SVViewCollection;
#pragma link C++ typedef TrigObjViewCollection;
#pragma link C++ typedef GenDressedLeptonViewCollection;
#pragma link C++ typedef GenIsolatedPhotonViewCollection;
#pragma link C++ typedef GenVisTauViewCollection;
