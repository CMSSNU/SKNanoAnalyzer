#include "Muon.h"

ClassImp(Muon)

Muon::Muon() {
    this->SetLeptonFlavour(MUON);
    // boolean ID bits
    j_looseId = false;
    j_mediumId = false;
    j_mediumPromptId = false;
    j_tightId = false;
    j_softId = false;
    j_softMvaId = false;
    j_triggerIdLoose = false;

    // unsigned char ID bits
    j_highPtId = 0;     // High PT cut-based ID (1 = tracker high pT, 2 = global high pT)
    j_miniIsoId = 0;    // 1=Loose, 2=Medium, 3=Tight, 4=VeryTight
    j_multiIsoId = 0;   // 1=Loose, 2=Medium
    j_mvaMuId = 0;      // 2=Medium, 3=Tight
    //j_mvaLowPtId = 0;   // 1=Loose, 2=Medium
    j_pfIsoId = 0;      // 1=VLoose, 2=Loose, 3=Medium, 4=Tight, 5=VTight, 6=VVTight
    j_puppiIsoId = 0;   // 1=Loose, 2=Medium, 3=Tight
    j_tkIsoId = 0;      // 1=Loose, 2=Tight

    // MVA ID scores
    j_softMva = -999.;
    j_mvaLowPt = -999.;
    j_mvaTTH = -999.;
}

Muon::~Muon() {}

void Muon::SetBIDBit(BooleanID id, bool idbit) {
    switch (id) {
        case BooleanID::LOOSE:           j_looseId = idbit; break;
        case BooleanID::MEDIUM:          j_mediumId = idbit; break;
        case BooleanID::MEDIUMPROMPT:    j_mediumPromptId = idbit; break;
        case BooleanID::TIGHT:           j_tightId = idbit; break;
        case BooleanID::SOFT:            j_softId = idbit; break;
        case BooleanID::SOFTMVA:         j_softMvaId = idbit; break;
        case BooleanID::TRIGGERLOOSE:    j_triggerIdLoose = idbit; break;
        default: break;
    }
}

void Muon::SetWIDBit(WorkingPointID id, unsigned char wp) {
    switch (id) {
        case WorkingPointID::HIGHPT:    j_highPtId = (unsigned char)(wp+1); break;
        case WorkingPointID::MINIISO:   j_miniIsoId = (unsigned char)(wp+1); break;
        case WorkingPointID::MULTIISO:  j_multiIsoId = (unsigned char)(wp+1); break;
        case WorkingPointID::MVAMU:     j_mvaMuId = (unsigned char)(wp+2); break;
        //case WorkingPointID::MVALOWPT:  j_mvaLowPtId = (unsigned char)(wp+1); break;
        case WorkingPointID::PFISO:     j_pfIsoId = (unsigned char)wp; break;
        case WorkingPointID::PUPPIISO:  j_puppiIsoId = (unsigned char)(wp+1); break;
        case WorkingPointID::TKISO:     j_tkIsoId = (unsigned char)(wp+1); break;
        default: break;
    }
}

void Muon::SetMVAID(MVAID id, float score) {
    switch (id) {
        case MVAID::SOFTMVA: j_softMva = score; break;
        case MVAID::MVALOWPT: j_mvaLowPt = score; break;
        case MVAID::MVATTH: j_mvaTTH = score; break;
        default: break;
    }
}

bool Muon::PassID(const TString ID) const {
    if (ID == "NOCUT")            return true;
    if (ID == "POGTight")         return isPOGTightId();
    if (ID == "POGMedium")        return isPOGMediumId();
    if (ID == "POGMediumPrompt")  return isPOGMediumPromptId();
    if (ID == "POGLoose")         return isPOGLooseId();
    if (ID == "POGSoft")          return isPOGSoftId();
    if (ID == "POGSoftMVA")       return isPOGSoftMvaId();
    if (ID == "POGTriggerLoose")  return isPOGTriggerIdLoose();
    if (ID == "POGTrackerHighPt") return (int)HighPtId() == 1;
    if (ID == "POGGlobalHighPt")  return (int)HighPtId() == 2;
    if (ID == "POGMiniIsoLoose")  return (int)MiniIsoId() >= (int)(WorkingPoint::LOOSE);
    if (ID == "POGMiniIsoMedium") return (int)MiniIsoId() >= (int)(WorkingPoint::MEDIUM);
    if (ID == "POGMiniIsoTight")  return (int)MiniIsoId() >= (int)(WorkingPoint::TIGHT);
    if (ID == "POGMiniIsoVTight") return (int)MiniIsoId() >= (int)(WorkingPoint::VTIGHT);
    if (ID == "POGMultiIsoLoose") return (int)MultiIsoId() >= (int)(WorkingPoint::LOOSE);
    if (ID == "POGMultiIsoMedium")return (int)MultiIsoId() >= (int)(WorkingPoint::MEDIUM);
    if (ID == "POGMvaMuMedium")   return (int)MvaMuId() >= (int)(WorkingPoint::MEDIUM);
    if (ID == "POGMvaMuTight")    return (int)MvaMuId() >= (int)(WorkingPoint::TIGHT);
    if (ID == "POGPfIsoVLoose")   return (int)PfIsoId() >= (int)(WorkingPoint::VLOOSE);
    if (ID == "POGPfIsoLoose")    return (int)PfIsoId() >= (int)(WorkingPoint::LOOSE);
    if (ID == "POGPfIsoMedium")   return (int)PfIsoId() >= (int)(WorkingPoint::MEDIUM);
    if (ID == "POGPfIsoTight")    return (int)PfIsoId() >= (int)(WorkingPoint::TIGHT);
    if (ID == "POGPfIsoVTight")   return (int)PfIsoId() >= (int)(WorkingPoint::VTIGHT);
    if (ID == "POGPfIsoVVTight")  return (int)PfIsoId() >= (int)(WorkingPoint::VVTIGHT);
    if (ID == "POGPuppiIsoLoose") return (int)PuppiIsoId() >= (int)(WorkingPoint::LOOSE);
    if (ID == "POGPuppiIsoMedium")return (int)PuppiIsoId() >= (int)(WorkingPoint::MEDIUM);
    if (ID == "POGPuppiIsoTight") return (int)PuppiIsoId() >= (int)(WorkingPoint::TIGHT);
    if (ID == "POGTkIsoLoose")    return (int)TkIsoId() == 1;
    if (ID == "POGTkIsoTight")    return (int)TkIsoId() == 2;
    cerr << "[Muon::PassID] " << ID << " is not implemented." << endl;
    exit(ENODATA);

    return false;
}





















