#include <gtest/gtest.h>

#include <set>
#include <string>

#include "TauTestSupport.h"

namespace {

using sknano_test::MakeTaus;

TEST(TauViewWorkingPoints, TightTauPassesLooserJetWorkingPoints) {
    auto f = MakeTaus({100.f}, {0.5f}, {6}, {6}, {4});
    const TauView tau = f->at(0);

    EXPECT_TRUE(tau.passTIDvJet());
    EXPECT_TRUE(tau.passMIDvJet());
    EXPECT_TRUE(tau.passLIDvJet());
    EXPECT_TRUE(tau.passVLIDvJet());
    EXPECT_TRUE(tau.passVVLIDvJet());
    EXPECT_TRUE(tau.passVVVLIDvJet());
}

TEST(TauViewWorkingPoints, TightTauFailsTighterJetWorkingPoints) {
    auto f = MakeTaus({100.f}, {0.5f}, {6}, {6}, {4});
    const TauView tau = f->at(0);

    EXPECT_FALSE(tau.passVTIDvJet());
    EXPECT_FALSE(tau.passVVTIDvJet());
}

TEST(TauViewWorkingPoints, ElectronAxisIsAlsoCumulative) {
    auto f = MakeTaus({100.f}, {0.5f}, {6}, {5}, {4});
    const TauView tau = f->at(0);

    EXPECT_TRUE(tau.passMIDvEl());
    EXPECT_TRUE(tau.passLIDvEl());
    EXPECT_TRUE(tau.passVVVLIDvEl());
    EXPECT_FALSE(tau.passTIDvEl());
}

TEST(TauViewWorkingPoints, MuonAxisUsesFourPointScale) {
    auto f = MakeTaus({100.f}, {0.5f}, {6}, {6}, {3});
    const TauView tau = f->at(0);

    EXPECT_TRUE(tau.passMIDvMu());
    EXPECT_TRUE(tau.passLIDvMu());
    EXPECT_TRUE(tau.passVLIDvMu());
    EXPECT_FALSE(tau.passTIDvMu());
}

TEST(TauViewWorkingPoints, UnidentifiedTauFailsEveryWorkingPoint) {
    auto f = MakeTaus({100.f}, {0.5f}, {0}, {0}, {0});
    const TauView tau = f->at(0);

    EXPECT_FALSE(tau.passVVVLIDvJet());
    EXPECT_FALSE(tau.passVVVLIDvEl());
    EXPECT_FALSE(tau.passVLIDvMu());
}

// test raw discrimator 

TEST(TauViewRawScores, RawDiscriminatorsAreReadable) {
    auto f = MakeTaus({100.f}, {0.5f}, {6}, {6}, {4});
    f->rawVsJet = sknano_test::VectorColumn<float>({0.97f});
    f->rawVsE = sknano_test::VectorColumn<float>({0.88f});
    f->rawVsMu = sknano_test::VectorColumn<float>({0.51f});
    f->soa.rawDeepTau2018v2p5VSjet.bind(&f->rawVsJet);
    f->soa.rawDeepTau2018v2p5VSe.bind(&f->rawVsE);
    f->soa.rawDeepTau2018v2p5VSmu.bind(&f->rawVsMu);

    const TauView tau = f->at(0);
    EXPECT_FLOAT_EQ(tau.RawVsJet(), 0.97f);
    EXPECT_FLOAT_EQ(tau.RawVsE(), 0.88f);
    EXPECT_FLOAT_EQ(tau.RawVsMu(), 0.51f);
}

TEST(TauViewRawScores, UnboundRawDiscriminatorsReturnSentinel) {
    auto f = MakeTaus({100.f}, {0.5f}, {6}, {6}, {4});
    const TauView tau = f->at(0);

    EXPECT_FLOAT_EQ(tau.RawVsJet(), -1.f);
    EXPECT_FLOAT_EQ(tau.RawVsE(), -1.f);
    EXPECT_FLOAT_EQ(tau.RawVsMu(), -1.f);
}

TEST(TauViewID, AcceptsTauMeetingEveryAxis) {
    auto f = MakeTaus({100.f}, {0.5f}, {6}, {6}, {4});
    const TauView::TauID id{LeptonID::TauWorkingPoint::MEDIUM, LeptonID::TauWorkingPoint::VVLOOSE, LeptonID::TauWorkingPointVsMu::LOOSE,
                         true, 0.2f};
    EXPECT_TRUE(f->at(0).PassID(id));
}

TEST(TauViewID, RejectsTauFailingTheJetAxis) {
    auto f = MakeTaus({100.f}, {0.5f}, {4}, {6}, {4});
    const TauView::TauID id{LeptonID::TauWorkingPoint::MEDIUM, LeptonID::TauWorkingPoint::VVLOOSE, LeptonID::TauWorkingPointVsMu::LOOSE,
                         true, 0.2f};
    EXPECT_FALSE(f->at(0).PassID(id));
}

TEST(TauViewID, RejectsTauFailingTheMuonAxis) {
    auto f = MakeTaus({100.f}, {0.5f}, {6}, {6}, {1});
    const TauView::TauID id{LeptonID::TauWorkingPoint::MEDIUM, LeptonID::TauWorkingPoint::VVLOOSE, LeptonID::TauWorkingPointVsMu::TIGHT,
                         true, 0.2f};
    EXPECT_FALSE(f->at(0).PassID(id));
}

TEST(TauViewID, NoneWorkingPointDisablesThatAxis) {
    auto f = MakeTaus({100.f}, {0.5f}, {6}, {0}, {0});
    const TauView::TauID id{LeptonID::TauWorkingPoint::TIGHT, LeptonID::TauWorkingPoint::NONE, LeptonID::TauWorkingPointVsMu::NONE,
                         true, 0.2f};
    EXPECT_TRUE(f->at(0).PassID(id));
}

TEST(TauViewID, RejectsTauFailingTheImpactParameterCut) {
    auto f = MakeTaus({100.f}, {0.5f}, {6}, {6}, {4}, {0.5f});
    const TauView::TauID id{LeptonID::TauWorkingPoint::MEDIUM, LeptonID::TauWorkingPoint::VVLOOSE, LeptonID::TauWorkingPointVsMu::LOOSE,
                         true, 0.2f};
    EXPECT_FALSE(f->at(0).PassID(id));
}

// Decay modes 2, 5 and 6 pass idDecayModeNewDMs but no TAU correction covers
// them: tau_energy_scale and VSe throw and tau_trigger returns -1.0, which
// flips the sign of the event weight. docs/SYSTEMATICS.md 2.1.
TEST(TauViewID, RejectsDecayModesThePOGDoesNotCalibrate) {
    const TauView::TauID id{LeptonID::TauWorkingPoint::MEDIUM, LeptonID::TauWorkingPoint::VVLOOSE, LeptonID::TauWorkingPointVsMu::LOOSE,
                         true, 0.2f, true};
    for (const unsigned char dm : {0, 1, 10, 11}) {
        auto f = MakeTaus({100.f}, {0.5f}, {6}, {6}, {4}, {0.f}, {dm});
        EXPECT_TRUE(f->at(0).PassID(id)) << "dm " << static_cast<int>(dm);
    }
    for (const unsigned char dm : {2, 5, 6}) {
        auto f = MakeTaus({100.f}, {0.5f}, {6}, {6}, {4}, {0.f}, {dm});
        EXPECT_FALSE(f->at(0).PassID(id)) << "dm " << static_cast<int>(dm);
    }
}

TEST(TauViewID, KeepsUncalibratedDecayModesWhenTheFlagIsOff) {
    TauView::TauID id{LeptonID::TauWorkingPoint::MEDIUM, LeptonID::TauWorkingPoint::VVLOOSE, LeptonID::TauWorkingPointVsMu::LOOSE,
                      true, 0.2f, false};
    for (const unsigned char dm : {2, 5, 6}) {
        auto f = MakeTaus({100.f}, {0.5f}, {6}, {6}, {4}, {0.f}, {dm});
        EXPECT_TRUE(f->at(0).PassID(id)) << "dm " << static_cast<int>(dm);
    }
}

TEST(TauViewID, DefaultIDKeepsUncalibratedDecayModes) {
    const TauView::TauID id{};
    EXPECT_FALSE(id.requireCalibratedDecayMode);
    for (const unsigned char dm : {2, 5, 6}) {
        auto f = MakeTaus({100.f}, {0.5f}, {6}, {6}, {4}, {0.f}, {dm});
        EXPECT_TRUE(f->at(0).PassID(id)) << "dm " << static_cast<int>(dm);
    }
}

TEST(TauViewID, PositionalInitializerKeepsItsMeaning) {
    const TauView::TauID id{LeptonID::TauWorkingPoint::MEDIUM, LeptonID::TauWorkingPoint::VVLOOSE, LeptonID::TauWorkingPointVsMu::LOOSE,
                            true, 0.5f};
    EXPECT_FLOAT_EQ(id.maxDz, 0.5f);
    EXPECT_FALSE(id.requireCalibratedDecayMode);
}

TEST(TauViewMomentum, ThrowsWhenNoProviderIsBound) {
    auto f = MakeTaus({100.f}, {0.5f}, {6}, {6}, {4});
    f->soa.populateMomentum = nullptr;
    EXPECT_THROW(f->at(0).Pt(), SKNano::LogicError);
}

TEST(TauViewESSource, ClassifiesEachTauIntoItsOwnSource) {
    EXPECT_EQ(TauView::ClassifyESSource(0, 5, 0.5f), TauESSource::GenuineDM0);
    EXPECT_EQ(TauView::ClassifyESSource(11, 5, 2.0f), TauESSource::GenuineDM11);
    EXPECT_EQ(TauView::ClassifyESSource(1, 1, 0.5f), TauESSource::EFakeDM1Barrel);
    EXPECT_EQ(TauView::ClassifyESSource(1, 3, -1.6f), TauESSource::EFakeDM1Endcap);
    EXPECT_EQ(TauView::ClassifyESSource(10, 1, 1.49f), TauESSource::EFakeDM10Barrel);
    EXPECT_EQ(TauView::ClassifyESSource(10, 2, 0.5f), TauESSource::MuFakeDM10);
    EXPECT_EQ(TauView::ClassifyESSource(0, 4, 0.5f), TauESSource::MuFakeDM0);
    EXPECT_EQ(TauView::ClassifyESSource(0, 0, 0.5f), TauESSource::None);
    EXPECT_EQ(TauView::ClassifyESSource(0, 6, 0.5f), TauESSource::None);
    EXPECT_EQ(TauView::ClassifyESSource(5, 5, 0.5f), TauESSource::None);
}

TEST(TauViewESSource, ShiftedMomentumOnlyMovesForTheTausOwnSource) {
    auto f = MakeTaus({100.f}, {0.5f}, {6}, {6}, {4});
    f->soa.populateMomentum = [target = &f->soa] {
        target->correctedPt = {101.f};
        target->correctedMass = {1.8f};
        target->tesPtUp = {103.f};
        target->tesPtDown = {99.f};
        target->tesMassUp = {1.83f};
        target->tesMassDown = {1.77f};
        target->esSource = {TauESSource::GenuineDM0};
        target->momentumReady = true;
    };
    const TauView tau = f->at(0);

    EXPECT_FLOAT_EQ(tau.ShiftedPt(TauESSource::GenuineDM0, true), 103.f);
    EXPECT_FLOAT_EQ(tau.ShiftedPt(TauESSource::GenuineDM0, false), 99.f);
    EXPECT_FLOAT_EQ(tau.ShiftedM(TauESSource::GenuineDM0, true), 1.83f);
    EXPECT_FLOAT_EQ(tau.ShiftedPt(TauESSource::Total, true), 103.f);
    EXPECT_FLOAT_EQ(tau.ShiftedPt(TauESSource::GenuineDM1, true), 101.f);
    EXPECT_FLOAT_EQ(tau.ShiftedM(TauESSource::GenuineDM1, true), 1.8f);
    EXPECT_FLOAT_EQ(tau.ShiftedPt(TauESSource::None, true), 101.f);
    EXPECT_FLOAT_EQ(tau.ShiftedP4(TauESSource::GenuineDM0, true).Pt(), 103.f);
}

TEST(TauViewESSource, DecorrelatedSourcesHaveDistinctNames) {
    std::set<std::string> names;
    for (const auto source : kTauESDecorrelatedSources)
        names.insert(TauESSourceName(source));
    EXPECT_EQ(names.size(), kTauESDecorrelatedSources.size());
    EXPECT_EQ(names.count("None"), 0u);
    EXPECT_EQ(names.count("Total"), 0u);
}

TEST(TauViewID, WorkingPointEnumMapsToPogStrings) {
    EXPECT_STREQ(ToCorrectionString(LeptonID::TauWorkingPoint::MEDIUM), "Medium");
    EXPECT_STREQ(ToCorrectionString(LeptonID::TauWorkingPoint::VVLOOSE), "VVLoose");
    EXPECT_STREQ(ToCorrectionString(LeptonID::TauWorkingPoint::VVTIGHT), "VVTight");
    EXPECT_STREQ(ToCorrectionString(LeptonID::TauWorkingPointVsMu::TIGHT), "Tight");
    EXPECT_STREQ(ToCorrectionString(LeptonID::TauWorkingPointVsMu::VLOOSE), "VLoose");
}

}
