#include "MyCorrection.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

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

float MyCorrection::GetMuonRECOSF(const MuonView &muon,
                                  const variation syst) const {
  if (Run == 3)
    return 1.f;
  auto cset = cset_muon->at("NUM_TrackerMuons_DEN_genTracks");
  return safeEvaluate(cset, "GetMuonRECOSF",
                      {muon.Eta(), std::max(40.f, muon.MiniAODPt()),
                       getSystString_MUO(syst)});
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
    static const string scale2022Key = "Scale";
    const string &key =
        GetEra().Contains("2022") ? scale2022Key : EGM_era_scale_key;
    auto cset = cset_electron_variation->at(key);
    vector<correction::Variable::Type> args = {"total_uncertainty",
                                               static_cast<int>(seedGain),
                                               static_cast<float>(runNumber),
                                               scEta,
                                               r9,
                                               pt};
    const float unc = safeEvaluate(cset, "GetElectronScaleSF", args);
    if (syst == variation::up)
      return 1. + unc;
    else if (syst == variation::down)
      return 1. - unc;
    else
      throw runtime_error(
          "[MyCorrection::GetElectronScaleUnc] Invalid syst value");
  }
  default:
    throw runtime_error(
        "[MyCorrection::GetElectronScaleUnc] Invalid run number");
  }

  // This should never be reached, but added to avoid compiler warning
  return 1.0;
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
  return GetElectronRECOSF(std::fabs(electron.Eta()), electron.Pt(),
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

float MyCorrection::GetElectronIDSF(const TString &key,
                                    const ElectronView &electron,
                                    const variation syst) const {
  return GetElectronIDSF(key, std::fabs(electron.Eta()), electron.Pt(),
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

float MyCorrection::GetTauES(const float pt, const float eta, const int dm,
                             const int genmatch, const TString &wp_vsJet,
                             const TString &wp_vsE,
                             const variation syst) const {
  if (IsDATA)
    return 1.f;
  const auto &cset = cachedTauES.get(cset_tau, "tau_energy_scale");
  return safeEvaluate(cset, "GetTauES",
                      {static_cast<double>(pt), static_cast<double>(eta), dm,
                       genmatch, string("DeepTau2018v2p5"),
                       string(wp_vsJet.Data()), string(wp_vsE.Data()),
                       getSystString_TAU(syst)});
}

float MyCorrection::GetTauES(const TauView::TauID &id, const TauView &tau,
                             const variation syst) const {
  return GetTauES(tau.MiniAODPt(), tau.Eta(), tau.DecayMode(),
                  tau.GenPartFlav(), ToCorrectionString(id.vsJet),
                  ToCorrectionString(id.vsE), syst);
}

namespace {

const char *tauTrigTypeString(const MyCorrection::TauTrigType type) {
  switch (type) {
  case MyCorrection::TauTrigType::DiTau:    return "ditau";
  case MyCorrection::TauTrigType::ETau:     return "etau";
  case MyCorrection::TauTrigType::MuTau:    return "mutau";
  case MyCorrection::TauTrigType::DiTauJet: return "ditaujet";
  }
  throw SKNano::LogicError("[MyCorrection::GetTauTriggerSF] unknown trigger type");
}

// A stale index would read a neighbouring tau and silently mis-weight the
// event, so it is reported rather than clamped.  Matches GetMuonIDSF.
inline void requireTauIndex(const char *where, const std::size_t index,
                            const std::size_t size) {
  if (index >= size)
    throw SKNano::LogicError(string("[MyCorrection::") + where +
                             "] index out of range");
}

} // namespace

float MyCorrection::GetTauTriggerSF(const float pt, const int dm,
                                    const TauTrigType type,
                                    const variation syst,
                                    const TString &wp) const {
  if (IsDATA)
    return 1.f;
  const auto &cset = cachedTauTrigger.get(cset_tau, "tau_trigger");
  // "Medium" is the only working point published for 2024, and it is a PNet
  // point rather than the DeepTau point the offline selection uses.
  const float sf =
      safeEvaluate(cset, "GetTauTriggerSF",
                   {static_cast<double>(pt), dm, string(tauTrigTypeString(type)),
                    string(wp.Data()), string("sf"),
                    getSystString_TAU(syst)});
  // The file returns -1 for a decay mode it does not cover. Multiplying that
  // into an event weight flips its sign, silently, so it is refused here.
  if (!(sf > 0.f)) {
    const int key = static_cast<int>(type) * 64 + (dm + 1);
    if (warnedTauTriggerCoverage.insert(key).second)
      cerr << "[MyCorrection::GetTauTriggerSF] Warning: " << tauTrigTypeString(type)
           << " trigger SF is not covered for dm " << dm << " (got " << sf
           << "), returning 1." << endl;
    return 1.f;
  }
  return sf;
}

float MyCorrection::GetTauTriggerSF(const TauTrigType type,
                                    const TauViewCollection &taus,
                                    const std::vector<std::size_t> &indices,
                                    const variation syst,
                                    const TString &wp) const {
  float weight = 1.f;
  for (const std::size_t index : indices) {
    requireTauIndex("GetTauTriggerSF", index, taus.size());
    weight *= GetTauTriggerSF(taus[index].Pt(), taus[index].DecayMode(), type,
                              syst, wp);
  }
  return weight;
}

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
