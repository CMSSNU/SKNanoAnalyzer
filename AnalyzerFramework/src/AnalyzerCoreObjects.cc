#include <AnalyzerFramework/AnalyzerCore.h>
#include "JetView.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TRandom3.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {
// Same seed for the nominal smearing and its variations, so they share the
// Gaussian pull and the variation stays a coherent shift. Depends only on the
// event and the muon direction, so it reproduces across passes.
inline unsigned int MuonSmearSeed(const unsigned long long eventNumber,
                                  const float eta, const float phi) {
  const unsigned int etaBits =
      static_cast<unsigned int>(std::fabs(eta) * 1000.f);
  const unsigned int phiBits =
      static_cast<unsigned int>(std::fabs(phi) * 1000.f);
  return static_cast<unsigned int>(eventNumber & 0xFFFFFFFFull) ^
         (etaBits << 8) ^ phiBits;
}
} // namespace

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

TauViewCollection AnalyzerCore::GetAllTauViews() {
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
  return TauViewCollection(std::move(storage));
}

std::vector<std::size_t>
AnalyzerCore::SelectTauIndices(const TauViewCollection &taus,
                               const std::vector<std::size_t> &seed_indices,
                               const TauView::TauID &id, const float ptmin,
                               const float fetamax) const {
  std::vector<std::size_t> selected;
  selected.reserve(taus.size());
  for (auto i : seed_indices) {
    const auto &tau = taus[i];
    if (!(tau.Pt() > ptmin))
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
                               const float fetamax) const {
  auto seed_indices = AllIndices(taus);
  return SelectTauIndices(taus, seed_indices, id, ptmin, fetamax);
}

TauViewCollection
AnalyzerCore::SelectTauViews(const TauViewCollection &taus,
                             std::vector<std::size_t> indices,
                             bool sortByPt) const {
  if (sortByPt) {
    std::sort(indices.begin(), indices.end(),
              [&](std::size_t lhs, std::size_t rhs) {
                return taus[lhs].Pt() > taus[rhs].Pt();
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
  storage->tunepRelPt.bind(&Muon_tunepRelPt);
  storage->softMva.bind(&Muon_softMva);
  storage->softMvaRun3.bind(&Muon_softMvaRun3);
  storage->mvaLowPt.bind(&Muon_mvaLowPt);
  storage->mvaPrompt.bind(&Muon_promptMVA);
  storage->genPartFlav.bind(&Muon_genPartFlav);
  storage->genPartIdx.bind(&Muon_genPartIdx);
  storage->jetIdx.bind(&Muon_jetIdx);

  storage->populateMomentum =
      [this, target = storage.get()] { PopulateMuonMomentum(*target); };
  storage->populateMomentumVariations = [this, target = storage.get()] {
    PopulateMuonMomentumVariations(*target);
  };
  storage->populateHighPt =
      [this, target = storage.get()] { PopulateMuonHighPt(*target); };
  storage->populateHighPtVariations = [this, target = storage.get()] {
    PopulateMuonHighPtVariations(*target);
  };

  MuonViewCollection result(std::move(storage));
  cachedMuonViews = result;
  cachedMuonViewsEntry = entry;
  return cachedMuonViews;
}

// Rochester corrects toward the matched generator muon; the POG recipe does
// not need the match, so only that path pays for it.
float AnalyzerCore::MuonRochesterMatchedGenPt(const MuonSoA &storage,
                                              const std::size_t index,
                                              GenViewCollection &truth,
                                              bool &truthLoaded) {
  const short genIndex = storage.genPartIdx[index];
  if (genIndex >= 0 && static_cast<std::size_t>(genIndex) < GenPart_pt.size() &&
      std::abs(GenPart_pdgId[genIndex]) == 13 &&
      GenPart_status[genIndex] == 1 &&
      GenPart_genPartIdxMother[genIndex] >= 0) {
    return GenPart_pt[genIndex];
  }
  if (!truthLoaded) {
    truth = GetAllGenViews();
    truthLoaded = true;
  }
  TLorentzVector reco;
  reco.SetPtEtaPhiM(storage.pt[index], storage.eta[index], storage.phi[index],
                    storage.mass[index]);
  float matchedPt = 0.f;
  float bestDistance = std::numeric_limits<float>::max();
  for (const auto gen : truth) {
    if (gen.Status() != 1 || std::abs(gen.PdgId()) != 13 ||
        gen.MotherIndex() < 0 || gen.P4().DeltaR(reco) > 0.2)
      continue;
    const float distance =
        std::pow(gen.P4().DeltaR(reco) / 0.005f, 2) +
        std::pow((storage.pt[index] / gen.Pt() - 1.f) / 0.02f, 2);
    if (distance < bestDistance) {
      bestDistance = distance;
      matchedPt = gen.Pt();
    }
  }
  return matchedPt;
}

// Stage 1: the nominal medium-pT momentum. This is what Pt() reads, so it is
// the only stage the default path materialises.
void AnalyzerCore::PopulateMuonMomentum(MuonSoA &storage) {
  const std::size_t count = storage.rawSize();
  storage.correctedPt.assign(count, 0.f);
  storage.miniAODPt.assign(count, 0.f);

  const bool usePOGScaleSmearing = myCorr && myCorr->HasMuonScaleSmearing();
  const unsigned long long eventNumber =
      static_cast<unsigned long long>(event.get());
  const unsigned int lumiBlock =
      static_cast<unsigned int>(luminosityBlock.get());

  GenViewCollection truth;
  bool truthLoaded = false;
  for (std::size_t index = 0; index < count; ++index) {
    const float pt = storage.pt[index];
    storage.miniAODPt[index] = pt;
    if (usePOGScaleSmearing) {
      const int trackerLayers =
          IsDATA ? 0 : static_cast<int>(storage.nTrackerLayers[index]);
      storage.correctedPt[index] =
          myCorr->GetMuonScaleSmearing(storage.charge[index], pt,
                                       storage.eta[index], storage.phi[index],
                                       trackerLayers, eventNumber, lumiBlock,
                                       /*withVariations=*/false)
              .nominal;
      continue;
    }
    if (!myCorr) {
      storage.correctedPt[index] = pt;
      continue;
    }
    const float matchedPt =
        IsDATA ? 0.f
               : MuonRochesterMatchedGenPt(storage, index, truth, truthLoaded);
    const int trackerLayers =
        IsDATA ? 0 : static_cast<int>(storage.nTrackerLayers[index]);
    storage.correctedPt[index] =
        pt * myCorr->GetMuonScaleAndError(storage.charge[index], pt,
                                          storage.eta[index],
                                          storage.phi[index], trackerLayers,
                                          matchedPt)
                 .scale;
  }
}

// Stage 2: scale and resolution nuisances of the medium-pT correction. The
// POG nominal is deterministic (the draw comes from the file's hash
// generator), so recomputing it here reproduces stage 1 exactly.
void AnalyzerCore::PopulateMuonMomentumVariations(MuonSoA &storage) {
  const std::size_t count = storage.rawSize();
  storage.momentumScaleUp.assign(count, 0.f);
  storage.momentumScaleDown.assign(count, 0.f);
  storage.momentumResUp.assign(count, 0.f);
  storage.momentumResDown.assign(count, 0.f);

  const bool usePOGScaleSmearing = myCorr && myCorr->HasMuonScaleSmearing();
  const unsigned long long eventNumber =
      static_cast<unsigned long long>(event.get());
  const unsigned int lumiBlock =
      static_cast<unsigned int>(luminosityBlock.get());

  GenViewCollection truth;
  bool truthLoaded = false;
  for (std::size_t index = 0; index < count; ++index) {
    const float pt = storage.pt[index];
    const int trackerLayers =
        IsDATA ? 0 : static_cast<int>(storage.nTrackerLayers[index]);
    if (usePOGScaleSmearing) {
      const MyCorrection::MuonMomentumLanes lanes = myCorr->GetMuonScaleSmearing(
          storage.charge[index], pt, storage.eta[index], storage.phi[index],
          trackerLayers, eventNumber, lumiBlock);
      storage.momentumScaleUp[index] = lanes.scaleUp;
      storage.momentumScaleDown[index] = lanes.scaleDown;
      storage.momentumResUp[index] = lanes.resUp;
      storage.momentumResDown[index] = lanes.resDown;
      continue;
    }
    MyCorrection::MuonScaleAndError scale{1.f, 0.f};
    if (myCorr) {
      const float matchedPt =
          IsDATA ? 0.f
                 : MuonRochesterMatchedGenPt(storage, index, truth, truthLoaded);
      scale = myCorr->GetMuonScaleAndError(storage.charge[index], pt,
                                           storage.eta[index],
                                           storage.phi[index], trackerLayers,
                                           matchedPt);
    }
    storage.momentumScaleUp[index] = pt * (scale.scale + scale.error);
    storage.momentumScaleDown[index] = pt * (scale.scale - scale.error);
    // Rochester carries no resolution nuisance of its own.
    storage.momentumResUp[index] = storage.correctedPt[index];
    storage.momentumResDown[index] = storage.correctedPt[index];
  }
}

// Stage 3: the high-pT momentum. The regime is decided on the pre-correction
// TuneP pt and latched here, so later passes cannot reclassify a muon that a
// correction moved across the boundary.
void AnalyzerCore::PopulateMuonHighPt(MuonSoA &storage) {
  const std::size_t count = storage.rawSize();
  storage.tunePPt.assign(count, 0.f);
  storage.highPtRegime.assign(count, 0);
  storage.highPtPt.assign(count, 0.f);

  for (std::size_t index = 0; index < count; ++index) {
    const float eta = storage.eta[index];
    const float phi = storage.phi[index];
    const int charge = storage.charge[index];
    const float tunePPt = storage.pt[index] * storage.tunepRelPt[index];
    const bool highPt = tunePPt >= MyCorrection::HIGHPT_MUON_MIN_PT;
    storage.tunePPt[index] = tunePPt;
    storage.highPtRegime[index] = highPt ? 1 : 0;

    if (!highPt) {
      // Below the boundary the medium-pT correction already covers scale and
      // resolution, so the high-pT lane mirrors it. Smearing here again
      // would double count against the medium-pT correction.
      storage.highPtPt[index] = storage.correctedPt[index];
      continue;
    }
    if (!myCorr) {
      storage.highPtPt[index] = tunePPt;
      continue;
    }
    // The Generalized Endpoint bias belongs to data, so it is removed there
    // and simulation is left alone.
    if (IsDATA) {
      storage.highPtPt[index] = myCorr->GetMuonGEScaledPt(
          tunePPt, eta, phi, charge, MyCorrection::variation::nom);
      continue;
    }
    // Extra resolution smearing, simulation and high-pT regime only.
    const float momentum = tunePPt * std::cosh(eta);
    const unsigned int seed = MuonSmearSeed(event, eta, phi);
    storage.highPtPt[index] =
        tunePPt * myCorr->GetMuonHighPtSmearFactor(
                      momentum, eta, seed, MyCorrection::variation::nom);
  }
}

// Stage 4: the high-pT nuisances. Below the boundary they mirror the
// medium-pT ones; above it the seeded smear factor reproduces stage 3.
void AnalyzerCore::PopulateMuonHighPtVariations(MuonSoA &storage) {
  const std::size_t count = storage.rawSize();
  storage.highPtScaleUp.assign(count, 0.f);
  storage.highPtScaleDown.assign(count, 0.f);
  storage.highPtResUp.assign(count, 0.f);
  storage.highPtResDown.assign(count, 0.f);

  for (std::size_t index = 0; index < count; ++index) {
    if (storage.highPtRegime[index] == 0) {
      storage.highPtScaleUp[index] = storage.momentumScaleUp[index];
      storage.highPtScaleDown[index] = storage.momentumScaleDown[index];
      storage.highPtResUp[index] = storage.momentumResUp[index];
      storage.highPtResDown[index] = storage.momentumResDown[index];
      continue;
    }
    const float highPtPt = storage.highPtPt[index];
    if (!myCorr || IsDATA) {
      // Data is never varied: the nuisance rides on the simulation templates.
      storage.highPtScaleUp[index] = highPtPt;
      storage.highPtScaleDown[index] = highPtPt;
      storage.highPtResUp[index] = highPtPt;
      storage.highPtResDown[index] = highPtPt;
      continue;
    }
    const float eta = storage.eta[index];
    const float phi = storage.phi[index];
    const int charge = storage.charge[index];
    const float tunePPt = storage.tunePPt[index];
    const float momentum = tunePPt * std::cosh(eta);
    const unsigned int seed = MuonSmearSeed(event, eta, phi);
    const float nomFactor = myCorr->GetMuonHighPtSmearFactor(
        momentum, eta, seed, MyCorrection::variation::nom);
    const float upFactor = myCorr->GetMuonHighPtSmearFactor(
        momentum, eta, seed, MyCorrection::variation::up);
    const float downFactor = myCorr->GetMuonHighPtSmearFactor(
        momentum, eta, seed, MyCorrection::variation::down);
    // Shift by the kappa uncertainty alone. Moving simulation by the full
    // kappa would make the nuisance the size of the correction rather than of
    // its error, which at TeV momenta is a large difference.
    storage.highPtScaleUp[index] =
        myCorr->GetMuonGESigmaShiftedPt(tunePPt, eta, phi, charge,
                                        MyCorrection::variation::up) *
        nomFactor;
    storage.highPtScaleDown[index] =
        myCorr->GetMuonGESigmaShiftedPt(tunePPt, eta, phi, charge,
                                        MyCorrection::variation::down) *
        nomFactor;
    // Variations ride on the nominal as a ratio so the shared pull cancels.
    const float ratioUp = (nomFactor > 0.f) ? upFactor / nomFactor : 1.f;
    const float ratioDown = (nomFactor > 0.f) ? downFactor / nomFactor : 1.f;
    storage.highPtResUp[index] = highPtPt * ratioUp;
    storage.highPtResDown[index] = highPtPt * ratioDown;
  }
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

std::vector<std::size_t> AnalyzerCore::SelectHighPtMuonIndices(
    const MuonViewCollection &muons,
    const std::vector<std::size_t> &seed_indices, const MuonView::MuonID ID,
    const float ptmin, const float fetamax) const {
  std::vector<std::size_t> selected;
  selected.reserve(muons.size());
  for (auto i : seed_indices) {
    const auto &mu = muons[i];
    // Cut on the high-pT momentum rather than Pt(): above the boundary that is
    // TuneP with the Generalized Endpoint scale, which is what the analysis
    // actually uses for these muons.
    if (!(mu.HighPtPt() > ptmin))
      continue;
    if (!(std::abs(mu.Eta()) < fetamax))
      continue;
    if (!mu.PassID(ID))
      continue;
    selected.push_back(i);
  }
  return selected;
}

std::vector<std::size_t> AnalyzerCore::SelectHighPtMuonIndices(
    const MuonViewCollection &muons, const MuonView::MuonID ID,
    const float ptmin, const float fetamax) const {
  auto seed_indices = AllIndices(muons);
  return SelectHighPtMuonIndices(muons, seed_indices, ID, ptmin, fetamax);
}

// Stage 1: the nominal EGM correction, a scale on data and a smearing in
// simulation. One Gaussian draw per electron is kept for the variation
// stage, which is what makes the variations coherent shifts rather than
// independent re-smearings.
void AnalyzerCore::PopulateElectronMomentum(ElectronSoA &storage) {
  const std::size_t count = storage.size();
  storage.correctedPt.assign(count, 0.f);
  storage.smearDraw.assign(count, 0.f);

  for (std::size_t index = 0; index < count; ++index) {
    const float rawPt = storage.pt[index];
    if (!myCorr) {
      storage.correctedPt[index] = rawPt;
      continue;
    }
    const float scEta = storage.scEta[index];
    const float r9 = storage.r9[index];
    if (IsDATA) {
      // Data takes the scale only; there is nothing to smear.
      storage.correctedPt[index] =
          rawPt * myCorr->GetElectronScaleCorr(scEta, storage.seedGain[index],
                                               RunNumber, r9, rawPt);
      continue;
    }
    const unsigned int seed = MuonSmearSeed(event, scEta, storage.phi[index]);
    TRandom3 rng(seed);
    const float draw = rng.Gaus(0.f, 1.f);
    storage.smearDraw[index] = draw;
    const float widthNom = myCorr->GetElectronSmearWidth(
        rawPt, r9, scEta, MyCorrection::variation::nom);
    storage.correctedPt[index] = rawPt * (1.f + widthNom * draw);
  }
}

// Stage 2: the scale and smearing nuisances. Smear variations rescale the
// raw momentum with the stage-1 draw; scale variations multiply the already
// smeared momentum. Data carries no variation.
void AnalyzerCore::PopulateElectronMomentumVariations(ElectronSoA &storage) {
  const std::size_t count = storage.size();
  storage.scaleUpPt.assign(count, 0.f);
  storage.scaleDownPt.assign(count, 0.f);
  storage.smearUpPt.assign(count, 0.f);
  storage.smearDownPt.assign(count, 0.f);

  for (std::size_t index = 0; index < count; ++index) {
    const float corrected = storage.correctedPt[index];
    if (!myCorr || IsDATA) {
      storage.scaleUpPt[index] = corrected;
      storage.scaleDownPt[index] = corrected;
      storage.smearUpPt[index] = corrected;
      storage.smearDownPt[index] = corrected;
      continue;
    }
    const float rawPt = storage.pt[index];
    const float scEta = storage.scEta[index];
    const float r9 = storage.r9[index];
    const unsigned char seedGain = storage.seedGain[index];
    const float draw = storage.smearDraw[index];
    storage.smearUpPt[index] =
        rawPt * (1.f + draw * myCorr->GetElectronSmearWidth(
                                  rawPt, r9, scEta, MyCorrection::variation::up));
    storage.smearDownPt[index] =
        rawPt * (1.f + draw * myCorr->GetElectronSmearWidth(
                                  rawPt, r9, scEta, MyCorrection::variation::down));
    storage.scaleUpPt[index] =
        corrected * myCorr->GetElectronScaleUnc(scEta, seedGain, RunNumber, r9,
                                                rawPt,
                                                MyCorrection::variation::up);
    storage.scaleDownPt[index] =
        corrected * myCorr->GetElectronScaleUnc(scEta, seedGain, RunNumber, r9,
                                                rawPt,
                                                MyCorrection::variation::down);
  }
}

ElectronViewCollection AnalyzerCore::GetAllElectronViews(bool skipCrack) {
  const Long64_t entry = CurrentEntry();
  // Only the default collection is cached; a crack-skipping request is a
  // different set of electrons and would otherwise poison the cache.
  if (!skipCrack && cachedElectronViewsEntry == entry)
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
  storage->populateMomentum = [this, storagePtr = storage.get()] {
    PopulateElectronMomentum(*storagePtr);
  };
  storage->populateMomentumVariations = [this, storagePtr = storage.get()] {
    PopulateElectronMomentumVariations(*storagePtr);
  };

  if (skipCrack)
    return ElectronViewCollection(std::move(storage), true);

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
  storage->populateNominal = [this, storagePtr = storage.get()] {
    PopulateFatJetNominal(*storagePtr);
  };
  storage->populateJerVariations = [this, storagePtr = storage.get()] {
    PopulateFatJetJerVariations(*storagePtr);
  };
  storage->populateJesVariations = [this, storagePtr = storage.get()] {
    PopulateFatJetJesVariations(*storagePtr);
  };
  return FatJetViewCollection(std::move(storage));

}

// Hybrid JER smearing factor for one AK8 jet from the stage-1 intermediates:
// scale toward the matched gen jet when there is one, otherwise the stochastic
// term with the jet's own unit draw.
float AnalyzerCore::FatJetSmearFactor(const FatJetSoA &storage,
                                      const std::size_t index,
                                      const MyCorrection::variation syst) const {
  const float correctedPt = storage.correctedPt[index];
  const float sf = myCorr->GetFJERSF(storage.eta[index], correctedPt, syst);
  const float matchedGenPt = storage.jerMatchedGenPt[index];
  if (matchedGenPt > 0.f) {
    const float ratio = (correctedPt - matchedGenPt) / correctedPt;
    return std::max(0.f, 1.f + (sf - 1.f) * ratio);
  }
  if (sf <= 1.f)
    return 1.f;
  const float width =
      storage.jerResolution[index] * std::sqrt(sf * sf - 1.f);
  return std::max(0.f, 1.f + width * storage.jerUnitDraw[index]);
}

// m_SD scaled by how the corrected subjet pair mass moves. Subjets are
// AK4-scale objects, so the AK4 JER and JES uncertainty apply. The nominal
// pass carries the JER nominal smearing: NanoAOD's msoftdrop has the subjet
// JEC but no smearing, and without it a JER variation could only ever add
// width, pushing Up and Down the same way.
float AnalyzerCore::FatJetSoftDropRatio(
    const FatJetSoA &storage, const std::size_t index,
    const MyCorrection::variation jesVar,
    const MyCorrection::variation jerVar) const {
  const std::size_t subIdx[2] = {
      static_cast<std::size_t>(storage.subJetIdx1[index]),
      static_cast<std::size_t>(storage.subJetIdx2[index])};
  TLorentzVector varied;
  for (int s = 0; s < 2; ++s) {
    const std::size_t j = subIdx[s];
    const float subPt = SubJet_pt[j];
    const float sf = myCorr->GetJERSF(SubJet_eta[j], subPt, jerVar);
    const float subGenPt = storage.sdSubGenPt[index][s];
    float factor = 1.f;
    if (subGenPt > 0.f) {
      factor = std::max(0.f, 1.f + (sf - 1.f) * (subPt - subGenPt) / subPt);
    } else if (sf > 1.f) {
      factor = std::max(0.f, 1.f + storage.sdSubResolution[index][s] *
                                       std::sqrt(sf * sf - 1.f) *
                                       storage.sdSubDraw[index][s]);
    }
    if (jesVar != MyCorrection::variation::nom)
      factor *= myCorr->GetJESUncertaintySF(SubJet_eta[j], subPt, jesVar,
                                            "Total");
    TLorentzVector v;
    v.SetPtEtaPhiM(subPt * factor, SubJet_eta[j], SubJet_phi[j],
                   SubJet_mass[j] * factor);
    varied += v;
  }
  return varied.M() / storage.sdReferenceMass[index];
}

// Stage 1: JEC re-derived from the raw momentum, the nominal JER smearing and
// the nominal SoftDrop mass. Every random draw and gen match the variation
// stages need is taken here, in the same order as before the split, and
// stored on the SoA.
void AnalyzerCore::PopulateFatJetNominal(FatJetSoA &storage) {
  const std::size_t count = storage.size();
  auto reset = [count](std::vector<float> &lane) { lane.assign(count, 0.f); };
  reset(storage.correctedPt);
  reset(storage.correctedMass);
  reset(storage.smearedPtNominal);
  reset(storage.smearedMassNominal);
  reset(storage.sdMassNominal);
  reset(storage.jerUnitDraw);
  storage.jerMatchedGenPt.assign(count, -1.f);
  reset(storage.jerResolution);
  reset(storage.sdReferenceMass);
  storage.sdSubDraw.assign(count, {0.f, 0.f});
  storage.sdSubGenPt.assign(count, {-1.f, -1.f});
  storage.sdSubResolution.assign(count, {0.f, 0.f});

  const float rho = static_cast<float>(Rho_fixedGridRhoFastjetAll.get());

  // Seed per event, as the AK4 path does, so the smearing reproduces across
  // runs. The offset keeps AK8 off the same draws as AK4.
  if (!IsDATA)
    gRandom->SetSeed(static_cast<int>(PuppiMET_pt * 1e6) + 1);

  // Gen-level AK8 jets, for the scaling half of the hybrid smearing. There is
  // no GenJetAK8 view, so the raw branches are read directly.
  const std::size_t genCount =
      IsDATA ? 0u : static_cast<std::size_t>(nGenJetAK8.get());

  for (std::size_t index = 0; index < count; ++index) {
    const float rawPt = storage.pt[index] * (1.f - storage.rawFactor[index]);
    const float rawMass = storage.mass[index] * (1.f - storage.rawFactor[index]);
    const float eta = storage.eta[index];
    const float phi = storage.phi[index];
    const float sdMass = storage.softDropMass[index];

    if (!myCorr) {
      storage.correctedPt[index] = storage.pt[index];
      storage.correctedMass[index] = storage.mass[index];
      storage.smearedPtNominal[index] = storage.pt[index];
      storage.smearedMassNominal[index] = storage.mass[index];
      storage.sdMassNominal[index] = sdMass;
      continue;
    }

    // Re-derive the JEC from the raw momentum rather than trusting whatever
    // JEC production happened to bake in.
    const float jec = myCorr->GetFJESSF(storage.area[index], eta, rawPt, phi,
                                        rho, RunNumber);
    const float correctedPt = rawPt * jec;
    const float correctedMass = rawMass * jec;
    storage.correctedPt[index] = correctedPt;
    storage.correctedMass[index] = correctedMass;

    if (IsDATA) {
      // Data gets neither smearing nor a JES variation: the uncertainty is a
      // simulation nuisance. The lanes mirror the corrected momentum so
      // callers can use one accessor regardless of sample type.
      storage.smearedPtNominal[index] = correctedPt;
      storage.smearedMassNominal[index] = correctedMass;
      storage.sdMassNominal[index] = sdMass;
      continue;
    }

    const float resolution = myCorr->GetFJER(eta, correctedPt, rho);
    storage.jerResolution[index] = resolution;
    float matchedGenPt = -1.f;
    float bestDR = 0.4f; // AK8 matching cone is R/2 = 0.4
    for (std::size_t g = 0; g < genCount; ++g) {
      const float dEta = eta - GenJetAK8_eta[g];
      float dPhi = phi - GenJetAK8_phi[g];
      while (dPhi > static_cast<float>(M_PI))
        dPhi -= 2.f * static_cast<float>(M_PI);
      while (dPhi < -static_cast<float>(M_PI))
        dPhi += 2.f * static_cast<float>(M_PI);
      const float dR = std::sqrt(dEta * dEta + dPhi * dPhi);
      if (dR >= bestDR)
        continue;
      // The gen jet also has to be compatible in momentum, else this is a
      // coincidental overlap rather than the same jet.
      if (std::fabs(correctedPt - GenJetAK8_pt[g]) >=
          3.f * resolution * correctedPt)
        continue;
      bestDR = dR;
      matchedGenPt = GenJetAK8_pt[g];
    }
    storage.jerMatchedGenPt[index] = matchedGenPt;
    // One unit draw per jet, reused by the nominal and both variations.
    storage.jerUnitDraw[index] = gRandom->Gaus(0.f, 1.f);

    const float nomFactor =
        FatJetSmearFactor(storage, index, MyCorrection::variation::nom);
    storage.smearedPtNominal[index] = correctedPt * nomFactor;
    storage.smearedMassNominal[index] = correctedMass * nomFactor;

    // ---- SoftDrop mass --------------------------------------------------
    // m_SD is the invariant mass of the two soft-drop subjets, so its JERC
    // response is the subjets' (AK4), not the fat jet's. Everything is taken
    // as a ratio to the uncorrected subjet pair mass, which NanoAOD already
    // agrees with to well under a percent.
    const short sub1 = storage.subJetIdx1[index];
    const short sub2 = storage.subJetIdx2[index];
    const std::size_t nSub = static_cast<std::size_t>(nSubJet.get());
    if (sub1 < 0 || sub2 < 0 || static_cast<std::size_t>(sub1) >= nSub ||
        static_cast<std::size_t>(sub2) >= nSub) {
      // No subjet pair recorded: leave m_SD alone rather than guess.
      storage.sdMassNominal[index] = sdMass;
      continue;
    }

    const std::size_t subIdx[2] = {static_cast<std::size_t>(sub1),
                                   static_cast<std::size_t>(sub2)};
    TLorentzVector reference;
    const std::size_t nSubGen =
        static_cast<std::size_t>(nSubGenJetAK8.get());
    for (int s = 0; s < 2; ++s) {
      const std::size_t j = subIdx[s];
      TLorentzVector v;
      v.SetPtEtaPhiM(SubJet_pt[j], SubJet_eta[j], SubJet_phi[j],
                     SubJet_mass[j]);
      reference += v;
      // One draw per subjet, reused by every pass so the variations stay a
      // coherent shift of the same pull.
      storage.sdSubDraw[index][s] = gRandom->Gaus(0.f, 1.f);
      // Subjets are AK4-scale objects, so the AK4 resolution applies.
      const float subRes = myCorr->GetJER(SubJet_eta[j], SubJet_pt[j], rho);
      storage.sdSubResolution[index][s] = subRes;
      float best = 0.2f; // AK4 matching cone, R/2
      for (std::size_t g = 0; g < nSubGen; ++g) {
        const float dEta = SubJet_eta[j] - SubGenJetAK8_eta[g];
        float dPhi = SubJet_phi[j] - SubGenJetAK8_phi[g];
        while (dPhi > static_cast<float>(M_PI))
          dPhi -= 2.f * static_cast<float>(M_PI);
        while (dPhi < -static_cast<float>(M_PI))
          dPhi += 2.f * static_cast<float>(M_PI);
        const float dR = std::sqrt(dEta * dEta + dPhi * dPhi);
        if (dR >= best)
          continue;
        if (std::fabs(SubJet_pt[j] - SubGenJetAK8_pt[g]) >=
            3.f * subRes * SubJet_pt[j])
          continue;
        best = dR;
        storage.sdSubGenPt[index][s] = SubGenJetAK8_pt[g];
      }
    }

    const float referenceMass = reference.M();
    if (!(referenceMass > 0.f)) {
      storage.sdMassNominal[index] = sdMass;
      continue;
    }
    storage.sdReferenceMass[index] = referenceMass;
    storage.sdMassNominal[index] =
        sdMass * FatJetSoftDropRatio(storage, index,
                                     MyCorrection::variation::nom,
                                     MyCorrection::variation::nom);
  }
}

// Stage 2: JER up/down on the fat jet and on m_SD, from the stage-1 draws.
void AnalyzerCore::PopulateFatJetJerVariations(FatJetSoA &storage) {
  const std::size_t count = storage.size();
  auto reset = [count](std::vector<float> &lane) { lane.assign(count, 0.f); };
  reset(storage.smearedPtUp);
  reset(storage.smearedPtDown);
  reset(storage.smearedMassUp);
  reset(storage.smearedMassDown);
  reset(storage.sdMassJerUp);
  reset(storage.sdMassJerDown);

  for (std::size_t index = 0; index < count; ++index) {
    const float sdMass = storage.softDropMass[index];
    if (!myCorr || IsDATA) {
      storage.smearedPtUp[index] = storage.smearedPtNominal[index];
      storage.smearedPtDown[index] = storage.smearedPtNominal[index];
      storage.smearedMassUp[index] = storage.smearedMassNominal[index];
      storage.smearedMassDown[index] = storage.smearedMassNominal[index];
      storage.sdMassJerUp[index] = sdMass;
      storage.sdMassJerDown[index] = sdMass;
      continue;
    }
    const float correctedPt = storage.correctedPt[index];
    const float correctedMass = storage.correctedMass[index];
    const float upFactor =
        FatJetSmearFactor(storage, index, MyCorrection::variation::up);
    const float downFactor =
        FatJetSmearFactor(storage, index, MyCorrection::variation::down);
    storage.smearedPtUp[index] = correctedPt * upFactor;
    storage.smearedPtDown[index] = correctedPt * downFactor;
    storage.smearedMassUp[index] = correctedMass * upFactor;
    storage.smearedMassDown[index] = correctedMass * downFactor;

    if (!(storage.sdReferenceMass[index] > 0.f)) {
      storage.sdMassJerUp[index] = sdMass;
      storage.sdMassJerDown[index] = sdMass;
      continue;
    }
    storage.sdMassJerUp[index] =
        sdMass * FatJetSoftDropRatio(storage, index,
                                     MyCorrection::variation::nom,
                                     MyCorrection::variation::up);
    storage.sdMassJerDown[index] =
        sdMass * FatJetSoftDropRatio(storage, index,
                                     MyCorrection::variation::nom,
                                     MyCorrection::variation::down);
  }
}

// Stage 3: JES up/down on top of the nominal smearing, matching AK4, and the
// subjet (AK4) JES uncertainty propagated into m_SD.
void AnalyzerCore::PopulateFatJetJesVariations(FatJetSoA &storage) {
  const std::size_t count = storage.size();
  auto reset = [count](std::vector<float> &lane) { lane.assign(count, 0.f); };
  reset(storage.jesPtUp);
  reset(storage.jesPtDown);
  reset(storage.jesMassUp);
  reset(storage.jesMassDown);
  reset(storage.sdMassJesUp);
  reset(storage.sdMassJesDown);

  for (std::size_t index = 0; index < count; ++index) {
    const float sdMass = storage.softDropMass[index];
    const float smearedPt = storage.smearedPtNominal[index];
    const float smearedMass = storage.smearedMassNominal[index];
    if (!myCorr || IsDATA) {
      storage.jesPtUp[index] = smearedPt;
      storage.jesPtDown[index] = smearedPt;
      storage.jesMassUp[index] = smearedMass;
      storage.jesMassDown[index] = smearedMass;
      storage.sdMassJesUp[index] = sdMass;
      storage.sdMassJesDown[index] = sdMass;
      continue;
    }
    // JES uncertainty is evaluated on the corrected momentum.
    const float eta = storage.eta[index];
    const float correctedPt = storage.correctedPt[index];
    const float jesUp = myCorr->GetFJESUncertaintySF(
        eta, correctedPt, MyCorrection::variation::up);
    const float jesDown = myCorr->GetFJESUncertaintySF(
        eta, correctedPt, MyCorrection::variation::down);
    storage.jesPtUp[index] = smearedPt * jesUp;
    storage.jesPtDown[index] = smearedPt * jesDown;
    storage.jesMassUp[index] = smearedMass * jesUp;
    storage.jesMassDown[index] = smearedMass * jesDown;

    if (!(storage.sdReferenceMass[index] > 0.f)) {
      storage.sdMassJesUp[index] = sdMass;
      storage.sdMassJesDown[index] = sdMass;
      continue;
    }
    storage.sdMassJesUp[index] =
        sdMass * FatJetSoftDropRatio(storage, index,
                                     MyCorrection::variation::up,
                                     MyCorrection::variation::nom);
    storage.sdMassJesDown[index] =
        sdMass * FatJetSoftDropRatio(storage, index,
                                     MyCorrection::variation::down,
                                     MyCorrection::variation::nom);
  }
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
