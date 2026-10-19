#include "MyCorrection.h"
#include "MLHelper.h"

#include <yaml-cpp/yaml.h>

#include <TLorentzVector.h>
#include <algorithm>
#include <execution>
#include <iostream>
#include <map>
#include <numeric>
#include <vector>

MyCorrection::MyCorrection() {}
MyCorrection::MyCorrection(const TString &era, const TString &period,
                           const TString &sample, const bool IsData,
                           const string &btagging_eff_file,
                           const string &ctagging_eff_file,
                           const string &btagging_R_file,
                           const string &ctagging_R_file) {
  cout << "[MyCorrection::MyCorrection] MyCorrection created for " << era
       << endl;
  SetEra(era);
  SetPeriod(period);
  SetSample(sample);
  setIsData(IsData);

  static_cast<void>(btagging_eff_file);
  static_cast<void>(ctagging_eff_file);
  static_cast<void>(btagging_R_file);
  static_cast<void>(ctagging_R_file);

  const EraConfig config = GetEraConfig(era);

  // Load correction sets.  Which ones are required, and which are simply not
  // published for this era yet, is decided by the era yml.
  loadCorrectionSet(config, "jetid", cset_jetid);
  loadCorrectionSet(config, "jerc", cset_jerc);
  loadCorrectionSet(config, "jerc_fatjet", cset_jerc_fatjet);
  loadCorrectionSet(config, "jetvetomap", cset_jetvetomap);
  loadCorrectionSet(config, "muon", cset_muon);
  loadCorrectionSet(config, "muon_highpt", cset_muon_highpt);
  if (loadCorrectionSet(config, "muon_scalesmearing",
                        cset_muon_scalesmearing)) {
    try {
      muSS_a_data = cset_muon_scalesmearing->at("a_data");
      muSS_m_data = cset_muon_scalesmearing->at("m_data");
      muSS_a_mc = cset_muon_scalesmearing->at("a_mc");
      muSS_m_mc = cset_muon_scalesmearing->at("m_mc");
      muSS_k_data = cset_muon_scalesmearing->at("k_data");
      muSS_k_mc = cset_muon_scalesmearing->at("k_mc");
      muSS_cb_params = cset_muon_scalesmearing->at("cb_params");
      muSS_poly_params = cset_muon_scalesmearing->at("poly_params");
      muSS_random = cset_muon_scalesmearing->at("RandomSmearing");
      muSS_loaded = true;
    } catch (const exception &e) {
      throw SKNano::ConfigError(
          string("[MyCorrection::MyCorrection] muon_scalesmearing file does "
                 "not have the MuonScaRe layout: ") +
          e.what());
    }
  }
  loadCorrectionSet(config, "tau", cset_tau);
  loadCorrectionSet(config, "muon_trig_eff", cset_muon_trig_eff);
  loadCorrectionSet(config, "muon_trig_sf", cset_muon_trig_sf);
  loadCorrectionSet(config, "puWeights", cset_puWeights);
  loadCorrectionSet(config, "btagging", cset_btagging);
  loadCorrectionSet(config, "ctagging", cset_ctagging);
  loadCorrectionSet(config, "btagging_eff", cset_btagging_eff);
  loadCorrectionSet(config, "ctagging_R", cset_ctagging_R);
  loadCorrectionSet(config, "electron", cset_electron);
  loadCorrectionSet(config, "electron_heep", cset_electron_heep);
  loadCorrectionSet(config, "electron_variation", cset_electron_variation);
  loadCorrectionSet(config, "electron_hlt", cset_electron_hlt);
  loadCorrectionSet(config, "met", cset_met);

  // Rochester is the fallback for eras without a POG scale/smearing file. An
  // era that configures neither would run with uncorrected muons, and nothing
  // downstream could tell, so refuse to construct rather than warn.
  if (!config.path("roccor").empty()) {
    loadRoccoR(config.path("roccor"), true);
  } else if (!muSS_loaded) {
    throw SKNano::ConfigError(
        "[MyCorrection::MyCorrection] neither muon_scalesmearing nor roccor "
        "is configured for era " + string(GetEra().Data()) +
        "; muon momenta would stay uncorrected");
  }
  loadGoldenJson(config.path("golden_json"), true);

  const auto loadModel = [&config](const string &name) {
    const string file = config.path(name);
    return file.empty() ? nullptr
                        : make_unique<MLHelper>(file, MLHelper::ModelType::ONNX);
  };
  MLHelper_hDampUp = loadModel("onnx_hDampUp");
  MLHelper_hDampDown = loadModel("onnx_hDampDown");
  MLHelper_TopPtReweight = loadModel("onnx_toppt_reweight");
  MLHelper_rBnom = loadModel("onnx_rBnom");
  MLHelper_rBUp = loadModel("onnx_rBUp");

  // The era is fixed for the lifetime of this object, so its correction keys
  // are resolved once here.  These used to be era->key map lookups performed
  // per event, per lepton or per jet.
  LUM_era_key = config.key("LUM");
  EGM_era_key = config.key("EGM");
  EGM_era_scale_key = EGM_era_key + "_ScaleJSON";
  // The 2024 electronSS_EtDependent file names these two without an era tag,
  // unlike the 2022-2023 campaigns (EGMScale_Compound_Ele_2023postBPIX, ...).
  // Only 2024 has an era yml today; when an older campaign gets one, resolve
  // the tagged spelling here instead of hard-coding it.
  EGM_scale_compound_key = "Scale";
  EGM_smear_syst_key = "SmearAndSyst";
  EGM_era_prompt = string(GetEra().Data()) + "Prompt";
  JME_vetomap_key = config.key("JME_vetomap");

  // "######" is substituted with the correction level by the JERC accessors.
  JER_global_tag = config.globalTag("JER");
  JES_global_tag = config.globalTag(IsData ? "JES_DATA" : "JES_MC");
  // Resolution is only ever measured in simulation, so the AK8 JER tag stays on
  // the MC one for data as well; the same holds for the JES uncertainties.
  FJER_global_tag = config.globalTag("FJER");
  FJES_global_tag = config.globalTag(IsData ? "FJES_DATA" : "FJES_MC");
  FJES_unc_global_tag = config.globalTag("FJES_MC");
}

MyCorrection::~MyCorrection() {}

MyCorrection::EraConfig MyCorrection::GetEraConfig(TString era) const {
  const char *json_pog_path = getenv("JSONPOG_REPO_PATH");
  const char *sknano_data = getenv("SKNANO_DATA");
  const char *external_roccor = getenv("ROCCOR_PATH");
  if (!json_pog_path || !sknano_data || !external_roccor) {
    throw SKNano::ConfigError(
        "[MyCorrection::GetEraConfig] JSONPOG_REPO_PATH, SKNANO_DATA or "
        "ROCCOR_PATH is not set");
  }

  const string era_dir = string(sknano_data) + "/" + era.Data();
  const string config_path = era_dir + "/Correction/era_config.yml";
  cout << "[MyCorrection::GetEraConfig] era config: " << config_path << endl;

  YAML::Node yaml;
  try {
    yaml = YAML::LoadFile(config_path);
  } catch (const exception &e) {
    throw SKNano::ConfigError(
        "[MyCorrection::GetEraConfig] Cannot read era config for era " +
        string(era.Data()) + " (" + config_path + "): " + e.what());
  }

  EraConfig config;

  const string campaign = yaml["campaign"].as<string>("");
  if (campaign.empty())
    throw SKNano::ConfigError("[MyCorrection::GetEraConfig] " + config_path +
                              " has no 'campaign'");
  cout << "[MyCorrection::GetEraConfig] campaign: " << campaign << endl;

  for (const auto &warning : yaml["warnings"]) {
    cout << "\033[1;31m[MyCorrection::GetEraConfig] Warning: "
         << warning.as<string>() << "\033[0m" << endl;
  }

  for (const auto &entry : yaml["keys"])
    config.keys[entry.first.as<string>()] = entry.second.as<string>();
  for (const auto &entry : yaml["global_tags"])
    config.global_tags[entry.first.as<string>()] = entry.second.as<string>();

  // Each POG publishes on its own cadence, so the snapshot is resolved per
  // POG (and optionally per file).  SKNANO_CORRECTION_TAG forces one snapshot
  // everywhere, for one-off comparisons against a different sync.
  const char *tag_override = getenv("SKNANO_CORRECTION_TAG");
  const YAML::Node snapshots = yaml["snapshots"];
  const string default_snapshot =
      snapshots ? snapshots["default"].as<string>("latest") : "latest";

  const auto resolveSnapshot = [&](const string &pog,
                                   const YAML::Node &spec) -> string {
    if (tag_override)
      return string(tag_override);
    if (spec["snapshot"])
      return spec["snapshot"].as<string>();
    if (snapshots && snapshots[pog])
      return snapshots[pog].as<string>();
    return default_snapshot;
  };

  // $JSONPOG_REPO_PATH/<pog>/<campaign>/<snapshot>/<name>
  std::map<string, string> resolved_snapshots; // pog -> snapshot, for the log
  std::vector<string> unpinned;
  for (const auto &entry : yaml["pog_files"]) {
    const string name = entry.first.as<string>();
    const YAML::Node &spec = entry.second;
    const string pog = spec["pog"].as<string>();
    const string snapshot = resolveSnapshot(pog, spec);

    config.paths[name] = string(json_pog_path) + "/" + pog + "/" + campaign +
                         "/" + snapshot + "/" + spec["name"].as<string>();
    if (spec["required"].as<bool>(false))
      config.required.insert(name);

    const auto inserted = resolved_snapshots.emplace(pog, snapshot);
    if (!inserted.second && inserted.first->second != snapshot)
      inserted.first->second += ", " + name + ":" + snapshot;
    if (snapshot == "latest")
      unpinned.push_back(name);
  }

  for (const auto &entry : resolved_snapshots) {
    cout << "[MyCorrection::GetEraConfig] snapshot " << entry.first << ": "
         << entry.second << endl;
  }
  if (!unpinned.empty()) {
    cout << "\033[1;33m[MyCorrection::GetEraConfig] Warning: "
         << unpinned.size()
         << " correction(s) read from the unpinned 'latest' snapshot; results "
            "are not reproducible. Pin dated snapshots in "
         << config_path << ". Affected:";
    for (const auto &name : unpinned)
      cout << " " << name;
    cout << "\033[0m" << endl;
  }

  // $SKNANO_DATA/<era>/<path>
  for (const auto &entry : yaml["data_files"]) {
    const string name = entry.first.as<string>();
    config.paths[name] = era_dir + "/" + entry.second.as<string>();
    config.required.insert(name);
  }
  config.required.erase("golden_json"); // absent for MC-only campaigns

  if (yaml["roccor"])
    config.paths["roccor"] =
        string(external_roccor) + "/" + yaml["roccor"].as<string>();

  return config;
}

void MyCorrection::throwNullCorrection(std::string_view function_name) const {
  string message = "[MyCorrection::";
  message.append(function_name);
  message += "] Correction set is null";
  throw SKNano::ConfigError(message);
}

void MyCorrection::throwEvaluationError(
    std::string_view function_name, const std::exception &e,
    const vector<correction::Variable::Type> &args) const {
  std::ostringstream oss;
  oss << "[MyCorrection::" << function_name
      << "] Error during evaluation: " << e.what() << "; arguments ("
      << args.size() << "): ";
  for (const auto &arg : args)
    std::visit([&oss](const auto &value) { oss << value << " "; }, arg);
  throw SKNano::CorrectionError(oss.str());
}

bool MyCorrection::isInputInCorrection(
    std::string_view key, const correction::Correction::Ref &cset) const {
  if (!cset)
    return false;
  for (const auto &input : cset->inputs()) {
    if (input.name() == key)
      return true;
  }
  return false;
}

const correction::Correction::Ref &
MyCorrection::cachedRefByKey(CorrectionRefCache &cache,
                             const unique_ptr<CorrectionSet> &set,
                             std::string_view key) const {
  auto it = cache.find(key);
  if (it == cache.end()) {
    // Optional sets stay null when the era yml does not list them.
    if (!set)
      throw SKNano::ConfigError(
          "[MyCorrection] no correction set loaded for this era, cannot "
          "resolve '" +
          string(key) + "'");
    it = cache.emplace(string(key), set->at(string(key))).first;
  }
  return it->second;
}

// GoldenLumi
bool MyCorrection::IsGoldenLumi(const unsigned int runNumber,
                                const unsigned int lumiSection) const {
  if (!IsDATA)
    return true;
  return golden_json_parser->isGood(runNumber, lumiSection);
}
