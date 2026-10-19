#ifndef FATJETVIEW_H
#define FATJETVIEW_H

#include <array>
#include <cstddef>
#include <functional>
#include <iterator>
#include <string>
#include <vector>

#include "AnalysisException.h"
#include "EventRange.h"
#include "JetTaggingParameter.h"
#include "TLorentzVector.h"
#include "ViewColumns.h"

struct FatJetSoA {
    static constexpr std::size_t taggerCount = 4;
    static constexpr std::size_t scoreCount =
        static_cast<std::size_t>(JetTagging::FatJetTaggerScoreType::MassCorrX2p) + 1;

    ColumnView<float> pt;
    ColumnView<float> eta;
    ColumnView<float> phi;
    ColumnView<float> mass;
    ColumnView<float> area;
    ColumnView<float> rawFactor;
    ColumnView<float> chEmEF;
    ColumnView<float> chHEF;
    ColumnView<float> hfEmEF;
    ColumnView<float> hfHEF;
    ColumnView<float> muEF;
    ColumnView<float> neEmEF;
    ColumnView<float> neHEF;
    ColumnView<short> chMultiplicity;
    ColumnView<short> neMultiplicity;
    ColumnView<unsigned char> nConstituents;
    ColumnView<short> genJetAK8Idx;
    ColumnView<short> subJetIdx1;
    ColumnView<short> subJetIdx2;
    ColumnView<unsigned char> hadronFlavour;
    ColumnView<float> softDropMass;
    ColumnView<float> tau1;
    ColumnView<float> tau2;
    ColumnView<float> tau3;
    ColumnView<float> n2b1;
    ColumnView<float> n3b1;
    ColumnView<float> lsf3;

    std::array<std::array<ColumnView<float>, scoreCount>, taggerCount> scores;

    ColumnView<int> constituentJetIdx;
    ColumnView<int> constituentPFCandIdx;
    ColumnView<float> pfCandPt;
    ColumnView<float> pfCandEta;
    ColumnView<float> pfCandPhi;
    ColumnView<float> pfCandMass;
    ColumnView<int> pfCandPdgId;
    ColumnView<float> pfCandPuppiWeight;

    // JEC/JER lanes, mirroring JetSoA. NanoAOD stores the momentum with the
    // JEC that production happened to use; these carry the current one plus
    // the smearing and its variations. Filled lazily on first access.
    std::vector<float> correctedPt;
    std::vector<float> correctedMass;
    std::vector<float> smearedPtNominal;
    std::vector<float> smearedMassNominal;
    std::vector<float> smearedPtUp;
    std::vector<float> smearedPtDown;
    std::vector<float> smearedMassUp;
    std::vector<float> smearedMassDown;
    std::vector<float> jesPtUp;
    std::vector<float> jesPtDown;
    std::vector<float> jesMassUp;
    std::vector<float> jesMassDown;
    // SoftDrop mass follows the same set: the subjet corrections propagate
    // into it, and the nominal one is not a no-op.
    std::vector<float> sdMassNominal;
    std::vector<float> sdMassJesUp;
    std::vector<float> sdMassJesDown;
    std::vector<float> sdMassJerUp;
    std::vector<float> sdMassJerDown;

    // Intermediates of the nominal pass, kept so the variation passes shift
    // the same draw and the same gen match instead of re-rolling them.
    std::vector<float> jerUnitDraw;
    std::vector<float> jerMatchedGenPt;   // < 0: no match, stochastic term
    std::vector<float> jerResolution;
    std::vector<float> sdReferenceMass;   // <= 0: no subjet pair, m_SD untouched
    std::vector<std::array<float, 2>> sdSubDraw;
    std::vector<std::array<float, 2>> sdSubGenPt;
    std::vector<std::array<float, 2>> sdSubResolution;

    // Providers, bound by AnalyzerCore. Staged like JetSoA: the nominal pass
    // is what CorrectedPt()/SmearedPtNominal()/SDMassNominal() read, and a
    // JER or JES variation lane materialises its own pass on first access.
    std::function<void()> populateNominal;
    std::function<void()> populateJerVariations;
    std::function<void()> populateJesVariations;
    mutable bool nominalReady = false;
    mutable bool nominalComputing = false;
    mutable bool jerVariationsReady = false;
    mutable bool jerVariationsComputing = false;
    mutable bool jesVariationsReady = false;
    mutable bool jesVariationsComputing = false;

    // No provider bound leaves the lanes empty; accessors fall back to the raw
    // values.
    void ensureNominal() const {
        // Validate the event epoch before a provider reads several branches.
        static_cast<void>(pt.size());
        materialise(populateNominal, nominalReady, nominalComputing, "nominal");
    }
    void ensureJerVariations() const {
        ensureNominal();
        materialise(populateJerVariations, jerVariationsReady,
                    jerVariationsComputing, "JER variation");
    }
    void ensureJesVariations() const {
        ensureNominal();
        materialise(populateJesVariations, jesVariationsReady,
                    jesVariationsComputing, "JES variation");
    }

    std::size_t size() const { return pt.size(); }
    ColumnView<float> &score(JetTagging::FatJetTagger tagger,
                             JetTagging::FatJetTaggerScoreType type) {
        return scores[static_cast<std::size_t>(tagger)]
                     [static_cast<std::size_t>(type)];
    }
    const ColumnView<float> &score(JetTagging::FatJetTagger tagger,
                                   JetTagging::FatJetTaggerScoreType type) const {
        return scores[static_cast<std::size_t>(tagger)]
                     [static_cast<std::size_t>(type)];
    }

private:
    static void materialise(const std::function<void()> &provider, bool &ready,
                            bool &computing, const char *what) {
        if (ready || !provider)
            return;
        if (computing)
            throw SKNano::LogicError(std::string("[FatJetSoA] recursive ") +
                                     what + " computation");
        computing = true;
        try {
            provider();
            ready = true;
            computing = false;
        } catch (...) {
            computing = false;
            throw;
        }
    }
};

class FatJetConstituentView {
public:
    FatJetConstituentView() = default;
    FatJetConstituentView(const FatJetSoA *storage, std::size_t pfIndex)
        : store_(storage), pfIndex_(pfIndex) {}

    float Pt() const { return store_->pfCandPt[pfIndex_]; }
    float Eta() const { return store_->pfCandEta[pfIndex_]; }
    float Phi() const { return store_->pfCandPhi[pfIndex_]; }
    float M() const { return store_->pfCandMass[pfIndex_]; }
    int PdgId() const { return store_->pfCandPdgId[pfIndex_]; }
    float PUPPIWeight() const {
        return store_->pfCandPuppiWeight.available()
                   ? store_->pfCandPuppiWeight[pfIndex_]
                   : 1.f;
    }
    TLorentzVector P4() const {
        TLorentzVector value;
        value.SetPtEtaPhiM(Pt(), Eta(), Phi(), M());
        return value;
    }

private:
    const FatJetSoA *store_ = nullptr;
    std::size_t pfIndex_ = 0;
};

class FatJetConstituentRange {
public:
    FatJetConstituentRange() = default;
    FatJetConstituentRange(const FatJetSoA *storage, std::size_t jetIndex)
        : store_(storage), jetIndex_(jetIndex) {}

    std::size_t size() const {
        std::size_t count = 0;
        const std::size_t associationCount = store_->constituentJetIdx.size();
        for (std::size_t i = 0; i < associationCount; ++i)
            if (store_->constituentJetIdx[i] == static_cast<int>(jetIndex_))
                ++count;
        return count;
    }
    bool empty() const { return size() == 0; }

    FatJetConstituentView operator[](std::size_t position) const {
        const std::size_t associationCount = store_->constituentJetIdx.size();
        std::size_t selected = 0;
        for (std::size_t i = 0; i < associationCount; ++i) {
            if (store_->constituentJetIdx[i] != static_cast<int>(jetIndex_))
                continue;
            if (selected++ != position)
                continue;
            const int pfIndex = store_->constituentPFCandIdx[i];
            if (pfIndex < 0 || static_cast<std::size_t>(pfIndex) >= store_->pfCandPt.size())
                throw SKNano::LogicError("[FatJetConstituentRange] invalid PFCand index");
            return FatJetConstituentView(store_, static_cast<std::size_t>(pfIndex));
        }
        throw SKNano::LogicError("[FatJetConstituentRange] index out of range");
    }

    class const_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = FatJetConstituentView;
        using difference_type = std::ptrdiff_t;
        using reference = value_type;
        using pointer = void;
        const_iterator() = default;
        const_iterator(const FatJetConstituentRange *range, std::size_t position)
            : range_(range), position_(position) {}
        value_type operator*() const { return (*range_)[position_]; }
        const_iterator &operator++() { ++position_; return *this; }
        bool operator==(const const_iterator &other) const {
            return range_ == other.range_ && position_ == other.position_;
        }
        bool operator!=(const const_iterator &other) const { return !(*this == other); }
    private:
        const FatJetConstituentRange *range_ = nullptr;
        std::size_t position_ = 0;
    };
    const_iterator begin() const { return const_iterator(this, 0); }
    const_iterator end() const { return const_iterator(this, size()); }

private:
    const FatJetSoA *store_ = nullptr;
    std::size_t jetIndex_ = 0;
};

class FatJetView {
public:
    enum class ID { NOCUT, TIGHT, TIGHTLEPVETO };

    FatJetView() = default;
    FatJetView(const FatJetSoA *storage, std::size_t index)
        : store_(storage), index_(index) {}

    bool valid() const { return store_ && index_ < store_->size(); }
    float Pt() const { return store_->pt[index_]; }
    float Eta() const { return store_->eta[index_]; }
    float Phi() const { return store_->phi[index_]; }
    float M() const { return store_->mass[index_]; }
    float Area() const { return store_->area[index_]; }
    float RawFactor() const { return store_->rawFactor[index_]; }
    float chEmEF() const { return store_->chEmEF[index_]; }
    float chHEF() const { return store_->chHEF[index_]; }
    float hfEmEF() const { return store_->hfEmEF[index_]; }
    float hfHEF() const { return store_->hfHEF[index_]; }
    float muEF() const { return store_->muEF[index_]; }
    float neEmEF() const { return store_->neEmEF[index_]; }
    float neHEF() const { return store_->neHEF[index_]; }
    short chMultiplicity() const { return store_->chMultiplicity[index_]; }
    short neMultiplicity() const { return store_->neMultiplicity[index_]; }
    unsigned char nConstituents() const { return store_->nConstituents[index_]; }
    short GenJetAK8Idx() const { return store_->genJetAK8Idx[index_]; }
    short SubJetIdx1() const { return store_->subJetIdx1[index_]; }
    short SubJetIdx2() const { return store_->subJetIdx2[index_]; }
    unsigned char hadronFlavour() const { return store_->hadronFlavour[index_]; }
    // Raw SoftDrop mass as stored in NanoAOD. Prefer SDMassNominal(), which
    // carries the subjet corrections; the nominal one is not a no-op.
    float SDMass() const { return store_->softDropMass[index_]; }

    // ---- JEC/JER lanes ------------------------------------------------------
    // Pt()/M() above stay raw, matching JetView. Pick a lane explicitly, or let
    // SelectFatJets hand back the projection you asked for.
    float CorrectedPt() const { return nominalLane(store_->correctedPt, Pt()); }
    float CorrectedMass() const { return nominalLane(store_->correctedMass, M()); }
    float SmearedPtNominal() const { return nominalLane(store_->smearedPtNominal, Pt()); }
    float SmearedMassNominal() const { return nominalLane(store_->smearedMassNominal, M()); }
    float SmearedPtUp() const { return jerLane(store_->smearedPtUp, SmearedPtNominal()); }
    float SmearedPtDown() const { return jerLane(store_->smearedPtDown, SmearedPtNominal()); }
    float SmearedMassUp() const { return jerLane(store_->smearedMassUp, SmearedMassNominal()); }
    float SmearedMassDown() const { return jerLane(store_->smearedMassDown, SmearedMassNominal()); }
    float JesPtUp() const { return jesLane(store_->jesPtUp, SmearedPtNominal()); }
    float JesPtDown() const { return jesLane(store_->jesPtDown, SmearedPtNominal()); }
    float JesMassUp() const { return jesLane(store_->jesMassUp, SmearedMassNominal()); }
    float JesMassDown() const { return jesLane(store_->jesMassDown, SmearedMassNominal()); }
    float SDMassNominal() const { return nominalLane(store_->sdMassNominal, SDMass()); }
    float SDMassJesUp() const { return jesLane(store_->sdMassJesUp, SDMassNominal()); }
    float SDMassJesDown() const { return jesLane(store_->sdMassJesDown, SDMassNominal()); }
    float SDMassJerUp() const { return jerLane(store_->sdMassJerUp, SDMassNominal()); }
    float SDMassJerDown() const { return jerLane(store_->sdMassJerDown, SDMassNominal()); }
    float Tau1() const { return store_->tau1[index_]; }
    float Tau2() const { return store_->tau2[index_]; }
    float Tau3() const { return store_->tau3[index_]; }
    float N2b1() const { return store_->n2b1[index_]; }
    float N3b1() const { return store_->n3b1[index_]; }
    float LSF3() const { return store_->lsf3[index_]; }
    std::size_t OriginalIndex() const { return index_; }

    float GetTaggerResult(JetTagging::FatJetTagger tagger,
                          JetTagging::FatJetTaggerScoreType type) const {
        const auto &column = store_->score(tagger, type);
        if (!column.available())
            throw SKNano::LogicError(
                "[FatJetView::GetTaggerResult] score is unavailable for this tagger");
        return column[index_];
    }
    FatJetConstituentRange PFConstituents() const {
        return FatJetConstituentRange(store_, index_);
    }
    TLorentzVector P4() const {
        TLorentzVector value;
        value.SetPtEtaPhiM(Pt(), Eta(), Phi(), M());
        return value;
    }

private:
    // Reads one correction lane, materialising its stage on first access and
    // falling back to the supplied value when no provider is bound.
    float nominalLane(const std::vector<float> &values, const float fallback) const {
        store_->ensureNominal();
        return index_ < values.size() ? values[index_] : fallback;
    }
    float jerLane(const std::vector<float> &values, const float fallback) const {
        store_->ensureJerVariations();
        return index_ < values.size() ? values[index_] : fallback;
    }
    float jesLane(const std::vector<float> &values, const float fallback) const {
        store_->ensureJesVariations();
        return index_ < values.size() ? values[index_] : fallback;
    }

    const FatJetSoA *store_ = nullptr;
    std::size_t index_ = 0;
};

using FatJetViewCollection = EventRange<FatJetSoA, FatJetView>;

#endif // FATJETVIEW_H
