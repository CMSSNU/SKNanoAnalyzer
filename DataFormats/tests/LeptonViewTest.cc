#include <gtest/gtest.h>

#include <vector>

#include "AnalysisException.h"
#include "ElectronView.h"
#include "MuonView.h"
#include "TauTestSupport.h"

using sknano_test::VectorColumn;

namespace {

// Two leptons; only pt is read by the staging machinery.
struct LeptonColumns {
    VectorColumn<float> pt{std::vector<float>{30.f, 50.f}};
};

TEST(LeptonViews, ElectronWithoutProviderThrows) {
    LeptonColumns columns;
    ElectronSoA soa;
    soa.pt.bind(&columns.pt);
    const ElectronView electron(&soa, 0);
    EXPECT_THROW(electron.Pt(), SKNano::LogicError);
    EXPECT_THROW(electron.ScaleUpPt(), SKNano::LogicError);
    // The raw value needs no correction stage.
    EXPECT_FLOAT_EQ(electron.MiniAODPt(), 30.f);
}

TEST(LeptonViews, MuonWithoutProviderThrows) {
    LeptonColumns columns;
    MuonSoA soa;
    soa.pt.bind(&columns.pt);
    EXPECT_THROW(MuonView(&soa, 0).Pt(), SKNano::LogicError);

    // Nominal bound, variation stage not: the variation must not quietly
    // read as the nominal.
    soa.populateMomentum = [&soa] {
        soa.correctedPt = {31.f, 51.f};
        soa.miniAODPt = {30.f, 50.f};
    };
    EXPECT_FLOAT_EQ(MuonView(&soa, 0).Pt(), 31.f);
    EXPECT_THROW(MuonView(&soa, 0).MomentumScaleUp(), SKNano::LogicError);
}

TEST(LeptonViews, EmptyLaneThrows) {
    LeptonColumns columns;
    ElectronSoA soa;
    soa.pt.bind(&columns.pt);
    soa.populateMomentum = [&soa] { soa.correctedPt = {31.f, 51.f}; };
    soa.populateMomentumVariations = [] {}; // forgets its lanes
    const ElectronView electron(&soa, 1);
    EXPECT_FLOAT_EQ(electron.Pt(), 51.f);
    EXPECT_THROW(electron.SmearUpPt(), SKNano::LogicError);
}

TEST(LeptonViews, NominalAccessLeavesVariationsUnpopulated) {
    LeptonColumns columns;
    MuonSoA soa;
    soa.pt.bind(&columns.pt);
    int variationCalls = 0;
    soa.populateMomentum = [&soa] {
        soa.correctedPt = {31.f, 51.f};
        soa.miniAODPt = {30.f, 50.f};
    };
    soa.populateMomentumVariations = [&soa, &variationCalls] {
        ++variationCalls;
        soa.momentumScaleUp = soa.momentumScaleDown = {32.f, 52.f};
        soa.momentumResUp = soa.momentumResDown = {33.f, 53.f};
    };
    const MuonView muon(&soa, 1);
    EXPECT_FLOAT_EQ(muon.Pt(), 51.f);
    EXPECT_FLOAT_EQ(muon.MiniAODPt(), 50.f);
    EXPECT_EQ(variationCalls, 0);
    EXPECT_TRUE(soa.momentumScaleUp.empty());

    EXPECT_FLOAT_EQ(muon.MomentumScaleUp(), 52.f);
    EXPECT_FLOAT_EQ(muon.MomentumResDown(), 53.f);
    EXPECT_EQ(variationCalls, 1);
}

} // namespace
