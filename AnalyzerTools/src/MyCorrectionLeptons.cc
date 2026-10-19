#include "MyCorrection.h"
#include "MuonScaRe.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "TRandom3.h"

namespace {

// ---------------------------------------------------------------------------
// Generalized Endpoint curvature bias kappa_b, in 1/TeV, with its uncertainty.
//
// Above ~200 GeV the momentum comes largely from the muon system, so the scale
// bias measured at the Z peak from tracker information alone does not describe
// it. The GE method injects additive biases into simulation and picks the one
// that reproduces data in q/pT. Source: MUO POG "High pT: Momentum Scale"
// (methodology in AN-2018/008).
//
// Grid is [phi bin][eta bin]. Phi bins are [-180,-60), [-60,60), [60,180]
// degrees; eta bins are [-2.4,-2.1), [-2.1,-1.2), [-1.2,0), [0,1.2), [1.2,2.1),
// [2.1,2.4]. Eta keeps its sign here -- do not fold to |eta|.
//
// 2024 and 2025 are deliberately absent: those measurements use a different
// binning and are not settled. Eras without a map are left uncorrected rather
// than treated as an error.
// ---------------------------------------------------------------------------
struct GECell {
  float kappa;
  float sigma;
};

// Binning is per era rather than fixed: the published 2022-2023 maps are
// 3 phi x 6 eta, while the 2024/2025 measurement uses a finer grid. Edges are
// the inner boundaries; a value below the first edge lands in bin 0 and one
// above the last edge in the final bin. Cells are indexed [phi][eta].
struct GEMap {
  std::vector<float> phiEdgesDeg; // inner boundaries in degrees
  std::vector<float> etaEdges;    // inner boundaries, signed eta
  std::vector<std::vector<GECell>> cells;

  const GECell *find(const float eta, const float phi) const {
    if (std::fabs(eta) > 2.4f)
      return nullptr;
    const float phiDeg = phi * 180.f / static_cast<float>(M_PI);
    std::size_t iphi = 0;
    while (iphi < phiEdgesDeg.size() && phiDeg >= phiEdgesDeg[iphi])
      ++iphi;
    std::size_t ieta = 0;
    while (ieta < etaEdges.size() && eta >= etaEdges[ieta])
      ++ieta;
    if (iphi >= cells.size() || ieta >= cells[iphi].size())
      return nullptr;
    return &cells[iphi][ieta];
  }
};

const std::vector<float> kGePhiEdges3 = {-60.f, 60.f};
const std::vector<float> kGeEtaEdges6 = {-2.1f, -1.2f, 0.f, 1.2f, 2.1f};

const std::unordered_map<std::string, GEMap> kGeKappa = {
    {"2022",
     {kGePhiEdges3, kGeEtaEdges6,
      {
          {{-0.16f, 0.1f}, {-0.03f, 0.05f}, {-0.05f, 0.04f}, {0.f, 0.04f}, {0.07f, 0.06f}, {-0.06f, 0.11f}},
          {{0.11f, 0.1f}, {-0.01f, 0.06f}, {0.06f, 0.04f}, {0.02f, 0.03f}, {0.05f, 0.05f}, {-0.06f, 0.1f}},
          {{0.17f, 0.11f}, {0.16f, 0.04f}, {-0.04f, 0.04f}, {-0.01f, 0.03f}, {0.04f, 0.06f}, {-0.f, 0.09f}},
      }}},
    {"2022EE",
     {kGePhiEdges3, kGeEtaEdges6,
      {
          {{-0.12f, 0.05f}, {-0.03f, 0.03f}, {0.013f, 0.022f}, {0.029f, 0.023f}, {-0.04f, 0.03f}, {-0.28f, 0.05f}},
          {{0.24f, 0.05f}, {0.1f, 0.03f}, {-0.006f, 0.022f}, {-0.047f, 0.022f}, {-0.14f, 0.03f}, {-0.48f, 0.05f}},
          {{0.28f, 0.05f}, {0.07f, 0.03f}, {-0.028f, 0.022f}, {0.018f, 0.022f}, {0.07f, 0.03f}, {0.05f, 0.05f}},
      }}},
    {"2023",
     {kGePhiEdges3, kGeEtaEdges6,
      {
          {{-0.21f, 0.06f}, {-0.01f, 0.04f}, {0.01f, 0.027f}, {-0.045f, 0.03f}, {0.01f, 0.04f}, {0.f, 0.07f}},
          {{0.08f, 0.07f}, {-0.04f, 0.04f}, {0.07f, 0.025f}, {0.03f, 0.027f}, {-0.13f, 0.04f}, {-0.36f, 0.06f}},
          {{0.27f, 0.07f}, {0.05f, 0.04f}, {0.054f, 0.026f}, {0.02f, 0.027f}, {0.f, 0.04f}, {-0.04f, 0.06f}},
      }}},
    {"2023BPix",
     {kGePhiEdges3, kGeEtaEdges6,
      {
          {{-0.25f, 0.08f}, {0.07f, 0.05f}, {0.02f, 0.04f}, {-0.02f, 0.04f}, {0.08f, 0.06f}, {0.12f, 0.09f}},
          {{-0.05f, 0.08f}, {0.f, 0.05f}, {0.05f, 0.03f}, {-0.05f, 0.04f}, {-0.17f, 0.05f}, {-0.33f, 0.09f}},
          {{0.24f, 0.09f}, {0.09f, 0.05f}, {-0.01f, 0.03f}, {0.03f, 0.04f}, {0.04f, 0.06f}, {-0.2f, 0.07f}},
      }}},
};

// ---------------------------------------------------------------------------
// High-pT muon momentum resolution.
//
// The width is a cubic in the FULL momentum p, not pT: the measurement fits
// (1/p - 1/p_gen)/(1/p_gen) in bins of p. Source: Schulte & Zhong, "High-pT
// muon resolution measurement for 2022 and 2023" (2024-06-03).
// Index [0] = barrel |eta| < 1.2, [1] = forward 1.2 - 2.4.
//
// 2024 is absent: the POG has that measurement in progress and currently asks
// analyses to evaluate the non-closure in their own phase space. With no entry
// the nominal smearing is a no-op and only the uncertainty remains.
// ---------------------------------------------------------------------------
struct MuonResPoly {
  float a0, a1, a2, a3;
};
using MuonResSet = std::array<MuonResPoly, 2>;

const std::unordered_map<std::string, MuonResSet> kMuonResPoly = {
    {"2022",
     {{{0.01152f, 5.95e-5f, -2.92e-8f, 5.14e-12f},
       {0.01405f, 5.28e-5f, -1.90e-8f, 3.01e-12f}}}},
    {"2022EE",
     {{{0.0126f, 5.89e-5f, -2.85e-8f, 4.92e-12f},
       {0.0150f, 4.81e-5f, -1.42e-8f, 1.95e-12f}}}},
    {"2023",
     {{{0.0172f, 6.15e-5f, -3.14e-8f, 5.82e-12f},
       {0.01424f, 5.31e-5f, -1.92e-8f, 3.21e-12f}}}},
    {"2023BPix",
     {{{0.0118f, 6.14e-5f, -3.12e-8f, 5.74e-12f},
       {0.0141f, 5.38e-5f, -2.00e-8f, 3.42e-12f}}}},
};

// Extra smearing strength f = sqrt(smearfac^2 - 1): 10% -> 0.458, 5% -> 0.320.
// f = 0 means simulation already matches data in that era and eta region.
const std::unordered_map<std::string, std::array<float, 2>> kMuonSmearF = {
    {"2022", {{0.000f, 0.320f}}},
    {"2022EE", {{0.320f, 0.460f}}},
    {"2023", {{0.320f, 0.460f}}},
    {"2023BPix", {{0.000f, 0.568f}}},
};

// The systematic is a flat ten percent regardless of era and eta.
constexpr float kMuonSmearSystF = 0.4583f;

int MuonResEtaBin(const float eta) { return (std::fabs(eta) < 1.2f) ? 0 : 1; }

} // namespace

MyCorrection::MuonScaleAndError MyCorrection::GetMuonScaleAndError(
    int charge, float pt, float eta, float phi, int trackerLayers,
    const float matched_pt) const {
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
// The order of operations and the uncertainty model follow the POG reference
// (jsonpog-integration examples/MuonScaRe.cc, muoScaleAndSmearingRDFExample):
//  * the scale uncertainty is evaluated on the fully corrected pt, from the
//    statistical errors of M and A and their correlation, and is applied to
//    simulation only;
//  * the resolution uncertainty shifts k by its statistical error while
//    keeping the same random draw, so the variation is a coherent shift of
//    the nominal rather than an independent re-smearing.
// The file also carries "syst" variations (mass-binning changes); the POG
// example does not use them and neither does this.
// ---------------------------------------------------------------------------
MyCorrection::MuonMomentumLanes MyCorrection::GetMuonScaleSmearing(
    const int charge, const float pt, const float eta, const float phi,
    const int trackerLayers, const unsigned long long eventNumber,
    const unsigned int lumiBlock, const bool withVariations) const {
  MuonMomentumLanes lanes{pt, pt, pt, pt, pt};
  if (!muSS_loaded)
    throwNullCorrection("GetMuonScaleSmearing");
  if (!(pt > SKNano::MUON_SCARE_MIN_PT) || !(pt < SKNano::MUON_SCARE_MAX_PT))
    return lanes;

  const double etaD = eta, phiD = phi, ptD = pt;
  const double q = charge;

  // ---- Scale ------------------------------------------------------------
  const correction::Correction::Ref &mRef = IsDATA ? muSS_m_data : muSS_m_mc;
  const correction::Correction::Ref &aRef = IsDATA ? muSS_a_data : muSS_a_mc;
  const double m = safeEvaluate(mRef, "GetMuonScaleSmearing/m", {etaD, phiD, "nom"});
  const double a = safeEvaluate(aRef, "GetMuonScaleSmearing/a", {etaD, phiD, "nom"});
  double ptScaled = 1. / (m / ptD + q * a);
  if (!SKNano::MuonScaReResultIsSane(ptScaled, ptD))
    ptScaled = ptD;

  if (IsDATA) {
    lanes.nominal = lanes.scaleUp = lanes.scaleDown = lanes.resUp =
        lanes.resDown = static_cast<float>(ptScaled);
    return lanes;
  }

  // ---- Resolution (simulation only) -------------------------------------
  // sigma(pt) from the quadratic, evaluated on the scaled pt as the POG does.
  const double nL = trackerLayers;
  const double absEta = std::fabs(etaD);
  const double p0 = safeEvaluate(muSS_poly_params, "GetMuonScaleSmearing/poly", {absEta, nL, 0});
  const double p1 = safeEvaluate(muSS_poly_params, "GetMuonScaleSmearing/poly", {absEta, nL, 1});
  const double p2 = safeEvaluate(muSS_poly_params, "GetMuonScaleSmearing/poly", {absEta, nL, 2});
  double sigma = p0 + p1 * ptScaled + p2 * ptScaled * ptScaled;
  if (sigma < 0.)
    sigma = 0.;

  const double kData = safeEvaluate(muSS_k_data, "GetMuonScaleSmearing/k_data", {absEta, "nom"});
  const double kMC = safeEvaluate(muSS_k_mc, "GetMuonScaleSmearing/k_mc", {absEta, "nom"});
  const double k = (kMC < kData) ? std::sqrt(kData * kData - kMC * kMC) : 0.;

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
  const double draw = cb.invcdf(u);

  // The POG window applies to the input of the smearing step as well.
  const bool smearable = ptScaled > SKNano::MUON_SCARE_MIN_PT &&
                         ptScaled < SKNano::MUON_SCARE_MAX_PT &&
                         std::isfinite(draw);
  double ptCorr = ptScaled;
  if (smearable) {
    const double candidate = ptScaled * (1. + k * sigma * draw);
    if (SKNano::MuonScaReResultIsSane(candidate, ptScaled))
      ptCorr = candidate;
  }
  lanes.nominal = static_cast<float>(ptCorr);
  if (!withVariations) {
    lanes.scaleUp = lanes.scaleDown = lanes.resUp = lanes.resDown =
        lanes.nominal;
    return lanes;
  }

  // ---- Scale uncertainty, on the corrected pt ----------------------------
  const double statA = safeEvaluate(muSS_a_mc, "GetMuonScaleSmearing/a_stat", {etaD, phiD, "stat"});
  const double statM = safeEvaluate(muSS_m_mc, "GetMuonScaleSmearing/m_stat", {etaD, phiD, "stat"});
  const double rho = safeEvaluate(muSS_m_mc, "GetMuonScaleSmearing/rho_stat", {etaD, phiD, "rho_stat"});
  const double var = statM * statM / (ptCorr * ptCorr) + statA * statA +
                     2. * q * rho * statM / ptCorr * statA;
  const double unc = (var > 0.) ? ptCorr * ptCorr * std::sqrt(var) : 0.;
  lanes.scaleUp = static_cast<float>(ptCorr + unc);
  lanes.scaleDown = static_cast<float>(ptCorr - unc);

  // ---- Resolution uncertainty: shift k, keep the draw --------------------
  lanes.resUp = lanes.resDown = lanes.nominal;
  if (k > 0. && smearable && ptCorr != ptScaled) {
    const double kUnc = safeEvaluate(muSS_k_mc, "GetMuonScaleSmearing/k_unc", {absEta, "stat"});
    const double pull = sigma * draw; // == (ptCorr/ptScaled - 1)/k
    const double up = ptScaled * (1. + (k + kUnc) * pull);
    const double down = ptScaled * (1. + (k - kUnc) * pull);
    if (SKNano::MuonScaReResultIsSane(up, ptScaled))
      lanes.resUp = static_cast<float>(up);
    if (SKNano::MuonScaReResultIsSane(down, ptScaled))
      lanes.resDown = static_cast<float>(down);
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
    return 1.f;
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
    return 1.f;
  // Binned in the full momentum p, not pt -- see the POG high-pT twiki.
  const float clampedEta =
      std::min(std::fabs(eta), HIGHPT_SF_MAX_ABSETA - 1e-3f);
  const float clampedP = std::max(p, HIGHPT_SF_MIN_MOMENTUM);
  auto cset = cset_muon_highpt->at("NUM_GlobalMuons_DEN_TrackerMuonProbes");
  return safeEvaluate(cset, "GetMuonHighPtRECOSF",
                      {clampedEta, clampedP, getSystString_MUO(syst)});
}

float MyCorrection::GetMuonGEScaledPt(const float pt, const float eta,
                                      const float phi, const int charge,
                                      const variation syst) const {
  const auto it = kGeKappa.find(GetEra().Data());
  if (it == kGeKappa.end())
    return pt; // No map for this era (Run 2, 2024+); leave the momentum alone.

  const GECell *cell = it->second.find(eta, phi);
  if (!cell)
    return pt;

  float kappa = cell->kappa;
  if (syst == variation::up)
    kappa += cell->sigma;
  else if (syst == variation::down)
    kappa -= cell->sigma;

  // k_meas = k_true + kappa  =>  pT_corr = pT / (1 - q * kappa * pT[TeV])
  const float ptTeV = pt / 1000.f;
  const float denom = 1.f - static_cast<float>(charge) * kappa * ptTeV;

  // Guard the pole: kappa*pT approaching 1 blows the correction up, which a
  // multi-TeV muon can reach. Emitting the uncorrected momentum beats emitting
  // a runaway one.
  if (std::fabs(denom) < 0.1f) {
    std::cerr << "[MyCorrection::GetMuonGEScaledPt] Near-singular GE correction"
              << " (denom = " << denom << ", kappa = " << kappa
              << " TeV^-1, pT = " << pt << " GeV, q = " << charge
              << "); keeping the uncorrected pT." << std::endl;
    return pt;
  }
  return pt / denom;
}

float MyCorrection::GetMuonGESigmaShiftedPt(const float pt, const float eta,
                                            const float phi, const int charge,
                                            const variation syst) const {
  if (syst == variation::nom)
    return pt;

  const auto it = kGeKappa.find(GetEra().Data());
  if (it == kGeKappa.end())
    return pt;
  const GECell *cell = it->second.find(eta, phi);
  if (!cell)
    return pt;

  // Centred on zero: the shift is the uncertainty on kappa, not kappa itself.
  const float sigma = cell->sigma;
  const float kappa = (syst == variation::up) ? sigma : -sigma;

  const float ptTeV = pt / 1000.f;
  const float denom = 1.f - static_cast<float>(charge) * kappa * ptTeV;
  if (std::fabs(denom) < 0.1f) {
    std::cerr << "[MyCorrection::GetMuonGESigmaShiftedPt] Near-singular shift"
              << " (denom = " << denom << ", sigma = " << sigma
              << " TeV^-1, pT = " << pt << " GeV, q = " << charge
              << "); keeping the unshifted pT." << std::endl;
    return pt;
  }
  return pt / denom;
}

float MyCorrection::GetMuonHighPtResolution(const float p,
                                            const float eta) const {
  const auto it = kMuonResPoly.find(GetEra().Data());
  if (it == kMuonResPoly.end())
    return 0.f;
  const MuonResPoly &c = it->second[MuonResEtaBin(eta)];
  return c.a0 + c.a1 * p + c.a2 * p * p + c.a3 * p * p * p;
}

float MyCorrection::GetMuonHighPtSmearFactor(const float p, const float eta,
                                             const unsigned int seed,
                                             const variation syst) const {
  if (IsDATA)
    return 1.f;
  const auto itF = kMuonSmearF.find(GetEra().Data());
  if (itF == kMuonSmearF.end())
    return 1.f; // No map for this era (Run 2, 2024+); no extra smearing.

  const float fNom = itF->second[MuonResEtaBin(eta)];
  float f = fNom;
  if (syst == variation::up) {
    // Independent Gaussians, so the widths add in quadrature.
    f = std::sqrt(fNom * fNom + kMuonSmearSystF * kMuonSmearSystF);
  } else if (syst == variation::down) {
    // Convolution can only widen a resolution, so this floors at zero, which
    // makes the nuisance one-sided wherever the nominal smearing is already 0.
    f = std::sqrt(
        std::max(fNom * fNom - kMuonSmearSystF * kMuonSmearSystF, 0.f));
  }
  if (f <= 0.f)
    return 1.f;

  const float sigma = GetMuonHighPtResolution(p, eta);
  if (sigma <= 0.f)
    return 1.f;

  // Nominal and variations share the seed so they share the Gaussian pull:
  // the variation is then a coherent shift rather than an independent
  // re-smearing, which would pile resolution on top of the nominal.
  TRandom3 rng(seed);
  return 1.f + rng.Gaus(0.f, sigma * f);
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
  // Eras whose EGM scale-and-smearing file is absent get no variation rather
  // than a null dereference.
  if (!cset_electron_variation)
    return 1.0;

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
    return 1.f;

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
    return 0.f;

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
    const char *ptRange =
        pt < 20.f ? "RecoBelow20" : (pt < 75.f ? "Reco20to75" : "RecoAbove75");
    return safeEvaluate(cset, "GetElectronRECOSF",
                        {EGM_era_prompt, getSystString_EGM(syst), ptRange, eta,
                         pt});
  }
  default:
    throw runtime_error("[MyCorrection::GetElectronRECOSF] Invalid run number");
  }
}

float MyCorrection::GetElectronRECOSF(const ElectronView &electron,
                                      const variation syst) const {
  // Uncalibrated pt, as the muon SFs read MiniAODPt(): Pt() now carries the
  // EGM scale and smearing, which would move electrons across SF pt bins.
  return GetElectronRECOSF(std::fabs(electron.Eta()), electron.MiniAODPt(),
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
                       eta, pt < 999.9f ? pt : 999.9f});
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

float MyCorrection::GetElectronIDSF(const TString &key,
                                    const ElectronView &electron,
                                    const variation syst) const {
  // Uncalibrated pt, as the muon SFs read MiniAODPt(): Pt() now carries the
  // EGM scale and smearing, which would move electrons across SF pt bins.
  return GetElectronIDSF(key, std::fabs(electron.Eta()), electron.MiniAODPt(),
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
    failData *= 1.f - GetMuonTriggerEff(
        key, std::fabs(muon.Eta()), muon.Pt(), true, syst);
    failMc *= 1.f - GetMuonTriggerEff(
        key, std::fabs(muon.Eta()), muon.Pt(), false, syst);
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

  try {
    if (hasPhi == 0)
      return safeEvaluate(cset, "GetTriggerEff",
                          {EGM_era_key, ValType,
                           Electron_Trigger_SF_Key.Data(), eta, pt});
    return safeEvaluate(cset, "GetTriggerEff",
                        {EGM_era_key, ValType, Electron_Trigger_SF_Key.Data(),
                         eta, pt, phi});
  } catch (const exception &e) {
    cerr << "[MyCorrection::GetElectronTriggerEff] " << e.what() << endl;
    throw;
  }
}

float MyCorrection::GetElectronTriggerSF(const TString &Electron_Trigger_SF_Key,
                                         const float eta, const float pt,
                                         const float phi,
                                         const variation syst) const {
  const auto &cset = cachedElectronHltSF.get(cset_electron_hlt,
                                             "Electron-HLT-SF");
  if (electronHltSFHasPhi < 0)
    electronHltSFHasPhi = isInputInCorrection("phi", cset) ? 1 : 0;

  try {
    if (electronHltSFHasPhi == 0)
      return safeEvaluate(cset, "GetTriggerEff",
                          {EGM_era_key, getSystString_EGM(syst),
                           Electron_Trigger_SF_Key.Data(), eta, pt});
    return safeEvaluate(cset, "GetTriggerEff",
                        {EGM_era_key, getSystString_EGM(syst),
                         Electron_Trigger_SF_Key.Data(), eta, pt, phi});
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
