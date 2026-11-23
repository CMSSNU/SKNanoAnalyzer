#ifndef MUONVIEW_H
#define MUONVIEW_H

#include <cmath>
#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include "EventRange.h"
#include "TLorentzVector.h"
#include "TString.h"
#include "LeptonIDEnums.h"
#include "ViewColumns.h"

/**
 * Lightweight view over nanoAOD muon branches.
 * Exposes read-only accessors mirroring the high-level Muon API
 * without materialising per-object payloads.
 */

// Intermediates of the MUO POG nominal correction, kept on the SoA so the
// variation stage reuses them instead of re-running the nominal chain.
struct MuonScaReState {
    double scaledPt = 0.;    // after the scale step (POG pt_woresol)
    double correctedPt = 0.; // after the smearing step (POG pt_wresol)
    double k = 0.;           // residual smearing factor; 0 when not smeared
};

struct MuonSoA {
    ColumnView<float> pt;
    ColumnView<float> eta;
    ColumnView<float> phi;
    ColumnView<float> mass;
    ColumnView<int> charge;
    ColumnView<float> tkRelIso;
    ColumnView<float> pfRelIso03;
    ColumnView<float> pfRelIso04;
    ColumnView<float> miniPFRelIsoAll;
    ColumnView<float> dxy;
    ColumnView<float> dxyErr;
    ColumnView<float> dz;
    ColumnView<float> dzErr;
    ColumnView<float> ip3d;
    ColumnView<float> sip3d;
    ColumnView<unsigned char> highPtId;
    BoolColumnView looseId;
    BoolColumnView mediumId;
    BoolColumnView mediumPromptId;
    BoolColumnView tightId;
    BoolColumnView softId;
    BoolColumnView softMvaId;
    BoolColumnView triggerLooseId;
    ColumnView<unsigned char> miniIsoId;
    ColumnView<unsigned char> multiIsoId;
    ColumnView<unsigned char> mvaMuId;
    ColumnView<unsigned char> pfIsoId;
    ColumnView<unsigned char> puppiIsoId;
    ColumnView<unsigned char> tkIsoId;
    ColumnView<unsigned char> nTrackerLayers;
    ColumnView<float> tunepRelPt;
    ColumnView<float> softMva;
    ColumnView<float> softMvaRun3;
    ColumnView<float> mvaLowPt;
    ColumnView<float> mvaPrompt;
    ColumnView<unsigned char> genPartFlav;
    ColumnView<short> genPartIdx;
    ColumnView<short> jetIdx;

    std::vector<float> correctedPt;
    std::vector<float> miniAODPt;
    std::vector<MuonScaReState> scaRe; // POG recipe only
    std::vector<float> momentumScaleUp;
    std::vector<float> momentumScaleDown;
    // Resolution variation of the medium-pT correction. Only the MUO POG
    // scale/smearing recipe fills these with something other than
    // correctedPt; Rochester has no resolution nuisance of its own.
    std::vector<float> momentumResUp;
    std::vector<float> momentumResDown;
    // High-pT lanes. Filled alongside the ones above so a muon carries both
    // treatments and the analyzer picks which one it selected on.
    std::vector<float> tunePPt;
    std::vector<unsigned char> highPtRegime;
    std::vector<float> highPtPt;
    // Providers, bound by AnalyzerCore. Each stage is materialised on first
    // access to one of its lanes and never before: the nominal path pays for
    // the nominal only.
    std::function<void()> populateMomentum;            // correctedPt, miniAODPt, scaRe
    std::function<void()> populateMomentumVariations;  // momentumScale*/Res*
    std::function<void()> populateHighPt;              // tunePPt, highPtRegime, highPtPt
    mutable bool momentumReady = false;
    mutable bool momentumComputing = false;
    mutable bool momentumVariationsReady = false;
    mutable bool momentumVariationsComputing = false;
    mutable bool highPtReady = false;
    mutable bool highPtComputing = false;

    std::size_t size() const { return pt.size(); }
    std::size_t rawSize() const { return size(); }

    void ensureMomentum() const {
        // Validate the event epoch before a provider reads several branches.
        static_cast<void>(pt.size());
        materialise(populateMomentum, momentumReady, momentumComputing,
                    "momentum");
    }
    void ensureMomentumVariations() const {
        ensureMomentum();
        materialise(populateMomentumVariations, momentumVariationsReady,
                    momentumVariationsComputing, "momentum variation");
    }
    void ensureHighPt() const {
        ensureMomentum();
        materialise(populateHighPt, highPtReady, highPtComputing, "high-pT");
    }

private:
    // Every stage is bound by AnalyzerCore; a missing provider would leave
    // its lanes empty, so it is an error rather than a fallback.
    static void materialise(const std::function<void()> &provider, bool &ready,
                            bool &computing, const char *what) {
        if (ready)
            return;
        if (!provider)
            throw SKNano::LogicError(std::string("[MuonSoA] ") + what +
                                     " provider is not bound");
        if (computing)
            throw SKNano::LogicError(std::string("[MuonSoA] recursive ") + what +
                                     " computation");
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

class MuonView {
public:
    using WorkingPoint = LeptonID::MuonWorkingPoint;
    using MuonID = LeptonID::MuonID;

    MuonView() = default;
    MuonView(const MuonSoA *data, std::size_t index)
        : store(data), idx(index) {}

    bool valid() const { return store && idx < store->rawSize(); }

    float Pt() const { assertCurrentEvent(); return store->correctedPt[idx]; }
    float Eta() const { return store->eta[idx]; }
    float Phi() const { return store->phi[idx]; }
    float M() const { return store->mass[idx]; }
    int Charge() const { return store->charge[idx]; }

    float MiniAODPt() const { assertCurrentEvent(); return store->miniAODPt[idx]; }
    float MomentumScaleUp() const {
        store->ensureMomentumVariations();
        return lane(store->momentumScaleUp);
    }
    float MomentumScaleDown() const {
        store->ensureMomentumVariations();
        return lane(store->momentumScaleDown);
    }
    // Resolution nuisance of the medium-pT correction (simulation only; equals
    // Pt() on data and for eras corrected with Rochester).
    float MomentumResUp() const {
        store->ensureMomentumVariations();
        return lane(store->momentumResUp);
    }
    float MomentumResDown() const {
        store->ensureMomentumVariations();
        return lane(store->momentumResDown);
    }

    // ---- High-pT treatment (MUO POG "High pT" prescription) -----------------
    // Opt-in: the default Pt() above is unchanged. Use SelectHighPtMuonIndices
    // and these accessors when the analysis runs muons past ~200 GeV.

    // TuneP momentum before any correction. This is what decides the regime.
    float TunePPt() const {
        store->ensureHighPt();
        return lane(store->tunePPt);
    }
    // Latched at populate time from the pre-correction TuneP pt. Deciding this
    // later, from corrected values, would misclassify any muon that the scale
    // correction or the smearing pushed across the boundary.
    bool IsHighPtRegime() const {
        store->ensureHighPt();
        return lane(store->highPtRegime) != 0;
    }
    // Below the boundary this is the medium-pT momentum, Pt(); above it, the
    // uncorrected TuneP momentum.
    float HighPtPt() const {
        store->ensureHighPt();
        return lane(store->highPtPt);
    }
    // Full momentum from the high-pT lane. The reco scale factor is binned in
    // p rather than pt, so callers need this rather than HighPtPt().
    float HighPtMomentum() const {
        return HighPtPt() * std::cosh(Eta());
    }

    float TkRelIso() const { return store->tkRelIso[idx]; }
    float PfRelIso03() const { return store->pfRelIso03[idx]; }
    float PfRelIso04() const { return store->pfRelIso04[idx]; }
    float MiniPFRelIso() const { return store->miniPFRelIsoAll[idx]; }
    float dXY() const { return store->dxy[idx]; }
    float dXYerr() const { return store->dxyErr[idx]; }
    float dZ() const { return store->dz[idx]; }
    float dZerr() const { return store->dzErr[idx]; }
    float IP3D() const { return store->ip3d[idx]; }
    float SIP3D() const { return store->sip3d[idx]; }

    unsigned char nTrackerLayers() const { return store->nTrackerLayers[idx]; }
    float SoftMva() const { return store->softMva[idx]; }
    float MvaLowPt() const { return store->mvaLowPt[idx]; }
    float MvaPrompt() const { return store->mvaPrompt[idx]; }
    float SoftMvaRun3() const { return store->softMvaRun3[idx]; }

    unsigned char GenPartFlav() const {
        return store->genPartFlav.available() ? store->genPartFlav[idx] : 0;
    }
    short GenPartIdx() const {
        return store->genPartIdx.available() ? store->genPartIdx[idx] : -999;
    }
    short JetIdx() const { return store->jetIdx[idx]; }

    TLorentzVector P4() const {
        TLorentzVector v;
        v.SetPtEtaPhiM(Pt(), Eta(), Phi(), M());
        return v;
    }

    float DeltaR(const TLorentzVector &other) const { return P4().DeltaR(other); }
    float DeltaR(const MuonView &other) const { return P4().DeltaR(other.P4()); }

    bool isPOGTightId() const { return store->tightId[idx]; }
    bool isPOGMediumId() const { return store->mediumId[idx]; }
    bool isPOGMediumPromptId() const { return store->mediumPromptId[idx]; }
    bool isPOGLooseId() const { return store->looseId[idx]; }
    bool isPOGSoftId() const { return store->softId[idx]; }
    bool isPOGSoftMvaId() const { return store->softMvaId[idx]; }
    bool isPOGTriggerIdLoose() const { return store->triggerLooseId[idx]; }

    bool PassID(MuonID id) const;
    bool PassID(const TString &id) const;

private:
    bool workingPointAtLeast(const ColumnView<unsigned char> &column, WorkingPoint target) const {
        return column[idx] >= static_cast<unsigned char>(target);
    }

    bool Pass_HcToWATight() const;
    bool Pass_HcToWALoose() const;

    void assertCurrentEvent() const {
        static_cast<void>(store->rawSize());
        store->ensureMomentum();
    }
    // A stage that ran fills every lane it owns, data included; an empty lane
    // is a provider bug, not a cue to fall back to the nominal.
    template <typename T>
    T lane(const std::vector<T> &values) const {
        if (idx >= values.size())
            throw SKNano::LogicError("[MuonView] correction lane is not populated");
        return values[idx];
    }
    const MuonSoA *store = nullptr;
    std::size_t idx = 0;
};

using MuonViewCollection = EventRange<MuonSoA, MuonView>;

#endif // MUONVIEW_H
