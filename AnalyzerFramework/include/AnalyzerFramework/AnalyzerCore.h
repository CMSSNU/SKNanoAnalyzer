#ifndef AnalyzerCore_h
#define AnalyzerCore_h

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <map>
#include <memory>
#include <numeric>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/hash/hash.h"
#include <nlohmann/json.hpp>

#include "TFile.h"
#include "TH1.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RRawPtrWriteEntry.hxx>
#include "TString.h"
#include "TObjString.h"
#include "TMath.h"
#include "Compression.h"  

#include <AnalyzerFramework/SKNanoLoader.h>
#include "Event.h"
#include "Geometry.h"
#include "Particle.h"
#include "Lepton.h"
#include "GenView.h"
#include "LHEView.h"
#include "MuonView.h"
#include "ElectronView.h"
#include "JetView.h"
#include "SelectedJetView.h"
#include "FatJetView.h"
#include "TauView.h"
#include "PhotonView.h"
#include "SVView.h"
#include "GenJetView.h"
#include "GenAuxView.h"
#include "TrigObjView.h"

#include "LHAPDFHandler.h"
#include "PDFReweight.h"
#include "MyCorrection.h"
#include "JetTaggingParameter.h"
#include "PhysicalConstants.h"

struct TransparentStringHash {
    using is_transparent = void;
    std::size_t operator()(std::string_view s) const noexcept {
        return absl::Hash<std::string_view>{}(s);
    }
    std::size_t operator()(const std::string& s) const noexcept {
        return absl::Hash<std::string_view>{}(s);
    }
};

struct TransparentStringEq {
    using is_transparent = void;
    bool operator()(std::string_view a, std::string_view b) const noexcept {
        return a == b;
    }
    bool operator()(const std::string& a, const std::string& b) const noexcept {
        return a == b;
    }
    bool operator()(const std::string& a, std::string_view b) const noexcept {
        return a == b;
    }
    bool operator()(std::string_view a, const std::string& b) const noexcept {
        return a == b;
    }
}; 

class AnalyzerTaskRegistryState;
class RNTupleOutputState;

template <typename HistT>
class HistogramHandle {
public:
    HistogramHandle() = default;
    explicit HistogramHandle(HistT *histogram) : hist_(histogram) {}

    template <typename... Args>
    void Fill(Args &&...args) const {
        if (!hist_)
            throw SKNano::LogicError("[HistogramHandle] empty handle access");
        hist_->Fill(std::forward<Args>(args)...);
    }

    HistT *get() const {
        if (!hist_)
            throw SKNano::LogicError("[HistogramHandle] empty handle access");
        return hist_;
    }
    explicit operator bool() const noexcept { return hist_ != nullptr; }

private:
    HistT *hist_ = nullptr;
};

using Hist1DHandle = HistogramHandle<TH1>;
using Hist2DHandle = HistogramHandle<TH2>;
using Hist3DHandle = HistogramHandle<TH3>;

class IDContainer {
public:
    IDContainer() {}
    IDContainer(const TString &tight, const TString &loose):
        j_tight(tight), j_loose(loose) {}

    TString GetID(const TString &wp) const {
        if (wp == "tight") return j_tight;
        else if (wp == "loose") return j_loose;
        else throw runtime_error("Invalid WP: " + wp);
    }

private:
    TString j_tight, j_loose;
};

class AnalyzerCore: public SKNanoLoader {
public:
    enum class RNTupleOutputProfile { Fast, Sparse };
    class RNTupleHandle;

    template <typename T>
    class OutputField {
    public:
        OutputField() = default;
        void Set(const T &value) { *value_ = value; }
        void Set(T &&value) { *value_ = std::move(value); }
        OutputField &operator=(const T &value) {
            Set(value);
            return *this;
        }
        T &Get() { return *value_; }
        const T &Get() const { return *value_; }
        explicit operator bool() const noexcept {
            return static_cast<bool>(value_);
        }

    private:
        friend class RNTupleHandle;
        explicit OutputField(std::shared_ptr<T> value)
            : value_(std::move(value)) {}
        std::shared_ptr<T> value_;
    };
    enum class TaskSystematicPolicy {
        CentralOnly,
        AllVariations,
    };

    struct TaskOptions {
        bool enabledByDefault = false;
        TaskSystematicPolicy systematicPolicy =
            TaskSystematicPolicy::CentralOnly;
    };

    using TaskHook = std::function<void()>;

    class RNTupleHandle {
    public:
        RNTupleHandle() = default;

        template <typename T>
        RNTupleHandle &Field(std::string_view name, T &value);

        template <typename T>
        OutputField<T> MakeField(std::string_view name);

        // Store every float/double leaf under `name` (scalars, vectors,
        // arrays, nested) as IEEE half precision on disk (RNTuple kReal16).
        // The in-memory type is unchanged; readers get floats back.  Call it
        // after Field()/MakeField() and before the first Fill().
        RNTupleHandle &SetHalfPrecision(std::string_view name);

        RNTupleHandle &Set(const TString &name, float value);
        RNTupleHandle &Set(const TString &name, double value);
        RNTupleHandle &Set(const TString &name, int value);
        RNTupleHandle &Set(const TString &name, bool value);
        void Fill() const;
        std::uint64_t GetEntries() const;
        explicit operator bool() const noexcept { return owner_ != nullptr; }

    private:
        friend class AnalyzerCore;
        RNTupleHandle(AnalyzerCore *owner, std::string ntupleName)
            : owner_(owner), ntupleName_(std::move(ntupleName)) {}
        void RequireValid() const;

        AnalyzerCore *owner_ = nullptr;
        std::string ntupleName_;
    };

    class OutputRegistry {
    public:
        RNTupleHandle Book(
            std::string_view name,
            RNTupleOutputProfile profile = RNTupleOutputProfile::Fast) const;
        RNTupleHandle Get(std::string_view name) const;

    private:
        friend class AnalyzerCore;
        explicit OutputRegistry(AnalyzerCore *owner) : owner_(owner) {}
        AnalyzerCore *owner_;
    };

    class HistogramGroup {
    public:
        HistogramGroup() = default;

        HistogramGroup Group(std::string_view child) const;
        Hist1DHandle Book1D(std::string_view name, int bins, double minimum,
                            double maximum) const;
        Hist1DHandle Book1D(std::string_view name, int bins,
                            const float *edges) const;
        Hist2DHandle Book2D(std::string_view name, int binsX, double minX,
                            double maxX, int binsY, double minY,
                            double maxY) const;
        Hist2DHandle Book2D(std::string_view name, int binsX,
                            const float *edgesX, int binsY,
                            const float *edgesY) const;
        Hist3DHandle Book3D(std::string_view name, int binsX, double minX,
                            double maxX, int binsY, double minY, double maxY,
                            int binsZ, double minZ, double maxZ) const;
        Hist3DHandle Book3D(std::string_view name, int binsX,
                            const float *edgesX, int binsY,
                            const float *edgesY, int binsZ,
                            const float *edgesZ) const;

        void Fill(std::string_view name, float value, float weight, int bins,
                  float minimum, float maximum) const;
        void Fill(std::string_view name, float value, float weight, int bins,
                  float *edges) const;
        void Fill(std::string_view name, float x, float y, float weight,
                  int binsX, float minX, float maxX, int binsY, float minY,
                  float maxY) const;
        void Fill(std::string_view name, float x, float y, float weight,
                  int binsX, float *edgesX, int binsY, float *edgesY) const;
        void Fill(std::string_view name, float x, float y, float z,
                  float weight, int binsX, float minX, float maxX, int binsY,
                  float minY, float maxY, int binsZ, float minZ,
                  float maxZ) const;
        void Fill(std::string_view name, float x, float y, float z,
                  float weight, int binsX, float *edgesX, int binsY,
                  float *edgesY, int binsZ, float *edgesZ) const;

        explicit operator bool() const noexcept { return owner_ != nullptr; }

    private:
        friend class AnalyzerCore;
        HistogramGroup(AnalyzerCore *owner, std::string prefix)
            : owner_(owner), prefix_(std::move(prefix)) {}
        std::string Resolve(std::string_view name) const;
        void RequireValid() const;

        AnalyzerCore *owner_ = nullptr;
        std::string prefix_;
    };

    AnalyzerCore();
    ~AnalyzerCore();
    void SetOutputThreads(unsigned int threads);

    virtual void initializeAnalyzer() {};
    virtual void executeEvent() {};

    inline bool HasFlag(const TString &flag) const { return std::find(Userflags.begin(), Userflags.end(), flag) != Userflags.end(); }

    inline static bool PtComparing(const Particle& p1, const Particle& p2) { return p1.Pt() > p2.Pt();}
    inline static bool PtComparingPtr(const Particle* p1, const Particle* p2) { return p1->Pt() > p2->Pt();}

    inline static Lepton MakeLeptonSnapshot(const MuonView &muon) {
        Lepton result;
        result.SetLeptonFlavour(Lepton::MUON);
        result.SetPtEtaPhiM(muon.Pt(), muon.Eta(), muon.Phi(), muon.M());
        result.SetCharge(muon.Charge());
        result.SetTkRelIso(muon.TkRelIso());
        result.SetPfRelIso03(muon.PfRelIso03());
        result.SetPfRelIso04(muon.PfRelIso04());
        result.SetMiniPFRelIso(muon.MiniPFRelIso());
        result.SetdXY(muon.dXY(), muon.dXYerr());
        result.SetdZ(muon.dZ(), muon.dZerr());
        result.SetIP3D(muon.IP3D(), muon.SIP3D());
        return result;
    }
    inline static Lepton MakeLeptonSnapshot(const ElectronView &electron) {
        Lepton result;
        result.SetLeptonFlavour(Lepton::ELECTRON);
        result.SetPtEtaPhiM(electron.Pt(), electron.Eta(), electron.Phi(),
                           electron.M());
        result.SetCharge(electron.Charge());
        result.SetPfRelIso03(electron.PfRelIso03());
        result.SetMiniPFRelIso(electron.MiniPFRelIso());
        result.SetdXY(electron.dXY(), electron.dXYerr());
        result.SetdZ(electron.dZ(), electron.dZerr());
        result.SetIP3D(electron.IP3D(), electron.SIP3D());
        return result;
    }


    //MetFilter
    bool PassNoiseFilter(const JetViewCollection &AllJets, const Event &ev, Event::MET_Type met_type = Event::MET_Type::PUPPI);
    bool PassMetFilter(const JetViewCollection &AllJets, const Event &ev, Event::MET_Type met_type = Event::MET_Type::PUPPI);
    // PDF reweight
    PDFReweight *pdfReweight;
    float GetPDFWeight(LHAPDF::PDF *pdf_);
    float GetPDFReweight();
    float GetPDFReweight(int member);    
    // Correction
    MyCorrection *myCorr;
    //unique_ptr<CorrectionSet> csetMuon;
    //unique_ptr<CorrectionSet> csetElectron;;

    // MC weights
    float MCweight(bool usesign = true, bool norm_1invpb = true) const;

    struct ModellingPatch {
        std::vector<float> patch_ScaleVariation;
        std::vector<float> patch_PSVariation;
        float patch_hdamp_up = 0.f;
        float patch_hdamp_down = 0.f;
        float patch_minnlo = 0.f;
    };

    void load_modelling_json(const TString &filename);

    template <typename ViewCollection>
    static std::vector<std::size_t> AllIndices(const ViewCollection &objects) {
        std::vector<std::size_t> indices(objects.size());
        std::iota(indices.begin(), indices.end(), 0);
        return indices;
    }

    // Event-scoped, read-only input access. Selections retain raw indices.
    Event GetEvent();
    MuonViewCollection GetAllMuonViews();
    void PopulateMuonMomentum(MuonSoA &storage);
    GenViewCollection GetAllGenViews();
    JetViewCollection GetAllJetViews();
    SelectedJetViewCollection SelectJetViews(
        const JetViewCollection &jets,
        std::vector<std::size_t> indices,
        const MyCorrection::variation &jesVariation = MyCorrection::variation::nom,
        const MyCorrection::variation &jerVariation = MyCorrection::variation::nom,
        bool sortByPt = true) const;
    void SmearJetViews(const JetViewCollection &jets, const float rho);
    GenJetViewCollection GetAllGenJetViews();
    LHEViewCollection GetAllLHEViews();
    TauViewCollection GetAllTauViews();
    std::vector<std::size_t>
    SelectTauIndices(const TauViewCollection &taus,
                     const std::vector<std::size_t> &seed_indices,
                     const TauView::TauID &id, const float ptmin,
                     const float fetamax) const;
    std::vector<std::size_t> SelectTauIndices(const TauViewCollection &taus,
                                              const TauView::TauID &id,
                                              const float ptmin,
                                              const float fetamax) const;
    TauViewCollection SelectTauViews(const TauViewCollection &taus,
        std::vector<std::size_t> indices, bool sortByPt = true) const;
    GenDressedLeptonViewCollection GetAllGenDressedLeptonViews();
    GenIsolatedPhotonViewCollection GetAllGenIsolatedPhotonViews();
    GenVisTauViewCollection GetAllGenVisTauViews();
    std::vector<std::size_t> SelectMuonIndices(const MuonViewCollection &muons, const std::vector<std::size_t> &seed_indices, const MuonView::MuonID ID, const float ptmin, const float fetamax) const;
    std::vector<std::size_t> SelectMuonIndices(const MuonViewCollection &muons, const MuonView::MuonID ID, const float ptmin, const float fetamax) const;
    MuonViewCollection SelectMuonViews(const MuonViewCollection &muons,
        std::vector<std::size_t> indices, bool sortByPt = true) const;
    ElectronViewCollection GetAllElectronViews();
    std::vector<std::size_t> SelectElectronIndices(const ElectronViewCollection &electrons, const std::vector<size_t> &seed_indices, const ElectronView::ElectronID ID, const float ptmin, const float fetamax, bool vetoHEM = false) const;
    std::vector<std::size_t> SelectElectronIndices(const ElectronViewCollection &electrons, const ElectronView::ElectronID ID, const float ptmin, const float fetamax, bool vetoHEM = false) const;
    ElectronViewCollection SelectElectronViews(
        const ElectronViewCollection &electrons,
        std::vector<std::size_t> indices, bool sortByPt = true) const;
    FatJetViewCollection GetAllFatJets();
    SVViewCollection GetAllSVViews();

    PhotonViewCollection GetAllPhotons();
    PhotonViewCollection GetPhotons(TString id, double ptmin, double fetamax);
    TrigObjViewCollection GetAllTrigObjViews();
    std::vector<std::size_t> SelectTrigObjIndices(const TrigObjViewCollection &trigobjs, const std::vector<std::size_t> &seed_indices, const int id, const float ptmin, const float fetamax) const;
    std::vector<std::size_t> SelectTrigObjIndices(const TrigObjViewCollection &trigobjs, const int id, const float ptmin, const float fetamax) const;

    // Select objects
    std::vector<std::size_t> SelectJetIndices(const JetViewCollection &jets, const std::vector<size_t> &seed_indices, const JetView::JetID, const float ptmin, const float fetamax, const MyCorrection::variation &JESVariation = MyCorrection::variation::nom, const MyCorrection::variation &JERVariation = MyCorrection::variation::nom) const;
    std::vector<std::size_t> SelectJetIndices(const JetViewCollection &jets, const JetView::JetID, const float ptmin, const float fetamax, const MyCorrection::variation &JESVariation = MyCorrection::variation::nom, const MyCorrection::variation &JERVariation = MyCorrection::variation::nom) const;
    FatJetViewCollection SelectFatJets(const FatJetViewCollection &fatjets,
                                       FatJetView::ID id, float ptmin,
                                       float fetamax) const;
    std::vector<std::size_t> JetsVetoLeptonInside(const JetViewCollection& jets,
                                   const std::vector<std::size_t>& jet_indices,
                                   const ElectronViewCollection& electrons,
                                   const std::vector<std::size_t>& electron_indices,
                                   const MuonViewCollection& muons,
                                   const std::vector<std::size_t>& muon_indices,
                                   const float dR = 0.3) const;

    // Generic overlap removal: keeps the entries of keep_indices that are at
    // least dRmin away from every entry of veto_indices.  Input order is
    // preserved and the returned values index into keep, not into
    // keep_indices.  Chain it to veto against more than one collection.
    template <typename KeepColl, typename VetoColl>
    static std::vector<std::size_t>
    RemoveOverlapIndices(const KeepColl &keep,
                         const std::vector<std::size_t> &keep_indices,
                         const VetoColl &veto,
                         const std::vector<std::size_t> &veto_indices,
                         const float dRmin) {
        if (veto_indices.empty())
            return keep_indices;

        // Read the veto side once.  The inner loop then runs on plain floats
        // instead of paying a column lookup per pair.
        std::vector<float> vetoEta, vetoPhi;
        vetoEta.reserve(veto_indices.size());
        vetoPhi.reserve(veto_indices.size());
        for (const std::size_t j : veto_indices) {
            const auto &b = veto[j];
            vetoEta.push_back(b.Eta());
            vetoPhi.push_back(b.Phi());
        }

        std::vector<std::size_t> selected;
        selected.reserve(keep_indices.size());
        const float dR2min = dRmin * dRmin;
        for (const std::size_t i : keep_indices) {
            const auto &a = keep[i];
            const float eta = a.Eta();
            const float phi = a.Phi();
            bool overlaps = false;
            for (std::size_t j = 0; j < vetoEta.size(); ++j) {
                if (SKNano::Geometry::DeltaR2(eta, phi, vetoEta[j],
                                              vetoPhi[j]) < dR2min) {
                    overlaps = true;
                    break;
                }
            }
            if (!overlaps)
                selected.push_back(i);
        }
        return selected;
    }
    // Functions
    float GetScaleVariation(const MyCorrection::variation &muF_syst, const MyCorrection::variation &muR_syst);
    float GetPSWeight(const MyCorrection::variation &ISR_syst, const MyCorrection::variation &FSR_syst);
    inline float GetBTaggingWP(const JetTagging::JetFlavTagger &tagger, const JetTagging::JetFlavTaggerWP &wp) { return myCorr->GetBTaggingWP(tagger, wp); }
    inline pair<float, float> GetCTaggingWP(const JetTagging::JetFlavTagger &tagger, const JetTagging::JetFlavTaggerWP &wp) { return myCorr->GetCTaggingWP(tagger, wp); }
    inline float GetBTaggingWP(){ return myCorr->GetBTaggingWP(); }
    inline pair<float, float> GetCTaggingWP(){ return myCorr->GetCTaggingWP(); }
    float GetHT(const JetViewCollection &jets, const std::vector<std::size_t> &indices,
                const MyCorrection::variation &JESVariation = MyCorrection::variation::nom,
                const MyCorrection::variation &JERVariation = MyCorrection::variation::nom) const;
    float GetHT(const SelectedJetViewCollection &jets) const;
    bool IsHEMElectron(const ElectronView &electron) const;

    // Scale and smear
    void METType1Propagation(Particle &MET, RVec<Particle> &original_objects, RVec<Particle> &corrected_objects);
    struct Type1METInfo {
        Particle met;
        MyCorrection::variation jesVar = MyCorrection::variation::nom;
        MyCorrection::variation jerVar = MyCorrection::variation::nom;
        bool skip = false;
    };
    Type1METInfo PropagateType1MET(const Event &ev, JetViewCollection &jets, const std::string &systTarget, MyCorrection::variation variation, const TString &systSource, bool doBreakdown = false, Event::MET_Type met_type = Event::MET_Type::PUPPI) const;
    float GetL1PrefireWeight(MyCorrection::variation syst = MyCorrection::variation::nom);
    unordered_map<int, int> deltaRMatching(const RVec<Particle> &objs1, const RVec<Particle> &objs2, const float dR = 0.4);
    template <typename Collection1, typename Collection2>
    unordered_map<int, int> deltaRMatchingViews(
        const Collection1 &objs1, const Collection2 &objs2,
        const float dR = 0.4) const {
      std::vector<std::tuple<std::size_t, std::size_t, float>> candidates;
      std::vector<bool> used1(objs1.size(), false);
      std::vector<bool> used2(objs2.size(), false);
      for (std::size_t i = 0; i < objs1.size(); ++i) {
        for (std::size_t j = 0; j < objs2.size(); ++j) {
          const float distance = objs1[i].P4().DeltaR(objs2[j].P4());
          if (distance < dR)
            candidates.emplace_back(i, j, distance);
        }
      }
      std::sort(candidates.begin(), candidates.end(),
                [](const auto &lhs, const auto &rhs) {
                  return std::get<2>(lhs) < std::get<2>(rhs);
                });
      unordered_map<int, int> matched;
      for (const auto &candidate : candidates) {
        const auto i = std::get<0>(candidate);
        const auto j = std::get<1>(candidate);
        if (used1[i] || used2[j])
          continue;
        matched[static_cast<int>(i)] = static_cast<int>(j);
        used1[i] = true;
        used2[j] = true;
      }
      for (std::size_t i = 0; i < objs1.size(); ++i)
        if (!used1[i])
          matched[static_cast<int>(i)] = -999;
      return matched;
    }

    template <typename RecoJets, typename GenJets>
    unordered_map<int, int> GenJetMatchingViews(
        const RecoJets &jets, const GenJets &genjets, const float rho,
        const float dR = 0.2, const float pTJerCut = 3.) const {
      std::vector<std::tuple<int, int, float, float>> candidates;
      for (std::size_t i = 0; i < jets.size(); ++i) {
        for (std::size_t j = 0; j < genjets.size(); ++j) {
          const TLorentzVector recoP4 = [&]() {
            if constexpr (requires { jets[i].P4(); })
              return jets[i].P4();
            else
              return TLorentzVector(jets[i]);
          }();
          const float distance = recoP4.DeltaR(genjets[j].P4());
          const float ptDiff = std::fabs(jets[i].Pt() - genjets[j].Pt());
          const float jer = myCorr->GetJER(jets[i].Eta(), jets[i].Pt(), rho) *
                            jets[i].Pt();
          if (distance < dR && ptDiff < pTJerCut * jer)
            candidates.emplace_back(static_cast<int>(i), static_cast<int>(j),
                                    distance, ptDiff);
        }
      }
      std::sort(candidates.begin(), candidates.end(),
                [](const auto &lhs, const auto &rhs) {
                  if (std::get<2>(lhs) == std::get<2>(rhs))
                    return std::get<3>(lhs) < std::get<3>(rhs);
                  return std::get<2>(lhs) < std::get<2>(rhs);
                });
      std::vector<bool> usedReco(jets.size(), false);
      std::vector<bool> usedGen(genjets.size(), false);
      unordered_map<int, int> matched;
      for (const auto &candidate : candidates) {
        const int i = std::get<0>(candidate);
        const int j = std::get<1>(candidate);
        if (usedReco[i] || usedGen[j])
          continue;
        matched[i] = j;
        usedReco[i] = true;
        usedGen[j] = true;
      }
      for (std::size_t i = 0; i < jets.size(); ++i)
        if (!usedReco[i])
          matched[static_cast<int>(i)] = -999;
      return matched;
    }
    template <typename ViewCollection>
    std::vector<int> MatchViewsToTrigObjs(const ViewCollection &objects, const TrigObjViewCollection &trigobjs, float dR = 0.4f, int trigId = -1) const;
    bool PrepareJetJESVariations(JetViewCollection &jets, const TString &source, bool doBreakdown) const;
    void ApplyJetScaleVariation(JetViewCollection &jets, const TString &source = "total") const;
    bool PropagateJetSystToMET(const JetViewCollection &jets, Particle &met,
                               const MyCorrection::variation &jesVar = MyCorrection::variation::nom,
                               const MyCorrection::variation &jerVar = MyCorrection::variation::nom) const;
    
    // Histogram Handlers
    TFile* GetOutfile() { return outfile; }
    void SetOutfilePath(const TString &outpath);
    TH1D* GetHist1D(const string &histname);
    Hist1DHandle BookHist1D(std::string_view histname, int nbin,
                            double xmin, double xmax);
    Hist1DHandle BookHist1D(std::string_view histname, int nbin,
                            const float *bins);
    Hist2DHandle BookHist2D(std::string_view histname, int nbinx,
                            double xmin, double xmax, int nbiny,
                            double ymin, double ymax);
    Hist2DHandle BookHist2D(std::string_view histname, int nbinx,
                            const float *xbins, int nbiny,
                            const float *ybins);
    Hist3DHandle BookHist3D(std::string_view histname, int nbinx,
                            double xmin, double xmax, int nbiny,
                            double ymin, double ymax, int nbinz,
                            double zmin, double zmax);
    Hist3DHandle BookHist3D(std::string_view histname, int nbinx,
                            const float *xbins, int nbiny,
                            const float *ybins, int nbinz,
                            const float *zbins);
    bool PassJetVetoMap(const JetViewCollection &AllJets, const TString mapCategory="jetvetomap");
    inline void FillCutFlow(const int &val,const int &maxCutN=10){
        if (!cutFlowHist) {
            cutFlowMax = maxCutN;
            cutFlowHist = BookHist1D("CutFlow", cutFlowMax, 0., cutFlowMax);
        } else if (cutFlowMax != maxCutN) {
            throw SKNano::ConfigError(
                "[AnalyzerCore::FillCutFlow] incompatible maxCutN");
        }
        cutFlowHist.Fill(val, 1.);
    }
    void FillHist(std::string_view histname, float value, float weight, int n_bin, float x_min, float x_max);
    void FillHist(std::string_view histname, float value, float weight, int n_bin, float *xbins);
    void FillHist(std::string_view histname, float value_x, float value_y, float weight, 
                                          int n_binx, float x_min, float x_max, 
                                          int n_biny, float y_min, float y_max);
    void FillHist(std::string_view histname, float value_x, float value_y, float weight,
                                          int n_binx, float *xbins,
                                          int n_biny, float *ybins);
    void FillHist(std::string_view histname, float value_x, float value_y, float value_z, float weight,
                                          int n_binx, float x_min, float x_max,
                                          int n_biny, float y_min, float y_max,
                                          int n_binz, float z_min, float z_max);
    void FillHist(std::string_view histname, float value_x, float value_y, float value_z, float weight,
                                          int n_binx, float *xbins,
                                          int n_biny, float *ybins,
                                          int n_binz, float *zbins);
    inline void FillHist(std::string_view histname, float value, float weight, const RVec<float> &xbins) { FillHist(histname, value, weight, xbins.size()-1, const_cast<float*>(xbins.data())); } 
    inline void FillHist(std::string_view histname, float value_x, float value_y, float weight, const RVec<float> &xbins, const RVec<float> &ybins) {FillHist(histname, value_x, value_y, weight, xbins.size() - 1, const_cast<float *>(xbins.data()), ybins.size() - 1, const_cast<float *>(ybins.data())); }
    inline void FillHist(std::string_view histname, float value_x, float value_y, float value_z, float weight, const RVec<float> &xbins, const RVec<float> &ybins, const RVec<float> &zbins) {FillHist(histname, value_x, value_y, value_z, weight, xbins.size() - 1, const_cast<float *>(xbins.data()), ybins.size() - 1, const_cast<float *>(ybins.data()), zbins.size() - 1, const_cast<float *>(zbins.data())); }


    OutputRegistry Output() { return OutputRegistry(this); }
    HistogramGroup Hists(std::string_view prefix = {});
    virtual void WriteHist();

protected:
    // Register all tasks before InitializeTasks(). Task flags are additive;
    // when none is explicit, only tasks marked enabledByDefault are selected.
    void RegisterTask(std::string flag, TaskOptions options,
                      TaskHook validate, TaskHook book, TaskHook run);
    void InitializeTasks(std::string_view enableAllFlag = {});
    bool HasTasksForSystematic(bool isCentralSystematic) const;
    void RunTasks(bool isCentralSystematic);

    // Queue a schema-preserving skim of the input dataset. The selected global
    // entry numbers are snapshotted as an RNTuple during WriteHist(), after the
    // regular output file has been closed.
    void SnapshotSelectedInput(std::vector<Long64_t> entries,
                               std::string outputName = "Events");

    bool PassNoiseFilterCommon(const JetViewCollection &AllJets, const Event &ev, Event::MET_Type met_type) const;
    std::shared_ptr<JetSoA> CreateJetSoA() const;
    void PopulateJetNominal(const std::shared_ptr<JetSoA> &storage);
    void PopulateJetJERVariations(const std::shared_ptr<JetSoA> &storage);
    void InitialiseJetSystematics(JetSoA &storage) const;
    void PopulateJetStorageWithoutCorrections(JetSoA &storage) const;
    void ApplyJetEnergyCorrections(JetSoA &storage, float rho);
    const std::vector<int> &MatchJetsToGenJets(const JetViewCollection& jets,
    const GenJetViewCollection& genjets,
    float rho) const;
    void ApplyJetSmearingAndUncertainties(const JetViewCollection& jets,
    const std::vector<int>& matchedGenIdx,
    const GenJetViewCollection& genjets,
    bool isMC,
    float rho);
    const std::vector<TString> &JetEnergyScaleSources() const;
    bool useTH1F = false;
    // Reused by the per-event jet/genjet matcher. Keeping the capacity on the
    // analyzer removes allocator traffic without changing matching order.
    mutable std::vector<int> jetMatchIndicesScratch;
    mutable std::vector<std::tuple<std::size_t, std::size_t, float, float>>
        jetMatchCandidatesScratch;
    mutable std::vector<unsigned char> jetMatchUsedJetScratch;
    mutable std::vector<unsigned char> jetMatchUsedGenScratch;
    absl::flat_hash_map<std::string, TH1*, TransparentStringHash, TransparentStringEq> histmap1d;
    absl::flat_hash_map<std::string, TH2*, TransparentStringHash, TransparentStringEq> histmap2d;
    absl::flat_hash_map<std::string, TH3*, TransparentStringHash, TransparentStringEq> histmap3d;
    Hist1DHandle cutFlowHist;
    int cutFlowMax = -1;
    static constexpr Long64_t kInvalidCacheEntry = -2;
    Long64_t cachedEventEntry = kInvalidCacheEntry;
    Event cachedEvent;
    Long64_t cachedMuonViewsEntry = kInvalidCacheEntry;
    MuonViewCollection cachedMuonViews;
    Long64_t cachedElectronViewsEntry = kInvalidCacheEntry;
    ElectronViewCollection cachedElectronViews;
    Long64_t cachedJetViewsEntry = kInvalidCacheEntry;
    JetViewCollection cachedJetViews;
    Long64_t cachedGenViewsEntry = kInvalidCacheEntry;
    GenViewCollection cachedGenViews;
    Long64_t cachedGenJetViewsEntry = kInvalidCacheEntry;
    GenJetViewCollection cachedGenJetViews;
    std::unordered_map<std::string, std::unique_ptr<float>> scalar_float_storage;
    std::unordered_map<std::string, std::unique_ptr<int>> scalar_int_storage;
    std::unordered_map<std::string, std::unique_ptr<bool>> scalar_bool_storage;
    std::unordered_map<std::string, ModellingPatch> modelling_patches;
    nlohmann::json modelling_json;
    std::unique_ptr<AnalyzerTaskRegistryState> taskRegistryState_; //!
    std::unordered_map<std::string, std::shared_ptr<RNTupleOutputState>>
        rntupleOutputs_; //!
    TFile *outfile;
    std::string outputFinalPath_;
    std::string outputPartialPath_;
    bool outputFinalized_ = false;
    unsigned int outputCompressionThreads_ = 1;
    std::vector<Long64_t> selectedInputEntries_;
    std::string selectedInputOutputName_;
    RNTupleHandle BookRNTuple(
        const TString &ntupleName,
        RNTupleOutputProfile profile = RNTupleOutputProfile::Fast);
    RNTupleHandle OutputRNTuple(const TString &ntupleName);
    void RegisterRNTupleField(
        const std::string &ntupleName, const std::string &fieldName,
        std::type_index type, const void *address,
        std::function<void(ROOT::RNTupleModel &)> addField,
        std::function<void(ROOT::Detail::RRawPtrWriteEntry &)> bindField);
    void FillRNTuple(const std::string &ntupleName);
    void MarkRNTupleFieldHalfPrecision(const std::string &ntupleName,
                                       const std::string &fieldName);
    std::uint64_t GetRNTupleEntries(const std::string &ntupleName) const;
    bool HasRNTupleOutput(std::string_view name) const noexcept;
    void FinalizeRNTuples();
    void SetRNTupleValue(const TString &ntupleName, const TString &fieldName,
                         float value);
    void SetRNTupleValue(const TString &ntupleName, const TString &fieldName,
                         double value);
    void SetRNTupleValue(const TString &ntupleName, const TString &fieldName,
                         int value);
    void SetRNTupleValue(const TString &ntupleName, const TString &fieldName,
                         bool value);
    void ValidateTreePath(std::string_view treeName) const;
    void ValidateHistogramPath(std::string_view histogramName) const;
};

template <typename T>
AnalyzerCore::RNTupleHandle &
AnalyzerCore::RNTupleHandle::Field(std::string_view name, T &value) {
    RequireValid();
    using Value = std::remove_cv_t<T>;
    const std::string fieldName(name);
    owner_->RegisterRNTupleField(
        ntupleName_, fieldName, std::type_index(typeid(Value)),
        static_cast<const void *>(&value),
        [fieldName](ROOT::RNTupleModel &model) {
            model.MakeField<Value>(fieldName);
        },
        [fieldName, address = &value](ROOT::Detail::RRawPtrWriteEntry &entry) {
            entry.BindRawPtr<Value>(fieldName, address);
        });
    return *this;
}

template <typename T>
AnalyzerCore::OutputField<T>
AnalyzerCore::RNTupleHandle::MakeField(std::string_view name) {
    RequireValid();
    const std::string fieldName(name);
    auto value = std::make_shared<T>();
    owner_->RegisterRNTupleField(
        ntupleName_, fieldName, std::type_index(typeid(T)), value.get(),
        [fieldName](ROOT::RNTupleModel &model) {
            model.MakeField<T>(fieldName);
        },
        [fieldName, value](ROOT::Detail::RRawPtrWriteEntry &entry) {
            entry.BindRawPtr<T>(fieldName, value.get());
        });
    return OutputField<T>(std::move(value));
}

inline bool AnalyzerCore::PassNoiseFilterCommon(const JetViewCollection &AllJets, const Event &ev, Event::MET_Type met_type) const
{
    if (Run == 2)
    {
        bool passNoiseFilter = Flag_goodVertices
                            && Flag_globalSuperTightHalo2016Filter
                            && Flag_HBHENoiseFilter
                            && Flag_HBHENoiseIsoFilter
                            && Flag_EcalDeadCellTriggerPrimitiveFilter
                            && Flag_BadPFMuonFilter
                            && Flag_BadPFMuonDzFilter
                            && Flag_hfNoisyHitsFilter
                            && Flag_eeBadScFilter;
        if (DataEra.Contains("2017") || DataEra.Contains("2018"))
            passNoiseFilter = passNoiseFilter && Flag_ecalBadCalibFilter;
        return passNoiseFilter;
    }

    bool passNoiseFilter = Flag_goodVertices
                        && Flag_globalSuperTightHalo2016Filter
                        && Flag_EcalDeadCellTriggerPrimitiveFilter
                        && Flag_BadPFMuonFilter
                        && Flag_BadPFMuonDzFilter
                        && Flag_hfNoisyHitsFilter
                        && Flag_eeBadScFilter;
    if (!passNoiseFilter)
        return false;

    if (!IsDATA)
        return passNoiseFilter;
    if (!(362433 <= RunNumber && RunNumber <= 367144))
        return passNoiseFilter;

    const Particle METv = ev.GetMETVector(met_type);
    if (METv.Pt() <= 100.)
        return passNoiseFilter;

    auto problematicJetIndices = SelectJetIndices(AllJets, JetView::JetID::NOCUT, 50., 0.5);
    for (const auto idx : problematicJetIndices)
    {
        const auto jet = AllJets[idx];
        const float dphi = std::acos(std::cos(jet.Phi() - METv.Phi()));
        bool badEcal = (jet.CorrectedPt() > 50.);
        badEcal = badEcal && (-0.5 < jet.Eta() && jet.Eta() < -0.1);
        badEcal = badEcal && (-2.1 < jet.Phi() && jet.Phi() < -1.8);
        badEcal = badEcal && (jet.NeEmEF() > 0.9 || jet.ChEmEF() > 0.9);
        badEcal = badEcal && dphi > 2.9;
        if (badEcal)
            return false;
    }
    return true;
}

template <typename ViewCollection>
std::vector<int> AnalyzerCore::MatchViewsToTrigObjs(
    const ViewCollection &objects, const TrigObjViewCollection &trigobjs,
    float dR, int trigId) const {
  const std::size_t nObj = objects.size();
  std::vector<int> matchedIdx(nObj, -999);
  if (nObj == 0 || trigobjs.size() == 0)
    return matchedIdx;

  const auto &storage = trigobjs.storage();
  if (!storage)
    return matchedIdx;

  const std::size_t nTrig = storage->size();
  if (nTrig == 0)
    return matchedIdx;

  const float maxDR2 = dR * dR;

  std::vector<std::tuple<std::size_t, std::size_t, float>> candidates;
  candidates.reserve(nObj * 2);

  for (std::size_t i = 0; i < nObj; ++i) {
    const auto &obj = objects[i];
    const float objEta = obj.Eta();
    const float objPhi = obj.Phi();
    for (std::size_t j = 0; j < nTrig; ++j) {
      if (trigId >= 0 && static_cast<int>(storage->id[j]) != trigId)
        continue;

      const float dr2 = SKNano::Geometry::DeltaR2(objEta, objPhi,
                                                  storage->eta[j],
                                                  storage->phi[j]);
      if (dr2 < maxDR2)
        candidates.emplace_back(i, j, dr2);
    }
  }

  std::sort(candidates.begin(), candidates.end(),
            [](const auto &a, const auto &b) {
              return std::get<2>(a) < std::get<2>(b);
            });

  std::vector<bool> usedObj(nObj, false);
  std::vector<bool> usedTrig(nTrig, false);
  for (const auto &candidate : candidates) {
    const std::size_t objIdx = std::get<0>(candidate);
    const std::size_t trigIdx = std::get<1>(candidate);
    if (usedObj[objIdx] || usedTrig[trigIdx])
      continue;

    matchedIdx[objIdx] = static_cast<int>(trigIdx);
    usedObj[objIdx] = true;
    usedTrig[trigIdx] = true;
  }

  return matchedIdx;
}

#endif
