#include "FatJet.h"

ClassImp(FatJet)

FatJet::FatJet() : Particle() {

    j_msoftdrop = 0.;
    j_area = 0.;
    // Gen Matching
    j_genJetAK8Idx = 0;
    j_subJetIdx1   = 0;
    j_subJetIdx2   = 0;
    // FatJet ID
    j_jetId = 0;  // Jet ID flags bit1 is loose (always false in 2017 since it does not exist), bit2 is tight, bit3 is tightLepVeto
    // Constituent Info
    j_nBHadrons = 0, j_nCHadrons = 0, j_nConstituents = 0;
    j_lsf3 = 0.;
    // BTagging Info
    j_btagDDBvLV2 = -999.; // DeepDoubleX V2 (mass-decorrelated) discriminator for H(Z)->bb vs QCD
    j_btagDDCvBV2 = -999.; // DeepDoubleX V2 (mass-decorrelated) discriminator for H(Z)->cc vs H(Z)->bb
    j_btagDDCvLV2 = -999.; // DeepDoubleX V2 (mass-decorrelated) discriminator for H(Z)->cc vs QCD
    j_btagDeepB   = -999.; // DeepCSV b+bb tag discriminator
    j_btagHbb     = -999.; // Higgs to BB tagger discriminator
    // PNet w/ Mass Discriminators
    j_particleNetWithMass_H4qvsQCD = -999.; // H(->VV->qqqq) vs QCD discriminator
    j_particleNetWithMass_HccvsQCD = -999.; // H(->cc) vs QCD discriminator
    j_particleNetWithMass_HbbvsQCD = -999.; // H(->bb) vs QCD discriminator
    j_particleNetWithMass_QCD      = -999.; // QCD(bb,cc,b,c,others) sum
    j_particleNetWithMass_TvsQCD   = -999.; // top vs QCD discriminator
    j_particleNetWithMass_WvsQCD   = -999.; // W vs QCD discriminator
    j_particleNetWithMass_ZvsQCD   = -999.; // Z vs QCD discriminator
    // PNet w/o Mass Discriminators
    j_particleNet_QCD       = -999.; // QCD(0+1+2HF) sum
    j_particleNet_QCD0HF    = -999.; // QCD 0 HF (b/c) score  
    j_particleNet_QCD1HF    = -999.; // QCD 1 HF (b/c) score 
    j_particleNet_QCD2HF    = -999.; // QCD 2 HF (b/c) score 
    j_particleNet_XbbVsQCD  = -999.; // X->bb vs. QCD score: Xbb/(Xbb+QCD)
    j_particleNet_XccVsQCD  = -999.; // X->cc vs. QCD score: Xcc/(Xcc+QCD)
    j_particleNet_XqqVsQCD  = -999.; // X->qq (uds) vs. QCD score: Xqq/(Xqq+QCD)
    j_particleNet_XggVsQCD  = -999.; // X->gg vs. QCD score: Xgg/(Xgg+QCD)
    j_particleNet_XteVsQCD  = -999.; // X->e tau_h vs. QCD score: Xte/(Xte+QCD)
    j_particleNet_XtmVsQCD  = -999.; // X->mu tau_h vs. QCD score: Xtm/(Xtm+QCD)
    j_particleNet_XttVsQCD  = -999.; // X->tau_h tau_h vs. QCD score: Xtt/(Xtt+QCD)
    j_particleNet_massCorr  = -999.; // ParticleNet mass regression, relative correction to JEC-corrected jet mass (no softdrop)
    // Subjettiness
    j_tau1 = -999.; 
    j_tau2 = -999.;
    j_tau3 = -999.;
    j_tau4 = -999.;

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

/* TODO implement FatJet::GetTaggerResult

double FatJet::GetTaggerResult(JetTagging::Tagger tg) const{
    
    if     (tg==JetTagging::DeepDoubleX_HZbbvsQCD)        return j_btagDDBvLV2;
    else if(tg==JetTagging::DeepDoubleX_HZccvsHZbb)       return j_btagDDCvBV2;
    else if(tg==JetTagging::DeepDoubleX_HZccvsQCD)        return j_btagDDCvLV2;
    else if(tg==JetTagging::DeepCSV_?)                    return j_btagDeepB;
    else if(tg==JetTagging::HiggsToBB)                    return j_btagHbb;
    else if(tg==JetTagging::particleNetWithMass_H4qvsQCD) return j_particleNetWithMass_H4qvsQCD;
    else if(tg==JetTagging::particleNetWithMass_HccvsQCD) return j_particleNetWithMass_HccvsQCD;
    else if(tg==JetTagging::particleNetWithMass_HbbvsQCD) return j_particleNetWithMass_HbbvsQCD;
    else if(tg==JetTagging::particleNetWithMass_QCD)      return j_particleNetWithMass_QCD;
    else if(tg==JetTagging::particleNetWithMass_TvsQCD)   return j_particleNetWithMass_TvsQCD;
    else if(tg==JetTagging::particleNetWithMass_WvsQCD)   return j_particleNetWithMass_WvsQCD;
    else if(tg==JetTagging::particleNetWithMass_ZvsQCD)   return j_particleNetWithMass_ZvsQCD;
    else if(tg==JetTagging::particleNet_QCD)              return j_particleNet_QCD;
    else if(tg==JetTagging::particleNet_QCD0HF)           return j_particleNet_QCD0HF;
    else if(tg==JetTagging::particleNet_QCD1HF)           return j_particleNet_QCD1HF;
    else if(tg==JetTagging::particleNet_QCD2HF)           return j_particleNet_QCD2HF;
    else if(tg==JetTagging::particleNet_XbbVsQCD)         return j_particleNet_XbbVsQCD;
    else if(tg==JetTagging::particleNet_XccVsQCD)         return j_particleNet_XccVsQCD;
    else if(tg==JetTagging::particleNet_XqqVsQCD)         return j_particleNet_XqqVsQCD;
    else if(tg==JetTagging::particleNet_XggVsQCD)         return j_particleNet_XggVsQCD;
    else if(tg==JetTagging::particleNet_XteVsQCD)         return j_particleNet_XteVsQCD;
    else if(tg==JetTagging::particleNet_XtmVsQCD)         return j_particleNet_XtmVsQCD;
    else if(tg==JetTagging::particleNet_XttVsQCD)         return j_particleNet_XttVsQCD;
    else if(tg==JetTagging::particleNet_massCorr)         return j_particleNet_massCorr;
    else{
        cout << "[FatJet::GetTaggerResult] ERROR; Wrong tagger" << endl;
        return -999;
    }

}*/