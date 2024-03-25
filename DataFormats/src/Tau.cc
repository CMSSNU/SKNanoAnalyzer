#include "Tau.h"

ClassImp(Tau)

Tau::Tau() {
    
    this->SetLeptonFlavour(TAU);

    // unsigned char ID bits (Decay Mode)
    j_decayMode   = 0; 
    j_genPartFlav = 0; // 1 = prompt electron, 2 = prompt muon, 3 = tau->e decay, 4 = tau->mu decay, 5 = hadronic tau decay, 0 = unknown or unmatched

    // unsigned char ID bits (DeepTau2018v2p5)
    j_idDeepTau2018v2p5VSe   = 0; // 1 = VVVLoose, 2 = VVLoose, 3 = VLoose, 4 = Loose, 5 = Medium, 6 = Tight, 7 = VTight, 8 = VVTight
    j_idDeepTau2018v2p5VSjet = 0; // 1 = VVVLoose, 2 = VVLoose, 3 = VLoose, 4 = Loose, 5 = Medium, 6 = Tight, 7 = VTight, 8 = VVTight
    j_idDeepTau2018v2p5VSmu  = 0; // 1 = VLoose, 2 = Loose, 3 = Medium, 4 = Tight

}

Tau::~Tau() {}

bool Tau::PassID(TString ID) const{
    
    if(ID == "NoCut") return true;
    /* 
        user defined Tau IDs 
    */
    if(ID == "TestID"){
        if(!DecayModeNewDM())     return false;
        if(!( fabs(dZ()) < 0.2 )) return false;
        if(!(passTIDvEl() && passTIDvJet() && passTIDvMu())) return false;
        return true;
    }

    return false;

}