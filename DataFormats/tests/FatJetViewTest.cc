#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include "SelectedFatJetView.h"
#include "TauTestSupport.h"

namespace {

using sknano_test::VectorColumn;

// Two fat jets with raw kinematics only; the providers fill every lane with a
// distinct offset so a test can tell which one an accessor read.
struct FatJetFixture {
    VectorColumn<float> pt{std::vector<float>{300.f, 200.f}};
    VectorColumn<float> eta{std::vector<float>{0.5f, -1.0f}};
    VectorColumn<float> phi{std::vector<float>{0.1f, 2.0f}};
    VectorColumn<float> mass{std::vector<float>{80.f, 40.f}};
    VectorColumn<float> softDropMass{std::vector<float>{75.f, 30.f}};
    FatJetSoA soa;
    int nominalCalls = 0;
    int jerCalls = 0;
    int jesCalls = 0;

    FatJetFixture() {
        soa.pt.bind(&pt);
        soa.eta.bind(&eta);
        soa.phi.bind(&phi);
        soa.mass.bind(&mass);
        soa.softDropMass.bind(&softDropMass);
        soa.populateNominal = [this] {
            ++nominalCalls;
            soa.correctedPt = {301.f, 201.f};
            soa.correctedMass = {81.f, 41.f};
            soa.smearedPtNominal = {302.f, 202.f};
            soa.smearedMassNominal = {82.f, 42.f};
            soa.sdMassNominal = {76.f, 31.f};
        };
        soa.populateJerVariations = [this] {
            ++jerCalls;
            soa.smearedPtUp = {303.f, 203.f};
            soa.smearedPtDown = {304.f, 204.f};
            soa.smearedMassUp = {83.f, 43.f};
            soa.smearedMassDown = {84.f, 44.f};
            soa.sdMassJerUp = {77.f, 32.f};
            soa.sdMassJerDown = {78.f, 33.f};
        };
        soa.populateJesVariations = [this] {
            ++jesCalls;
            soa.jesPtUp = {305.f, 205.f};
            soa.jesPtDown = {306.f, 206.f};
            soa.jesMassUp = {85.f, 45.f};
            soa.jesMassDown = {86.f, 46.f};
            soa.sdMassJesUp = {79.f, 34.f};
            soa.sdMassJesDown = {80.f, 35.f};
        };
    }
};

TEST(FatJetSoA, NominalLanesDoNotRunTheVariationStages) {
    FatJetFixture f;
    const FatJetView jet(&f.soa, 0);

    EXPECT_FLOAT_EQ(jet.CorrectedPt(), 301.f);
    EXPECT_FLOAT_EQ(jet.SmearedPtNominal(), 302.f);
    EXPECT_FLOAT_EQ(jet.SDMassNominal(), 76.f);
    EXPECT_EQ(f.nominalCalls, 1);
    EXPECT_EQ(f.jerCalls, 0);
    EXPECT_EQ(f.jesCalls, 0);

    EXPECT_FLOAT_EQ(jet.SmearedPtUp(), 303.f);
    EXPECT_EQ(f.jerCalls, 1);
    EXPECT_EQ(f.jesCalls, 0);

    EXPECT_FLOAT_EQ(jet.JesPtDown(), 306.f);
    EXPECT_FLOAT_EQ(jet.SDMassJesUp(), 79.f);
    EXPECT_EQ(f.nominalCalls, 1);
    EXPECT_EQ(f.jerCalls, 1);
    EXPECT_EQ(f.jesCalls, 1);
}

TEST(FatJetSoA, UnboundProvidersFallBackToRawValues) {
    FatJetFixture f;
    f.soa.populateNominal = nullptr;
    f.soa.populateJerVariations = nullptr;
    f.soa.populateJesVariations = nullptr;
    const FatJetView jet(&f.soa, 1);

    EXPECT_FLOAT_EQ(jet.CorrectedPt(), 200.f);
    EXPECT_FLOAT_EQ(jet.SmearedPtUp(), 200.f);
    EXPECT_FLOAT_EQ(jet.JesMassUp(), 40.f);
    EXPECT_FLOAT_EQ(jet.SDMassJerDown(), 30.f);
}

TEST(FatJetSoA, RecursiveStageThrows) {
    FatJetFixture f;
    f.soa.populateNominal = [&f] { f.soa.ensureNominal(); };
    const FatJetView jet(&f.soa, 0);
    EXPECT_THROW(jet.CorrectedPt(), SKNano::LogicError);
}

TEST(SelectedFatJetView, ProjectionPicksTheLane) {
    FatJetFixture f;
    struct Expected {
        JetKinematicProjection projection;
        float pt, mass, sdMass;
    };
    const Expected cases[] = {
        {JetKinematicProjection::Corrected, 301.f, 81.f, 75.f},
        {JetKinematicProjection::Nominal, 302.f, 82.f, 76.f},
        {JetKinematicProjection::JerUp, 303.f, 83.f, 77.f},
        {JetKinematicProjection::JerDown, 304.f, 84.f, 78.f},
        {JetKinematicProjection::JesUp, 305.f, 85.f, 79.f},
        {JetKinematicProjection::JesDown, 306.f, 86.f, 80.f},
    };
    for (const auto &c : cases) {
        const SelectedFatJetView jet(&f.soa, 0, c.projection);
        SCOPED_TRACE(static_cast<int>(c.projection));
        EXPECT_FLOAT_EQ(jet.Pt(), c.pt);
        EXPECT_FLOAT_EQ(jet.M(), c.mass);
        EXPECT_FLOAT_EQ(jet.SDMass(), c.sdMass);
        EXPECT_NEAR(jet.P4().Pt(), c.pt, 1e-3);
        // The unprojected accessors stay available through the base view.
        EXPECT_FLOAT_EQ(jet.FatJetView::Pt(), 300.f);
    }
}

TEST(SelectedFatJetViewCollection, KeepsIndicesAndProjection) {
    auto f = std::make_shared<FatJetFixture>();
    const SelectedFatJetViewCollection jets(
        std::shared_ptr<FatJetSoA>(f, &f->soa), {1, 0},
        JetKinematicProjection::JesUp);

    ASSERT_EQ(jets.size(), 2u);
    EXPECT_EQ(jets.rawIndex(0), 1u);
    EXPECT_FLOAT_EQ(jets[0].Pt(), 205.f);
    EXPECT_FLOAT_EQ(jets[1].SDMass(), 79.f);
    EXPECT_THROW(jets[2], std::out_of_range);
}

} // namespace
