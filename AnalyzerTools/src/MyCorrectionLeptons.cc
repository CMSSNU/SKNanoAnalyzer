#include "MyCorrection.h"
#include "MuonScaRe.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "TRandom3.h"

MyCorrection::MuonScaleAndError MyCorrection::GetMuonScaleAndError(
    int charge, float pt, float eta, float phi, int trackerLayers,
    const float matched_pt) const {
  if (!roccorLoaded)
    throw SKNano::ConfigError("[MyCorrection::GetMuonScaleAndError] RoccoR is "
                              "not loaded for this era");
  float roccor = 1.;
  float roccor_err = 0.;

  // few GeVs of muon shuold not use this correction, because the authors did
  // not consider the radiations of low pt muons inside detectors still true for
  // Run3?
  if (pt < 10.)
    return {1.f, 0.f};

  if (IsDATA) {
    roccor =
        rc.kScaleDT(charge, pt, eta, phi, 0, 0);
    roccor_err =
        rc.kScaleDTerror(charge, pt, eta, phi);
  } else {
    // Random seed is initialized in SKNanoLoader::Init()
    gRandom->SetSeed(int(pt / eta));
    float u = gRandom->Rndm();
    if (matched_pt > 0) { // matched
      roccor = rc.kSpreadMC(charge, pt, eta, phi, matched_pt, 0, 0);
      roccor_err = rc.kSpreadMCerror(charge, pt, eta, phi, matched_pt);
    } else {
      // roccor = rc.kScaleMC(muon.Charge(), muon.Pt(), muon.Eta(), muon.Phi(),
      // 0, 0); roccor_err = 0.; roccor_err = rc.kScaleMCerror(muon.Charge(),
      // muon.Pt(), muon.Eta(), muon.Phi());
      roccor = rc.kSmearMC(charge, pt, eta, phi, trackerLayers, u, 0, 0);
      roccor_err = rc.kSmearMCerror(charge, pt, eta, phi, trackerLayers, u);
    }
  }

  return {roccor, roccor_err};
}

// ---------------------------------------------------------------------------
// MUO POG scale and resolution correction (MuonScaRe).
//
// Scale, data and simulation alike, corrects to the generator-level momentum:
//     1/pt  ->  M(eta,phi)/pt + q * A(eta,phi)
// with M multiplicative (magnetic field) and A additive and charge-odd
// (alignment).  Simulation is then smeared on top of the scaled pt,
//     pt  ->  pt * (1 + k * sigma(pt) * dsCB)
// where sigma is a quadratic in pt, dsCB a draw from a double-sided Crystal
// Ball and k = sqrt(k_data^2 - k_mc^2) the residual smearing needed to reach
// the data width (0 when simulation is already wider).
//
// The order of operations, the validity window and the uncertainty model
// follow the POG C++ reference (jsonpog-integration examples/MuonScaRe.cc,
// pt_scale / pt_resol / pt_scale_var / pt_resol_var):
//  * the scale applies from 26 GeV up, with no upper bound;
//  * the smearing applies only while the scaled pt is inside [26, 200] GeV;
//  * the scale uncertainty is evaluated on the fully corrected pt at any pt,
//    from the statistical errors of M and A and their correlation, and is
//    applied to simulation only;
//  * the resolution uncertainty shifts k by its statistical error while
//    keeping the same random draw, so the variation is a coherent shift of
//    the nominal rather than an independent re-smearing; a shifted value that
//    fails the sanity check falls back to the scale-only pt.
// The file also carries "syst" variations (mass-binning changes); the POG
// example does not use them and neither does this.
// ---------------------------------------------------------------------------
MuonScaReState MyCorrection::GetMuonScaleSmearing(
    const int charge, const float pt, const float eta, const float phi,
    const int trackerLayers, const unsigned long long eventNumber,
    const unsigned int lumiBlock) const {
  if (!muSS_loaded)
    throwNullCorrection("GetMuonScaleSmearing");

  const double etaD = eta, phiD = phi, ptD = pt;
  const double q = charge;

  // ---- Scale, from the lower bound up (pt_scale) ---------------------------
  double ptScaled = ptD;
  if (ptD >= SKNano::MUON_SCARE_MIN_PT) {
    const correction::Correction::Ref &mRef = IsDATA ? muSS_m_data : muSS_m_mc;
    const correction::Correction::Ref &aRef = IsDATA ? muSS_a_data : muSS_a_mc;
    const double m = safeEvaluate(mRef, "GetMuonScaleSmearing/m", {etaD, phiD, "nom"});
    const double a = safeEvaluate(aRef, "GetMuonScaleSmearing/a", {etaD, phiD, "nom"});
    ptScaled = 1. / (m / ptD + q * a);
    // Not in the POG reference: 1/(m/pt + q*a) has a pole at pt = m/|a|,
    // which the largest 2024 data alignment terms put near 4 TeV.
    if (!SKNano::MuonScaReResultIsSane(ptScaled, ptD))
      ptScaled = ptD;
  }

  MuonScaReState state{ptScaled, ptScaled, 0.};
  if (IsDATA)
    return state;

  // ---- Resolution, simulation only, inside the window (pt_resol) -----------
  // k stays 0 outside the window: there is nothing to vary there.
  const double absEta = std::fabs(etaD);
  if (ptScaled >= SKNano::MUON_SCARE_MIN_PT &&
      ptScaled <= SKNano::MUON_SCARE_MAX_PT) {
    // sigma(pt) from the quadratic, evaluated on the scaled pt.
    const double nL = trackerLayers;
    const double p0 = safeEvaluate(muSS_poly_params, "GetMuonScaleSmearing/poly", {absEta, nL, 0});
    const double p1 = safeEvaluate(muSS_poly_params, "GetMuonScaleSmearing/poly", {absEta, nL, 1});
    const double p2 = safeEvaluate(muSS_poly_params, "GetMuonScaleSmearing/poly", {absEta, nL, 2});
    double sigma = p0 + p1 * ptScaled + p2 * ptScaled * ptScaled;
    if (sigma < 0.)
      sigma = 0.;

    const double kData = safeEvaluate(muSS_k_data, "GetMuonScaleSmearing/k_data", {absEta, "nom"});
    const double kMC = safeEvaluate(muSS_k_mc, "GetMuonScaleSmearing/k_mc", {absEta, "nom"});
    const double k = (kMC < kData) ? std::sqrt(kData * kData - kMC * kMC) : 0.;
    state.k = k;

    // Flat random number from the file's hash generator, then shaped by the
    // Crystal Ball fitted for this |eta| and tracker-layer multiplicity.  The
    // event number is folded to 32 bits the way the POG reference seeds its
    // generator; correctionlib's integer input is 32-bit anyway.
    const int evtSeed = static_cast<int>(static_cast<unsigned int>(eventNumber & 0xFFFFFFFFull));
    const int lumiSeed = static_cast<int>(lumiBlock);
    const double u = safeEvaluate(muSS_random, "GetMuonScaleSmearing/RandomSmearing", {evtSeed, lumiSeed, phiD});
    const SKNano::MuonCrystalBall cb(
        safeEvaluate(muSS_cb_params, "GetMuonScaleSmearing/cb", {absEta, nL, 0}),
        safeEvaluate(muSS_cb_params, "GetMuonScaleSmearing/cb", {absEta, nL, 1}),
        safeEvaluate(muSS_cb_params, "GetMuonScaleSmearing/cb", {absEta, nL, 3}),
        safeEvaluate(muSS_cb_params, "GetMuonScaleSmearing/cb", {absEta, nL, 2}));
    const double candidate = ptScaled * (1. + k * sigma * cb.invcdf(u));
    if (SKNano::MuonScaReResultIsSane(candidate, ptScaled))
      state.correctedPt = candidate;
  }
  return state;
}

MyCorrection::MuonMomentumLanes MyCorrection::GetMuonScaleSmearingVariations(
    const MuonScaReState &state, const int charge, const float eta,
    const float phi) const {
  if (!muSS_loaded)
    throwNullCorrection("GetMuonScaleSmearingVariations");
  MuonMomentumLanes lanes;
  lanes.nominal = static_cast<float>(state.correctedPt);
  // The POG publishes no data-side uncertainty; the nuisance rides on the
  // simulation templates.
  if (IsDATA) {
    lanes.scaleUp = lanes.scaleDown = lanes.resUp = lanes.resDown =
        lanes.nominal;
    return lanes;
  }
  const double etaD = eta, phiD = phi;
  const double q = charge;
  const double ptScaled = state.scaledPt;
  const double ptCorr = state.correctedPt;
  const double k = state.k;

  // ---- Scale uncertainty, on the corrected pt (pt_scale_var) ---------------
  const double statA = safeEvaluate(muSS_a_mc, "GetMuonScaleSmearing/a_stat", {etaD, phiD, "stat"});
  const double statM = safeEvaluate(muSS_m_mc, "GetMuonScaleSmearing/m_stat", {etaD, phiD, "stat"});
  const double rho = safeEvaluate(muSS_m_mc, "GetMuonScaleSmearing/rho_stat", {etaD, phiD, "rho_stat"});
  const double var = statM * statM / (ptCorr * ptCorr) + statA * statA +
                     2. * q * rho * statM / ptCorr * statA;
  const double unc = (var > 0.) ? ptCorr * ptCorr * std::sqrt(var) : 0.;
  lanes.scaleUp = static_cast<float>(ptCorr + unc);
  lanes.scaleDown = static_cast<float>(ptCorr - unc);

  // ---- Resolution uncertainty: shift k, keep the draw (pt_resol_var) -------
  lanes.resUp = lanes.resDown = lanes.nominal;
  if (k != 0.) {
    const double kUnc = safeEvaluate(muSS_k_mc, "GetMuonScaleSmearing/k_unc", {std::fabs(etaD), "stat"});
    const double stdTimesDraw = (ptCorr / ptScaled - 1.) / k;
    const auto shifted = [&](const double kShifted) {
      const double value = ptScaled * (1. + kShifted * stdTimesDraw);
      return static_cast<float>(
          SKNano::MuonScaReResultIsSane(value, ptScaled) ? value : ptScaled);
    };
    lanes.resUp = shifted(k + kUnc);
    lanes.resDown = shifted(k - kUnc);
  }
  return lanes;
}

float MyCorrection::GetMuonRECOSF(const MuonView &muon,
                                  const variation syst) const {
  if (Run == 3)
    return 1.f;
  auto cset = cset_muon->at("NUM_TrackerMuons_DEN_genTracks");
  return safeEvaluate(cset, "GetMuonRECOSF",
                      {muon.Eta(), std::max(40.f, muon.MiniAODPt()),
                       getSystString_MUO(syst)});
}

float MyCorrection::GetMuonHighPtSF(const TString &key, const float eta,
                                    const float pt,
                                    const variation syst) const {
  if (!cset_muon_highpt)
    throwNullCorrection("GetMuonHighPtSF");
  // The maps stop at |eta| = 2.4 and start at 50 GeV; correctionlib throws
  // outside them. The JSON applies abs() to eta itself.
  const float clampedEta =
      std::min(std::fabs(eta), HIGHPT_SF_MAX_ABSETA - 1e-3f);
  const float clampedPt = std::max(pt, HIGHPT_SF_MIN_MOMENTUM);
  auto cset = cset_muon_highpt->at(string(key.Data()));
  return safeEvaluate(cset, "GetMuonHighPtSF",
                      {clampedEta, clampedPt, getSystString_MUO(syst)});
}

float MyCorrection::GetMuonHighPtRECOSF(const float eta, const float p,
                                        const variation syst) const {
  if (!cset_muon_highpt)
    throwNullCorrection("GetMuonHighPtRECOSF");
  // Binned in the full momentum p, not pt -- see the POG high-pT twiki.
  const float clampedEta =
      std::min(std::fabs(eta), HIGHPT_SF_MAX_ABSETA - 1e-3f);
  const float clampedP = std::max(p, HIGHPT_SF_MIN_MOMENTUM);
  auto cset = cset_muon_highpt->at("NUM_GlobalMuons_DEN_TrackerMuonProbes");
  return safeEvaluate(cset, "GetMuonHighPtRECOSF",
                      {clampedEta, clampedP, getSystString_MUO(syst)});
}

float MyCorrection::GetMuonRECOSF(const MuonViewCollection &muons,
                                  const variation syst) const {
  float weight = 1.f;
  for (const auto muon : muons)
    weight *= GetMuonRECOSF(muon, syst);
  return weight;
}

float MyCorrection::GetMuonIDSF(const TString &key, const MuonView &muon,
                                const variation syst) const {
  const auto &cset = cachedRefByKey(
      cachedMuonIDSF, cset_muon,
      std::string_view(key.Data(), static_cast<std::size_t>(key.Length())));
  return safeEvaluate(
      cset, "GetMuonIDSF",
      {fabs(muon.Eta()), muon.MiniAODPt(), getSystString_MUO(syst)});
}

float MyCorrection::GetMuonIDSF(
    const TString &key, const MuonViewCollection &muons,
    const std::vector<std::size_t> &indices, const variation syst) const {
  float weight = 1.f;
  for (const std::size_t index : indices) {
    if (index >= muons.size())
      throw SKNano::LogicError("[MyCorrection::GetMuonIDSF] index out of range");
    weight *= GetMuonIDSF(key, muons[index], syst);
  }
  return weight;
}

float MyCorrection::GetMuonIDSF(const TString &key,
                                const MuonViewCollection &muons,
                                const variation syst) const {
  float weight = 1.f;
  for (const auto muon : muons)
    weight *= GetMuonIDSF(key, muon, syst);
  return weight;
}

// Electron
// For Run2, scale uncertainty is not stored in the NanoAODv9.
// Should patch from https://github.com/cms-egamma/ScaleFactorsJSON
// https://twiki.cern.ch/twiki/bin/view/CMS/EgammSFandSSRun3
float MyCorrection::GetElectronScaleUnc(const float scEta,
                                        const unsigned char seedGain,
                                        const unsigned int runNumber,
                                        const float r9, const float pt,
                                        const variation syst) const {
  if (IsDATA)
    return 1.0;
  if (!cset_electron_variation)
    throwNullCorrection("GetElectronScaleUnc");

  switch (Run) {
  case 2: {
    if (syst == variation::nom)
      return 1.;
    auto cset = cset_electron_variation->at("UL-EGM_ScaleUnc");
    vector<correction::Variable::Type> args = {
        GetEra().Data(), getSystString_EGMScale(syst), scEta,
        static_cast<int>(seedGain)};
    return safeEvaluate(cset, "GetElectronScaleSF", args);
  }
  case 3: {
    if (syst == variation::nom)
      return 1.;
    // The EtDependent JSON keeps the scale uncertainties in the SmearAndSyst
    // correction, keyed by syst, and they are already multiplicative factors
    // (scale_up = 1 + escale). The older schema exposed them through the scale
    // correction under "total_uncertainty", which this file does not have.
    static_cast<void>(seedGain);
    static_cast<void>(runNumber);
    auto cset = cset_electron_variation->at(EGM_smear_syst_key);
    const string systKey = (syst == variation::up) ? "scale_up" : "scale_down";
    // Inputs are (syst, pt, r9, ScEta) -- signed eta, not folded.
    return safeEvaluate(cset, "GetElectronScaleUnc",
                        {systKey, pt, r9, scEta});
  }
  default:
    throw runtime_error(
        "[MyCorrection::GetElectronScaleUnc] Invalid run number");
  }

  // This should never be reached, but added to avoid compiler warning
  return 1.0;
}

float MyCorrection::GetElectronScaleCorr(const float scEta,
                                         const unsigned char seedGain,
                                         const unsigned int runNumber,
                                         const float r9,
                                         const float pt) const {
  // The scale is a data-only correction; simulation carries the uncertainty
  // instead (GetElectronScaleUnc).
  if (!IsDATA)
    return 1.f;
  // Run 2 NanoAOD already has the EGM calibration applied, and its JSON ships
  // uncertainties only -- reapplying here would double count.
  if (Run != 3)
    return 1.f;
  if (!cset_electron_variation)
    throwNullCorrection("GetElectronScaleCorr");

  // seedGain is a binned axis (edges 0/5/10/15) with clamp flow, not a
  // category, so gains 1, 6 and 12 land in their own bins and anything else
  // clamps rather than throwing. No guard needed.
  //
  // Compound correction stacking EGMScaleVsRun, EleEtaR9, EleFineEtaR9, ElePT,
  // EleGain and ElePTsplit. Inputs are (syst, run, ScEta, r9, pt, seedGain) in
  // that order, with signed eta throughout.
  auto cset = cset_electron_variation->compound().at(EGM_scale_compound_key);
  return safeEvaluate(cset, "GetElectronScaleCorr",
                      {"scale", static_cast<float>(runNumber), scEta, r9, pt,
                       static_cast<float>(seedGain)});
}

float MyCorrection::GetElectronSmearWidth(const float pt, const float r9,
                                          const float scEta,
                                          const variation syst) const {
  // Smearing applies to simulation only.
  if (IsDATA)
    return 0.f;
  if (Run != 3)
    return 0.f;
  if (!cset_electron_variation)
    throwNullCorrection("GetElectronSmearWidth");

  const string systKey = (syst == variation::up)     ? "smear_up"
                         : (syst == variation::down) ? "smear_down"
                                                     : "smear";
  // Inputs are (syst, pt, r9, ScEta) -- signed eta, not folded.
  auto cset = cset_electron_variation->at(EGM_smear_syst_key);
  const float width = safeEvaluate(cset, "GetElectronSmearWidth",
                                   {systKey, pt, r9, scEta});
  // smear_down can reach zero; a negative width would be meaningless.
  return std::max(width, 0.f);
}

float MyCorrection::GetElectronRECOSF(const float eta, const float pt,
                                      const float phi,
                                      const variation syst) const {
  switch (Run) {
  case 2:
    if (pt < 20.)
      return GetElectronIDSF("RecoBelow20", eta, pt, phi, syst);
    else
      return GetElectronIDSF("RecoAbove20", eta, pt, phi, syst);
  case 3: {
    // EGM_era_prompt is precomputed; concatenating it here allocated a string
    // for every electron.
    const auto &cset = cachedElectronIDSF.get(cset_electron, "Electron-ID-SF");
    const float clampedPt = std::max(pt, ELECTRON_SF_MIN_PT);
    const char *ptRange = clampedPt < 20.f
                              ? "RecoBelow20"
                              : (clampedPt < 75.f ? "Reco20to75" : "RecoAbove75");
    return safeEvaluate(cset, "GetElectronRECOSF",
                        {EGM_era_prompt, getSystString_EGM(syst), ptRange, eta,
                         clampedPt});
  }
  default:
    throw runtime_error("[MyCorrection::GetElectronRECOSF] Invalid run number");
  }
}

float MyCorrection::GetElectronRECOSF(const ElectronView &electron,
                                      const variation syst) const {
  // Uncalibrated pt, as the muon SFs read MiniAODPt(): Pt() now carries the
  // EGM scale and smearing, which would move electrons across SF pt bins.
  return GetElectronRECOSF(electron.ScEta(), electron.MiniAODPt(),
                           electron.Phi(), syst);
}

float MyCorrection::GetElectronRECOSF(
    const ElectronViewCollection &electrons, const variation syst) const {
  float weight = 1.f;
  for (const auto electron : electrons)
    weight *= GetElectronRECOSF(electron, syst);
  return weight;
}

float MyCorrection::GetElectronIDSF(const TString &Electron_ID_SF_Key,
                                    const float eta, const float pt,
                                    const float phi,
                                    const variation syst) const {
  const char *setKey = nullptr;
  if (Run == 2)
    setKey = "UL-Electron-ID-SF";
  else if (Run == 3)
    setKey = "Electron-ID-SF";
  else
    throw runtime_error("[MyCorrection::GetElectronIDSF] Invalid run number");

  const auto &cset = cachedElectronIDSF.get(cset_electron, setKey);
  return safeEvaluate(cset, "GetElectronIDSF",
                      {EGM_era_prompt, getSystString_EGM(syst),
                       Electron_ID_SF_Key.Data(),
                       eta, std::clamp(pt, ELECTRON_SF_MIN_PT, 999.9f)});
}

float MyCorrection::GetElectronHEEPIDSF(const float scEta, const float pt,
                                        const variation syst) const {
  if (!cset_electron_heep)
    throwNullCorrection("GetElectronHEEPIDSF");
  // Same "Electron-ID-SF" layout as the main file, with the HEEPID working
  // point added; only the Run 3 spelling exists on the heep-id branch.
  const auto &cset = cachedElectronHEEPIDSF.get(cset_electron_heep, "Electron-ID-SF");
  // The pt axis starts at 35 GeV and ends at infinity, and the flow is
  // "error": below the edge correctionlib throws rather than clamping.
  const float clampedPt = std::max(pt, HEEP_SF_MIN_PT);
  return safeEvaluate(cset, "GetElectronHEEPIDSF",
                      {EGM_era_prompt, getSystString_EGM(syst), "HEEPID",
                       scEta, clampedPt});
}

float MyCorrection::GetElectronHEEPIDSF(const ElectronView &electron,
                                        const variation syst) const {
  return GetElectronHEEPIDSF(electron.ScEta(), electron.MiniAODPt(), syst);
}

float MyCorrection::GetElectronIDSF(const TString &key,
                                    const ElectronView &electron,
                                    const variation syst) const {
  // Uncalibrated pt, as the muon SFs read MiniAODPt(): Pt() now carries the
  // EGM scale and smearing, which would move electrons across SF pt bins.
  return GetElectronIDSF(key, electron.ScEta(), electron.MiniAODPt(),
                         electron.Phi(), syst);
}

float MyCorrection::GetElectronIDSF(
    const TString &key, const ElectronViewCollection &electrons,
    const variation syst) const {
  float weight = 1.f;
  for (const auto electron : electrons)
    weight *= GetElectronIDSF(key, electron, syst);
  return weight;
}

// Trigger
float MyCorrection::GetMuonTriggerEff(const TString &Muon_Trigger_Eff_Key,
                                      const float eta, const float pt,
                                      const bool isData,
                                      const variation syst) const {
  static bool warned_missing_trig_eff = false;
  if (!cset_muon_trig_eff) {
    if (!warned_missing_trig_eff) {
      cerr << "[MyCorrection::GetMuonTriggerEff] Warning: trigger efficiency "
              "correction set is not loaded, returning 1."
           << endl;
      warned_missing_trig_eff = true;
    }
    return 1.;
  }

  correction::Correction::Ref cset;
  try {
    cset = cachedRefByKey(cachedMuonTriggerEff, cset_muon_trig_eff,
                          std::string_view(Muon_Trigger_Eff_Key.Data(),
                                           static_cast<std::size_t>(
                                               Muon_Trigger_Eff_Key.Length())));
  } catch (const std::out_of_range &e) {
    if (!warned_missing_trig_eff) {
      cerr << "[MyCorrection::GetMuonTriggerEff] Warning: key "
           << Muon_Trigger_Eff_Key
           << " not found in trigger efficiency set, returning 1. (" << e.what()
           << ")" << endl;
      warned_missing_trig_eff = true;
    }
    return 1.;
  }
  if (isData)
    return safeEvaluate(cset, "GetTriggerEff",
                        {"data", getSystString_MUO(syst), eta, pt});
  else
    return safeEvaluate(cset, "GetTriggerEff",
                        {"mc", getSystString_MUO(syst), eta, pt});
}

// The efficiency map is binned in signed eta.
float MyCorrection::GetMuonTriggerEff(const TString &Muon_Trigger_Eff_Key,
                                      const MuonView &muon, const bool isData,
                                      const variation syst) const {
  return GetMuonTriggerEff(Muon_Trigger_Eff_Key, muon.Eta(), muon.MiniAODPt(),
                           isData, syst);
}

float MyCorrection::GetMuonTriggerSF(const TString &key,
                                     const MuonView &muon,
                                     const variation syst) const {
  if (IsDATA)
    return 1.f;
  float weight = 1.f;
  auto tryEval = [&](const unique_ptr<CorrectionSet> &set) {
    if (!set)
      return false;
    try {
      const auto cset = set->at(key.Data());
      weight = safeEvaluate(cset, "GetMuonTriggerSF",
                            {muon.Eta(), std::max(26.f, muon.MiniAODPt()),
                             getSystString_MUO(syst)});
      return true;
    } catch (const std::out_of_range &) {
      return false;
    }
  };
  if (tryEval(cset_muon_trig_sf) || tryEval(cset_muon))
    return weight;
  return 1.f;
}

float MyCorrection::GetMuonTriggerSF(const TString &key,
                                     const MuonViewCollection &muons,
                                     const variation syst) const {
  if (!cset_muon_trig_eff) {
    float weight = 1.f;
    for (const auto muon : muons)
      weight *= GetMuonTriggerSF(key, muon, syst);
    return weight;
  }

  float failData = 1.f;
  float failMc = 1.f;
  for (const auto muon : muons) {
    failData *= 1.f - GetMuonTriggerEff(key, muon, true, syst);
    failMc *= 1.f - GetMuonTriggerEff(key, muon, false, syst);
  }
  const float passMc = 1.f - failMc;
  return passMc == 0.f ? 1.f : (1.f - failData) / passMc;
}

float MyCorrection::GetElectronTriggerEff(
    const TString &Electron_Trigger_SF_Key, const float eta, const float pt,
    const float phi, const bool isDATA, const variation syst) const {
  const auto &cset =
      isDATA ? cachedElectronHltDataEff.get(cset_electron_hlt,
                                            "Electron-HLT-DataEff")
             : cachedElectronHltMcEff.get(cset_electron_hlt,
                                          "Electron-HLT-McEff");
  // The efficiency corrections spell the variations differently from the SFs.
  const char *ValType = getSystString_CUSTOM(syst);

  // Whether the correction takes phi depends only on the file, so probe it
  // once -- isInputInCorrection allocates a string per declared input.
  int &hasPhi = isDATA ? electronHltDataEffHasPhi : electronHltMcEffHasPhi;
  if (hasPhi < 0)
    hasPhi = isInputInCorrection("phi", cset) ? 1 : 0;

  const float clampedPt = std::max(pt, ELECTRON_HLT_SF_MIN_PT);
  try {
    if (hasPhi == 0)
      return safeEvaluate(cset, "GetTriggerEff",
                          {EGM_era_key, ValType,
                           Electron_Trigger_SF_Key.Data(), eta, clampedPt});
    return safeEvaluate(cset, "GetTriggerEff",
                        {EGM_era_key, ValType, Electron_Trigger_SF_Key.Data(),
                         eta, clampedPt, phi});
  } catch (const exception &e) {
    cerr << "[MyCorrection::GetElectronTriggerEff] " << e.what() << endl;
    throw;
  }
}

float MyCorrection::GetElectronTriggerEff(const TString &Electron_ID_SF_Key,
                                          const ElectronView &electron,
                                          const bool isDATA,
                                          const variation syst) const {
  return GetElectronTriggerEff(Electron_ID_SF_Key, electron.ScEta(),
                               electron.MiniAODPt(), electron.Phi(), isDATA,
                               syst);
}

float MyCorrection::GetElectronTriggerSF(const TString &Electron_Trigger_SF_Key,
                                         const ElectronView &electron,
                                         const variation syst) const {
  return GetElectronTriggerSF(Electron_Trigger_SF_Key, electron.ScEta(),
                              electron.MiniAODPt(), electron.Phi(), syst);
}

float MyCorrection::GetElectronTriggerSF(const TString &Electron_Trigger_SF_Key,
                                         const float eta, const float pt,
                                         const float phi,
                                         const variation syst) const {
  const auto &cset = cachedElectronHltSF.get(cset_electron_hlt,
                                             "Electron-HLT-SF");
  if (electronHltSFHasPhi < 0)
    electronHltSFHasPhi = isInputInCorrection("phi", cset) ? 1 : 0;

  const float clampedPt = std::max(pt, ELECTRON_HLT_SF_MIN_PT);
  try {
    if (electronHltSFHasPhi == 0)
      return safeEvaluate(cset, "GetTriggerEff",
                          {EGM_era_key, getSystString_EGM(syst),
                           Electron_Trigger_SF_Key.Data(), eta, clampedPt});
    return safeEvaluate(cset, "GetTriggerEff",
                        {EGM_era_key, getSystString_EGM(syst),
                         Electron_Trigger_SF_Key.Data(), eta, clampedPt, phi});
  } catch (const exception &e) {
    cerr << "[MyCorrection::GetElectronTriggerSF] " << e.what() << endl;
    throw;
  }
}

// Pileup
float MyCorrection::GetPUWeight(const float nTrueInt, const variation syst,
                                const TString &source) const {
  // nota bene: Input should be nTrueInt, not nPileUp
  static_cast<void>(source);
  const auto &cset = cachedPUWeight.get(cset_puWeights, LUM_era_key.c_str());
  try {
    return safeEvaluate(cset, "GetPUWeight",
                        {nTrueInt, getSystString_LUM(syst)});
  } catch (const exception &e) {
    cerr << "[MyCorrection::GetPUWeight] " << e.what() << endl;
    return 1.;
  }
}

// ---------------------------------------------------------------------------
// Tau identification scale factors (DeepTau 2018v2p5)
//
// Argument order below is fixed by the tau.json.gz input axes:
//   DeepTau2018v2p5VSjet : pt, dm, genmatch, wp, wp_VSe, syst, flag
//   DeepTau2018v2p5VSe   : eta, dm, genmatch, wp, syst
//   DeepTau2018v2p5VSmu  : eta, genmatch, wp, wp_VSe, wp_VSjet, syst
// correctionlib matches positionally, so the order must not be rearranged.
// ---------------------------------------------------------------------------

float MyCorrection::GetTauIDSF_vsJetRaw(const TauView::TauID &id,
                                        const float pt, const int dm,
                                        const int genmatch,
                                        const variation syst,
                                        const TString &flag) const {
  if (IsDATA)
    return 1.f;
  const auto &cset = cachedTauIDSFvsJet.get(cset_tau, "DeepTau2018v2p5VSjet");
  return safeEvaluate(cset, "GetTauIDSF_vsJet",
                      {static_cast<double>(pt), dm, genmatch,
                       string(ToCorrectionString(id.vsJet)),
                       string(ToCorrectionString(id.vsE)),
                       getSystString_TAU(syst), string(flag.Data())});
}

float MyCorrection::GetTauIDSF_vsERaw(const TauView::TauID &id, const float eta,
                                      const int dm, const int genmatch,
                                      const variation syst) const {
  if (IsDATA)
    return 1.f;
  const auto &cset = cachedTauIDSFvsE.get(cset_tau, "DeepTau2018v2p5VSe");
  return safeEvaluate(cset, "GetTauIDSF_vsE",
                      {static_cast<double>(eta), dm, genmatch,
                       string(ToCorrectionString(id.vsE)),
                       getSystString_TAU(syst)});
}

float MyCorrection::GetTauIDSF_vsMuRaw(const TauView::TauID &id,
                                       const float eta, const int genmatch,
                                       const variation syst) const {
  if (IsDATA)
    return 1.f;
  const auto &cset = cachedTauIDSFvsMu.get(cset_tau, "DeepTau2018v2p5VSmu");
  return safeEvaluate(cset, "GetTauIDSF_vsMu",
                      {static_cast<double>(eta), genmatch,
                       string(ToCorrectionString(id.vsMu)),
                       string(ToCorrectionString(id.vsE)),
                       string(ToCorrectionString(id.vsJet)),
                       getSystString_TAU(syst)});
}

float MyCorrection::GetTauIDSF_vsJet(const TauView::TauID &id,
                                     const TauView &tau,
                                     const variation syst) const {
  return GetTauIDSF_vsJetRaw(id, tau.Pt(), tau.DecayMode(), tau.GenPartFlav(),
                             syst);
}

float MyCorrection::GetTauIDSF_vsE(const TauView::TauID &id, const TauView &tau,
                                   const variation syst) const {
  return GetTauIDSF_vsERaw(id, tau.Eta(), tau.DecayMode(), tau.GenPartFlav(),
                           syst);
}

float MyCorrection::GetTauIDSF_vsMu(const TauView::TauID &id,
                                    const TauView &tau,
                                    const variation syst) const {
  return GetTauIDSF_vsMuRaw(id, tau.Eta(), tau.GenPartFlav(), syst);
}

namespace {

// A stale index would read a neighbouring tau and silently mis-weight the
// event, so it is reported rather than clamped.  Matches GetMuonIDSF.
inline void requireTauIndex(const char *where, const std::size_t index,
                            const std::size_t size) {
  if (index >= size)
    throw SKNano::LogicError(string("[MyCorrection::") + where +
                             "] index out of range");
}

} // namespace

float MyCorrection::GetTauIDSF_vsJet(const TauView::TauID &id,
                                     const TauViewCollection &taus,
                                     const std::vector<std::size_t> &indices,
                                     const variation syst) const {
  float weight = 1.f;
  for (const std::size_t index : indices) {
    requireTauIndex("GetTauIDSF_vsJet", index, taus.size());
    weight *= GetTauIDSF_vsJet(id, taus[index], syst);
  }
  return weight;
}

float MyCorrection::GetTauIDSF_vsE(const TauView::TauID &id,
                                   const TauViewCollection &taus,
                                   const std::vector<std::size_t> &indices,
                                   const variation syst) const {
  float weight = 1.f;
  for (const std::size_t index : indices) {
    requireTauIndex("GetTauIDSF_vsE", index, taus.size());
    weight *= GetTauIDSF_vsE(id, taus[index], syst);
  }
  return weight;
}

float MyCorrection::GetTauIDSF_vsMu(const TauView::TauID &id,
                                    const TauViewCollection &taus,
                                    const std::vector<std::size_t> &indices,
                                    const variation syst) const {
  float weight = 1.f;
  for (const std::size_t index : indices) {
    requireTauIndex("GetTauIDSF_vsMu", index, taus.size());
    weight *= GetTauIDSF_vsMu(id, taus[index], syst);
  }
  return weight;
}

// Whole-collection forms, for a collection that is already the selection.
float MyCorrection::GetTauIDSF_vsJet(const TauView::TauID &id,
                                     const TauViewCollection &taus,
                                     const variation syst) const {
  float weight = 1.f;
  for (const auto tau : taus)
    weight *= GetTauIDSF_vsJet(id, tau, syst);
  return weight;
}

float MyCorrection::GetTauIDSF_vsE(const TauView::TauID &id,
                                   const TauViewCollection &taus,
                                   const variation syst) const {
  float weight = 1.f;
  for (const auto tau : taus)
    weight *= GetTauIDSF_vsE(id, tau, syst);
  return weight;
}

float MyCorrection::GetTauIDSF_vsMu(const TauView::TauID &id,
                                    const TauViewCollection &taus,
                                    const variation syst) const {
  float weight = 1.f;
  for (const auto tau : taus)
    weight *= GetTauIDSF_vsMu(id, tau, syst);
  return weight;
}
