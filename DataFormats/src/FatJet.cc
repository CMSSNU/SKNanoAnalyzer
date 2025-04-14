#include "FatJet.h"

ClassImp(FatJet)

FatJet::FatJet() : Particle() {

    // ───── Basic Jet Properties ─────────────
    j_pt = 0.;
    j_eta = 0.;
    j_phi = 0.;
    j_mass = 0.;
    j_rawFactor = 0.;
    j_area = 0.;

    // ───── Energy Fractions ────────────────
    j_chEmEF = 0.;
    j_chHEF = 0.;
    j_hfEmEF = 0.;
    j_hfHEF = 0.;
    j_muEF = 0.;
    j_neEmEF = 0.;
    j_neHEF = 0.;

    // ───── Multiplicities ──────────────────
    j_chMultiplicity = 0;
    j_neMultiplicity = 0;
    j_nConstituents = 0;

    // ───── Indices and Flavors ─────────────
    j_genJetAK8Idx = -1;
    j_subJetIdx1 = -1;
    j_subJetIdx2 = -1;
    j_hadronFlavour = 0;

    // ───── Jet Substructure ────────────────
    j_msoftdrop = 0.;
    j_n2b1 = -999.;
    j_n3b1 = -999.;
    j_lsf3 = 0.;
    j_tau1 = -999.;
    j_tau2 = -999.;
    j_tau3 = -999.;
    j_tau4 = -999.;

    // ───── Jet ID / Constituent Info ───────
    //j_jetId = 0;


    // ───── ParticleNet WithMass ────────────
    j_particleNetWithMass_H4qvsQCD = -999.;
    j_particleNetWithMass_HccvsQCD = -999.;
    j_particleNetWithMass_HbbvsQCD = -999.;
    j_particleNetWithMass_QCD      = -999.;
    j_particleNetWithMass_TvsQCD   = -999.;
    j_particleNetWithMass_WvsQCD   = -999.;
    j_particleNetWithMass_ZvsQCD   = -999.;

    // ───── ParticleNet Inclusive ───────────
    j_particleNet_QCD       = -999.;
    j_particleNet_QCD0HF    = -999.;
    j_particleNet_QCD1HF    = -999.;
    j_particleNet_QCD2HF    = -999.;
    j_particleNet_XbbVsQCD  = -999.;
    j_particleNet_XccVsQCD  = -999.;
    j_particleNet_XqqVsQCD  = -999.;
    j_particleNet_XggVsQCD  = -999.;
    j_particleNet_XteVsQCD  = -999.;
    j_particleNet_XtmVsQCD  = -999.;
    j_particleNet_XttVsQCD  = -999.;
    j_particleNet_massCorr  = -999.;

    // ───── GlobalParT3 Taggers ─────────────
    j_globalParT3_QCD = -999.;
    j_globalParT3_TopbWev = -999.;
    j_globalParT3_TopbWmv = -999.;
    j_globalParT3_TopbWq = -999.;
    j_globalParT3_TopbWqq = -999.;
    j_globalParT3_TopbWtauhv = -999.;
    j_globalParT3_WvsQCD = -999.;
    j_globalParT3_XWW3q = -999.;
    j_globalParT3_XWW4q = -999.;
    j_globalParT3_XWWqqev = -999.;
    j_globalParT3_XWWqqmv = -999.;
    j_globalParT3_Xbb = -999.;
    j_globalParT3_Xcc = -999.;
    j_globalParT3_Xcs = -999.;
    j_globalParT3_Xqq = -999.;
    j_globalParT3_Xtauhtaue = -999.;
    j_globalParT3_Xtauhtauh = -999.;
    j_globalParT3_Xtauhtaum = -999.;
    j_globalParT3_massCorrGeneric = -999.;
    j_globalParT3_massCorrX2p = -999.;
    j_globalParT3_withMassTopvsQCD = -999.;
    j_globalParT3_withMassWvsQCD = -999.;
    j_globalParT3_withMassZvsQCD = -999.;
}


FatJet::~FatJet() { 

}

bool FatJet::PassID(TString ID) const {

    if (ID == "Loose")             return PassLoose();
    else if (ID == "Tight")        return PassTight();
    else if (ID == "TightLepVeto") return PassTightLepVeto();
    else cout << "[FatJet::PassID] No id : " << ID << endl;
    
    exit(ENODATA);
    return false;

}

float FatJet::GetTaggerResult(JetTagging::FatJetTagger tagger, JetTagging::FatJetTaggerScoreType scoreType) const{
    // First inspect correct tagger - score type is given
    // declare as null reference
    const unordered_map<JetTagging::FatJetTaggerScoreType, float> *taggerResult = nullptr;
    switch (tagger)
    {
    case JetTagging::FatJetTagger::ParticleNetWithMass:
        taggerResult = &j_particleNetWithMass;
    case JetTagging::FatJetTagger::ParticleNet:
        taggerResult = &j_particleNet;
    case JetTagging::FatJetTagger::ParT:
        taggerResult = &j_globalParT3;
    case JetTagging::FatJetTagger::ParTWithMass:
        taggerResult = &j_globalParT3WithMass;
        break;
    default:
        cout << "[FatJet::GetTaggerResult] No tagger " << JetTagging::GetFatJetTaggerCorrectionLibStr(tagger) << endl;
        exit(ENODATA);
    }
    // Now inspect correct score type
    auto it = taggerResult->find(scoreType); // <--- use '->' for pointer access
    if (it == taggerResult->end())
    {
        cout << "[FatJet::GetTaggerResult] No tagger score type "
            << JetTagging::GetFatJetScoreTypeStr(scoreType) 
            << " for tagger " << JetTagging::GetFatJetTaggerCorrectionLibStr(tagger) << endl;
        exit(ENODATA);
    }
    return it->second;
}