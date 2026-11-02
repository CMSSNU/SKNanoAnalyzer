#include <AnalyzerFramework/AnalyzerCore.h>
#include "JetView.h"
#include "TObjArray.h"
#include "TObjString.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

// Objects
Event AnalyzerCore::GetEvent() {
  const Long64_t entry = CurrentEntry();
  if (cachedEventEntry == entry)
    return cachedEvent;

  Event ev;
  ev.SetRunLumiEvent(RunNumber, luminosityBlock, event);
  if (!IsDATA) {
    ev.SetnPileUp(Pileup_nPU);
    ev.SetnTrueInt(Pileup_nTrueInt);
  }
  ev.SetnPVsGood(PV_npvsGood);

  // HLT decisions are resolved only when Event::PassTrigger requests them.
  // This preserves the legacy Event API without activating every path in the
  // era JSON on every event.
  ev.SetTriggerProvider(this);
  ev.SetEra(GetEra());
  RVec<float> MET_pts = {PuppiMET_pt, PuppiMET_ptUnclusteredUp,
                         PuppiMET_ptUnclusteredDown};
  RVec<float> MET_phis = {PuppiMET_phi, PuppiMET_phiUnclusteredUp,
                          PuppiMET_phiUnclusteredDown};
  ev.SetMET(MET_pts, MET_phis);
  ev.setRho(Rho_fixedGridRhoFastjetAll);
  cachedEvent = ev;
  cachedEventEntry = entry;
  return cachedEvent;
}

GenViewCollection AnalyzerCore::GetAllGenViews() {
  const Long64_t entry = CurrentEntry();
  if (cachedGenViewsEntry == entry)
    return cachedGenViews;

  if (IsDATA) {
    cachedGenViews = GenViewCollection();
    cachedGenViewsEntry = entry;
    return cachedGenViews;
  }

  auto storage = std::make_shared<GenSoA>();
  storage->pt.bind(&GenPart_pt);
  storage->eta.bind(&GenPart_eta);
  storage->phi.bind(&GenPart_phi);
  storage->mass.bind(&GenPart_mass);
  storage->pdgId.bind(&GenPart_pdgId);
  storage->status.bind(&GenPart_status);
  storage->motherIdx.bind(&GenPart_genPartIdxMother);
  storage->statusFlags.bind(&GenPart_statusFlags);
  GenViewCollection result(std::move(storage));
  cachedGenViews = result;
  cachedGenViewsEntry = entry;
  return cachedGenViews;
}

GenJetViewCollection AnalyzerCore::GetAllGenJetViews() {
  const Long64_t entry = CurrentEntry();
  if (cachedGenJetViewsEntry == entry)
    return cachedGenJetViews;

  if (IsDATA) {
    cachedGenJetViews = GenJetViewCollection();
    cachedGenJetViewsEntry = entry;
    return cachedGenJetViews;
  }

  auto storage = std::make_shared<GenJetSoA>();
  storage->pt.bind(&GenJet_pt);
  storage->eta.bind(&GenJet_eta);
  storage->phi.bind(&GenJet_phi);
  storage->mass.bind(&GenJet_mass);
  storage->partonFlavour.bind(&GenJet_partonFlavour);
  storage->hadronFlavour.bind(&GenJet_hadronFlavour);
  GenJetViewCollection result(std::move(storage));
  cachedGenJetViews = result;
  cachedGenJetViewsEntry = entry;
  return cachedGenJetViews;
}

LHEViewCollection AnalyzerCore::GetAllLHEViews() {
  if (IsDATA)
    return {};
  auto storage = std::make_shared<LHESoA>();
  storage->pt.bind(&LHEPart_pt);
  storage->eta.bind(&LHEPart_eta);
  storage->phi.bind(&LHEPart_phi);
  storage->mass.bind(&LHEPart_mass);
  storage->status.bind(&LHEPart_status);
  storage->spin.bind(&LHEPart_spin);
  storage->incomingPz.bind(&LHEPart_incomingpz);
  storage->pdgId.bind(&LHEPart_pdgId);
  return LHEViewCollection(std::move(storage));
}

namespace {

float TauPtUnder(const TauView &tau, const TauESSource source,
                 const MyCorrection::variation variation) {
  if (variation == MyCorrection::variation::nom)
    return tau.Pt();
  return tau.ShiftedPt(source, variation == MyCorrection::variation::up);
}

} // namespace

void AnalyzerCore::RefreshTauViewCache() {
  const Long64_t entry = CurrentEntry();
  if (cachedTauViewsEntry == entry)
    return;
  cachedTauViews = TauViewCollection();
  cachedTauESViews.clear();
  cachedTauViewsEntry = entry;
}

TauViewCollection AnalyzerCore::MakeTauViews(const TauView::TauID *id) {
  auto storage = std::make_shared<TauSoA>();
  storage->pt.bind(&Tau_pt);
  storage->eta.bind(&Tau_eta);
  storage->phi.bind(&Tau_phi);
  storage->mass.bind(&Tau_mass);
  storage->dxy.bind(&Tau_dxy);
  storage->dz.bind(&Tau_dz);
  storage->charge.bind(&Tau_charge);
  storage->decayMode.bind(&Tau_decayMode);
  storage->genPartFlav.bind(&Tau_genPartFlav);
  storage->genPartIdx.bind(&Tau_genPartIdx);
  storage->idDecayModeNewDMs.bind(&Tau_idDecayModeNewDMs);
  storage->idDeepTau2018v2p5VSe.bind(&Tau_idDeepTau2018v2p5VSe);
  storage->idDeepTau2018v2p5VSjet.bind(&Tau_idDeepTau2018v2p5VSjet);
  storage->idDeepTau2018v2p5VSmu.bind(&Tau_idDeepTau2018v2p5VSmu);
  storage->rawDeepTau2018v2p5VSe.bind(&Tau_rawDeepTau2018v2p5VSe);
  storage->rawDeepTau2018v2p5VSjet.bind(&Tau_rawDeepTau2018v2p5VSjet);
  storage->rawDeepTau2018v2p5VSmu.bind(&Tau_rawDeepTau2018v2p5VSmu);
  if (id) {
    storage->hasEnergyScaleID = true;
    storage->energyScaleVsJet = id->vsJet;
    storage->energyScaleVsE = id->vsE;
  }

  storage->populateMomentum = [this, target = storage.get()] {
    PopulateTauMomentum(*target);
  };
  return TauViewCollection(std::move(storage));
}

TauViewCollection AnalyzerCore::GetAllTauViews() {
  if (tauESEnabled) {
    if (tauESIds.size() != 1)
      throw SKNano::ConfigError(
          "[AnalyzerCore::GetAllTauViews] the tau energy scale is enabled for "
          "several IDs, so the ID must be passed to GetAllTauViews(id)");
    return GetAllTauViews(tauESIds.front());
  }

  RefreshTauViewCache();
  if (!cachedTauViews.storage())
    cachedTauViews = MakeTauViews(nullptr);
  return cachedTauViews;
}

TauViewCollection AnalyzerCore::GetAllTauViews(const TauView::TauID &id) {
  if (!tauESEnabled) {
    RefreshTauViewCache();
    if (!cachedTauViews.storage())
      cachedTauViews = MakeTauViews(nullptr);
    return cachedTauViews;
  }

  const auto registered =
      std::find_if(tauESIds.begin(), tauESIds.end(), [&](const auto &other) {
        return other.vsJet == id.vsJet && other.vsE == id.vsE;
      });
  if (registered == tauESIds.end())
    throw SKNano::ConfigError(
        std::string("[AnalyzerCore::GetAllTauViews] no tau energy scale is "
                    "registered for vsJet ") +
        ToCorrectionString(id.vsJet) + " / vsE " + ToCorrectionString(id.vsE) +
        "; pass this ID to EnableTauEnergyScale");

  RefreshTauViewCache();
  for (const auto &cached : cachedTauESViews)
    if (cached.vsJet == id.vsJet && cached.vsE == id.vsE)
      return cached.views;
  cachedTauESViews.push_back({id.vsJet, id.vsE, MakeTauViews(&*registered)});
  return cachedTauESViews.back().views;
}

void AnalyzerCore::EnableTauEnergyScale(const TauView::TauID &id) {
  EnableTauEnergyScale(std::vector<TauView::TauID>{id});
}

void AnalyzerCore::EnableTauEnergyScale(
    const std::vector<TauView::TauID> &ids) {
  if (ids.empty())
    throw SKNano::ConfigError(
        "[AnalyzerCore::EnableTauEnergyScale] at least one tau ID is required");
  for (const auto &id : ids)
    if (!id.requireCalibratedDecayMode)
      throw SKNano::ConfigError(
          "[AnalyzerCore::EnableTauEnergyScale] the tau energy scale is "
          "defined only for decay modes 0, 1, 10 and 11, so it cannot be "
          "enabled for an ID with requireCalibratedDecayMode = false");

  tauESEnabled = true;
  tauESIds = ids;
  cachedTauViewsEntry = kInvalidCacheEntry;
  RefreshTauViewCache();
}

void AnalyzerCore::RequireTauESVariation(
    const char *where, const MyCorrection::variation variation) const {
  if (variation != MyCorrection::variation::nom && !tauESEnabled)
    throw SKNano::ConfigError(
        std::string("[AnalyzerCore::") + where +
        "] a tau energy scale variation was requested, but "
        "EnableTauEnergyScale was never called");
}

void AnalyzerCore::PopulateTauMomentum(TauSoA &storage) {
  const std::size_t count = storage.pt.size();
  storage.correctedPt.resize(count);
  storage.correctedMass.resize(count);
  storage.tesPtUp.resize(count);
  storage.tesPtDown.resize(count);
  storage.tesMassUp.resize(count);
  storage.tesMassDown.resize(count);
  storage.esSource.resize(count);

  const bool apply = storage.hasEnergyScaleID && !IsDATA && myCorr;
  TauView::TauID id;
  id.vsJet = storage.energyScaleVsJet;
  id.vsE = storage.energyScaleVsE;
  for (std::size_t index = 0; index < count; ++index) {
    const TauView tau(&storage, index);
    const float pt = tau.MiniAODPt();
    const float mass = tau.MiniAODM();
    float scale = 1.f;
    float scaleUp = 1.f;
    float scaleDown = 1.f;
    TauESSource source = TauESSource::None;

    if (apply && TauView::IsCalibratedDecayMode(tau.DecayMode())) {
      scale = myCorr->GetTauES(id, tau);
      scaleUp = myCorr->GetTauES(id, tau, MyCorrection::variation::up);
      scaleDown = myCorr->GetTauES(id, tau, MyCorrection::variation::down);
      source = TauView::ClassifyESSource(tau.DecayMode(), tau.GenPartFlav(),
                                         tau.Eta());
    }

    storage.correctedPt[index] = pt * scale;
    storage.correctedMass[index] = mass * scale;
    storage.tesPtUp[index] = pt * scaleUp;
    storage.tesPtDown[index] = pt * scaleDown;
    storage.tesMassUp[index] = mass * scaleUp;
    storage.tesMassDown[index] = mass * scaleDown;
    storage.esSource[index] = source;
  }
  storage.momentumReady = true;
}

void AnalyzerCore::PropagateTauSystToMET(
    const TauViewCollection &taus, Particle &met, const TauESSource source,
    const MyCorrection::variation variation) const {
  RequireTauESVariation("PropagateTauSystToMET", variation);
  if (!tauESEnabled || IsDATA)
    return;

  float deltaPx = 0.f;
  float deltaPy = 0.f;
  for (const auto &tau : taus) {
    const float phi = tau.Phi();
    const float shift = TauPtUnder(tau, source, variation) - tau.MiniAODPt();
    deltaPx += shift * std::cos(phi);
    deltaPy += shift * std::sin(phi);
  }

  met.SetXYZM(met.Px() - deltaPx, met.Py() - deltaPy, 0., 0.);
}

std::vector<std::size_t>
AnalyzerCore::SelectTauIndices(const TauViewCollection &taus,
                               const std::vector<std::size_t> &seed_indices,
                               const TauView::TauID &id, const float ptmin,
                               const float fetamax, const TauESSource source,
                               const MyCorrection::variation variation) const {
  RequireTauESVariation("SelectTauIndices", variation);
  if (tauESEnabled) {
    const auto &storage = taus.storage();
    if (!storage || !storage->hasEnergyScaleID)
      throw SKNano::ConfigError(
          "[AnalyzerCore::SelectTauIndices] the tau energy scale is enabled, "
          "but these taus carry no energy scale ID; get them from "
          "GetAllTauViews(id)");
    if (storage->energyScaleVsJet != id.vsJet ||
        storage->energyScaleVsE != id.vsE)
      throw SKNano::ConfigError(
          std::string("[AnalyzerCore::SelectTauIndices] the taus were scaled "
                      "with vsJet ") +
          ToCorrectionString(storage->energyScaleVsJet) + " / vsE " +
          ToCorrectionString(storage->energyScaleVsE) +
          " but are selected with vsJet " + ToCorrectionString(id.vsJet) +
          " / vsE " + ToCorrectionString(id.vsE));
  }

  std::vector<std::size_t> selected;
  selected.reserve(taus.size());
  for (auto i : seed_indices) {
    const auto &tau = taus[i];
    if (!(TauPtUnder(tau, source, variation) > ptmin))
      continue;
    if (!(std::abs(tau.Eta()) < fetamax))
      continue;
    if (!tau.PassID(id))
      continue;
    selected.push_back(i);
  }
  return selected;
}

std::vector<std::size_t>
AnalyzerCore::SelectTauIndices(const TauViewCollection &taus,
                               const TauView::TauID &id, const float ptmin,
                               const float fetamax, const TauESSource source,
                               const MyCorrection::variation variation) const {
  auto seed_indices = AllIndices(taus);
  return SelectTauIndices(taus, seed_indices, id, ptmin, fetamax, source,
                          variation);
}

TauViewCollection
AnalyzerCore::SelectTauViews(const TauViewCollection &taus,
                             std::vector<std::size_t> indices,
                             bool sortByPt, const TauESSource source,
                             const MyCorrection::variation variation) const {
  RequireTauESVariation("SelectTauViews", variation);
  if (sortByPt) {
    std::sort(indices.begin(), indices.end(),
              [&](std::size_t lhs, std::size_t rhs) {
                return TauPtUnder(taus[lhs], source, variation) >
                       TauPtUnder(taus[rhs], source, variation);
              });
  }
  return TauViewCollection(taus.storage(), std::move(indices));
}

GenDressedLeptonViewCollection AnalyzerCore::GetAllGenDressedLeptonViews() {
  if (IsDATA)
    return {};
  auto storage = std::make_shared<GenDressedLeptonSoA>();
  storage->pt.bind(&GenDressedLepton_pt);
  storage->eta.bind(&GenDressedLepton_eta);
  storage->phi.bind(&GenDressedLepton_phi);
  storage->mass.bind(&GenDressedLepton_mass);
  storage->pdgId.bind(&GenDressedLepton_pdgId);
  storage->hasTauAnc.bind(&GenDressedLepton_hasTauAnc);
  return GenDressedLeptonViewCollection(std::move(storage));
}

GenIsolatedPhotonViewCollection AnalyzerCore::GetAllGenIsolatedPhotonViews() {
  if (IsDATA)
    return {};
  auto storage = std::make_shared<GenIsolatedPhotonSoA>();
  storage->pt.bind(&GenIsolatedPhoton_pt);
  storage->eta.bind(&GenIsolatedPhoton_eta);
  storage->phi.bind(&GenIsolatedPhoton_phi);
  storage->mass.bind(&GenIsolatedPhoton_mass);
  return GenIsolatedPhotonViewCollection(std::move(storage));
}

GenVisTauViewCollection AnalyzerCore::GetAllGenVisTauViews() {
  if (IsDATA)
    return {};
  auto storage = std::make_shared<GenVisTauSoA>();
  storage->pt.bind(&GenVisTau_pt);
  storage->eta.bind(&GenVisTau_eta);
  storage->phi.bind(&GenVisTau_phi);
  storage->mass.bind(&GenVisTau_mass);
  storage->charge.bind(&GenVisTau_charge);
  storage->genPartIdxMother.bind(&GenVisTau_genPartIdxMother);
  storage->status.bind(&GenVisTau_status);
  return GenVisTauViewCollection(std::move(storage));
}

SVViewCollection AnalyzerCore::GetAllSVViews() {
  auto storage = std::make_shared<SVSoA>();
  storage->pt.bind(&SV_pt);
  storage->eta.bind(&SV_eta);
  storage->phi.bind(&SV_phi);
  storage->mass.bind(&SV_mass);
  storage->charge.bind(&SV_charge);
  storage->chi2.bind(&SV_chi2);
  storage->dlen.bind(&SV_dlen);
  storage->dlenSig.bind(&SV_dlenSig);
  storage->dxy.bind(&SV_dxy);
  storage->dxySig.bind(&SV_dxySig);
  storage->ndof.bind(&SV_ndof);
  storage->ntracks.bind(&SV_ntracks);
  storage->pAngle.bind(&SV_pAngle);
  storage->x.bind(&SV_x);
  storage->y.bind(&SV_y);
  storage->z.bind(&SV_z);
  return SVViewCollection(std::move(storage));
}

MuonViewCollection AnalyzerCore::GetAllMuonViews() {
  const Long64_t entry = CurrentEntry();
  if (cachedMuonViewsEntry == entry)
    return cachedMuonViews;

  auto storage = std::make_shared<MuonSoA>();
  storage->pt.bind(&Muon_pt);
  storage->eta.bind(&Muon_eta);
  storage->phi.bind(&Muon_phi);
  storage->mass.bind(&Muon_mass);
  storage->charge.bind(&Muon_charge);
  storage->tkRelIso.bind(&Muon_tkRelIso);
  storage->pfRelIso03.bind(&Muon_pfRelIso03_all);
  storage->pfRelIso04.bind(&Muon_pfRelIso04_all);
  storage->miniPFRelIsoAll.bind(&Muon_miniPFRelIso_all);
  storage->dxy.bind(&Muon_dxy);
  storage->dxyErr.bind(&Muon_dxyErr);
  storage->dz.bind(&Muon_dz);
  storage->dzErr.bind(&Muon_dzErr);
  storage->ip3d.bind(&Muon_ip3d);
  storage->sip3d.bind(&Muon_sip3d);
  storage->highPtId.bind(&Muon_highPtId);
  storage->looseId.bind(&Muon_looseId);
  storage->mediumId.bind(&Muon_mediumId);
  storage->mediumPromptId.bind(&Muon_mediumPromptId);
  storage->tightId.bind(&Muon_tightId);
  storage->softId.bind(&Muon_softId);
  storage->softMvaId.bind(&Muon_softMvaId);
  storage->triggerLooseId.bind(&Muon_triggerIdLoose);
  storage->miniIsoId.bind(&Muon_miniIsoId);
  storage->multiIsoId.bind(&Muon_multiIsoId);
  storage->mvaMuId.bind(&Muon_mvaMuID_WP);
  storage->pfIsoId.bind(&Muon_pfIsoId);
  storage->puppiIsoId.bind(&Muon_puppiIsoId);
  storage->tkIsoId.bind(&Muon_tkIsoId);
  storage->nTrackerLayers.bind(&Muon_nTrackerLayers);
  storage->softMva.bind(&Muon_softMva);
  storage->softMvaRun3.bind(&Muon_softMvaRun3);
  storage->mvaLowPt.bind(&Muon_mvaLowPt);
  storage->mvaPrompt.bind(&Muon_promptMVA);
  storage->genPartFlav.bind(&Muon_genPartFlav);
  storage->genPartIdx.bind(&Muon_genPartIdx);
  storage->jetIdx.bind(&Muon_jetIdx);

  storage->populateMomentum =
      [this, target = storage.get()] { PopulateMuonMomentum(*target); };

  MuonViewCollection result(std::move(storage));
  cachedMuonViews = result;
  cachedMuonViewsEntry = entry;
  return cachedMuonViews;
}

void AnalyzerCore::PopulateMuonMomentum(MuonSoA &storage) {
  const std::size_t count = storage.rawSize();
  storage.correctedPt.assign(count, 0.f);
  storage.miniAODPt.assign(count, 0.f);
  storage.momentumScaleUp.assign(count, 0.f);
  storage.momentumScaleDown.assign(count, 0.f);

  GenViewCollection truth;
  bool truthLoaded = false;
  for (std::size_t index = 0; index < count; ++index) {
    const float pt = storage.pt[index];
    const float eta = storage.eta[index];
    const float phi = storage.phi[index];
    const int charge = storage.charge[index];
    float matchedPt = 0.f;

    if (!IsDATA) {
      const short genIndex = storage.genPartIdx[index];
      if (genIndex >= 0 &&
          static_cast<std::size_t>(genIndex) < GenPart_pt.size() &&
          std::abs(GenPart_pdgId[genIndex]) == 13 &&
          GenPart_status[genIndex] == 1 &&
          GenPart_genPartIdxMother[genIndex] >= 0) {
        matchedPt = GenPart_pt[genIndex];
      } else {
        if (!truthLoaded) {
          truth = GetAllGenViews();
          truthLoaded = true;
        }
        TLorentzVector reco;
        reco.SetPtEtaPhiM(pt, eta, phi, storage.mass[index]);
        float bestDistance = std::numeric_limits<float>::max();
        for (const auto gen : truth) {
          if (gen.Status() != 1 || std::abs(gen.PdgId()) != 13 ||
              gen.MotherIndex() < 0 || gen.P4().DeltaR(reco) > 0.2)
            continue;
          const float distance =
              std::pow(gen.P4().DeltaR(reco) / 0.005f, 2) +
              std::pow((pt / gen.Pt() - 1.f) / 0.02f, 2);
          if (distance < bestDistance) {
            bestDistance = distance;
            matchedPt = gen.Pt();
          }
        }
      }
    }

    MyCorrection::MuonScaleAndError scale{1.f, 0.f};
    if (myCorr) {
      const int trackerLayers =
          IsDATA ? 0 : static_cast<int>(storage.nTrackerLayers[index]);
      scale = myCorr->GetMuonScaleAndError(charge, pt, eta, phi,
                                            trackerLayers, matchedPt);
    }
    storage.miniAODPt[index] = pt;
    storage.momentumScaleUp[index] = pt * (scale.scale + scale.error);
    storage.momentumScaleDown[index] = pt * (scale.scale - scale.error);
    storage.correctedPt[index] = pt * scale.scale;
  }
  storage.momentumReady = true;
}

MuonViewCollection AnalyzerCore::SelectMuonViews(
    const MuonViewCollection &muons, std::vector<std::size_t> indices,
    bool sortByPt) const {
  if (sortByPt) {
    std::sort(indices.begin(), indices.end(), [&](std::size_t lhs,
                                                  std::size_t rhs) {
      return muons[lhs].Pt() > muons[rhs].Pt();
    });
  }
  return MuonViewCollection(muons.storage(), std::move(indices));
}

std::vector<std::size_t>
AnalyzerCore::SelectMuonIndices(const MuonViewCollection &muons,
                                const std::vector<std::size_t> &seed_indices,
                                const MuonView::MuonID ID, const float ptmin,
                                const float fetamax) const {
  std::vector<std::size_t> selected;
  selected.reserve(muons.size());
  for (auto i : seed_indices) {
    const auto &mu = muons[i];
    if (!(mu.Pt() > ptmin))
      continue;
    if (!(std::abs(mu.Eta()) < fetamax))
      continue;
    if (!mu.PassID(ID))
      continue;
    selected.push_back(i);
  }
  return selected;
}

std::vector<std::size_t>
AnalyzerCore::SelectMuonIndices(const MuonViewCollection &muons,
                                const MuonView::MuonID ID, const float ptmin,
                                const float fetamax) const {
  auto seed_indices = AllIndices(muons);
  return SelectMuonIndices(muons, seed_indices, ID, ptmin, fetamax);
}

ElectronViewCollection AnalyzerCore::GetAllElectronViews() {
  const Long64_t entry = CurrentEntry();
  if (cachedElectronViewsEntry == entry)
    return cachedElectronViews;

  auto storage = std::make_shared<ElectronSoA>();
  storage->pt.bind(&Electron_pt);
  storage->eta.bind(&Electron_eta);
  storage->phi.bind(&Electron_phi);
  storage->mass.bind(&Electron_mass);
  storage->charge.bind(&Electron_charge);
  storage->pfRelIso03.bind(&Electron_pfRelIso03_all);
  storage->miniPFRelIso.bind(&Electron_miniPFRelIso_all);
  storage->dxy.bind(&Electron_dxy);
  storage->dxyErr.bind(&Electron_dxyErr);
  storage->dz.bind(&Electron_dz);
  storage->dzErr.bind(&Electron_dzErr);
  storage->ip3d.bind(&Electron_ip3d);
  storage->sip3d.bind(&Electron_sip3d);
  storage->convVeto.bind(&Electron_convVeto);
  storage->lostHits.bind(&Electron_lostHits);
  storage->seedGain.bind(&Electron_seedGain);
  storage->tightCharge.bind(&Electron_tightCharge);
  storage->sieie.bind(&Electron_sieie);
  storage->hoe.bind(&Electron_hoe);
  storage->eInvMinusPInv.bind(&Electron_eInvMinusPInv);
  storage->dr03EcalRecHitSumEt.bind(&Electron_dr03EcalRecHitSumEt);
  storage->dr03HcalDepth1TowerSumEt.bind(&Electron_dr03HcalDepth1TowerSumEt);
  storage->dr03TkSumPt.bind(&Electron_dr03TkSumPt);
  storage->dr03TkSumPtHEEP.bind(&Electron_dr03TkSumPtHEEP);
  storage->r9.bind(&Electron_r9);
  storage->energyErr.bind(&Electron_energyErr);
  storage->cutBasedHEEP.bind(&Electron_cutBased_HEEP);
  storage->promptMVA.bind(&Electron_promptMVA);
  storage->mvaIsoWP80.bind(&Electron_mvaIso_WP80);
  storage->mvaIsoWP90.bind(&Electron_mvaIso_WP90);
  storage->mvaNoIsoWP80.bind(&Electron_mvaNoIso_WP80);
  storage->mvaNoIsoWP90.bind(&Electron_mvaNoIso_WP90);
  storage->mvaIso.bind(&Electron_mvaIso);
  storage->mvaNoIso.bind(&Electron_mvaNoIso);
  storage->cutBased.bind(&Electron_cutBased);
  storage->genPartFlav.bind(&Electron_genPartFlav);
  storage->genPartIdx.bind(&Electron_genPartIdx);
  storage->jetIdx.bind(&Electron_jetIdx);
  storage->scEta.bind(&Electron_superclusterEta);
  storage->deltaEtaSC.bind(&Electron_deltaEtaSC);
  storage->readRho = [this] {
    return static_cast<float>(Rho_fixedGridRhoFastjetAll.get());
  };

  ElectronViewCollection result(std::move(storage));
  cachedElectronViews = result;
  cachedElectronViewsEntry = entry;
  return cachedElectronViews;
}

std::vector<std::size_t> AnalyzerCore::SelectElectronIndices(
    const ElectronViewCollection &electrons,
    const std::vector<size_t> &seed_indices, const ElectronView::ElectronID ID,
    const float ptmin, const float fetamax, bool vetoHEM) const {
  std::vector<std::size_t> selected;
  selected.reserve(electrons.size());
  for (auto i : seed_indices) {
    const auto &electron = electrons[i];
    if (!(electron.Pt() > ptmin))
      continue;
    if (!(std::abs(electron.Eta()) < fetamax))
      continue;
    bool passID = true;
    passID = electron.PassID(ID);
    if (!passID)
      continue;
    if (vetoHEM && IsHEMElectron(electron))
      continue;
    selected.push_back(i);
  }
  return selected;
}

std::vector<std::size_t> AnalyzerCore::SelectElectronIndices(
    const ElectronViewCollection &electrons, const ElectronView::ElectronID ID,
    const float ptmin, const float fetamax, bool vetoHEM) const {
  auto seed_indices = AllIndices(electrons);
  return SelectElectronIndices(electrons, seed_indices, ID, ptmin, fetamax,
                               vetoHEM);
}

ElectronViewCollection AnalyzerCore::SelectElectronViews(
    const ElectronViewCollection &electrons,
    std::vector<std::size_t> indices, bool sortByPt) const {
  if (sortByPt) {
    std::sort(indices.begin(), indices.end(), [&](std::size_t lhs,
                                                  std::size_t rhs) {
      return electrons[lhs].Pt() > electrons[rhs].Pt();
    });
  }
  return ElectronViewCollection(electrons.storage(), std::move(indices));
}

bool AnalyzerCore::IsHEMElectron(const ElectronView &electron) const {
  if (DataYear != 2018)
    return false;

  return electron.Eta() < -1.25 && electron.Phi() > -1.62 &&
         electron.Phi() < -0.82;
}

PhotonViewCollection AnalyzerCore::GetAllPhotons() {
  auto storage = std::make_shared<PhotonSoA>();
  storage->pt.bind(&Photon_pt);
  storage->eta.bind(&Photon_eta);
  storage->phi.bind(&Photon_phi);
  storage->sieie.bind(&Photon_sieie);
  storage->hoe.bind(&Photon_hoe);
  storage->energyRaw.bind(&Photon_energyRaw);
  storage->pixelSeed.bind(&Photon_pixelSeed);
  storage->isScEtaEB.bind(&Photon_isScEtaEB);
  storage->isScEtaEE.bind(&Photon_isScEtaEE);
  storage->mvaIdWP80.bind(&Photon_mvaID_WP80);
  storage->mvaIdWP90.bind(&Photon_mvaID_WP90);
  storage->cutBased.bind(&Photon_cutBased);
  storage->mvaId.bind(&Photon_mvaID);
  return PhotonViewCollection(std::move(storage));
}

PhotonViewCollection AnalyzerCore::GetPhotons(TString id, double ptmin,
                                               double fetamax) {
  auto photons = GetAllPhotons();
  std::vector<std::size_t> selected;
  selected.reserve(photons.size());
  for (std::size_t i = 0; i < photons.size(); ++i) {
    const auto photon = photons[i];
    if (!(photon.Pt() > ptmin)) {
      continue;
    }
    if (!(fabs(photon.scEta()) < fetamax)) {
      continue;
    }
    if (!photon.PassID(id)) {
      continue;
    }
    selected.push_back(photons.rawIndex(i));
  }
  return PhotonViewCollection(photons.storage(), std::move(selected));
}

FatJetViewCollection AnalyzerCore::GetAllFatJets() {
  auto storage = std::make_shared<FatJetSoA>();
  storage->pt.bind(&FatJet_pt);
  storage->eta.bind(&FatJet_eta);
  storage->phi.bind(&FatJet_phi);
  storage->mass.bind(&FatJet_mass);
  storage->area.bind(&FatJet_area);
  storage->rawFactor.bind(&FatJet_rawFactor);
  storage->chEmEF.bind(&FatJet_chEmEF);
  storage->chHEF.bind(&FatJet_chHEF);
  storage->hfEmEF.bind(&FatJet_hfEmEF);
  storage->hfHEF.bind(&FatJet_hfHEF);
  storage->muEF.bind(&FatJet_muEF);
  storage->neEmEF.bind(&FatJet_neEmEF);
  storage->neHEF.bind(&FatJet_neHEF);
  storage->chMultiplicity.bind(&FatJet_chMultiplicity);
  storage->neMultiplicity.bind(&FatJet_neMultiplicity);
  storage->nConstituents.bind(&FatJet_nConstituents);
  storage->genJetAK8Idx.bind(&FatJet_genJetAK8Idx);
  storage->subJetIdx1.bind(&FatJet_subJetIdx1);
  storage->subJetIdx2.bind(&FatJet_subJetIdx2);
  storage->hadronFlavour.bind(&FatJet_hadronFlavour);
  storage->softDropMass.bind(&FatJet_msoftdrop);
  storage->tau1.bind(&FatJet_tau1);
  storage->tau2.bind(&FatJet_tau2);
  storage->tau3.bind(&FatJet_tau3);
  storage->n2b1.bind(&FatJet_n2b1);
  storage->n3b1.bind(&FatJet_n3b1);
  storage->lsf3.bind(&FatJet_lsf3);

  using Tagger = JetTagging::FatJetTagger;
  using Score = JetTagging::FatJetTaggerScoreType;
  storage->score(Tagger::ParticleNetWithMass, Score::H4qvsQCD).bind(&FatJet_particleNetWithMass_H4qvsQCD);
  storage->score(Tagger::ParticleNetWithMass, Score::HbbvsQCD).bind(&FatJet_particleNetWithMass_HbbvsQCD);
  storage->score(Tagger::ParticleNetWithMass, Score::HccvsQCD).bind(&FatJet_particleNetWithMass_HccvsQCD);
  storage->score(Tagger::ParticleNetWithMass, Score::QCD).bind(&FatJet_particleNetWithMass_QCD);
  storage->score(Tagger::ParticleNetWithMass, Score::TopvsQCD).bind(&FatJet_particleNetWithMass_TvsQCD);
  storage->score(Tagger::ParticleNetWithMass, Score::WvsQCD).bind(&FatJet_particleNetWithMass_WvsQCD);
  storage->score(Tagger::ParticleNetWithMass, Score::ZvsQCD).bind(&FatJet_particleNetWithMass_ZvsQCD);

  storage->score(Tagger::ParticleNet, Score::QCD).bind(&FatJet_particleNet_QCD);
  storage->score(Tagger::ParticleNet, Score::QCD0HF).bind(&FatJet_particleNet_QCD0HF);
  storage->score(Tagger::ParticleNet, Score::QCD1HF).bind(&FatJet_particleNet_QCD1HF);
  storage->score(Tagger::ParticleNet, Score::QCD2HF).bind(&FatJet_particleNet_QCD2HF);
  storage->score(Tagger::ParticleNet, Score::WVsQCD).bind(&FatJet_particleNet_WVsQCD);
  storage->score(Tagger::ParticleNet, Score::XbbVsQCD).bind(&FatJet_particleNet_XbbVsQCD);
  storage->score(Tagger::ParticleNet, Score::XccVsQCD).bind(&FatJet_particleNet_XccVsQCD);
  storage->score(Tagger::ParticleNet, Score::XggVsQCD).bind(&FatJet_particleNet_XggVsQCD);
  storage->score(Tagger::ParticleNet, Score::XqqVsQCD).bind(&FatJet_particleNet_XqqVsQCD);
  storage->score(Tagger::ParticleNet, Score::XteVsQCD).bind(&FatJet_particleNet_XteVsQCD);
  storage->score(Tagger::ParticleNet, Score::XtmVsQCD).bind(&FatJet_particleNet_XtmVsQCD);
  storage->score(Tagger::ParticleNet, Score::XttVsQCD).bind(&FatJet_particleNet_XttVsQCD);
  storage->score(Tagger::ParticleNet, Score::MassCorr).bind(&FatJet_particleNet_massCorr);

  storage->score(Tagger::ParT, Score::QCD).bind(&FatJet_globalParT3_QCD);
  storage->score(Tagger::ParT, Score::TopbWev).bind(&FatJet_globalParT3_TopbWev);
  storage->score(Tagger::ParT, Score::TopbWmv).bind(&FatJet_globalParT3_TopbWmv);
  storage->score(Tagger::ParT, Score::TopbWq).bind(&FatJet_globalParT3_TopbWq);
  storage->score(Tagger::ParT, Score::TopbWqq).bind(&FatJet_globalParT3_TopbWqq);
  storage->score(Tagger::ParT, Score::TopbWtauhv).bind(&FatJet_globalParT3_TopbWtauhv);
  storage->score(Tagger::ParT, Score::WvsQCD).bind(&FatJet_globalParT3_WvsQCD);
  storage->score(Tagger::ParT, Score::XWW3q).bind(&FatJet_globalParT3_XWW3q);
  storage->score(Tagger::ParT, Score::XWW4q).bind(&FatJet_globalParT3_XWW4q);
  storage->score(Tagger::ParT, Score::XWWqqev).bind(&FatJet_globalParT3_XWWqqev);
  storage->score(Tagger::ParT, Score::XWWqqmv).bind(&FatJet_globalParT3_XWWqqmv);
  storage->score(Tagger::ParT, Score::Xbb).bind(&FatJet_globalParT3_Xbb);
  storage->score(Tagger::ParT, Score::Xcc).bind(&FatJet_globalParT3_Xcc);
  storage->score(Tagger::ParT, Score::Xcs).bind(&FatJet_globalParT3_Xcs);
  storage->score(Tagger::ParT, Score::Xqq).bind(&FatJet_globalParT3_Xqq);
  storage->score(Tagger::ParT, Score::Xtauhtaue).bind(&FatJet_globalParT3_Xtauhtaue);
  storage->score(Tagger::ParT, Score::Xtauhtauh).bind(&FatJet_globalParT3_Xtauhtauh);
  storage->score(Tagger::ParT, Score::Xtauhtaum).bind(&FatJet_globalParT3_Xtauhtaum);
  storage->score(Tagger::ParT, Score::MassCorrGeneric).bind(&FatJet_globalParT3_massCorrGeneric);
  storage->score(Tagger::ParT, Score::MassCorrX2p).bind(&FatJet_globalParT3_massCorrX2p);
  storage->score(Tagger::ParTWithMass, Score::TopvsQCD).bind(&FatJet_globalParT3_withMassTopvsQCD);
  storage->score(Tagger::ParTWithMass, Score::WvsQCD).bind(&FatJet_globalParT3_withMassWvsQCD);
  storage->score(Tagger::ParTWithMass, Score::ZvsQCD).bind(&FatJet_globalParT3_withMassZvsQCD);

  storage->constituentJetIdx.bind(&FatJetPFCand_jetIdx);
  storage->constituentPFCandIdx.bind(&FatJetPFCand_pfCandIdx);
  storage->pfCandPt.bind(&PFCand_pt);
  storage->pfCandEta.bind(&PFCand_eta);
  storage->pfCandPhi.bind(&PFCand_phi);
  storage->pfCandMass.bind(&PFCand_mass);
  storage->pfCandPdgId.bind(&PFCand_pdgId);
  storage->pfCandPuppiWeight.bind(&PFCand_puppiWeight);
  return FatJetViewCollection(std::move(storage));

}

TrigObjViewCollection AnalyzerCore::GetAllTrigObjViews() {
  auto storage = std::make_shared<TrigObjSoA>();
  storage->run = Run;
  storage->pt.bind(&TrigObj_pt);
  storage->eta.bind(&TrigObj_eta);
  storage->phi.bind(&TrigObj_phi);
  storage->id.bind(&TrigObj_id);
  storage->filterBits.bind(&TrigObj_filterBits);
  storage->l1charge.bind(&TrigObj_l1charge);
  storage->l1iso.bind(&TrigObj_l1iso);
  storage->l1pt.bind(&TrigObj_l1pt);
  storage->l1pt2.bind(&TrigObj_l1pt_2);
  storage->l2pt.bind(&TrigObj_l2pt);
  return TrigObjViewCollection(std::move(storage));
}

std::vector<std::size_t> AnalyzerCore::SelectTrigObjIndices(
    const TrigObjViewCollection &trigobjs,
    const std::vector<std::size_t> &seed_indices, const int id,
    const float ptmin, const float fetamax) const {
  std::vector<std::size_t> selected;
  selected.reserve(seed_indices.size());
  for (const auto idx : seed_indices) {
    const auto &obj = trigobjs[idx];
    if (obj.Id() != id)
      continue;
    if (!(obj.Pt() > ptmin))
      continue;
    if (!(std::abs(obj.Eta()) < fetamax))
      continue;
    selected.push_back(idx);
  }
  return selected;
}

std::vector<std::size_t> AnalyzerCore::SelectTrigObjIndices(
    const TrigObjViewCollection &trigobjs, const int id, const float ptmin,
    const float fetamax) const {
  size_t n = trigobjs.size();
  std::vector<std::size_t> seed_indices(n);
  std::iota(seed_indices.begin(), seed_indices.end(), 0);
  return SelectTrigObjIndices(trigobjs, seed_indices, id, ptmin, fetamax);
}
