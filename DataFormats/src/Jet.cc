#include "Jet.h"

ClassImp(Jet)

Jet::Jet() : Particle() {
  // Jet ID
  j_looseJetId = false;
  j_tightJetID = false;
  j_tightLepVetoJetID = false;

  // Corrections
  j_PNetRegPtRawCorr = -999.0;
  j_PNetRegPtRawCorrNeutrino = -999.0;
  j_PNetRegPtRawRes = -999.0;
  j_rawFactor = -999.0;
  j_bRegCorr = -999.0;
  j_bRegRes = -999.0;
  j_cRegCorr = -999.0;
  j_cRegRes = -999.0;

  // Flavour Tagging Scores
  j_btagDeepFlavB = -999.0;
  j_btagDeepFlavCvB = -999.0;
  j_btagDeepFlavCvL = -999.0;
  j_btagDeepFlavQG = -999.0;
  j_btagPNetB = -999.0;
  j_btagPNetCvB = -999.0;
  j_btagPNetCvL = -999.0;
  j_btagPNetQvG = -999.0;
  j_btagPNetTauVJet = -999.0;
  j_btagRobustParTAK4B = -999.0;
  j_btagRobustParTAK4CvB = -999.0;
  j_btagRobustParTAK4CvL = -999.0;
  j_btagRobustParTAK4QG = -999.0;

  // Jet Substructure
  j_chEmEF = -999.0;
  j_chHEF = -999.0;
  j_neEmEF = -999.0;
  j_neHEF = -999.0;
  j_muEF = -999.0;
  // j_muonSubtrFactor = -999.0;
  j_nConstituents = -999;
  j_nElectrons = -999;
  j_nMuons = -999;
  j_nSVs = -999;

  // Matching Information
  j_electronIdx1 = -999;
  j_electronIdx2 = -999;
  j_muonIdx1 = -999;
  j_muonIdx2 = -999;
  j_svIdx1 = -999;
  j_svIdx2 = -999;
  j_genJetIdx = -999;

  // Jet Flavour
  j_hadronFlavour = -999;
  j_partonFlavour = -999;

  // Others
  j_area = -999.0;
  j_originalIndex = -1;
  // j_hfadjacentEtaStripsSize = -999;
  // j_hfcentralEtaStripSize = -999;
  // j_hfsigmaEtaEta = -999.0;
  // j_hfsigmaPhiPhi = -999.0;
  j_m = -999.0;
  j_unsmearedP4.SetPxPyPzE(-999., -999., -999., -999.);
}

Jet::~Jet() {}

//////////

bool Jet::PassID(TString ID) const {
  if (ID == "loose")
    return j_looseJetId;
  else if (ID == "tight")
    return j_tightJetID;
  else if (ID == "tightLepVeto")
    return j_tightLepVetoJetID;
  else if (ID == "loosePuId")
    return j_loosePuId;
  else if (ID == "mediumPuId")
    return j_mediumPuId;
  else if (ID == "tightPuId")
    return j_tightPuId;
  else if (ID == "NOCUT") 
    return true;
  else
    cout << "[Jet::PassID] No id : " << ID << endl;

  exit(ENODATA);

  return false;
}

void Jet::SetJetID(unsigned char IDBit, int Run) {
  // bit 0 is loose, bit 1 is tight, bit 2 is tightLepVeto
  if(Run == 2) {
    j_looseJetId = (IDBit & 1);
    j_tightJetID = (IDBit & 2);
    j_tightLepVetoJetID = (IDBit & 4);
  }
  // due to the bug in the NanoAODv12, Below fix is necessary
  // Please refer https://gitlab.cern.ch/cms-jetmet/coordination/coordination/-/issues/117 or https://twiki.cern.ch/twiki/bin/viewauth/CMS/JetID13p6TeV#Recommendations_for_the_13_6_AN1
  // We are moving to NanoAODv13 for Run3, need different implementation
  else { // Run == 3
    j_tightJetID = false;
    if (abs(Eta()) <= 2.6) 
      j_tightJetID = (neHEF() < 0.99) && (neEmEF() < 0.9) && (chMultiplicity()+neMultiplicity() > 1) && (chHEF() > 0.01) && (chMultiplicity() > 0);
    else if (abs(Eta()) > 2.6 && abs(Eta()) <= 2.7) 
      j_tightJetID = (neHEF() < 0.90) && (neEmEF() < 0.99);
    else if (abs(Eta()) > 2.7 && abs(Eta()) <= 3.0) 
      j_tightJetID = (neHEF() < 0.99);
    else // abs(eta) > 3.0
      j_tightJetID = (neMultiplicity() >= 2) && (neEmEF() < 0.4);

    j_tightLepVetoJetID = false;
    if (abs(Eta()) <= 2.7) 
      j_tightLepVetoJetID = j_tightJetID && (muEF() < 0.8) && (chEmEF() < 0.8);
    else 
      j_tightLepVetoJetID = j_tightJetID;

    /* This is for NanoAODv12
    if (abs(eta) <= 2.7) Jet_passJetIdTight = b & (1 << 1);
    else if (abs(eta) > 2.7 && abs(eta) <= 3.0) Jet_passJetIdTight = (b & (1 << 1)) && (Jet_neHEF < 0.99);
    else if (abs(eta) > 3.0) Jet_passJetIdTight = (b & (1 << 1)) && (Jet_neEmEF < 0.4);      
    
    bool Jet_passJetIdTightLepVeto = false;      
    if (abs(eta) <= 2.7) Jet_passJetIdTightLepVeto = Jet_passJetIdTight && (Jet_muEF < 0.8) && (Jet_chEmEF < 0.8);      
    else Jet_passJetIdTightLepVeto = Jet_passJetIdTight;      
    j_looseJetId = (b & 1);      
    j_tightJetID = Jet_passJetIdTight;      
    j_tightLepVetoJetID = Jet_passJetIdTightLepVeto;      
    */
  }
}

bool Jet::PassID(JetID id) const {
  switch(id){
    case JetID::LOOSE:
      return j_looseJetId;
    case JetID::TIGHT:
      return j_tightJetID;
    case JetID::TIGHTLEPVETO:
      return j_tightLepVetoJetID;
    case JetID::PUID_LOOSE:
      return j_loosePuId;
    case JetID::PUID_MEDIUM:
      return j_mediumPuId;
    case JetID::PUID_TIGHT:
      return j_tightPuId;
    case JetID::NOCUT:
      return true;
    default:
      break;
  }

  return false;
}

float Jet::GetBTaggerResult(JetTagging::JetFlavTagger tagger) const{
  if(tagger == JetTagging::JetFlavTagger::DeepJet)
    return j_btagDeepFlavB;
  else if(tagger == JetTagging::JetFlavTagger::ParticleNet)
    return j_btagPNetB;
  else if(tagger == JetTagging::JetFlavTagger::ParT)
    return j_btagRobustParTAK4B;
  else
    cout << "[Jet::GetBTaggerResult] No tagger" << endl;
    exit(ENODATA);
  return -999.0;
}

pair<float,float> Jet::GetCTaggerResult(JetTagging::JetFlavTagger tagger) const{
  if(tagger == JetTagging::JetFlavTagger::DeepJet)
    return make_pair(j_btagDeepFlavCvB,j_btagDeepFlavCvL);
  else if(tagger == JetTagging::JetFlavTagger::ParticleNet)
    return make_pair(j_btagPNetCvB,j_btagPNetCvL);
  else if(tagger == JetTagging::JetFlavTagger::ParT)
    return make_pair(j_btagRobustParTAK4CvB,j_btagRobustParTAK4CvL);
  else
    cout << "[Jet::GetCTaggerResult] No tagger" << endl;
    exit(ENODATA);
  return make_pair(-999.0,-999.0);
}

float Jet::GetQvGTaggerResult(JetTagging::JetFlavTagger tagger) const{
  if(tagger == JetTagging::JetFlavTagger::DeepJet)
    return j_btagDeepFlavQG;
  else if(tagger == JetTagging::JetFlavTagger::ParticleNet)
    return j_btagPNetQvG;
  else if(tagger == JetTagging::JetFlavTagger::ParT)
    return j_btagRobustParTAK4QG;
  else
    cout << "[Jet::GetQvGTaggerResult] No tagger" << endl;
    exit(ENODATA);
  return -999.0;
}

TLorentzVector Jet::GetUnsmearedP4() const{
  return j_unsmearedP4;
}
