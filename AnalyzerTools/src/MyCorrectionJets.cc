#include "MyCorrection.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

// JetID

namespace {

// The jet ID corrections mix real and integer inputs, so they cannot go
// through safeEvaluateFloats.  Fill a reused buffer instead of letting the
// braced-init-list allocate a fresh 9-element vector for every jet.
float evaluateJetID(const correction::Correction::Ref &cset, float abseta,
                    float chHEF, float neHEF, float chEmEF, float neEmEF,
                    float muEF, int chMult, int neMult) {
  static thread_local vector<correction::Variable::Type> args(9);
  args[0] = static_cast<double>(abseta);
  args[1] = static_cast<double>(chHEF);
  args[2] = static_cast<double>(neHEF);
  args[3] = static_cast<double>(chEmEF);
  args[4] = static_cast<double>(neEmEF);
  args[5] = static_cast<double>(muEF);
  args[6] = chMult;
  args[7] = neMult;
  args[8] = chMult + neMult;
  return cset->evaluate(args);
}

} // namespace

bool MyCorrection::PassJetID(const JetView &jet, const JetView::JetID &id) const {
  const correction::Correction::Ref *cset = nullptr;
  switch (id) {
  case JetView::JetID::TIGHT:
    cset = &cachedJetIDTight.get(cset_jetid, "AK4PUPPI_Tight");
    break;
  case JetView::JetID::TIGHTLEPVETO:
    cset = &cachedJetIDTightLepVeto.get(cset_jetid, "AK4PUPPI_TightLeptonVeto");
    break;
  case JetView::JetID::NOCUT:
    return true;
  default:
    throw runtime_error("[MyCorrection::PassJetID] Invalid JetID type");
  }
  return evaluateJetID(*cset, fabs(jet.Eta()), jet.ChHEF(), jet.NeHEF(),
                       jet.ChEmEF(), jet.NeEmEF(), jet.MuEF(),
                       static_cast<int>(jet.ChMultiplicity()),
                       static_cast<int>(jet.NeMultiplicity())) > 0.5;
}

bool MyCorrection::PassFatJetID(const FatJetView &fatjet,
                                FatJetView::ID id) const {
  const correction::Correction::Ref *cset = nullptr;
  switch (id) {
  case FatJetView::ID::TIGHT:
    cset = &cachedFatJetIDTight.get(cset_jetid, "AK8PUPPI_Tight");
    break;
  case FatJetView::ID::TIGHTLEPVETO:
    cset =
        &cachedFatJetIDTightLepVeto.get(cset_jetid, "AK8PUPPI_TightLeptonVeto");
    break;
  case FatJetView::ID::NOCUT:
    return true;
  default:
    throw runtime_error("[MyCorrection::PassFatJetID] Invalid JetID type");
  }
  return evaluateJetID(*cset, fabs(fatjet.Eta()), fatjet.chHEF(),
                       fatjet.neHEF(), fatjet.chEmEF(), fatjet.neEmEF(),
                       fatjet.muEF(), fatjet.chMultiplicity(),
                       fatjet.neMultiplicity()) > 0.5;
}

// JERC
float MyCorrection::safeEvaluate2D(const correction::Correction::Ref &cset,
                                   std::string_view function_name, float x,
                                   float y) const {
  if (!cset)
    throwNullCorrection(function_name);

  static thread_local vector<correction::Variable::Type> args(2);
  args[0] = static_cast<double>(x);
  args[1] = static_cast<double>(y);
  try {
    return cset->evaluate(args);
  } catch (const std::exception &e) {
    throwEvaluationError(function_name, e, args);
  }
}

float MyCorrection::safeEvaluate3D(const correction::Correction::Ref &cset,
                                   std::string_view function_name, float x,
                                   float y, float z) const {
  if (!cset)
    throwNullCorrection(function_name);

  static thread_local vector<correction::Variable::Type> args(3);
  args[0] = static_cast<double>(x);
  args[1] = static_cast<double>(y);
  args[2] = static_cast<double>(z);
  try {
    return cset->evaluate(args);
  } catch (const std::exception &e) {
    throwEvaluationError(function_name, e, args);
  }
}

namespace {

// The JERC global tags carry a "######" placeholder that names the correction
// level.  Substituting it is a load-time operation, never a per-jet one.
string substituteJercLevel(const string &global_tag, std::string_view level,
                           const char *context) {
  const auto marker = global_tag.find("######");
  if (marker == string::npos)
    throw SKNano::ConfigError(string("[MyCorrection::") + context +
                              "] JERC global tag '" + global_tag +
                              "' has no ###### placeholder");
  string key = global_tag;
  key.replace(marker, 6, level);
  return key;
}

} // namespace

const correction::Correction::Ref &
MyCorrection::getJERPtResolutionCorrection() const {
  if (!cachedJERPtResolution)
    cachedJERPtResolution = cset_jerc->at(
        substituteJercLevel(JER_global_tag, "PtResolution", "GetJER"));
  return cachedJERPtResolution;
}

const correction::Correction::Ref &
MyCorrection::getJERScaleFactorCorrection() const {
  if (!cachedJERScaleFactor)
    cachedJERScaleFactor = cset_jerc->at(
        substituteJercLevel(JER_global_tag, "ScaleFactor", "GetJERSF"));
  return cachedJERScaleFactor;
}

const correction::Correction::Ref &
MyCorrection::getJERSFUncertaintyCorrection() const {
  if (!cachedJERSFUncertainty)
    cachedJERSFUncertainty = cset_jerc->at(
        substituteJercLevel(JER_global_tag, "SFUncertainty", "GetJERSF"));
  return cachedJERSFUncertainty;
}

const correction::Correction::Ref &
MyCorrection::getJESUncertaintyCorrection(std::string_view source) const {
  // The JSON spells the summed source "Total" (..._V5_MC_Total_AK4PFPuppi);
  // there is no lower-case key, so "total" resolves to nothing and throws.
  // Accept either spelling so the default argument and any lower-case caller
  // land on the same correction.
  std::string_view sourceKey = source.empty() ? "Total" : source;
  if (sourceKey == "total")
    sourceKey = "Total";
  auto it = cachedJESUncertaintyCorrections.find(sourceKey);
  if (it == cachedJESUncertaintyCorrections.end()) {
    const string key = substituteJercLevel(JES_global_tag, sourceKey,
                                           "GetJESUncertainty");
    it = cachedJESUncertaintyCorrections
             .emplace(string(sourceKey), cset_jerc->at(key))
             .first;
  }
  return it->second;
}

float MyCorrection::GetJER(const float eta, const float pt,
                           const float rho) const {
  return safeEvaluate3D(getJERPtResolutionCorrection(), "GetJER", eta, pt,
                        rho);
}

float MyCorrection::GetJERSF(const float eta, const float pt,
                             const variation syst,
                             const TString &source) const {
  const auto &cset = getJERScaleFactorCorrection();
  if (GetEra() == "2024") {
    const float sf = safeEvaluate2D(cset, "GetJERSF", eta, pt);
    if (syst == variation::nom) {
      return sf;
    }

    const float sf_unc =
        safeEvaluate2D(getJERSFUncertaintyCorrection(), "GetJERSFUncertainty",
                       eta, pt);
    if (syst == variation::up) {
      return sf + sf_unc;
    }
    if (syst == variation::down) {
      const float sf_down = sf - sf_unc;
      return sf_down > 0.f ? sf_down : 0.f;
    }
    return sf;
  }
  if (Run == 3) {
    return safeEvaluate(cset, "GetJERSF", {eta, pt, getSystString_JME(syst)});
  } else if (Run == 2) {
    return safeEvaluate(cset, "GetJERSF", {eta, getSystString_JME(syst)});
  }
  return 1.;
}

MyCorrection::JERSFSet MyCorrection::GetJERSFSet(const float eta,
                                                 const float pt,
                                                 const TString &source) const {
  const float sf = GetJERSF(eta, pt, variation::nom, source);
  const auto variations = GetJERSFVariations(eta, pt, sf, source);
  return {sf, variations.up, variations.down};
}

MyCorrection::JERSFVariations
MyCorrection::GetJERSFVariations(const float eta, const float pt,
                                 const float nominal,
                                 const TString &source) const {
  static_cast<void>(source);
  if (GetEra() == "2024") {
    const float sf_unc =
        safeEvaluate2D(getJERSFUncertaintyCorrection(), "GetJERSFUncertainty",
                       eta, pt);
    return {nominal + sf_unc, std::max(nominal - sf_unc, 0.f)};
  }
  const auto &cset = getJERScaleFactorCorrection();
  if (Run == 3) {
    return {safeEvaluate(cset, "GetJERSF",
                         {eta, pt, getSystString_JME(variation::up)}),
            safeEvaluate(cset, "GetJERSF",
                         {eta, pt, getSystString_JME(variation::down)})};
  }
  if (Run == 2) {
    return {safeEvaluate(cset, "GetJERSF",
                         {eta, getSystString_JME(variation::up)}),
            safeEvaluate(cset, "GetJERSF",
                         {eta, getSystString_JME(variation::down)})};
  }
  return {nominal, nominal};
}

// JESC
float MyCorrection::GetJESSF(const float area, const float eta, const float pt,
                             const float phi, const float rho,
                             const unsigned int runNumber) const {
  // The era and the data/MC mode are fixed once the object is constructed, so
  // bind the refs and the input layout on first use and never look at the era
  // string again -- this used to build a std::string per jet.
  if (!preparedJESValid) {
    const auto levelKey = [this](const char *level) {
      return substituteJercLevel(JES_global_tag, level, "GetJESSF");
    };

    if (GetEra() == "2024") {
      preparedJESLayout = JESLayout::Factorized;
      preparedJESL1 = cset_jerc->at(levelKey("L1FastJet"));
      preparedJESL2 = cset_jerc->at(levelKey("L2Relative"));
      preparedJESL3 = cset_jerc->at(levelKey("L3Absolute"));
      if (IsDATA)
        preparedJESResidual = cset_jerc->at(levelKey("L2L3Residual"));
    } else {
      preparedJESCompound = cset_jerc->compound().at(levelKey("L1L2L3Res"));
      if (GetEra() == "2023BPix")
        preparedJESLayout = IsDATA ? JESLayout::CompoundAreaEtaPtRhoPhiRun
                                   : JESLayout::CompoundAreaEtaPtRhoPhi;
      else if (GetEra() == "2023" && IsDATA)
        preparedJESLayout = JESLayout::CompoundAreaEtaPtRhoRun;
      else
        preparedJESLayout = JESLayout::CompoundAreaEtaPtRho;
    }
    preparedJESValid = true;
  }

  switch (preparedJESLayout) {
  case JESLayout::CompoundAreaEtaPtRhoPhiRun:
    return safeEvaluateFloats(
        preparedJESCompound, "GetJESSF",
        std::array<float, 6>{area, eta, pt, rho, phi,
                             static_cast<float>(runNumber)});
  case JESLayout::CompoundAreaEtaPtRhoPhi:
    return safeEvaluateFloats(preparedJESCompound, "GetJESSF",
                              std::array<float, 5>{area, eta, pt, rho, phi});
  case JESLayout::CompoundAreaEtaPtRhoRun:
    return safeEvaluateFloats(
        preparedJESCompound, "GetJESSF",
        std::array<float, 5>{area, eta, pt, rho,
                             static_cast<float>(runNumber)});
  case JESLayout::CompoundAreaEtaPtRho:
    return safeEvaluateFloats(preparedJESCompound, "GetJESSF",
                              std::array<float, 4>{area, eta, pt, rho});
  case JESLayout::Factorized:
    break;
  }

  float current_pt = pt;
  const float sf_L1 =
      safeEvaluateFloats(preparedJESL1, "GetJESCorrection",
                         std::array<float, 4>{area, eta, current_pt, rho});
  current_pt = current_pt * sf_L1;
  const float sf_L2 =
      safeEvaluateFloats(preparedJESL2, "GetJESCorrection",
                         std::array<float, 3>{eta, phi, current_pt});
  current_pt = current_pt * sf_L2;
  const float sf_L3 =
      safeEvaluateFloats(preparedJESL3, "GetJESCorrection",
                         std::array<float, 2>{eta, current_pt});
  current_pt = current_pt * sf_L3;
  float sf_res = 1.;
  if (IsDATA) {
    if (abs(eta) >= 2.0 && abs(eta) < 2.5)
      current_pt = std::max(30.001f, current_pt);
    sf_res = safeEvaluateFloats(
        preparedJESResidual, "GetJESCorrection",
        std::array<float, 3>{static_cast<float>(runNumber), eta, current_pt});
  }
  return sf_L1 * sf_L2 * sf_L3 * sf_res;
}

float MyCorrection::GetJESUncertainty(const float eta, const float pt,
                                      const TString &source) const {
  // AnalyzerCore::ApplyJetScaleVariation walks 27 sources for every jet, so
  // the source key is passed as a view and looked up without building a
  // std::string.
  const std::string_view sourceKey(source.Data(),
                                   static_cast<std::size_t>(source.Length()));
  return safeEvaluate2D(getJESUncertaintyCorrection(sourceKey),
                        "GetJESUncertainty", eta, pt);
}

float MyCorrection::GetJESUncertaintySF(const float eta, const float pt,
                                        const variation syst,
                                        const TString &source) const {
  int int_syst = 0;
  if (syst == variation::up)
    int_syst = 1;
  else if (syst == variation::down)
    int_syst = -1;
  else
    int_syst = 0;

  float this_factor = 1.;
  this_factor += int_syst * GetJESUncertainty(eta, pt, source);
  return this_factor;
}

// ---------------------------------------------------------------------------
// AK8 (fat jet) JERC, from fatJet_jerc.json.gz. Same shape as the AK4
// functions above but kept separate: the global tags, the correction set and
// the caching all differ, and the JES uncertainty levels are capitalised.
// ---------------------------------------------------------------------------

const correction::Correction::Ref &
MyCorrection::getFatJetCorrection(std::string_view level) const {
  auto it = cachedFatJetCorrections.find(level);
  if (it != cachedFatJetCorrections.end())
    return it->second;

  if (!cset_jerc_fatjet)
    throw SKNano::ConfigError(
        "[MyCorrection::getFatJetCorrection] fatJet_jerc is not configured for "
        "era " + string(GetEra().Data()));

  // Resolution levels come from the JER tag; JES uncertainties are only
  // published for simulation, so they use the MC JES tag even on data.
  const bool isResolution =
      (level == "PtResolution" || level == "ScaleFactor" ||
       level == "SFUncertainty");
  const string &tag = isResolution ? FJER_global_tag : FJES_unc_global_tag;
  if (tag.empty())
    throw SKNano::ConfigError(
        "[MyCorrection::getFatJetCorrection] No AK8 global tag configured for "
        "era " + string(GetEra().Data()));

  const string key = substituteJercLevel(tag, level, "GetFatJetCorrection");
  return cachedFatJetCorrections
      .emplace(string(level), cset_jerc_fatjet->at(key))
      .first->second;
}

float MyCorrection::GetFJER(const float eta, const float pt,
                            const float rho) const {
  return safeEvaluate3D(getFatJetCorrection("PtResolution"), "GetFJER", eta, pt,
                        rho);
}

float MyCorrection::GetFJERSF(const float eta, const float pt,
                              const variation syst,
                              const TString &source) const {
  static_cast<void>(source);
  const float sf =
      safeEvaluate2D(getFatJetCorrection("ScaleFactor"), "GetFJERSF", eta, pt);
  if (syst == variation::nom)
    return sf;
  // The uncertainty is its own correction, combined the way the AK4 side
  // does. A missing key or a failed evaluation throws: returning the nominal
  // here would leave the JER variation lanes equal to the nominal with
  // nothing to show for it.
  const float unc = safeEvaluate2D(getFatJetCorrection("SFUncertainty"),
                                   "GetFJERSFUncertainty", eta, pt);
  return (syst == variation::up) ? sf + unc : std::max(sf - unc, 0.f);
}

float MyCorrection::GetFJESSF(const float area, const float eta, const float pt,
                              const float phi, const float rho,
                              const unsigned int runNumber) const {
  if (!cset_jerc_fatjet)
    throw SKNano::ConfigError(
        "[MyCorrection::GetFJESSF] fatJet_jerc is not configured for era " +
        string(GetEra().Data()));
  if (FJES_global_tag.empty())
    throw SKNano::ConfigError(
        "[MyCorrection::GetFJESSF] No AK8 JES global tag for era " +
        string(GetEra().Data()));

  if (!cachedFatJetJESCompound) {
    const string key =
        substituteJercLevel(FJES_global_tag, "L1L2L3Res", "GetFJESSF");
    cachedFatJetJESCompound = cset_jerc_fatjet->compound().at(key);
  }

  // Which inputs the compound wants varies by era and by sample type: 2024
  // takes (JetA, JetEta, JetPt, Rho, JetPhi) in simulation and the same plus
  // run on data, while other campaigns drop phi. Read the declared inputs and
  // fill them by name rather than hard-coding a layout per era, so a new
  // campaign cannot silently pass the wrong arity.
  if (cachedFatJetJESArgs.empty()) {
    for (const auto &input : cachedFatJetJESCompound->inputs())
      cachedFatJetJESInputs.emplace_back(input.name());
    cachedFatJetJESArgs.resize(cachedFatJetJESInputs.size());
  }
  for (std::size_t i = 0; i < cachedFatJetJESInputs.size(); ++i) {
    const string &name = cachedFatJetJESInputs[i];
    double value = 0.;
    if (name == "JetA")
      value = area;
    else if (name == "JetEta")
      value = eta;
    else if (name == "JetPt")
      value = pt;
    else if (name == "Rho")
      value = rho;
    else if (name == "JetPhi")
      value = phi;
    else if (name == "run")
      value = runNumber;
    else
      throw SKNano::ConfigError(
          "[MyCorrection::GetFJESSF] Unhandled AK8 JES input '" + name + "'");
    cachedFatJetJESArgs[i] = value;
  }
  return safeEvaluate(cachedFatJetJESCompound, "GetFJESSF",
                      cachedFatJetJESArgs);
}

float MyCorrection::GetFJESUncertainty(const float eta, const float pt,
                                       const TString &source) const {
  const std::string_view level =
      source.IsNull()
          ? std::string_view("Total")
          : std::string_view(source.Data(),
                             static_cast<std::size_t>(source.Length()));
  return safeEvaluate2D(getFatJetCorrection(level), "GetFJESUncertainty", eta,
                        pt);
}

float MyCorrection::GetFJESUncertaintySF(const float eta, const float pt,
                                         const variation syst,
                                         const TString &source) const {
  if (syst == variation::nom)
    return 1.f;
  const float unc = GetFJESUncertainty(eta, pt, source);
  return (syst == variation::up) ? 1.f + unc : 1.f - unc;
}

void MyCorrection::EvaluateJetCorrectionBatch(
    const SKNano::JetCorrectionBatchInput &input,
    SKNano::JetCorrectionBatchOutput &output,
    SKNano::CorrectionLaneMask jerLanes,
    SKNano::CorrectionLaneMask jesLanes,
    const TString &jesSource) const {
  struct PreparedEvaluator {
    const MyCorrection &correction;
    const TString &source;
    float jec(float area, float eta, float rawPt, float phi, float rho,
              std::uint32_t run) const {
      return correction.GetJESSF(area, eta, rawPt, phi, rho, run);
    }
    std::array<float, 3> jerScaleFactors(float eta, float pt) const {
      const auto values = correction.GetJERSFSet(eta, pt);
      return {values.nom, values.up, values.down};
    }
    float jerResolution(float eta, float pt, float rho) const {
      return correction.GetJER(eta, pt, rho);
    }
    float jesUncertainty(float eta, float pt) const {
      return correction.GetJESUncertainty(eta, pt, source);
    }
    float sqrt(float value) const { return std::sqrt(value); }
  } evaluator{*this, jesSource};

  SKNano::JetCorrectionBatch batch(jerLanes, jesLanes);
  batch.evaluate(input, output, evaluator);
}

bool MyCorrection::IsJetVetoZone(const float eta, const float phi,
                                 const TString &mapCategory) const {
  const auto &cset =
      cachedJetVetoMap.get(cset_jetvetomap, JME_vetomap_key.c_str());
  return safeEvaluate(cset, "IsJetVetoZone", {mapCategory.Data(), eta, phi}) > 0;
}

void MyCorrection::METXYCorrection(Particle &Met, const int RunNumber,
                                   const int npvs,
                                   const XYCorrection_MetType MetType) {
  if (Run == 3) {
    // No supprot in Run3
    return;
  }
  correction::Correction::Ref cset_pt = nullptr;
  correction::Correction::Ref cset_phi = nullptr;
  switch (MetType) {
  case XYCorrection_MetType::Type1PFMET:
    if (IsDATA) {
      cset_pt = cset_met->at("pt_metphicorr_pfmet_data");
      cset_phi = cset_met->at("phi_metphicorr_pfmet_data");
    } else {
      cset_pt = cset_met->at("pt_metphicorr_pfmet_mc");
      cset_phi = cset_met->at("phi_metphicorr_pfmet_mc");
    }
    break;
  case XYCorrection_MetType::Type1PuppiMET:
    if (IsDATA) {
      cset_pt = cset_met->at("pt_metphicorr_puppimet_data");
      cset_phi = cset_met->at("phi_metphicorr_puppimet_data");
    } else {
      cset_pt = cset_met->at("pt_metphicorr_puppimet_mc");
      cset_phi = cset_met->at("phi_metphicorr_puppimet_mc");
    }
    break;
  }
  float this_pt =
      cset_pt->evaluate({Met.Pt(), Met.Phi(), static_cast<float>(npvs),
                         static_cast<float>(RunNumber)});
  float this_phi =
      cset_phi->evaluate({Met.Pt(), Met.Phi(), static_cast<float>(npvs),
                          static_cast<float>(RunNumber)});
  float this_eta = Met.Eta();
  float this_m = Met.M();
  Met.SetPtEtaPhiM(this_pt, this_eta, this_phi, this_m);
}
