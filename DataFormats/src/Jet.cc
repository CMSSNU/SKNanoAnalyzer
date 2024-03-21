#include "Jet.h"

ClassImp(Jet)

    Jet::Jet() : Particle()
{

  // Jet ID
  j_looseJetId = false;
  j_tightJetID = false;
  j_tightLepVetoJetID = false;

  // Corrections
  j_PNetRegPtRawCorr = -999.0;
  j_PNetRegPtRawCorrNeutrino = -999.0;
  j_PNetRegPtRawRes = -999.0;
  j_rawFactor = -999.0;

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
  // j_hfadjacentEtaStripsSize = -999;
  // j_hfcentralEtaStripSize = -999;
  // j_hfsigmaEtaEta = -999.0;
  // j_hfsigmaPhiPhi = -999.0;
  j_m = -999.0;
}

Jet::~Jet()
{
}

//////////

bool Jet::PassID(TString ID) const
{

  if (ID == "loose")
    return j_looseJetId;
  else if (ID == "tight")
    return j_tightJetID;
  else if (ID == "tightLepVeto")
    return j_tightLepVetoJetID;
  else
    cout << "[Jet::PassID] No id : " << ID << endl;

  exit(ENODATA);

  return false;
}

/*float Jet::GetTaggerResult(JetTagging::Tagger tg) const
{
  if (1)
    cout << "[Jet::GetTaggerResult] Not Implemented yet" << endl;
  else
  {
    cout << "[Jet::GetTaggerResult] ERROR; Wrong tagger : " << tg << endl;
    return -999;
  }
}*/
