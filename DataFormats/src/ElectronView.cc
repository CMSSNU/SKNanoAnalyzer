#include "ElectronView.h"

#include <algorithm>

ElectronView::EtaRegion ElectronView::etaRegion() const {
    const float absScEta = std::fabs(ScEta());
    if (absScEta < 0.8f)
        return EtaRegion::IB;
    if (absScEta < 1.444f)
        return EtaRegion::OB;
    if (absScEta < 1.566f)
        return EtaRegion::GAP;
    return EtaRegion::EC;
}

bool ElectronView::PassID(ElectronID id) const {
    switch (id) {
    case ElectronID::NOCUT:
        return true;
    case ElectronID::POG_VETO:
        return PassID("POGVeto");
    case ElectronID::POG_LOOSE:
        return PassID("POGLoose");
    case ElectronID::POG_MEDIUM:
        return PassID("POGMedium");
    case ElectronID::POG_TIGHT:
        return PassID("POGTight");
    case ElectronID::POG_HEEP:
        return PassID("POGHEEP");
    case ElectronID::POG_MVAISO_WP80:
        return PassID("POGMVAIsoWP80");
    case ElectronID::POG_MVAISO_WP90:
        return PassID("POGMVAIsoWP90");
    case ElectronID::POG_MVANOISO_WP80:
        return PassID("POGMVANoIsoWP80");
    case ElectronID::POG_MVANOISO_WP90:
        return PassID("POGMVANoIsoWP90");
    case ElectronID::POG_PROMPTMVA_TIGHT:
        return PassID("POGPromptMVA_Tight");
    case ElectronID::POG_PROMPTMVA_MEDIUM:
        return PassID("POGPromptMVA_Medium");
    default:
        throw std::runtime_error("[ElectronView::PassID] ElectronID not implemented.");
    }
    return false;
}

bool ElectronView::PassID(const TString &id) const {
    if (etaRegion() == EtaRegion::GAP)
        return false;

    if (id == "" || id == "NOCUT")
        return true;
    if (id == "POGVeto")
        return static_cast<int>(CutBased()) >= static_cast<int>(CutBasedWP::VETO);
    if (id == "POGLoose")
        return static_cast<int>(CutBased()) >= static_cast<int>(CutBasedWP::LOOSE);
    if (id == "POGMedium")
        return static_cast<int>(CutBased()) >= static_cast<int>(CutBasedWP::MEDIUM);
    if (id == "POGTight")
        return static_cast<int>(CutBased()) >= static_cast<int>(CutBasedWP::TIGHT);
    if (id == "POGHEEP")
        return isCutBasedHEEP();
    if (id == "POGMVAIsoWP80")
        return isMVAIsoWP80();
    if (id == "POGMVAIsoWP90")
        return isMVAIsoWP90();
    if (id == "POGMVANoIsoWP80")
        return isMVANoIsoWP80();
    if (id == "POGMVANoIsoWP90")
        return isMVANoIsoWP90();
    if (id == "HcToWATight")
        return Pass_HcToWATight();
    if (id == "HcToWALooseRun2")
        return Pass_HcToWALooseRun2();
    if (id == "HcToWALooseRun3")
        return Pass_HcToWALooseRun3();
    if (id == "POGPromptMVA_Tight")
        return PromptMVA() > 0.9f;
    if (id == "POGPromptMVA_Medium")
        return PromptMVA() > 0.8f;
    throw std::runtime_error("[ElectronView::PassID] " + std::string(id.Data()) + " is not implemented.");

    return false;
}

// HLT emulation: the online relative isolation is taken against the
// uncorrected energy, so the cuts scale with MiniAODPt(), not Pt().
bool ElectronView::Pass_CaloIdL_TrackIdL_IsoVL() const {
    const bool ApplyEA = true;

    if (etaRegion() == EtaRegion::GAP) {
        return false;
    } else if (etaRegion() == EtaRegion::IB || etaRegion() == EtaRegion::OB) {
        if (!(sieie() < 0.013f))
            return false;
        if (!(std::fabs(deltaEtaInSC()) < 0.01f))
            return false;
        if (!(std::fabs(deltaPhiInSeed()) < 0.07f))
            return false;
        if (!(hoe() < 0.13f))
            return false;
        if (!(std::max(0.f, ecalPFClusterIso() - Rho() * (ApplyEA ? 0.16544f : 0.f)) < 0.5f * MiniAODPt()))
            return false;
        if (!(std::max(0.f, hcalPFClusterIso() - Rho() * (ApplyEA ? 0.05956f : 0.f)) < 0.3f * MiniAODPt()))
            return false;
        if (!(dr03TkSumPt() < 0.2f * MiniAODPt()))
            return false;
        return true;
    } else {
        if (!(sieie() < 0.035f))
            return false;
        if (!(std::fabs(deltaEtaInSC()) < 0.015f))
            return false;
        if (!(std::fabs(deltaPhiInSeed()) < 0.1f))
            return false;
        if (!(hoe() < 0.13f))
            return false;
        if (!(std::max(0.f, ecalPFClusterIso() - Rho() * (ApplyEA ? 0.13212f : 0.f)) < 0.5f * MiniAODPt()))
            return false;
        if (!(std::max(0.f, hcalPFClusterIso() - Rho() * (ApplyEA ? 0.13052f : 0.f)) < 0.3f * MiniAODPt()))
            return false;
        if (!(dr03TkSumPt() < 0.2f * MiniAODPt()))
            return false;
        return true;
    }
}

bool ElectronView::Pass_HcToWABaseline() const {
    if (!Pass_CaloIdL_TrackIdL_IsoVL())
        return false;
    if (!ConvVeto())
        return false;
    if (!(LostHits() < 2))
        return false;
    if (!(std::fabs(dZ()) < 0.1f))
        return false;
    return true;
}

bool ElectronView::Pass_HcToWATight() const {
    if (!Pass_HcToWABaseline())
        return false;
    if (!isMVANoIsoWP90())
        return false;
    if (!(SIP3D() < 4.f))
        return false;
    if (!(MiniPFRelIso() < 0.1f))
        return false;
    return true;
}

bool ElectronView::Pass_HcToWALooseRun2() const {
    if (!Pass_HcToWABaseline())
        return false;
    if (!(SIP3D() < 8.f))
        return false;
    if (!(MiniPFRelIso() < 0.4f))
        return false;
    const float cutIB = 0.985f;
    const float cutOB = 0.96f;
    const float cutEC = 0.85f;
    bool passMVAIDNoIsoCut = false;
    switch (etaRegion()) {
    case EtaRegion::IB:
        if (!(MvaNoIso() > cutIB))
            passMVAIDNoIsoCut = true;
        break;
    case EtaRegion::OB:
        if (!(MvaNoIso() > cutOB))
            passMVAIDNoIsoCut = true;
        break;
    case EtaRegion::EC:
        if (!(MvaNoIso() > cutEC))
            passMVAIDNoIsoCut = true;
        break;
    default:
        break;
    }
    if (!(isMVANoIsoWP90() || passMVAIDNoIsoCut))
        return false;
    return true;
}

bool ElectronView::Pass_HcToWALooseRun3() const {
    if (!Pass_HcToWABaseline())
        return false;
    if (!(SIP3D() < 8.f))
        return false;
    if (!(MiniPFRelIso() < 0.4f))
        return false;
    const float cutIB = 0.5f;
    const float cutOB = -0.8f;
    const float cutEC = -0.5f;
    bool passMVAIDNoIsoCut = false;
    switch (etaRegion()) {
    case EtaRegion::IB:
        if (!(MvaNoIso() > cutIB))
            passMVAIDNoIsoCut = true;
        break;
    case EtaRegion::OB:
        if (!(MvaNoIso() > cutOB))
            passMVAIDNoIsoCut = true;
        break;
    case EtaRegion::EC:
        if (!(MvaNoIso() > cutEC))
            passMVAIDNoIsoCut = true;
        break;
    default:
        break;
    }
    if (!(isMVANoIsoWP90() || passMVAIDNoIsoCut))
        return false;
    return true;
}
