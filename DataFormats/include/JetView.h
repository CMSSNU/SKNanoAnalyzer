#ifndef JETVIEW_H
#define JETVIEW_H

#include <cstddef>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

#include "EventRange.h"
#include "JetIDEnums.h"
#include "JetTaggingParameter.h"
#include "TLorentzVector.h"
#include "ViewColumns.h"

struct JetSoA {
    std::function<void()> populateNominal;
    std::function<void()> populateJerVariations;
    std::function<void()> populateJesVariations;
    mutable bool nominalReady = false;
    mutable bool nominalComputing = false;
    mutable bool jerVariationsReady = false;
    mutable bool jerVariationsComputing = false;
    mutable bool jesVariationsReady = false;
    mutable bool jesVariationsComputing = false;

    ColumnView<float> pt;
    ColumnView<float> eta;
    ColumnView<float> phi;
    ColumnView<float> mass;
    ColumnView<float> rawFactor;
    ColumnView<float> area;
    ColumnView<float> chHEF;
    ColumnView<float> neHEF;
    ColumnView<float> neEmEF;
    ColumnView<float> chEmEF;
    ColumnView<float> muEF;
    ColumnView<short> partonFlavour;
    ColumnView<unsigned char> hadronFlavour;
    ColumnView<unsigned char> chMultiplicity;
    ColumnView<unsigned char> neMultiplicity;
    ColumnView<unsigned char> nConstituents;
    ColumnView<unsigned char> nElectrons;
    ColumnView<unsigned char> nMuons;
    ColumnView<unsigned char> nSVs;
    ColumnView<short> electronIdx1;
    ColumnView<short> electronIdx2;
    ColumnView<short> muonIdx1;
    ColumnView<short> muonIdx2;
    ColumnView<short> svIdx1;
    ColumnView<short> svIdx2;
    ColumnView<short> genJetIdx;
    ColumnView<float> deepFlavB;
    ColumnView<float> deepFlavCvB;
    ColumnView<float> deepFlavCvL;
    ColumnView<float> deepFlavQG;
    ColumnView<float> pnetB;
    ColumnView<float> pnetCvB;
    ColumnView<float> pnetCvL;
    ColumnView<float> pnetCvNotB;
    ColumnView<float> pnetQvG;
    ColumnView<float> pnetTauVJet;
    ColumnView<float> uparTAK4B;
    ColumnView<float> uparTAK4CvB;
    ColumnView<float> uparTAK4CvL;
    ColumnView<float> uparTAK4CvNotB;
    ColumnView<float> uparTAK4Ele;
    ColumnView<float> uparTAK4Mu;
    ColumnView<float> uparTAK4QvG;
    ColumnView<float> uparTAK4SvCB;
    ColumnView<float> uparTAK4SvUDG;
    ColumnView<float> uparTAK4TauVJet;
    ColumnView<float> uparTAK4UDG;
    ColumnView<float> uparTAK4ProbB;
    ColumnView<float> uparTAK4ProbBB;
    ColumnView<float> pnetRegPtRawCorr;
    ColumnView<float> pnetRegPtRawCorrNeutrino;
    ColumnView<float> pnetRegPtRawRes;
    ColumnView<float> uparTAK4RegPtRawCorr;
    ColumnView<float> uparTAK4RegPtRawCorrNeutrino;
    ColumnView<float> uparTAK4RegPtRawRes;
    ColumnView<float> uparTAK4V1RegPtRawCorr;
    ColumnView<float> uparTAK4V1RegPtRawCorrNeutrino;
    ColumnView<float> uparTAK4V1RegPtRawRes;
    ColumnView<float> puIdDisc;

    std::vector<float> jecFactor;
    std::vector<float> correctedPt;
    std::vector<float> correctedMass;
    std::vector<float> jerResolution;
    std::vector<float> jerScaleFactorNominal;
    std::vector<float> jerGaussianSample;
    std::vector<float> jerMatchedGenPt;
    std::vector<float> jerMinCorrection;
    std::vector<float> smearedPtNominal;
    std::vector<float> smearedPtUp;
    std::vector<float> smearedPtDown;
    std::vector<float> smearedMassNominal;
    std::vector<float> smearedMassUp;
    std::vector<float> smearedMassDown;
    std::vector<float> jesPtUp;
    std::vector<float> jesPtDown;
    std::vector<float> jesMassUp;
    std::vector<float> jesMassDown;
    std::vector<float> jesTotalUncertainty;
    std::string jesVariationSource;
    bool jesVariationValid = false;
    bool jesTotalUncertaintyValid = false;

    std::size_t size() const { return pt.size(); }

    void ensureNominal() const {
        // Validate the event epoch before invoking a provider that may read
        // several branches.  This also keeps a stale Ref from populating data
        // for the following event.
        static_cast<void>(pt.size());
        if (nominalReady)
            return;
        if (nominalComputing)
            throw std::logic_error("recursive Jet nominal materialization");
        if (!populateNominal)
            return;
        nominalComputing = true;
        try {
            populateNominal();
            nominalReady = true;
            nominalComputing = false;
        } catch (...) {
            nominalComputing = false;
            throw;
        }
    }

    // The JES lanes used to be filled only by an explicit
    // AnalyzerCore::ApplyJetScaleVariation call, which nothing in the repository
    // made, so JesPt*/JesMass* returned the zero-initialised lane. Populate them
    // on first access, exactly as the JER lanes are.
    void ensureJesVariations() const {
        ensureNominal();
        if (jesVariationsReady)
            return;
        if (jesVariationsComputing)
            throw std::logic_error("recursive Jet JES variation materialization");
        if (!populateJesVariations)
            return;
        jesVariationsComputing = true;
        try {
            populateJesVariations();
            jesVariationsReady = true;
            jesVariationsComputing = false;
        } catch (...) {
            jesVariationsComputing = false;
            throw;
        }
    }

    void ensureJerVariations() const {
        ensureNominal();
        if (jerVariationsReady)
            return;
        if (jerVariationsComputing)
            throw std::logic_error("recursive Jet JER variation materialization");
        if (!populateJerVariations)
            return;
        jerVariationsComputing = true;
        try {
            populateJerVariations();
            jerVariationsReady = true;
            jerVariationsComputing = false;
        } catch (...) {
            jerVariationsComputing = false;
            throw;
        }
    }
};

class JetView {
public:
    using JetID = JetSelection::JetID;

    JetView() = default;
    JetView(const JetSoA *storage, std::size_t index)
        : store(storage), idx(index) {}

    bool valid() const { return static_cast<bool>(store) && idx < store->size(); }

    float Pt() const { return store->pt[idx]; }
    float Eta() const { return store->eta[idx]; }
    float Phi() const { return store->phi[idx]; }
    float Mass() const { return store->mass[idx]; }
    float M() const { return Mass(); }
    float RawFactor() const { return store->rawFactor[idx]; }
    float Area() const { return store->area[idx]; }
    float ChHEF() const { return store->chHEF[idx]; }
    float NeHEF() const { return store->neHEF[idx]; }
    float NeEmEF() const { return store->neEmEF[idx]; }
    float ChEmEF() const { return store->chEmEF[idx]; }
    float MuEF() const { return store->muEF[idx]; }
    float muEF() const { return MuEF(); }
    float neEmEF() const { return NeEmEF(); }
    short PartonFlavour() const {
        return store->partonFlavour.available() ? store->partonFlavour[idx] : -999;
    }
    unsigned char HadronFlavour() const {
        return store->hadronFlavour.available() ? store->hadronFlavour[idx] : 0;
    }
    short partonFlavour() const { return PartonFlavour(); }
    unsigned char hadronFlavour() const { return HadronFlavour(); }
    unsigned char ChMultiplicity() const { return store->chMultiplicity[idx]; }
    unsigned char NeMultiplicity() const { return store->neMultiplicity[idx]; }
    unsigned char NConstituents() const { return store->nConstituents[idx]; }
    unsigned char NElectrons() const { return store->nElectrons[idx]; }
    unsigned char NMuons() const { return store->nMuons[idx]; }
    unsigned char NSVs() const { return store->nSVs[idx]; }
    short ElectronIdx1() const { return store->electronIdx1[idx]; }
    short ElectronIdx2() const { return store->electronIdx2[idx]; }
    short MuonIdx1() const { return store->muonIdx1[idx]; }
    short MuonIdx2() const { return store->muonIdx2[idx]; }
    short SvIdx1() const { return store->svIdx1[idx]; }
    short SvIdx2() const { return store->svIdx2[idx]; }
    short GenJetIdx() const {
        return store->genJetIdx.available() ? store->genJetIdx[idx] : -999;
    }
    float PNetRegPtRawCorr() const { return store->pnetRegPtRawCorr[idx]; }
    float PNetRegPtRawCorrNeutrino() const { return store->pnetRegPtRawCorrNeutrino[idx]; }
    float PNetRegPtRawRes() const { return store->pnetRegPtRawRes[idx]; }
    float UParTAK4RegPtRawCorr() const { return store->uparTAK4RegPtRawCorr[idx]; }
    float UParTAK4RegPtRawCorrNeutrino() const { return store->uparTAK4RegPtRawCorrNeutrino[idx]; }
    float UParTAK4RegPtRawRes() const { return store->uparTAK4RegPtRawRes[idx]; }
    float UParTAK4V1RegPtRawCorr() const { return store->uparTAK4V1RegPtRawCorr[idx]; }
    float UParTAK4V1RegPtRawCorrNeutrino() const { return store->uparTAK4V1RegPtRawCorrNeutrino[idx]; }
    float UParTAK4V1RegPtRawRes() const { return store->uparTAK4V1RegPtRawRes[idx]; }
    float PuIdDisc() const { return store->puIdDisc[idx]; }

    float GetTaggerResult(
        JetTagging::JetFlavTagger tagger,
        JetTagging::JetFlavTaggerScoreType score) const {
        using Tagger = JetTagging::JetFlavTagger;
        using Score = JetTagging::JetFlavTaggerScoreType;
        switch (tagger) {
        case Tagger::DeepJet:
            switch (score) {
            case Score::B: return store->deepFlavB[idx];
            case Score::CvB: return store->deepFlavCvB[idx];
            case Score::CvL: return store->deepFlavCvL[idx];
            case Score::QvG: return store->deepFlavQG[idx];
            default: break;
            }
            break;
        case Tagger::ParticleNet:
            switch (score) {
            case Score::B: return store->pnetB[idx];
            case Score::CvB: return store->pnetCvB[idx];
            case Score::CvL: return store->pnetCvL[idx];
            case Score::CvNotB: return store->pnetCvNotB[idx];
            case Score::QvG: return store->pnetQvG[idx];
            case Score::TauVJet: return store->pnetTauVJet[idx];
            default: break;
            }
            break;
        case Tagger::ParT:
            switch (score) {
            case Score::B: return store->uparTAK4B[idx];
            case Score::CvB: return store->uparTAK4CvB[idx];
            case Score::CvL: return store->uparTAK4CvL[idx];
            case Score::CvNotB: return store->uparTAK4CvNotB[idx];
            case Score::Ele: return store->uparTAK4Ele[idx];
            case Score::Mu: return store->uparTAK4Mu[idx];
            case Score::QvG: return store->uparTAK4QvG[idx];
            case Score::SvCB: return store->uparTAK4SvCB[idx];
            case Score::SvUDG: return store->uparTAK4SvUDG[idx];
            case Score::TauVJet: return store->uparTAK4TauVJet[idx];
            case Score::probUDG: return store->uparTAK4UDG[idx];
            case Score::probB: return store->uparTAK4ProbB[idx];
            case Score::probBB: return store->uparTAK4ProbBB[idx];
            default: break;
            }
            break;
        default:
            break;
        }
        throw SKNano::LogicError(
            "[JetView::GetTaggerResult] unsupported tagger/score");
    }

    float JECFactor() const { ensureNominal(); return idx < store->jecFactor.size() ? store->jecFactor[idx] : 1.f; }
    float CorrectedPt() const { ensureNominal(); return idx < store->correctedPt.size() ? store->correctedPt[idx] : Pt(); }
    float CorrectedMass() const { ensureNominal(); return idx < store->correctedMass.size() ? store->correctedMass[idx] : Mass(); }
    float SmearedPtNominal() const { ensureNominal(); return idx < store->smearedPtNominal.size() ? store->smearedPtNominal[idx] : Pt(); }
    float SmearedPtUp() const { ensureJerVariations(); return idx < store->smearedPtUp.size() ? store->smearedPtUp[idx] : Pt(); }
    float SmearedPtDown() const { ensureJerVariations(); return idx < store->smearedPtDown.size() ? store->smearedPtDown[idx] : Pt(); }
    float SmearedMassNominal() const { ensureNominal(); return idx < store->smearedMassNominal.size() ? store->smearedMassNominal[idx] : Mass(); }
    float SmearedMassUp() const { ensureJerVariations(); return idx < store->smearedMassUp.size() ? store->smearedMassUp[idx] : Mass(); }
    float SmearedMassDown() const { ensureJerVariations(); return idx < store->smearedMassDown.size() ? store->smearedMassDown[idx] : Mass(); }
    float JesPtUp() const { ensureJesVariations(); return idx < store->jesPtUp.size() ? store->jesPtUp[idx] : SmearedPtNominal(); }
    float JesPtDown() const { ensureJesVariations(); return idx < store->jesPtDown.size() ? store->jesPtDown[idx] : SmearedPtNominal(); }
    float JesMassUp() const { ensureJesVariations(); return idx < store->jesMassUp.size() ? store->jesMassUp[idx] : SmearedMassNominal(); }
    float JesMassDown() const { ensureJesVariations(); return idx < store->jesMassDown.size() ? store->jesMassDown[idx] : SmearedMassNominal(); }

    TLorentzVector P4() const {
        TLorentzVector v;
        v.SetPtEtaPhiM(Pt(), Eta(), Phi(), Mass());
        return v;
    }

    template <typename Other>
    float DeltaPhi(const Other &other) const {
        if constexpr (requires { other.P4(); })
            return P4().DeltaPhi(other.P4());
        else
            return P4().DeltaPhi(TLorentzVector(other));
    }

private:
    void assertCurrentEvent() const { static_cast<void>(store->size()); }
    void ensureNominal() const {
        assertCurrentEvent();
        store->ensureNominal();
    }
    void ensureJerVariations() const {
        assertCurrentEvent();
        store->ensureJerVariations();
    }
    void ensureJesVariations() const {
        assertCurrentEvent();
        store->ensureJesVariations();
    }
    const JetSoA *store = nullptr;
    std::size_t idx = 0;
};

using JetViewCollection = EventRange<JetSoA, JetView>;

#endif // JETVIEW_H
