#include <gtest/gtest.h>

#include <cmath>
#include <cstdlib>
#include <exception>
#include <memory>
#include <string>
#include <vector>

#include <AnalyzerFramework/AnalyzerCore.h>

#include "TauTestSupport.h"

namespace {

using sknano_test::MakeTauCollection;
using sknano_test::MakeTaus;
using sknano_test::TauFixture;
using sknano_test::VectorColumn;

TauView::TauID CalibratedID(const LeptonID::TauWorkingPoint vsJet) {
    TauView::TauID id{vsJet, LeptonID::TauWorkingPoint::VVLOOSE,
                      LeptonID::TauWorkingPointVsMu::LOOSE, true, 0.2f};
    id.requireCalibratedDecayMode = true;
    return id;
}

const TauView::TauID kMediumTau = CalibratedID(LeptonID::TauWorkingPoint::MEDIUM);
const TauView::TauID kLooseTau = CalibratedID(LeptonID::TauWorkingPoint::LOOSE);
const TauView::TauID kTightTau = CalibratedID(LeptonID::TauWorkingPoint::TIGHT);

void MarkScaledWith(TauFixture &f, const TauView::TauID &id) {
    f.soa.hasEnergyScaleID = true;
    f.soa.energyScaleVsJet = id.vsJet;
    f.soa.energyScaleVsE = id.vsE;
}

void UseCorePopulation(AnalyzerCore &core, TauFixture &f) {
    f.soa.populateMomentum = [&core, target = &f.soa] {
        core.PopulateTauMomentum(*target);
    };
}

void UseLanes(TauFixture &f, std::vector<float> corrected,
              std::vector<float> up, std::vector<float> down,
              std::vector<TauESSource> sources) {
    f.soa.populateMomentum = [target = &f.soa, corrected, up, down, sources] {
        const std::size_t n = target->pt.size();
        target->correctedPt = corrected;
        target->tesPtUp = up;
        target->tesPtDown = down;
        target->correctedMass.assign(n, 1.777f);
        target->tesMassUp.assign(n, 1.777f);
        target->tesMassDown.assign(n, 1.777f);
        target->esSource = sources;
        target->momentumReady = true;
    };
}

bool CorrectionEnvironmentSet() {
    return std::getenv("JSONPOG_REPO_PATH") && std::getenv("SKNANO_DATA") &&
           std::getenv("ROCCOR_PATH");
}

TEST(TauEnergyScale, EnableRejectsAnIDKeepingUncalibratedDecayModes) {
    AnalyzerCore core;

    EXPECT_THROW(core.EnableTauEnergyScale(TauView::TauID{}),
                 SKNano::ConfigError);
    EXPECT_FALSE(core.TauEnergyScaleEnabled());
}

TEST(TauEnergyScale, EnableRejectsTheListIfAnyIDKeepsUncalibratedDecayModes) {
    AnalyzerCore core;
    TauView::TauID relaxed = kLooseTau;
    relaxed.requireCalibratedDecayMode = false;

    EXPECT_THROW(core.EnableTauEnergyScale({kMediumTau, relaxed}),
                 SKNano::ConfigError);
    EXPECT_FALSE(core.TauEnergyScaleEnabled());
}

TEST(TauEnergyScale, EnableRejectsAnEmptyIDList) {
    AnalyzerCore core;

    EXPECT_THROW(core.EnableTauEnergyScale(std::vector<TauView::TauID>{}),
                 SKNano::ConfigError);
    EXPECT_FALSE(core.TauEnergyScaleEnabled());
}

TEST(TauViews, CarryTheIDTheyWereScaledWith) {
    AnalyzerCore core;
    core.EnableTauEnergyScale({kMediumTau, kLooseTau});

    const auto loose = core.GetAllTauViews(kLooseTau);

    ASSERT_TRUE(loose.storage());
    EXPECT_TRUE(loose.storage()->hasEnergyScaleID);
    EXPECT_EQ(loose.storage()->energyScaleVsJet, LeptonID::TauWorkingPoint::LOOSE);
    EXPECT_EQ(loose.storage()->energyScaleVsE, LeptonID::TauWorkingPoint::VVLOOSE);
}

TEST(TauViews, RejectAnUnregisteredID) {
    AnalyzerCore core;
    core.EnableTauEnergyScale({kMediumTau, kLooseTau});

    EXPECT_THROW(core.GetAllTauViews(kTightTau), SKNano::ConfigError);
}

TEST(TauViews, NeedAnIDWhenSeveralAreRegistered) {
    AnalyzerCore core;
    core.EnableTauEnergyScale({kMediumTau, kLooseTau});

    EXPECT_THROW(core.GetAllTauViews(), SKNano::ConfigError);
}

TEST(TauViews, UseTheOnlyRegisteredIDWhenNoneIsGiven) {
    AnalyzerCore core;
    core.EnableTauEnergyScale(kMediumTau);

    EXPECT_EQ(core.GetAllTauViews().storage().get(),
              core.GetAllTauViews(kMediumTau).storage().get());
}

TEST(TauViews, CarryNoIDWithoutTheScale) {
    AnalyzerCore core;

    EXPECT_FALSE(core.GetAllTauViews(kMediumTau).storage()->hasEnergyScaleID);
    EXPECT_FALSE(core.GetAllTauViews().storage()->hasEnergyScaleID);
}

TEST(TauViews, AreCachedPerIDAndEntry) {
    AnalyzerCore core;
    core.EnableTauEnergyScale({kMediumTau, kLooseTau});

    const auto medium = core.GetAllTauViews(kMediumTau);
    const auto loose = core.GetAllTauViews(kLooseTau);

    EXPECT_EQ(medium.storage().get(),
              core.GetAllTauViews(kMediumTau).storage().get());
    EXPECT_NE(medium.storage().get(), loose.storage().get());

    core.currentEntry += 1;

    EXPECT_NE(medium.storage().get(),
              core.GetAllTauViews(kMediumTau).storage().get());
}

TEST(TauViews, EnableDropsViewsCachedBeforeIt) {
    AnalyzerCore core;
    const auto before = core.GetAllTauViews();

    core.EnableTauEnergyScale(kMediumTau);
    const auto after = core.GetAllTauViews();

    EXPECT_NE(before.storage().get(), after.storage().get());
    EXPECT_TRUE(after.storage()->hasEnergyScaleID);
}

TEST(SelectTauIndicesWithScale, RejectsAMismatchedID) {
    AnalyzerCore core;
    core.EnableTauEnergyScale({kMediumTau, kLooseTau});
    auto f = MakeTaus({100.f}, {0.5f}, {6}, {6}, {4});
    MarkScaledWith(*f, kMediumTau);
    const auto taus = MakeTauCollection(f);

    EXPECT_THROW(core.SelectTauIndices(taus, kLooseTau, 50.f, 2.3f),
                 SKNano::ConfigError);
    EXPECT_EQ(core.SelectTauIndices(taus, kMediumTau, 50.f, 2.3f).size(), 1u);
}

TEST(SelectTauIndicesWithScale, RejectsAMismatchedIDOnData) {
    AnalyzerCore core;
    core.IsDATA = true;
    core.EnableTauEnergyScale({kMediumTau, kLooseTau});
    auto f = MakeTaus({100.f}, {0.5f}, {6}, {6}, {4});
    MarkScaledWith(*f, kMediumTau);

    EXPECT_THROW(core.SelectTauIndices(MakeTauCollection(f), kLooseTau, 50.f, 2.3f),
                 SKNano::ConfigError);
}

TEST(SelectTauIndicesWithScale, RejectsTausThatCarryNoID) {
    AnalyzerCore core;
    core.EnableTauEnergyScale(kMediumTau);
    auto f = MakeTaus({100.f}, {0.5f}, {6}, {6}, {4});

    EXPECT_THROW(core.SelectTauIndices(MakeTauCollection(f), kMediumTau, 50.f, 2.3f),
                 SKNano::ConfigError);
}

TEST(SelectTauIndicesWithScale, CutsOnTheRequestedVariation) {
    AnalyzerCore core;
    core.EnableTauEnergyScale(kMediumTau);
    auto f = MakeTaus({50.f, 50.f}, {0.5f, 0.5f}, {6, 6}, {6, 6}, {4, 4});
    MarkScaledWith(*f, kMediumTau);
    UseLanes(*f, {49.f, 52.f}, {51.f, 54.f}, {47.f, 50.f},
             {TauESSource::GenuineDM0, TauESSource::GenuineDM1});
    const auto taus = MakeTauCollection(f);
    const auto up = MyCorrection::variation::up;
    const auto down = MyCorrection::variation::down;

    EXPECT_EQ(core.SelectTauIndices(taus, kMediumTau, 50.f, 2.3f),
              (std::vector<std::size_t>{1}));
    EXPECT_EQ(core.SelectTauIndices(taus, kMediumTau, 50.f, 2.3f,
                                    TauESSource::GenuineDM0, up),
              (std::vector<std::size_t>{0, 1}));
    EXPECT_EQ(core.SelectTauIndices(taus, kMediumTau, 50.f, 2.3f,
                                    TauESSource::GenuineDM1, down),
              (std::vector<std::size_t>{}));
    EXPECT_EQ(core.SelectTauIndices(taus, kMediumTau, 50.f, 2.3f,
                                    TauESSource::Total, up),
              (std::vector<std::size_t>{0, 1}));
}

TEST(SelectTauIndicesWithScale, NominalPtIsUnchangedByAVariedSelection) {
    AnalyzerCore core;
    core.EnableTauEnergyScale(kMediumTau);
    auto f = MakeTaus({50.f}, {0.5f}, {6}, {6}, {4});
    MarkScaledWith(*f, kMediumTau);
    UseLanes(*f, {49.f}, {51.f}, {47.f}, {TauESSource::GenuineDM0});
    const auto taus = MakeTauCollection(f);

    core.SelectTauIndices(taus, kMediumTau, 50.f, 2.3f, TauESSource::Total,
                          MyCorrection::variation::up);

    EXPECT_FLOAT_EQ(taus[0].Pt(), 49.f);
}

TEST(SelectTauViewsWithScale, SortsByTheRequestedVariation) {
    AnalyzerCore core;
    core.EnableTauEnergyScale(kMediumTau);
    auto f = MakeTaus({60.f, 65.f}, {0.5f, 0.5f}, {6, 6}, {6, 6}, {4, 4});
    MarkScaledWith(*f, kMediumTau);
    UseLanes(*f, {60.f, 65.f}, {70.f, 66.f}, {55.f, 64.f},
             {TauESSource::GenuineDM0, TauESSource::GenuineDM1});
    const auto taus = MakeTauCollection(f);

    const auto nominal = core.SelectTauViews(taus, {0, 1});
    const auto shifted = core.SelectTauViews(taus, {0, 1}, true,
                                             TauESSource::GenuineDM0,
                                             MyCorrection::variation::up);

    EXPECT_FLOAT_EQ(nominal[0].MiniAODPt(), 65.f);
    EXPECT_FLOAT_EQ(shifted[0].MiniAODPt(), 60.f);
}

TEST(TauEnergyScaleVariation, ThrowsWhenTheScaleWasNeverEnabled) {
    AnalyzerCore core;
    auto f = MakeTaus({100.f}, {0.5f}, {6}, {6}, {4});
    const auto taus = MakeTauCollection(f);
    const auto up = MyCorrection::variation::up;
    Particle met;

    EXPECT_THROW(core.SelectTauIndices(taus, kMediumTau, 50.f, 2.3f,
                                       TauESSource::Total, up),
                 SKNano::ConfigError);
    EXPECT_THROW(core.SelectTauViews(taus, {0}, true, TauESSource::Total, up),
                 SKNano::ConfigError);
    EXPECT_THROW(core.PropagateTauSystToMET(taus, met, TauESSource::Total, up),
                 SKNano::ConfigError);
}

TEST(TauEnergyScale, PopulationWithoutCorrectionsKeepsRawMomentum) {
    AnalyzerCore core;
    core.EnableTauEnergyScale(kMediumTau);
    auto f = MakeTaus({100.f, 40.f}, {0.5f, -1.2f}, {6, 6}, {6, 6}, {4, 4});
    MarkScaledWith(*f, kMediumTau);
    UseCorePopulation(core, *f);

    for (std::size_t i = 0; i < 2; ++i) {
        const TauView tau = f->at(i);
        EXPECT_FLOAT_EQ(tau.Pt(), tau.MiniAODPt()) << "tau " << i;
        EXPECT_FLOAT_EQ(tau.M(), tau.MiniAODM()) << "tau " << i;
        EXPECT_FLOAT_EQ(tau.TesPtUp(), tau.MiniAODPt()) << "tau " << i;
        EXPECT_FLOAT_EQ(tau.TesPtDown(), tau.MiniAODPt()) << "tau " << i;
        EXPECT_EQ(tau.ESSource(), TauESSource::None) << "tau " << i;
    }
}

TEST(TauEnergyScale, PopulationLeavesDataUncorrected) {
    AnalyzerCore core;
    core.IsDATA = true;
    core.EnableTauEnergyScale(kMediumTau);
    auto f = MakeTaus({100.f}, {0.5f}, {6}, {6}, {4});
    MarkScaledWith(*f, kMediumTau);
    UseCorePopulation(core, *f);

    EXPECT_FLOAT_EQ(f->at(0).Pt(), 100.f);
    EXPECT_FLOAT_EQ(f->at(0).TesPtUp(), 100.f);
    EXPECT_EQ(f->at(0).ESSource(), TauESSource::None);
}

class TauEnergyScaleOnMC : public ::testing::Test {
protected:
    void SetUp() override {
        if (!CorrectionEnvironmentSet())
            GTEST_SKIP() << "correction environment not set; source setup.sh first";
        try {
            core_.myCorr = new MyCorrection("2024", "CDE", "TTLJ_powheg", false);
        } catch (const std::exception &e) {
            GTEST_SKIP() << "tau corrections unavailable: " << e.what();
        }
        core_.EnableTauEnergyScale({kMediumTau, kLooseTau});
    }

    AnalyzerCore core_;
};

TEST_F(TauEnergyScaleOnMC, AppliesTheScaleOfEachID) {
    const std::vector<unsigned char> dms{0, 1, 10, 11};
    const std::size_t n = dms.size();
    const auto make = [&](const TauView::TauID &id) {
        auto f = MakeTaus(std::vector<float>(n, 100.f), std::vector<float>(n, 0.5f),
                          std::vector<unsigned char>(n, 6),
                          std::vector<unsigned char>(n, 6),
                          std::vector<unsigned char>(n, 4),
                          std::vector<float>(n, 0.f), dms);
        MarkScaledWith(*f, id);
        UseCorePopulation(core_, *f);
        return f;
    };
    auto medium = make(kMediumTau);
    auto loose = make(kLooseTau);
    const std::vector<TauESSource> sources{
        TauESSource::GenuineDM0, TauESSource::GenuineDM1,
        TauESSource::GenuineDM10, TauESSource::GenuineDM11};

    for (const auto &[f, id] : {std::pair{medium, kMediumTau}, std::pair{loose, kLooseTau}}) {
        for (std::size_t i = 0; i < n; ++i) {
            const TauView tau = f->at(i);
            const int dm = dms[i];
            const auto up = MyCorrection::variation::up;
            const auto down = MyCorrection::variation::down;
            EXPECT_FLOAT_EQ(tau.Pt(), tau.MiniAODPt() * core_.myCorr->GetTauES(id, tau)) << "dm " << dm;
            EXPECT_FLOAT_EQ(tau.M(), tau.MiniAODM() * core_.myCorr->GetTauES(id, tau)) << "dm " << dm;
            EXPECT_FLOAT_EQ(tau.TesPtUp(), tau.MiniAODPt() * core_.myCorr->GetTauES(id, tau, up)) << "dm " << dm;
            EXPECT_FLOAT_EQ(tau.TesPtDown(), tau.MiniAODPt() * core_.myCorr->GetTauES(id, tau, down)) << "dm " << dm;
            EXPECT_EQ(tau.ESSource(), sources[i]) << "dm " << dm;
        }
    }
    EXPECT_NE(medium->at(3).Pt(), loose->at(3).Pt());
}

TEST_F(TauEnergyScaleOnMC, TagsElectronFakesByRegion) {
    auto f = MakeTaus({100.f, 100.f}, {0.5f, 2.0f}, {6, 6}, {6, 6}, {4, 4},
                      {0.f, 0.f}, {0, 0});
    f->genPartFlav = VectorColumn<unsigned char>({1, 1});
    f->soa.genPartFlav.bind(&f->genPartFlav);
    MarkScaledWith(*f, kMediumTau);
    UseCorePopulation(core_, *f);

    EXPECT_EQ(f->at(0).ESSource(), TauESSource::EFakeDM0Barrel);
    EXPECT_EQ(f->at(1).ESSource(), TauESSource::EFakeDM0Endcap);
    EXPECT_FLOAT_EQ(f->at(1).ShiftedPt(TauESSource::EFakeDM0Barrel, true),
                    f->at(1).Pt());
    EXPECT_NE(f->at(1).ShiftedPt(TauESSource::EFakeDM0Endcap, true),
              f->at(1).Pt());
}

TEST_F(TauEnergyScaleOnMC, EachTauMovesOnlyUnderItsOwnSourceByTheTotalShift) {
    const std::vector<unsigned char> dms{0, 1, 10, 11, 1, 1, 10};
    const std::vector<unsigned char> genmatch{5, 5, 5, 5, 1, 3, 2};
    const std::vector<float> etas{0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 2.0f, -1.f};
    const std::size_t n = dms.size();
    auto f = MakeTaus(std::vector<float>(n, 100.f), etas,
                      std::vector<unsigned char>(n, 6),
                      std::vector<unsigned char>(n, 6),
                      std::vector<unsigned char>(n, 4),
                      std::vector<float>(n, 0.f), dms);
    f->genPartFlav = VectorColumn<unsigned char>(genmatch);
    f->soa.genPartFlav.bind(&f->genPartFlav);
    MarkScaledWith(*f, kMediumTau);
    UseCorePopulation(core_, *f);

    for (std::size_t i = 0; i < n; ++i) {
        const TauView tau = f->at(i);
        ASSERT_NE(tau.ESSource(), TauESSource::None) << "tau " << i;
        EXPECT_FLOAT_EQ(tau.ShiftedPt(TauESSource::Total, true), tau.TesPtUp()) << "tau " << i;
        EXPECT_FLOAT_EQ(tau.ShiftedPt(TauESSource::Total, false), tau.TesPtDown()) << "tau " << i;
        EXPECT_NE(tau.TesPtUp(), tau.Pt()) << "tau " << i;
        for (const TauESSource source : kTauESDecorrelatedSources) {
            const bool own = source == tau.ESSource();
            EXPECT_FLOAT_EQ(tau.ShiftedPt(source, true), own ? tau.TesPtUp() : tau.Pt())
                << "tau " << i << " source " << TauESSourceName(source);
            EXPECT_FLOAT_EQ(tau.ShiftedPt(source, false), own ? tau.TesPtDown() : tau.Pt())
                << "tau " << i << " source " << TauESSourceName(source);
        }
    }
}

TEST(PropagateTauSystToMET, SubtractsTheTauMomentumShift) {
    AnalyzerCore core;
    core.EnableTauEnergyScale(kMediumTau);
    auto f = MakeTaus({100.f, 50.f}, {0.5f, 0.5f}, {6, 6}, {6, 6}, {4, 4});
    f->phi = VectorColumn<float>({0.f, static_cast<float>(M_PI / 2.)});
    f->soa.phi.bind(&f->phi);
    UseLanes(*f, {110.f, 45.f}, {112.f, 46.f}, {108.f, 44.f},
             {TauESSource::GenuineDM0, TauESSource::GenuineDM1});
    Particle met;
    met.SetPxPyPzE(30., 20., 0., 0.);

    core.PropagateTauSystToMET(MakeTauCollection(f), met);

    EXPECT_NEAR(met.Px(), 20., 1e-4);
    EXPECT_NEAR(met.Py(), 25., 1e-4);
}

TEST(PropagateTauSystToMET, UsesTheRequestedVariation) {
    AnalyzerCore core;
    core.EnableTauEnergyScale(kMediumTau);
    auto f = MakeTaus({100.f, 50.f}, {0.5f, 0.5f}, {6, 6}, {6, 6}, {4, 4});
    f->phi = VectorColumn<float>({0.f, static_cast<float>(M_PI / 2.)});
    f->soa.phi.bind(&f->phi);
    UseLanes(*f, {110.f, 45.f}, {112.f, 46.f}, {108.f, 44.f},
             {TauESSource::GenuineDM0, TauESSource::GenuineDM1});
    Particle met;
    met.SetPxPyPzE(30., 20., 0., 0.);

    core.PropagateTauSystToMET(MakeTauCollection(f), met,
                               TauESSource::GenuineDM0,
                               MyCorrection::variation::up);

    EXPECT_NEAR(met.Px(), 18., 1e-4);
    EXPECT_NEAR(met.Py(), 25., 1e-4);
}

TEST(PropagateTauSystToMET, LeavesMETAloneWhenTheScaleIsDisabled) {
    AnalyzerCore core;
    auto f = MakeTaus({100.f}, {0.5f}, {6}, {6}, {4});
    UseLanes(*f, {110.f}, {112.f}, {108.f}, {TauESSource::GenuineDM0});
    Particle met;
    met.SetPxPyPzE(30., 20., 0., 0.);

    core.PropagateTauSystToMET(MakeTauCollection(f), met);

    EXPECT_DOUBLE_EQ(met.Px(), 30.);
    EXPECT_DOUBLE_EQ(met.Py(), 20.);
}

TEST(PropagateTauSystToMET, LeavesMETAloneOnData) {
    AnalyzerCore core;
    core.IsDATA = true;
    core.EnableTauEnergyScale(kMediumTau);
    auto f = MakeTaus({100.f}, {0.5f}, {6}, {6}, {4});
    UseLanes(*f, {110.f}, {112.f}, {108.f}, {TauESSource::GenuineDM0});
    Particle met;
    met.SetPxPyPzE(30., 20., 0., 0.);

    core.PropagateTauSystToMET(MakeTauCollection(f), met);

    EXPECT_DOUBLE_EQ(met.Px(), 30.);
    EXPECT_DOUBLE_EQ(met.Py(), 20.);
}

}
