#ifndef MyCorrection_h
#define MyCorrection_h

#include <array>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <memory>
#include <sstream>
using namespace std;

#include "AnalysisException.h"
#include "TString.h"
#include "TRandom3.h"
#include "TMath.h"
#include "correction.h"
#include "RoccoR.h"
#include "JetView.h"
#include "JetTaggingParameter.h"
#include "GenView.h"
#include "MuonView.h"
#include "TauView.h"
#include "ElectronView.h"
#include "FatJetView.h"
#include "Particle.h"
#include "GoldenJsonParser.h"
#include "Variation.h"
#include "CorrectionBatch.h"

#include "MLHelper.h"
#include "TString.h"

using correction::CorrectionSet;

namespace SKNano {

// Heterogeneous lookup: lets a memo be probed with a string_view so the hot
// path never materialises a std::string for an already-bound key.  At
// namespace scope because rootcling has to name it when it generates the
// dictionary for the maps that use it.
struct TransparentStringHash {
    using is_transparent = void;
    std::size_t operator()(std::string_view key) const noexcept {
        return std::hash<std::string_view>{}(key);
    }
};

using CorrectionRefCache =
    std::unordered_map<std::string, correction::Correction::Ref,
                       TransparentStringHash, std::equal_to<>>;

} // namespace SKNano

class MyCorrection {
public:
    using variation = SKNano::Variation;

    struct MuonScaleAndError {
        float scale = 1.f;
        float error = 0.f;
    };

    // Every momentum lane of one medium-pT muon, in GeV, as produced by the
    // MUO POG scale and resolution correction (muon_scalesmearing.json.gz).
    // Data carries only the nominal; its scale and resolution lanes equal the
    // nominal because the POG publishes no data-side uncertainty and the
    // nuisance rides on the simulation templates.
    struct MuonMomentumLanes {
        float nominal = 0.f;
        float scaleUp = 0.f;
        float scaleDown = 0.f;
        float resUp = 0.f;
        float resDown = 0.f;
    };

    struct JERSFSet {
        float nom = 1.f;
        float up = 1.f;
        float down = 1.f;
    };

    struct JERSFVariations {
        float up = 1.f;
        float down = 1.f;
    };

    enum class XYCorrection_MetType {
        Type1PFMET,
        Type1PuppiMET
    };
    MyCorrection();
    MyCorrection(const TString &era, const TString &period, const TString &sample, bool IsData,const string &btagging_eff_file = "btaggingEff.json", const string &ctagging_eff_file = "ctaggingEff.json", const string &btagging_R_file = "btaggingR.json", const string &ctagging_R_file = "ctaggingR.json");
    ~MyCorrection();


    //GoldenLumi
    bool IsGoldenLumi(const unsigned int runNumber, const unsigned int lumiSection) const;

    // Muon
    // ---- Medium-pT momentum correction ----------------------------------
    // Two implementations live side by side and the era yml picks one:
    //
    //  * muon_scalesmearing (MUO POG "MuonScaRe", Run 3): scale on data and
    //    simulation, extra resolution smearing on simulation, uncertainties
    //    from the file.  GetMuonScaleSmearing() is the entry point.
    //  * roccor (Rochester, Run 2 and eras without a POG file):
    //    GetMuonScaleAndError() returns a multiplicative factor and its error.
    //
    // HasMuonScaleSmearing() tells the caller which one is configured.
    bool HasMuonScaleSmearing() const { return muSS_loaded; }
    // POG recipe, valid for 26 < pt < 200 GeV; outside that window every lane
    // equals pt.  trackerLayers is ignored for data.  The resolution smearing
    // draws its random number from the file's own deterministic generator
    // (event number, lumi section, phi), so it reproduces across passes and
    // does not depend on the framework RNG mode.
    // withVariations=false stops after the nominal: the scale and resolution
    // uncertainties cost four more correctionlib evaluations per muon, and the
    // nominal path has no use for them. The nominal is deterministic, so a
    // later full call reproduces it exactly.
    MuonMomentumLanes GetMuonScaleSmearing(int charge, float pt, float eta,
                                           float phi, int trackerLayers,
                                           unsigned long long eventNumber,
                                           unsigned int lumiBlock,
                                           bool withVariations = true) const;
    MuonScaleAndError GetMuonScaleAndError(int charge, float pt, float eta,
                                           float phi, int trackerLayers,
                                           float matchedPt = 0.f) const;
    float GetMuonRECOSF(const MuonView &muon, const variation syst = variation::nom) const;
    float GetMuonRECOSF(const MuonViewCollection &muons, const variation syst = variation::nom) const;

    // ---- High-pT muon scale factors (muon_HighPt.json.gz) --------------------
    // These take explicit numbers rather than a MuonView on purpose: which
    // momentum to feed them is the caller's decision and getting it wrong is
    // silent. The reco SF is binned in the full momentum p, the other keys in
    // pt, and a muon selected through the high-pT path carries TuneP momentum
    // rather than MuonView::Pt(). Every key is binned from 50 GeV to infinity
    // over |eta| < 2.4; both are clamped here because correctionlib throws
    // outside the map.
    static constexpr float HIGHPT_MUON_MIN_PT = 200.f;   // regime boundary
    static constexpr float HIGHPT_SF_MIN_MOMENTUM = 50.f; // map lower edge
    static constexpr float HIGHPT_SF_MAX_ABSETA = 2.4f;   // map upper edge

    // key: NUM_HighPtID_DEN_GlobalMuonProbes, NUM_HLT_DEN_HighPtLooseRelIsoProbes,
    //      NUM_probe_LooseRelTkIso_DEN_HighPtProbes, ... (binned in pt)
    float GetMuonHighPtSF(const TString &key, const float eta, const float pt,
                          const variation syst = variation::nom) const;
    // NUM_GlobalMuons_DEN_TrackerMuonProbes, binned in the full momentum p.
    float GetMuonHighPtRECOSF(const float eta, const float p,
                              const variation syst = variation::nom) const;

    // Generalized Endpoint momentum scale, for pt above HIGHPT_MUON_MIN_PT and
    // data only: the bias belongs to data, so it is removed there rather than
    // injected into simulation. Eras without a kappa map return pt unchanged.
    float GetMuonGEScaledPt(const float pt, const float eta, const float phi,
                            const int charge,
                            const variation syst = variation::nom) const;
    // The same shift driven by the kappa uncertainty alone, with no central
    // bias. This is the simulation nuisance: data already carries the nominal
    // correction, so the template only has to move by the measurement error.
    // Returns pt unchanged for the nominal variation.
    float GetMuonGESigmaShiftedPt(const float pt, const float eta,
                                  const float phi, const int charge,
                                  const variation syst) const;

    // High-pT resolution width, a cubic in the full momentum p (not pt).
    float GetMuonHighPtResolution(const float p, const float eta) const;
    // Extra smearing applied to simulation in the high-pT regime. Returns 1 for
    // data and for eras with no resolution map.
    float GetMuonHighPtSmearFactor(const float p, const float eta,
                                   const unsigned int seed,
                                   const variation syst = variation::nom) const;
    float GetMuonIDSF(const TString &key, const MuonView &muon,
                      variation syst = variation::nom) const;
    float GetMuonIDSF(const TString &key, const MuonViewCollection &muons,
                      const std::vector<std::size_t> &indices,
                      variation syst = variation::nom) const;
    float GetMuonIDSF(const TString &key, const MuonViewCollection &muons,
                      variation syst = variation::nom) const;
    inline float GetMuonISOSF(const TString &key,
                              const MuonViewCollection &muons,
                              variation syst = variation::nom,
                              const TString &source = "") const {
      static_cast<void>(source);
      return GetMuonIDSF(key, muons, syst);
    }

    // electron
    float GetElectronScaleUnc(const float scEta, const unsigned char seedGain, const unsigned int runNumber, const float r9, const float pt, const variation syst = variation::nom) const;

    // ---- Electron energy scale and smearing (electronSS_EtDependent.json.gz) -
    // Run 3 NanoAOD ships without the EGM calibration applied, so the nominal
    // correction has to happen here: a scale on data, a smearing in simulation.
    // Returns 1 outside Run 3, where NanoAOD already carries it.
    //
    // Multiplicative scale for data. Simulation gets 1.
    float GetElectronScaleCorr(const float scEta, const unsigned char seedGain,
                               const unsigned int runNumber, const float r9,
                               const float pt) const;
    // Gaussian WIDTH of the extra smearing for simulation, not a factor: the
    // caller draws the random number so nominal and variations can share it.
    // Data gets 0. syst selects smear / smear_up / smear_down.
    float GetElectronSmearWidth(const float pt, const float r9,
                                const float scEta,
                                const variation syst = variation::nom) const;
    float GetElectronRECOSF(const float abseta, const float pt, const float phi, const variation syst = variation::nom) const;
    float GetElectronRECOSF(const ElectronView &electron, const variation syst = variation::nom) const;
    float GetElectronRECOSF(const ElectronViewCollection &electrons, const variation syst = variation::nom) const;
    float GetElectronIDSF(const TString &Electron_ID_SF_Key, const float abseta, const float pt, const float phi, const variation syst = variation::nom) const;
    float GetElectronIDSF(const TString &key, const ElectronView &electron, const variation syst = variation::nom) const;
    float GetElectronIDSF(const TString &key, const ElectronViewCollection &electrons, const variation syst = variation::nom) const;
    // ---- HEEP (high-pT) electron ID scale factor ----------------------------
    // EGM publishes the HEEPID working point on a separate branch of the
    // electron SF file (EGM/<campaign>/heep-id/electron.json.gz), wired in the
    // era yml as "electron_heep". Binned in signed supercluster eta and pt from
    // 35 GeV with error flow, so scEta must be the SC eta and pt is clamped to
    // the map's lower edge here. HasElectronHEEPIDSF() is false for eras that
    // do not configure the file; calling the getter then throws.
    bool HasElectronHEEPIDSF() const { return static_cast<bool>(cset_electron_heep); }
    float GetElectronHEEPIDSF(const float scEta, const float pt,
                              const variation syst = variation::nom) const;
    static constexpr float HEEP_SF_MIN_PT = 35.f; // map lower edge
    // photon

    // tau
    float GetTauIDSF_vsJetRaw(const TauView::TauID &id, const float pt,
                              const int dm, const int genmatch,
                              const variation syst = variation::nom,
                              const TString &flag = "pt") const;
    float GetTauIDSF_vsERaw(const TauView::TauID &id, const float eta,
                            const int dm, const int genmatch,
                            const variation syst = variation::nom) const;
    float GetTauIDSF_vsMuRaw(const TauView::TauID &id, const float eta,
                             const int genmatch,
                             const variation syst = variation::nom) const;
    float GetTauIDSF_vsJet(const TauView::TauID &id, const TauView &tau,
                           const variation syst = variation::nom) const;
    float GetTauIDSF_vsE(const TauView::TauID &id, const TauView &tau,
                         const variation syst = variation::nom) const;
    float GetTauIDSF_vsMu(const TauView::TauID &id, const TauView &tau,
                          const variation syst = variation::nom) const;
    // Collection forms return the product over the given indices.
    float GetTauIDSF_vsJet(const TauView::TauID &id, const TauViewCollection &taus,
                           const std::vector<std::size_t> &indices,
                           const variation syst = variation::nom) const;
    float GetTauIDSF_vsE(const TauView::TauID &id, const TauViewCollection &taus,
                         const std::vector<std::size_t> &indices,
                         const variation syst = variation::nom) const;
    float GetTauIDSF_vsMu(const TauView::TauID &id, const TauViewCollection &taus,
                          const std::vector<std::size_t> &indices,
                          const variation syst = variation::nom) const;
    float GetTauIDSF_vsJet(const TauView::TauID &id, const TauViewCollection &taus,
                           const variation syst = variation::nom) const;
    float GetTauIDSF_vsE(const TauView::TauID &id, const TauViewCollection &taus,
                         const variation syst = variation::nom) const;
    float GetTauIDSF_vsMu(const TauView::TauID &id, const TauViewCollection &taus,
                          const variation syst = variation::nom) const;

    // Trigger
    // Single lepton trigger from POG
    float GetMuonTriggerEff(const TString &Muon_Trigger_Eff_Key, const float abseta, const float pt, const bool isData, const variation syst = variation::nom) const;
    float GetMuonTriggerSF(const TString &key, const MuonView &muon, const variation syst = variation::nom) const;
    float GetMuonTriggerSF(const TString &key, const MuonViewCollection &muons, const variation syst = variation::nom) const;
    float GetElectronTriggerEff(const TString &Electron_ID_SF_Key, const float eta, const float pt, const float phi, const bool isDATA, const variation syst = variation::nom) const;
    inline float GetElectronTriggerDataEff(const TString &Electron_ID_SF_Key, const float eta, const float pt, const float phi, const variation syst = variation::nom) {
        return GetElectronTriggerEff(Electron_ID_SF_Key, eta, pt, phi, true, syst);
    };
    inline float GetElectronTriggerMCEff(const TString &Electron_ID_SF_Key, const float eta, const float pt, const float phi, const variation syst = variation::nom) {
        return GetElectronTriggerEff(Electron_ID_SF_Key, eta, pt, phi, false, syst);
    };
    float GetElectronTriggerSF(const TString &Electron_Trigger_SF_Key, const float eta, const float pt, const float phi, const variation syst = variation::nom) const;

    // PUWeights
    float GetPUWeight(const float nTrueInt, const variation syst = variation::nom, const TString &source = "") const;

    // tagging param
    void SetTaggingParam(JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerWP wp);

    // btaging
    float GetBTaggingWP() const;
    float GetBTaggingWP(JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerWP wp) const;
    float GetBTaggingEff(const float eta, const float pt, const int flav, JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerWP wp, const variation syst = variation::nom);
    // Fixed-working-point BTV scale factor. `cset_btagging` was already loaded
    // for its _wp_values thresholds but the SF corrections in the same file had
    // no accessor, so no analysis could apply a b-tagging weight at all.
    // Method selects the correction suffix: "comb"/"mujets" are measured for
    // heavy flavour (flav 5/4), "light" for flav 0 -- pass the one that matches
    // the jet, which GetBTaggingSF does by default when method is empty.
    float GetBTaggingSF(const float eta, const float pt, const int flav, JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerWP wp, const variation syst = variation::nom, const TString &method = "") const;

    // ctagging
    pair<float, float> GetCTaggingWP() const;
    pair<float, float> GetCTaggingWP(JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerWP wp) const;
    float GetCTaggingEff(const float eta, const float pt, const int flav, JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerWP wp, const variation syst = variation::nom);
    float GetCTaggingR(const float npvs, const float HT, const JetTagging::JetFlavTagger tagger, const TString &processName = "", const TString &ttBarCategory = "total", const TString &syst_str = "") const;

    // Jet ID
    bool PassJetID(const JetView &jet, const JetView::JetID &id) const;
    bool PassFatJetID(const FatJetView &fatjet, FatJetView::ID id) const;

    // JERC
    float GetJER(const float eta, const float pt, const float rho) const;
    float GetJERSF(const float eta, const float pt, const variation syst = variation::nom, const TString &source = "total") const;
    JERSFSet GetJERSFSet(const float eta, const float pt, const TString &source = "total") const;
    JERSFVariations GetJERSFVariations(const float eta, const float pt,
                                       const float nominal,
                                       const TString &source = "total") const;
    float GetJESSF(const float area, const float eta, const float pt, const float phi, const float rho, const unsigned int runNumber) const;
    float GetJESUncertaintySF(const float eta, const float pt, const variation syst = variation::nom, const TString &source = "total") const;
    float GetJESUncertainty(const float eta, const float pt, const TString &source = "total") const;
    void EvaluateJetCorrectionBatch(
        const SKNano::JetCorrectionBatchInput &input,
        SKNano::JetCorrectionBatchOutput &output,
        SKNano::CorrectionLaneMask jerLanes = {},
        SKNano::CorrectionLaneMask jesLanes = {},
        const TString &jesSource = "total") const;
    // jerc_fatjet
    // Mirrors the AK4 functions above, reading fatJet_jerc.json.gz. Kept
    // separate rather than parameterised because the global tags, the
    // correction set and the caching all differ. The JES uncertainty source
    // names are capitalised ("Total"), matching the AK8 key naming.
    float GetFJER(const float eta, const float pt, const float rho) const;
    float GetFJERSF(const float eta, const float pt, const variation syst = variation::nom, const TString &source = "Total") const;
    float GetFJESSF(const float area, const float eta, const float pt, const float phi, const float rho, const unsigned int runNumber) const;
    float GetFJESUncertainty(const float eta, const float pt, const TString &source = "Total") const;
    float GetFJESUncertaintySF(const float eta, const float pt, const variation syst = variation::nom, const TString &source = "Total") const;

    // jetvetomap
    bool IsJetVetoZone(const float eta, const float phi, const TString &mapCategory) const;
    
    // MET
    void METXYCorrection(Particle &Met, const int RunNumber, const int npvs, const XYCorrection_MetType MetType);
    
    // reweighting
    float GetTopPtReweight(const TLorentzVector &LastCopyTop, const TLorentzVector &LastCopyAntiTop) const;
    float GethDampReweight(const TLorentzVector &FirstCopyTop, const TLorentzVector &FirstCopyAntiTop, const variation &syst) const;
    float GetBFragReweight(const TLorentzVector &LastCopyTop, const TLorentzVector &LastCopyAntiTop, const TLorentzVector &LastCopyWPlus, const TLorentzVector &LastCopyWMinus,
                            const TLorentzVector &FirstCopyBHadronFromTop, const TLorentzVector &FirstCopyBHadronFromAntiTop, const variation &syst) const;
    // helper function for getbfrag
    std::array<size_t, 6> GetGenIdxofTopDecayProducts(const GenViewCollection &gens) const;
    // Safe evaluation for both Correction::Ref and CompoundCorrection::Ref.
    // function_name is a string_view so that the (very common) string-literal
    // call sites cost nothing on the success path; it is only materialised
    // when an error message actually has to be built.
    template <typename CorrectionRef>
    inline float safeEvaluate(const CorrectionRef &cset,
                              std::string_view function_name,
                              const vector<correction::Variable::Type> &args) const {
        if (!cset)
            throwNullCorrection(function_name);

        try {
            return cset->evaluate(args);
        } catch (const std::exception &e) {
            throwEvaluationError(function_name, e, args);
        }
        return 1.f; // unreachable; throwEvaluationError is [[noreturn]]
    }

    // correctionlib's public evaluator accepts a vector<variant>.  JEC calls
    // have a fixed, all-floating signature, so keep one buffer per arity and
    // thread instead of allocating and constructing an initializer-list
    // vector for every correction level of every jet.
    template <typename CorrectionRef, std::size_t N>
    inline float safeEvaluateFloats(const CorrectionRef &cset,
                                    std::string_view function_name,
                                    const std::array<float, N> &values) const {
        static thread_local vector<correction::Variable::Type> args(N);
        if (args.size() != N)
            args.resize(N);
        for (std::size_t index = 0; index < N; ++index)
            args[index] = static_cast<double>(values[index]);
        return safeEvaluate(cset, function_name, args);
    }

private:
    [[noreturn]] void throwNullCorrection(std::string_view function_name) const;
    [[noreturn]] void
    throwEvaluationError(std::string_view function_name, const std::exception &e,
                         const vector<correction::Variable::Type> &args) const;

    // Everything era-dependent, read from
    // $SKNANO_DATA/<era>/Correction/era_config.yml.  Adding an era is a new
    // yml file, not a code change.
    struct EraConfig {
        // Logical correction name -> resolved absolute path.  A name that is
        // absent from the yml is absent here too, and gets skipped at load.
        unordered_map<string, string> paths;
        unordered_set<string> required;
        unordered_map<string, string> keys;        // LUM, EGM, JME_vetomap
        unordered_map<string, string> global_tags; // JER, JES_MC, JES_DATA

        string path(const string &name) const {
            const auto it = paths.find(name);
            return it == paths.end() ? string() : it->second;
        }
        string key(const string &name) const {
            const auto it = keys.find(name);
            return it == keys.end() ? string() : it->second;
        }
        string globalTag(const string &name) const {
            const auto it = global_tags.find(name);
            return it == global_tags.end() ? string() : it->second;
        }
    };
    EraConfig GetEraConfig(TString era) const;

    inline void SetEra(TString era) {
        DataEra = era;
        if (era.Contains("2016") || era.Contains("2017") || era.Contains("2018")) {
            Run = 2;
        } else if (era.Contains("2022") || era.Contains("2023") || era.Contains("2024")) {
            Run = 3;
        } else {
            throw runtime_error("Invalid era: " + era);
        }
    }
    inline void SetPeriod(TString period) { DataPeriod = period; }
    inline const TString &GetEra() const { return DataEra; }
    inline const TString &GetPeriod() const { return DataPeriod; }
    inline void SetSample(TString sample) { Sample = sample; }
    inline void setIsData(bool isData) { IsDATA = isData; }

    inline bool loadCorrectionSet(const EraConfig &config, const string &name,
                                  unique_ptr<CorrectionSet> &cset) {
        const string file = config.path(name);
        const bool optional = config.required.count(name) == 0;
        if (file.empty()) {
            if (!optional)
                throw SKNano::ConfigError(
                    "[MyCorrection::loadCorrectionSet] Required correction '" +
                    name + "' is not configured for era " + string(GetEra().Data()));
            cout << "[MyCorrection::loadCorrectionSet] " << name
                 << ": not configured for this era, skipping" << endl;
            return false;
        }
        cout << "[MyCorrection::loadCorrectionSet] " << name << ": " << file << endl;
        try {
            cset = CorrectionSet::from_file(file);
            return true;
        } catch (const exception &e) {
            if (optional) {
                cerr << "[MyCorrection::loadCorrectionSet] Warning: Failed to load " << name << " (" << file << "): " << e.what() << endl;
                return false;
            } else {
                cerr << "[MyCorrection::loadCorrectionSet] Error: Failed to load " << name << " (" << file << "): " << e.what() << endl;
                throw;
            }
        }
    }

    inline bool loadRoccoR(const string &file, bool optional=false) {
        cout << "[MyCorrection::loadRoccoR] " << file << endl;
        try {
            rc.init(file);
            return true;
        } catch (const exception &e) {
            if (optional) {
                cerr << "[MyCorrection::loadRoccoR] Warning: Failed to load " << file << " (" << e.what() << ")" << endl;
                return false;
            } else {
                cerr << "[MyCorrection::loadRoccoR] Error: Failed to load " << file << " (" << e.what() << ")" << endl;
                throw;
            }
        }
    }

    inline bool loadGoldenJson(const string &file, bool optional = false) {
        cout << "[MyCorrection::loadGoldenJson] " << file << endl;
        try {
            golden_json_parser = make_unique<GoldenJsonParser>(file);
            return true;
        } catch (const exception &e) {
            if (optional) {
                cerr << "[MyCorrection::loadGoldenJson] Warning: Failed to load " << file << " (" << e.what() << ")" << endl;
                return false;
            } else {
                cerr << "[MyCorrection::loadGoldenJson] Error: Failed to load " << file << " (" << e.what() << ")" << endl;
                throw;
            }
        }
    }

    // correction::Variable::name() returns by value, so this allocates one
    // string per declared input.  Only call it while binding a cached ref,
    // never per object.
    bool isInputInCorrection(std::string_view key, const correction::Correction::Ref &cset) const;

    // CorrectionSet::at() hashes the key and returns a shared_ptr copy, and
    // every call site here would otherwise have to build the key string
    // first.  Bind each ref once and reuse it for the rest of the job.
    class LazyRef {
    public:
        const correction::Correction::Ref &
        get(const unique_ptr<CorrectionSet> &set, const char *key) const {
            if (!ref_) {
                // Optional sets stay null when the era yml does not list them.
                // Say so here rather than dereferencing a null unique_ptr.
                if (!set)
                    throw SKNano::ConfigError(
                        string("[MyCorrection] no correction set loaded for "
                               "this era, cannot resolve '") +
                        key + "'");
                ref_ = set->at(key);
            }
            return ref_;
        }

    private:
        mutable correction::Correction::Ref ref_;
    };

    using CorrectionRefCache = SKNano::CorrectionRefCache;

    const correction::Correction::Ref &
    cachedRefByKey(CorrectionRefCache &cache,
                   const unique_ptr<CorrectionSet> &set,
                   std::string_view key) const;

    const correction::Correction::Ref &getJERPtResolutionCorrection() const;
    const correction::Correction::Ref &getJERScaleFactorCorrection() const;
    const correction::Correction::Ref &getJERSFUncertaintyCorrection() const;
    const correction::Correction::Ref &getJESUncertaintyCorrection(std::string_view source) const;
    // Resolves and caches one AK8 correction level from fatJet_jerc.json.gz.
    const correction::Correction::Ref &getFatJetCorrection(std::string_view level) const;
    float safeEvaluate2D(const correction::Correction::Ref &cset, std::string_view function_name, float x, float y) const;
    float safeEvaluate3D(const correction::Correction::Ref &cset, std::string_view function_name, float x, float y, float z) const;

    JetTagging::JetFlavTaggerWP global_wp;
    JetTagging::JetFlavTagger global_tagger;
    string global_wpStr;
    string global_taggerStr;
    string global_wpValuesKey; // global_taggerStr + "_wp_values"
    TString DataEra;
    TString DataPeriod;
    int Run;
    TString Sample;
    bool IsDATA;

    unique_ptr<MLHelper> MLHelper_TopPtReweight;
    unique_ptr<MLHelper> MLHelper_hDampUp;
    unique_ptr<MLHelper> MLHelper_hDampDown;
    unique_ptr<MLHelper> MLHelper_rBnom;
    unique_ptr<MLHelper> MLHelper_rBUp;
    

    unique_ptr<CorrectionSet> cset_muon;
    unique_ptr<CorrectionSet> cset_muon_highpt;
    // MUO POG scale/resolution (MuonScaRe).  The individual corrections are
    // bound once at construction: GetMuonScaleSmearing evaluates up to nine
    // of them per muon and a CorrectionSet::at() is a string-keyed lookup.
    unique_ptr<CorrectionSet> cset_muon_scalesmearing;
    bool muSS_loaded = false;
    correction::Correction::Ref muSS_a_data, muSS_m_data, muSS_a_mc, muSS_m_mc;
    correction::Correction::Ref muSS_k_data, muSS_k_mc;
    correction::Correction::Ref muSS_cb_params, muSS_poly_params;
    correction::Correction::Ref muSS_random;
    unique_ptr<CorrectionSet> cset_tau;
    unique_ptr<CorrectionSet> cset_muon_trig_eff;
    unique_ptr<CorrectionSet> cset_muon_trig_sf;
    unique_ptr<CorrectionSet> cset_puWeights;
    unique_ptr<CorrectionSet> cset_btagging;
    unique_ptr<CorrectionSet> cset_ctagging;
    unique_ptr<CorrectionSet> cset_btagging_eff;
    unique_ptr<CorrectionSet> cset_ctagging_R;
    unique_ptr<CorrectionSet> cset_electron;
    unique_ptr<CorrectionSet> cset_electron_heep; // EGM heep-id branch
    unique_ptr<CorrectionSet> cset_electron_hlt;
    unique_ptr<CorrectionSet> cset_electron_variation;
    unique_ptr<CorrectionSet> cset_jetid;
    unique_ptr<CorrectionSet> cset_jerc;
    unique_ptr<CorrectionSet> cset_jerc_fatjet;
    unique_ptr<CorrectionSet> cset_jetvetomap;
    unique_ptr<CorrectionSet> cset_met;

    // The configured era's correction keys, resolved from the era yml at
    // construction.  The era cannot change afterwards, so looking these up per
    // event only cost a hash and a string construction.
    string LUM_era_key;      // pileup profile, e.g. "Collisions24_..._goldenJSON"
    string EGM_era_key;      // EGM campaign tag, e.g. "2024Prompt"
    string EGM_era_scale_key;
    // electronSS_EtDependent.json.gz keys. Every correction in that file
    // carries the era tag, e.g. EGMScale_Compound_Ele_2023postBPIX.
    string EGM_scale_compound_key; // data: the full scale chain
    string EGM_smear_syst_key;     // simulation: smearing and its systematics
    string EGM_era_prompt;   // era + "Prompt", the Run3 electron SF campaign key
    string JME_vetomap_key;
    // JERC global tags, still carrying the "######" level placeholder.
    string JER_global_tag;
    string JES_global_tag;
    // AK8 counterparts, from fatJet_jerc.json.gz. Resolution and JES
    // uncertainties are only ever provided for simulation, so those two stay on
    // the MC tag for both sample types; only the JES levels differ.
    string FJER_global_tag;
    string FJES_global_tag;
    string FJES_unc_global_tag;

    mutable correction::Correction::Ref cachedJERPtResolution;
    mutable correction::Correction::Ref cachedJERScaleFactor;
    mutable correction::Correction::Ref cachedJERSFUncertainty;
    mutable CorrectionRefCache cachedJESUncertaintyCorrections;
    // One entry per resolved AK8 correction level ("PtResolution",
    // "ScaleFactor", "SFUncertainty", "Total", ...).
    mutable CorrectionRefCache cachedFatJetCorrections;
    mutable correction::CompoundCorrection::Ref cachedFatJetJESCompound;
    // Input names of the AK8 JES compound, resolved once, plus the buffer they
    // are filled into. The set of inputs differs by era and sample type.
    mutable vector<string> cachedFatJetJESInputs;
    mutable vector<correction::Variable::Type> cachedFatJetJESArgs;
    mutable CorrectionRefCache cachedMuonIDSF;
    mutable CorrectionRefCache cachedMuonTriggerEff;

    LazyRef cachedPUWeight;
    LazyRef cachedJetVetoMap;
    LazyRef cachedJetIDTight;
    LazyRef cachedJetIDTightLepVeto;
    LazyRef cachedFatJetIDTight;
    LazyRef cachedFatJetIDTightLepVeto;
    LazyRef cachedElectronIDSF;
    LazyRef cachedElectronHEEPIDSF;
    LazyRef cachedElectronHltSF;
    LazyRef cachedElectronHltDataEff;
    LazyRef cachedElectronHltMcEff;
    LazyRef cachedBTaggingWP;
    LazyRef cachedCTaggingWP;
    LazyRef cachedTauIDSFvsJet;
    LazyRef cachedTauIDSFvsE;
    LazyRef cachedTauIDSFvsMu;

    // Whether the electron HLT corrections declare a "phi" input.  Probing
    // this per electron used to allocate a vector of all input names.
    mutable int electronHltSFHasPhi = -1;
    mutable int electronHltDataEffHasPhi = -1;
    mutable int electronHltMcEffHasPhi = -1;

    // Which inputs the era's JES correction takes.  Resolved together with the
    // refs below so the per-jet path is a switch on an enum rather than a
    // chain of era string comparisons.
    enum class JESLayout {
        Factorized,          // L1/L2/L3 (+L2L3Residual) applied in sequence
        CompoundAreaEtaPtRho,
        CompoundAreaEtaPtRhoPhi,
        CompoundAreaEtaPtRhoRun,
        CompoundAreaEtaPtRhoPhiRun
    };

    // The first JEC request binds the correctionlib refs once instead of doing
    // string construction and CorrectionSet lookups for every jet.
    mutable bool preparedJESValid = false;
    mutable JESLayout preparedJESLayout = JESLayout::Factorized;
    mutable correction::CompoundCorrection::Ref preparedJESCompound;
    mutable correction::Correction::Ref preparedJESL1;
    mutable correction::Correction::Ref preparedJESL2;
    mutable correction::Correction::Ref preparedJESL3;
    mutable correction::Correction::Ref preparedJESResidual;

    RoccoR rc;
    unique_ptr<GoldenJsonParser> golden_json_parser;
    // Every POG spells the systematics differently, so the enum has to be
    // mapped per POG.  These return a string literal rather than a
    // std::string: correctionlib takes the value as a variant alternative and
    // builds its own string, so returning by value only added one heap
    // allocation per scale-factor evaluation.
    static inline const char *pickSystString(const variation syst,
                                             const char (&table)[3][12]) {
        return table[static_cast<int>(syst)];
    }

    inline const char *getSystString_CUSTOM(const variation syst) const {
        static constexpr char table[3][12] = {"nom", "up", "down"};
        return pickSystString(syst, table);
    }

    inline const char *getSystString_MUO(const variation syst) const {
        static constexpr char table[3][12] = {"nominal", "systup", "systdown"};
        return pickSystString(syst, table);
    }

    inline const char *getSystString_TAU(const variation syst) const {
        static constexpr char table[3][12] = {"nom", "up", "down"};
        return pickSystString(syst, table);
    }

    inline const char *getSystString_LUM(const variation syst) const {
        static constexpr char table[3][12] = {"nominal", "up", "down"};
        return pickSystString(syst, table);
    }

    inline const char *getSystString_BTV(const variation syst) const {
        static constexpr char table[3][12] = {"central", "up", "down"};
        return pickSystString(syst, table);
    }

    inline const char *getSystString_EGM(const variation syst) const {
        static constexpr char table[3][12] = {"sf", "sfup", "sfdown"};
        return pickSystString(syst, table);
    }

    inline const char *getSystString_EGMScale(const variation syst) const {
        // Only for Run2
        if (!(Run == 2)) {
            throw runtime_error("[MyCorrection::getSystString_EGMScale] Use getSystString_EGM for Run3");
        }
        static constexpr char table[3][12] = {"", "scaleup", "scaledown"};
        return pickSystString(syst, table);
    }

    inline const char *getSystString_JME(const variation syst) const {
        static constexpr char table[3][12] = {"nom", "up", "down"};
        return pickSystString(syst, table);
    }
};

#endif
