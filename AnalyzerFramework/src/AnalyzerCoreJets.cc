#include <AnalyzerFramework/AnalyzerCore.h>
#include "JetView.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <tuple>
#include <vector>

unordered_map<int, int>
AnalyzerCore::deltaRMatching(const RVec<Particle> &objs1,
                             const RVec<Particle> &objs2, const float dR) {
  RVec<tuple<size_t, size_t, float>> possible_matches;
  RVec<bool> used_obj1(objs1.size(), false);
  RVec<bool> used_obj2(objs2.size(), false);
  for (size_t i = 0; i < objs1.size(); i++) {
    for (size_t j = 0; j < objs2.size(); j++) {
      float this_dR = objs1[i].DeltaR(objs2[j]);
      if (this_dR < dR) {
        possible_matches.emplace_back(i, j, this_dR);
      }
    }
  }
  sort(possible_matches.begin(), possible_matches.end(),
       [](const tuple<size_t, size_t, float> &a,
          const tuple<size_t, size_t, float> &b) {
         return get<2>(a) < get<2>(b);
       });
  unordered_map<int, int> matched_idx;
  // print all
  for (size_t i = 0; i < possible_matches.size(); i++) {
    int obj1_idx = get<0>(possible_matches[i]);
    int obj2_idx = get<1>(possible_matches[i]);
    if (used_obj1[obj1_idx] || used_obj2[obj2_idx])
      continue;
    matched_idx[obj1_idx] = obj2_idx;
    used_obj1[obj1_idx] = true;
    used_obj2[obj2_idx] = true;
  }

  for (size_t i = 0; i < objs1.size(); i++) {
    if (used_obj1[i])
      continue;
    matched_idx[i] = -999;
  }

  return matched_idx;
}

std::shared_ptr<JetSoA> AnalyzerCore::CreateJetSoA() const {
  auto storage = std::make_shared<JetSoA>();
  storage->pt.bind(&Jet_pt);
  storage->eta.bind(&Jet_eta);
  storage->phi.bind(&Jet_phi);
  storage->mass.bind(&Jet_mass);
  storage->rawFactor.bind(&Jet_rawFactor);
  storage->area.bind(&Jet_area);
  storage->chHEF.bind(&Jet_chHEF);
  storage->neHEF.bind(&Jet_neHEF);
  storage->neEmEF.bind(&Jet_neEmEF);
  storage->chEmEF.bind(&Jet_chEmEF);
  storage->muEF.bind(&Jet_muEF);
  storage->partonFlavour.bind(&Jet_partonFlavour);
  storage->hadronFlavour.bind(&Jet_hadronFlavour);
  storage->chMultiplicity.bind(&Jet_chMultiplicity);
  storage->neMultiplicity.bind(&Jet_neMultiplicity);
  storage->nConstituents.bind(&Jet_nConstituents);
  storage->nElectrons.bind(&Jet_nElectrons);
  storage->nMuons.bind(&Jet_nMuons);
  storage->nSVs.bind(&Jet_nSVs);
  storage->electronIdx1.bind(&Jet_electronIdx1);
  storage->electronIdx2.bind(&Jet_electronIdx2);
  storage->muonIdx1.bind(&Jet_muonIdx1);
  storage->muonIdx2.bind(&Jet_muonIdx2);
  storage->svIdx1.bind(&Jet_svIdx1);
  storage->svIdx2.bind(&Jet_svIdx2);
  storage->genJetIdx.bind(&Jet_genJetIdx);

  storage->deepFlavB.bind(&Jet_btagDeepFlavB);
  storage->deepFlavCvB.bind(&Jet_btagDeepFlavCvB);
  storage->deepFlavCvL.bind(&Jet_btagDeepFlavCvL);
  storage->deepFlavQG.bind(&Jet_btagDeepFlavQG);
  storage->pnetB.bind(&Jet_btagPNetB);
  storage->pnetCvB.bind(&Jet_btagPNetCvB);
  storage->pnetCvL.bind(&Jet_btagPNetCvL);
  storage->pnetCvNotB.bind(&Jet_btagPNetCvNotB);
  storage->pnetQvG.bind(&Jet_btagPNetQvG);
  storage->pnetTauVJet.bind(&Jet_btagPNetTauVJet);
  storage->uparTAK4B.bind(&Jet_btagUParTAK4B);
  storage->uparTAK4CvB.bind(&Jet_btagUParTAK4CvB);
  storage->uparTAK4CvL.bind(&Jet_btagUParTAK4CvL);
  storage->uparTAK4CvNotB.bind(&Jet_btagUParTAK4CvNotB);
  storage->uparTAK4Ele.bind(&Jet_btagUParTAK4Ele);
  storage->uparTAK4Mu.bind(&Jet_btagUParTAK4Mu);
  storage->uparTAK4QvG.bind(&Jet_btagUParTAK4QvG);
  storage->uparTAK4SvCB.bind(&Jet_btagUParTAK4SvCB);
  storage->uparTAK4SvUDG.bind(&Jet_btagUParTAK4SvUDG);
  storage->uparTAK4TauVJet.bind(&Jet_btagUParTAK4TauVJet);
  storage->uparTAK4UDG.bind(&Jet_btagUParTAK4UDG);
  storage->uparTAK4ProbB.bind(&Jet_btagUParTAK4probb);
  storage->uparTAK4ProbBB.bind(&Jet_btagUParTAK4probbb);
  storage->pnetRegPtRawCorr.bind(&Jet_PNetRegPtRawCorr);
  storage->pnetRegPtRawCorrNeutrino.bind(&Jet_PNetRegPtRawCorrNeutrino);
  storage->pnetRegPtRawRes.bind(&Jet_PNetRegPtRawRes);
  storage->uparTAK4RegPtRawCorr.bind(&Jet_UParTAK4RegPtRawCorr);
  storage->uparTAK4RegPtRawCorrNeutrino.bind(&Jet_UParTAK4RegPtRawCorrNeutrino);
  storage->uparTAK4RegPtRawRes.bind(&Jet_UParTAK4RegPtRawRes);
  storage->uparTAK4V1RegPtRawCorr.bind(&Jet_UParTAK4V1RegPtRawCorr);
  storage->uparTAK4V1RegPtRawCorrNeutrino.bind(
      &Jet_UParTAK4V1RegPtRawCorrNeutrino);
  storage->uparTAK4V1RegPtRawRes.bind(&Jet_UParTAK4V1RegPtRawRes);
  storage->puIdDisc.bind(&Jet_puIdDisc);
  return storage;
}

void AnalyzerCore::InitialiseJetSystematics(JetSoA &storage) const {
  const std::size_t n = storage.size();
  storage.jecFactor.assign(n, 1.f);
  storage.correctedPt.assign(n, 0.f);
  storage.correctedMass.assign(n, 0.f);
  storage.jerResolution.assign(n, 0.f);
  storage.jerScaleFactorNominal.assign(n, 1.f);
  storage.jerGaussianSample.assign(n, 0.f);
  storage.jerMatchedGenPt.assign(n, -1.f);
  storage.jerMinCorrection.assign(n, 0.f);
  storage.smearedPtNominal.assign(n, 0.f);
  storage.smearedMassNominal.assign(n, 0.f);
  if (IsDATA) {
    // Preserve the legacy data-lane shape. JER variations are not meaningful
    // for data, but existing callers may still inspect the backing vectors.
    storage.smearedPtUp.assign(n, 0.f);
    storage.smearedPtDown.assign(n, 0.f);
    storage.smearedMassUp.assign(n, 0.f);
    storage.smearedMassDown.assign(n, 0.f);
  } else {
    // MC variation lanes are materialized only when a JER up/down projection
    // is actually requested.
    storage.smearedPtUp.clear();
    storage.smearedPtDown.clear();
    storage.smearedMassUp.clear();
    storage.smearedMassDown.clear();
  }
  storage.jesPtUp.assign(n, 0.f);
  storage.jesPtDown.assign(n, 0.f);
  storage.jesMassUp.assign(n, 0.f);
  storage.jesMassDown.assign(n, 0.f);
  storage.jesTotalUncertainty.assign(n, 0.f);
  storage.jesVariationSource.clear();
  storage.jesVariationValid = false;
  storage.jesTotalUncertaintyValid = false;
  storage.jerVariationsReady = IsDATA;
  storage.jerVariationsComputing = false;
}

void AnalyzerCore::PopulateJetStorageWithoutCorrections(JetSoA &storage) const {
  const std::size_t n = storage.size();
  storage.smearedPtUp.assign(n, -999.f);
  storage.smearedPtDown.assign(n, -999.f);
  storage.smearedMassUp.assign(n, -999.f);
  storage.smearedMassDown.assign(n, -999.f);
  for (std::size_t i = 0; i < n; ++i) {
    storage.jecFactor[i] = 1.f;
    storage.correctedPt[i] = -999.f;
    storage.correctedMass[i] = -999.f;
    storage.smearedPtNominal[i] = -999.f;
    storage.smearedPtUp[i] = -999.f;
    storage.smearedPtDown[i] = -999.f;
    storage.smearedMassNominal[i] = -999.f;
    storage.smearedMassUp[i] = -999.f;
    storage.smearedMassDown[i] = -999.f;
    storage.jesPtUp[i] = -999.f;
    storage.jesPtDown[i] = -999.f;
    storage.jesMassUp[i] = -999.f;
    storage.jesMassDown[i] = -999.f;
    storage.jesTotalUncertainty[i] = -999.f;
  }
  storage.jerVariationsReady = true;
}

void AnalyzerCore::ApplyJetEnergyCorrections(JetSoA &storage, float rho) {
  auto performancePhase = MeasurePerformancePhase("correction");
  const auto ptView = storage.pt.snapshot();
  const auto etaView = storage.eta.snapshot();
  const auto phiView = storage.phi.snapshot();
  const auto massView = storage.mass.snapshot();
  const auto rawFactorView = storage.rawFactor.snapshot();
  const auto areaView = storage.area.snapshot();
  const std::size_t n = ptView.size();
  if (etaView.size() != n || phiView.size() != n || massView.size() != n ||
      rawFactorView.size() != n || areaView.size() != n)
    throw SKNano::EventDataError(
        "[ApplyJetEnergyCorrections] inconsistent Jet column sizes");
  const float *pt = ptView.data();
  const float *eta = etaView.data();
  const float *phi = phiView.data();
  const float *mass = massView.data();
  const float *rawFactor = rawFactorView.data();
  const float *area = areaView.data();
  for (std::size_t i = 0; i < n; ++i) {
    const float rawPt = pt[i] * (1.f - rawFactor[i]);
    const float rawMass = mass[i] * (1.f - rawFactor[i]);
    const float jecSF =
        myCorr ? myCorr->GetJESSF(area[i], eta[i], rawPt, phi[i], rho,
                                  RunNumber)
               : 1.f;
    storage.jecFactor[i] = jecSF;
    storage.correctedPt[i] = rawPt * jecSF;
    storage.correctedMass[i] = rawMass * jecSF;
  }
}

const std::vector<int> &
AnalyzerCore::MatchJetsToGenJets(const JetViewCollection &jets,
                                 const GenJetViewCollection &genjets,
                                 float rho) const {
  const auto &jetSoAPtr = jets.storage();
  const auto &genSoAPtr = genjets.storage();
  const std::size_t njet = jetSoAPtr ? jetSoAPtr->size() : 0;
  const std::size_t ngen = genSoAPtr ? genSoAPtr->size() : 0;

  auto &matchedGenIdx = jetMatchIndicesScratch;
  matchedGenIdx.assign(njet, -999);
  if (njet == 0)
    return matchedGenIdx;

  JetSoA &jsoa = *jetSoAPtr;
  const auto jetPtView = jsoa.pt.snapshot();
  const auto jetEtaView = jsoa.eta.snapshot();
  const auto jetPhiView = jsoa.phi.snapshot();
  if (jetPtView.size() != njet || jetEtaView.size() != njet ||
      jetPhiView.size() != njet)
    throw SKNano::EventDataError(
        "[MatchJetsToGenJets] inconsistent Jet/GenJet column sizes");
  const float *jetPtInput = jetPtView.data();
  const float *jetEtaInput = jetEtaView.data();
  const float *jetPhiInput = jetPhiView.data();

  // Resolution is needed both by matching and by unmatched-jet smearing.
  // Cache the first evaluation in the event-owned SoA instead of invoking the
  // correctionlib AST a second time after matching.
  for (std::size_t i = 0; i < njet; ++i) {
    const float pt =
        jsoa.correctedPt.empty() ? jetPtInput[i] : jsoa.correctedPt[i];
    if (pt > 0.f)
      jsoa.jerResolution[i] = myCorr->GetJER(jetEtaInput[i], pt, rho);
  }
  if (ngen == 0)
    return matchedGenIdx;

  const GenJetSoA &gsoa = *genSoAPtr;
  const auto genPtView = gsoa.pt.snapshot();
  const auto genEtaView = gsoa.eta.snapshot();
  const auto genPhiView = gsoa.phi.snapshot();
  if (genPtView.size() != ngen || genEtaView.size() != ngen ||
      genPhiView.size() != ngen)
    throw SKNano::EventDataError(
        "[MatchJetsToGenJets] inconsistent Jet/GenJet column sizes");
  const float *genPtInput = genPtView.data();
  const float *genEtaInput = genEtaView.data();
  const float *genPhiInput = genPhiView.data();

  auto &candidates = jetMatchCandidatesScratch;
  candidates.clear();
  candidates.reserve(njet * 2);

  const float maxDR = 0.2f;
  const float maxDR2 = maxDR * maxDR;
  const float ptJerCut = 3.f;

  // 후보 수집
  for (std::size_t i = 0; i < njet; ++i) {
    const float jetPt =
        jsoa.correctedPt.empty() ? jetPtInput[i] : jsoa.correctedPt[i];
    const float jetEta = jetEtaInput[i];
    const float jetPhi = jetPhiInput[i];
    const float sigma = jsoa.jerResolution[i] * jetPt;
    if (jetPt <= 0.f || sigma <= 0.f)
      continue;

    const float maxPtDiff = ptJerCut * sigma;

    for (std::size_t j = 0; j < ngen; ++j) {
      const float genEta = genEtaInput[j];
      const float genPhi = genPhiInput[j];
      const float genPt = genPtInput[j];

      const float dr2 =
          SKNano::Geometry::DeltaR2(jetEta, jetPhi, genEta, genPhi);
      if (dr2 >= maxDR2)
        continue;

      const float ptDiff = std::fabs(jetPt - genPt);
      if (ptDiff < maxPtDiff)
        candidates.emplace_back(i, j, dr2, ptDiff);
    }
  }

  // 가까운 순서로 정렬
  std::sort(candidates.begin(), candidates.end(),
            [](const auto &a, const auto &b) {
              if (std::get<2>(a) == std::get<2>(b))
                return std::get<3>(a) < std::get<3>(b);
              return std::get<2>(a) < std::get<2>(b);
            });

  // 1:1 할당
  auto &usedJet = jetMatchUsedJetScratch;
  auto &usedGen = jetMatchUsedGenScratch;
  usedJet.assign(njet, 0);
  usedGen.assign(ngen, 0);
  for (const auto &c : candidates) {
    const auto i = std::get<0>(c);
    const auto j = std::get<1>(c);
    if (usedJet[i] || usedGen[j])
      continue;
    matchedGenIdx[i] = static_cast<int>(j);
    usedJet[i] = 1;
    usedGen[j] = 1;
  }

  return matchedGenIdx;
}

void AnalyzerCore::ApplyJetSmearingAndUncertainties(
    const JetViewCollection &jets, const std::vector<int> &matchedGenIdx,
    const GenJetViewCollection &genjets, bool isMC, float rho) {
  auto performancePhase = MeasurePerformancePhase("correction");
  if (IsDATA) {
    auto storage = jets.storage();
    if (storage) {
      for (size_t i = 0; i < storage->size(); ++i) {
        storage->smearedPtNominal[i] = storage->correctedPt[i];
        storage->smearedMassNominal[i] = storage->correctedMass[i];
        // Data has nothing to vary, but ResetJetLanes sizes the variation
        // vectors, so the view accessors read them rather than falling back to
        // the nominal. Mirror the nominal into them: an analyzer that loops
        // over systematics on data then sees the nominal jet instead of a
        // momentum of zero. AK8 gets this for free by leaving its lanes empty.
        storage->smearedPtUp[i] = storage->correctedPt[i];
        storage->smearedPtDown[i] = storage->correctedPt[i];
        storage->smearedMassUp[i] = storage->correctedMass[i];
        storage->smearedMassDown[i] = storage->correctedMass[i];
        storage->jesPtUp[i] = storage->correctedPt[i];
        storage->jesPtDown[i] = storage->correctedPt[i];
        storage->jesMassUp[i] = storage->correctedMass[i];
        storage->jesMassDown[i] = storage->correctedMass[i];
      }
    }
    return;
  }
  constexpr float MIN_JET_ENERGY = 1e-2f;

  auto &storage = *jets.storage();
  const std::size_t n = storage.size();
  const auto etaView = storage.eta.snapshot();
  if (etaView.size() != n)
    throw SKNano::EventDataError(
        "[ApplyJetSmearingAndUncertainties] inconsistent Jet column sizes");
  const float *etaInput = etaView.data();
  for (std::size_t i = 0; i < n; ++i) {
    const float correctedPt = storage.correctedPt[i];
    const float correctedMass = storage.correctedMass[i];
    const float eta = etaInput[i];

    const float energy =
        std::sqrt(correctedPt * correctedPt * std::cosh(eta) * std::cosh(eta) +
                  correctedMass * correctedMass);
    const float minCorr =
        MIN_JET_ENERGY /
        std::max(energy, MIN_JET_ENERGY);

    float smearNom = 1.f;

    if (isMC && correctedPt > 0.f) {
      const int matched = (!matchedGenIdx.empty() && i < matchedGenIdx.size())
                              ? matchedGenIdx[i]
                              : -1;
      const float jerSF =
          myCorr->GetJERSF(eta, correctedPt, MyCorrection::variation::nom);
      storage.jerScaleFactorNominal[i] = jerSF;

      const float jerSigma =
          (matched < 0) ? storage.jerResolution[i] : 0.f;
      float gausSample = 0.f;
      if (matched < 0) {
        if (GetRngMode() == SKNano::RngMode::CounterBased) {
          gausSample = static_cast<float>(SKNano::CounterRNG::normal(
                           static_cast<std::uint64_t>(RunNumber.get()),
                           static_cast<std::uint64_t>(luminosityBlock.get()),
                           static_cast<std::uint64_t>(event.get()), i,
                           0x4a45525f534d4541ULL)) * jerSigma;
        } else {
          gausSample = gRandom->Gaus(0., jerSigma);
        }
      }
      storage.jerGaussianSample[i] = gausSample;
      storage.jerMinCorrection[i] = minCorr;
      if (matched >= 0 && matched < genjets.size())
        storage.jerMatchedGenPt[i] = genjets[matched].Pt();

      const auto computeCorr = [&](float sf) {
        float corr = 1.f;
        if (storage.jerMatchedGenPt[i] >= 0.f) {
          const float genPt = storage.jerMatchedGenPt[i];
          const float scale = 1.f - genPt / std::max(correctedPt, 1e-6f);
          corr += (sf - 1.f) * scale;
        } else {
          const float variance = std::max(sf * sf - 1.f, 0.f);
          corr += gausSample * std::sqrt(variance);
        }
        return std::max(corr, minCorr);
      };

      smearNom = computeCorr(jerSF);
    }

    storage.smearedPtNominal[i] = correctedPt * smearNom;
    storage.smearedMassNominal[i] = correctedMass * smearNom;
  }
}

void AnalyzerCore::PopulateJetJERVariations(
    const std::shared_ptr<JetSoA> &storagePtr) {
  auto performancePhase = MeasurePerformancePhase("correction");
  if (!storagePtr || IsDATA)
    return;

  JetSoA &storage = *storagePtr;
  const std::size_t n = storage.size();
  storage.smearedPtUp.resize(n);
  storage.smearedPtDown.resize(n);
  storage.smearedMassUp.resize(n);
  storage.smearedMassDown.resize(n);
  const auto etaView = storage.eta.snapshot();
  if (etaView.size() != n)
    throw SKNano::EventDataError(
        "[PopulateJetJERVariations] inconsistent Jet column sizes");
  const float *etaInput = etaView.data();

  for (std::size_t i = 0; i < n; ++i) {
    const float correctedPt = storage.correctedPt[i];
    const float correctedMass = storage.correctedMass[i];
    float smearUp = 1.f;
    float smearDown = 1.f;
    if (correctedPt > 0.f) {
      const auto variations = myCorr->GetJERSFVariations(
          etaInput[i], correctedPt, storage.jerScaleFactorNominal[i]);
      const auto computeCorr = [&](float sf) {
        float corr = 1.f;
        if (storage.jerMatchedGenPt[i] >= 0.f) {
          const float scale =
              1.f - storage.jerMatchedGenPt[i] /
                        std::max(correctedPt, 1e-6f);
          corr += (sf - 1.f) * scale;
        } else {
          const float variance = std::max(sf * sf - 1.f, 0.f);
          corr += storage.jerGaussianSample[i] * std::sqrt(variance);
        }
        return std::max(corr, storage.jerMinCorrection[i]);
      };
      smearUp = computeCorr(variations.up);
      smearDown = computeCorr(variations.down);
    }
    storage.smearedPtUp[i] = correctedPt * smearUp;
    storage.smearedPtDown[i] = correctedPt * smearDown;
    storage.smearedMassUp[i] = correctedMass * smearUp;
    storage.smearedMassDown[i] = correctedMass * smearDown;
  }
}

void AnalyzerCore::ApplyJetScaleVariation(JetViewCollection &jets,
                                          const TString &source) const {
  auto storagePtr = jets.storage();
  if (!storagePtr)
    return;
  PopulateJetJESVariations(storagePtr, source);
}

// Also reachable lazily through JetSoA::ensureJesVariations, which is how the
// JesPt*/JesMass* accessors get their lanes filled.
void AnalyzerCore::PopulateJetJESVariations(
    const std::shared_ptr<JetSoA> &storagePtr, const TString &source) const {
  if (!myCorr || IsDATA)
    return;
  if (!storagePtr)
    return;
  storagePtr->ensureNominal();

  JetSoA &storage = *storagePtr;
  const std::size_t n = storage.size();
  if (n == 0)
    return;

  const bool useTotalSource =
      source.IsNull() || source.EqualTo("total", TString::kIgnoreCase);
  const std::string sourceKey =
      useTotalSource ? std::string("total") : std::string(source.Data());
  if (storage.jesVariationValid && storage.jesVariationSource == sourceKey)
    return;

  if (useTotalSource && !storage.jesTotalUncertaintyValid) {
    // JME publishes the quadrature sum of every source as its own "Total"
    // correction, so one lookup per jet replaces walking the 27 sources
    // (checked on the 2024 file: the two agree to 0.3%).
    for (std::size_t i = 0; i < n; ++i) {
      storage.jesTotalUncertainty[i] = myCorr->GetJESUncertainty(
          storage.eta[i], storage.correctedPt[i], "Total");
    }
    storage.jesTotalUncertaintyValid = true;
  }

  for (std::size_t i = 0; i < n; ++i) {
    float unc = 0.f;

    if (useTotalSource) {
      unc = storage.jesTotalUncertainty[i];
    } else {
      unc = myCorr->GetJESUncertainty(storage.eta[i], storage.correctedPt[i],
                                      source);
    }

    // storage.jecFactor[i]; untouched
    // storage.correctedPt[i]; untouched
    /// storage.correctedMass[i]; untouched
    // storage.smearedPtNominal[i]  = -1.f; // to indicate that nominal has
    // changed storage.smearedPtUp[i] = -1.f; storage.smearedPtDown[i] = -1.f;
    // storage.smearedMassNominal[i] = -1.f;
    // storage.smearedMassUp[i] = -1.f;
    // storage.smearedMassDown[i] = -1.f;

    storage.jesPtUp[i] = storage.smearedPtNominal[i] * (1.f + unc);
    storage.jesPtDown[i] = storage.smearedPtNominal[i] * (1.f - unc);
    storage.jesMassUp[i] = storage.smearedMassNominal[i] * (1.f + unc);
    storage.jesMassDown[i] = storage.smearedMassNominal[i] * (1.f - unc);
  }
  storage.jesVariationSource = sourceKey;
  storage.jesVariationValid = true;
}

bool AnalyzerCore::PrepareJetJESVariations(JetViewCollection &jets,
                                           const TString &source,
                                           bool doBreakdown) const {
  const bool isTotal =
      source.IsNull() || source.EqualTo("total", TString::kIgnoreCase);
  if (doBreakdown) {
    if (isTotal)
      return false;
    ApplyJetScaleVariation(jets, source);
    return true;
  }
  if (!isTotal)
    return false;
  ApplyJetScaleVariation(jets, "total");
  return true;
}

bool AnalyzerCore::PropagateJetSystToMET(
    const JetViewCollection &jets, Particle &met,
    const MyCorrection::variation &jesVar,
    const MyCorrection::variation &jerVar) const {
  const bool jesIsNom = (jesVar == MyCorrection::variation::nom);
  const bool jerIsNom = (jerVar == MyCorrection::variation::nom);
  if (!jesIsNom && !jerIsNom) {
    throw std::runtime_error(
        "[AnalyzerCore::PropagateJetSystToMET] Both JES and JER are "
        "non-nominal, not supported");
  }

  float nominalPx = 0.f;
  float nominalPy = 0.f;
  float shiftedPx = 0.f;
  float shiftedPy = 0.f;
  for (const auto &jetView : jets) {
    const float phi = jetView.Phi();
    nominalPx += jetView.Pt() * std::cos(phi);
    nominalPy += jetView.Pt() * std::sin(phi);

    float pt = jetView.SmearedPtNominal();
    if (!jesIsNom) {
      if (jesVar == MyCorrection::variation::up) {
        pt = jetView.JesPtUp();
      } else if (jesVar == MyCorrection::variation::down) {
        pt = jetView.JesPtDown();
      }
    } else if (!jerIsNom) {
      if (jerVar == MyCorrection::variation::up) {
        pt = jetView.SmearedPtUp();
      } else if (jerVar == MyCorrection::variation::down) {
        pt = jetView.SmearedPtDown();
      }
    }

    shiftedPx += pt * std::cos(phi);
    shiftedPy += pt * std::sin(phi);
  }

  met.SetXYZM(met.Px() - (shiftedPx - nominalPx),
              met.Py() - (shiftedPy - nominalPy), 0., 0.);
  return true;
}

const std::vector<TString> &AnalyzerCore::JetEnergyScaleSources() const {
  static const std::vector<TString> sources = {
      "AbsoluteMPFBias", "AbsoluteScale",   "AbsoluteStat",   "FlavorQCD",
      "Fragmentation",   "PileUpDataMC",    "PileUpPtBB",     "PileUpPtEC1",
      "PileUpPtEC2",     "PileUpPtHF",      "PileUpPtRef",    "RelativeJEREC1",
      "RelativeJEREC2",  "RelativeJERHF",   "RelativePtBB",   "RelativePtEC1",
      "RelativePtEC2",   "RelativePtHF",    "RelativeBal",    "RelativeSample",
      "RelativeFSR",     "RelativeStatFSR", "RelativeStatEC", "RelativeStatHF",
      "SinglePionECAL",  "SinglePionHCAL",  "TimePtEta"};
  ;
  return sources;
}

JetViewCollection AnalyzerCore::GetAllJetViews() {
  const Long64_t entry = CurrentEntry();
  if (cachedJetViewsEntry == entry)
    return cachedJetViews;

  auto storage = CreateJetSoA();
  const std::weak_ptr<JetSoA> weakStorage = storage;
  storage->populateNominal = [this, weakStorage]() {
    auto locked = weakStorage.lock();
    if (!locked)
      throw SKNano::EventDataError(
          "[GetAllJetViews] expired Jet storage during lazy population");
    PopulateJetNominal(locked);
  };
  storage->populateJerVariations = [this, weakStorage]() {
    auto locked = weakStorage.lock();
    if (!locked)
      throw SKNano::EventDataError(
          "[GetAllJetViews] expired Jet storage during JER variation "
          "materialization");
    PopulateJetJERVariations(locked);
  };
  storage->populateJesVariations = [this, weakStorage]() {
    auto locked = weakStorage.lock();
    if (!locked)
      throw SKNano::EventDataError(
          "[GetAllJetViews] expired Jet storage during JES variation "
          "materialization");
    PopulateJetJESVariations(locked);
  };

  cachedJetViews = JetViewCollection(std::move(storage));
  cachedJetViewsEntry = entry;
  return cachedJetViews;
}

SelectedJetViewCollection AnalyzerCore::SelectJetViews(
    const JetViewCollection &jets, std::vector<std::size_t> indices,
    const MyCorrection::variation &jesVariation,
    const MyCorrection::variation &jerVariation, bool sortByPt) const {
  if (jesVariation != MyCorrection::variation::nom &&
      jerVariation != MyCorrection::variation::nom)
    throw SKNano::LogicError(
        "[AnalyzerCore::SelectJetViews] simultaneous JES and JER variation");

  JetKinematicProjection projection = IsDATA
      ? JetKinematicProjection::Corrected
      : JetKinematicProjection::Nominal;
  if (!IsDATA) {
    if (jesVariation == MyCorrection::variation::up)
      projection = JetKinematicProjection::JesUp;
    else if (jesVariation == MyCorrection::variation::down)
      projection = JetKinematicProjection::JesDown;
    else if (jerVariation == MyCorrection::variation::up)
      projection = JetKinematicProjection::JerUp;
    else if (jerVariation == MyCorrection::variation::down)
      projection = JetKinematicProjection::JerDown;
  }

  SelectedJetViewCollection selected(jets.storage(), std::move(indices),
                                     projection);
  if (!sortByPt)
    return selected;

  std::vector<std::size_t> sorted = selected.indices();
  std::sort(sorted.begin(), sorted.end(), [&](std::size_t lhs, std::size_t rhs) {
    const SelectedJetView left(jets.storage().get(), lhs, projection);
    const SelectedJetView right(jets.storage().get(), rhs, projection);
    return left.Pt() > right.Pt();
  });
  return SelectedJetViewCollection(jets.storage(), std::move(sorted),
                                   projection);
}

void AnalyzerCore::PopulateJetNominal(
    const std::shared_ptr<JetSoA> &storage) {
  InitialiseJetSystematics(*storage);
  if (storage->size() == 0)
    return;

  if (!myCorr) {
    PopulateJetStorageWithoutCorrections(*storage);
    return;
  }

  const float rho = Rho_fixedGridRhoFastjetAll;
  ApplyJetEnergyCorrections(*storage, rho);
  SmearJetViews(JetViewCollection(storage), rho);
}

void AnalyzerCore::SmearJetViews(const JetViewCollection &jets,
                                 const float rho) {

  // Data has no GenJet/JER matching path.  Return before touching any MC-only
  // branch or RNG state; ApplyJetSmearingAndUncertainties copies the nominal
  // corrected values into the public smeared lanes.
  if (IsDATA) {
    ApplyJetSmearingAndUncertainties(jets, {}, GenJetViewCollection(), false,
                                     rho);
    return;
  }

  GenJetViewCollection genjetviews = GetAllGenJetViews();
  const auto &matchedGenIdx = MatchJetsToGenJets(jets, genjetviews, rho);
  gRandom->SetSeed(static_cast<int>(PuppiMET_pt * 1e6));

  ApplyJetSmearingAndUncertainties(jets, matchedGenIdx, genjetviews, true, rho);
}

std::vector<std::size_t> AnalyzerCore::SelectJetIndices(
    const JetViewCollection &jets, const std::vector<std::size_t> &seed_indices,
    const JetView::JetID id, const float ptmin, const float fetamax,
    const MyCorrection::variation &jesVar,
    const MyCorrection::variation &jerVar) const {
  std::vector<std::size_t> selected;
  if (jets.empty() || seed_indices.empty())
    return selected;

  selected.reserve(seed_indices.size());
  const float etaMax2 = fetamax * fetamax;

  const bool isData = IsDATA;
  const bool jesIsNom = (jesVar == MyCorrection::variation::nom);
  const bool jerIsNom = (jerVar == MyCorrection::variation::nom);

  // 둘 다 non-nominal은 아직 지원 안 한다
  if (!jesIsNom && !jerIsNom) {
    throw std::runtime_error("[AnalyzerCore::SelectJetIndices] Both JES and "
                             "JER are non-nominal, not supported");
  }

// 벡터라이저에게 힌트
#pragma GCC ivdep
  for (std::size_t k = 0; k < seed_indices.size(); ++k) {
    const auto idx = seed_indices[k];
    const auto &jetView = jets[idx];
    if (!jetView.valid())
      continue;

    float jetPt = 0.f;
    if (isData) {
      jetPt = jetView.CorrectedPt();
    } else {
      if (jerIsNom) {
        if (jesIsNom) {
          jetPt = jetView.SmearedPtNominal();
        } else if (jesVar == MyCorrection::variation::up) {
          jetPt = jetView.JesPtUp();
        } else if (jesVar == MyCorrection::variation::down) {
          jetPt = jetView.JesPtDown();
        } else {
          continue;
        }
      } else {
        if (jerVar == MyCorrection::variation::up) {
          jetPt = jetView.SmearedPtUp();
        } else if (jerVar == MyCorrection::variation::down) {
          jetPt = jetView.SmearedPtDown();
        } else {
          continue;
        }
      }
    }

    if (jetPt < ptmin)
      continue;

    const float eta = jetView.Eta();
    if (eta * eta > etaMax2)
      continue;

    if (!myCorr->PassJetID(jetView, id))
      continue;

    selected.push_back(idx);
  }

  return selected;
}

std::vector<std::size_t> AnalyzerCore::SelectJetIndices(
    const JetViewCollection &jets, const JetView::JetID ID, const float ptmin,
    const float fetamax, const MyCorrection::variation &JESVariation,
    const MyCorrection::variation &JERVariation) const {
  auto seed_indices = AllIndices(jets);
  return SelectJetIndices(jets, seed_indices, ID, ptmin, fetamax, JESVariation,
                          JERVariation);
}

FatJetViewCollection
AnalyzerCore::SelectFatJets(const FatJetViewCollection &fatjets,
                            FatJetView::ID id, float ptmin,
                            float fetamax) const {
  std::vector<std::size_t> selected;
  selected.reserve(fatjets.size());
  for (std::size_t index = 0; index < fatjets.size(); ++index) {
    const auto fatjet = fatjets[index];
    if (fatjet.Pt() < ptmin)
      continue;
    if (fabs(fatjet.Eta()) > fetamax)
      continue;
    if (!myCorr->PassFatJetID(fatjet, id))
      continue;
    selected.push_back(fatjets.rawIndex(index));
  }
  return FatJetViewCollection(fatjets.storage(), std::move(selected));
}

std::vector<std::size_t> AnalyzerCore::JetsVetoLeptonInside(
    const JetViewCollection &jets, const std::vector<std::size_t> &jet_indices,
    const ElectronViewCollection &electrons,
    const std::vector<std::size_t> &electron_indices,
    const MuonViewCollection &muons,
    const std::vector<std::size_t> &muon_indices, const float dR) const {
  if (jets.empty() || jet_indices.empty())
    return {};

  // A stale index is a caller bug, but this has always dropped them quietly
  // rather than throwing, so keep doing that.
  std::vector<std::size_t> seed;
  seed.reserve(jet_indices.size());
  for (const std::size_t idx : jet_indices)
    if (idx < jets.size())
      seed.push_back(idx);

  return RemoveOverlapIndices(
      jets,
      RemoveOverlapIndices(jets, seed, electrons, electron_indices, dR),
      muons, muon_indices, dR);
}

bool AnalyzerCore::PassJetVetoMap(const JetViewCollection &AllJets,
                                  const TString mapCategory) {
  // https://cms-jerc.web.cern.ch/Recommendations/#jet-veto-maps
  if (!(Run == 3))
    throw std::runtime_error(
        "[AnalyzerCore::PassJetVetoMap] vetoeing event only supported in Run 3");
  std::vector<size_t> this_jet_indices =
      SelectJetIndices(AllJets, JetView::JetID::TIGHTLEPVETO, 15., 5.0);

  for (const auto idx : this_jet_indices) {
    if (idx >= AllJets.size())
      throw std::runtime_error(
          "[AnalyzerCore::PassJetVetoMap] Jet index out of range");

    const auto &jet = AllJets[idx]; // JetView
    if (jet.ChEmEF() + jet.NeEmEF() > 0.9)
      continue;
    if (myCorr->IsJetVetoZone(jet.Eta(), jet.Phi(), mapCategory)) {
      return false;
    }
  }
  return true;
}
