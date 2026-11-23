#include <gtest/gtest.h>

#include <vector>

#include <AnalyzerFramework/AnalyzerCore.h>

#include "TauTestSupport.h"

namespace {

using sknano_test::MakeTauCollection;
using sknano_test::MakeTaus;

// Medium vsJet, VVLoose vsE, Loose vsMu.
const TauView::TauID kMediumTau{LeptonID::TauWorkingPoint::MEDIUM, LeptonID::TauWorkingPoint::VVLOOSE, LeptonID::TauWorkingPointVsMu::LOOSE,
                             true, 0.2f};

TEST(SelectTauIndices, KeepsOnlyTausPassingEveryCut) {
    // 0: passes.  1: fails pT.  2: fails eta.  3: fails the vsJet axis.
    auto f = MakeTaus({100.f, 20.f, 100.f, 100.f},
                      {0.5f, 0.5f, 2.9f, 0.5f},
                      {6, 6, 6, 3},
                      {6, 6, 6, 6},
                      {4, 4, 4, 4});
    const auto taus = MakeTauCollection(f);
    AnalyzerCore core;

    const auto selected = core.SelectTauIndices(taus, kMediumTau, 50.f, 2.3f);

    ASSERT_EQ(selected.size(), 1u);
    EXPECT_EQ(selected[0], 0u);
}

TEST(SelectTauIndices, PreservesInputOrderAndDoesNotSort) {
    auto f = MakeTaus({60.f, 200.f, 90.f}, {0.5f, 0.5f, 0.5f}, {6, 6, 6},
                      {6, 6, 6}, {4, 4, 4});
    const auto taus = MakeTauCollection(f);
    AnalyzerCore core;

    const auto selected = core.SelectTauIndices(taus, kMediumTau, 50.f, 2.3f);

    ASSERT_EQ(selected.size(), 3u);
    EXPECT_EQ(selected[0], 0u);
    EXPECT_EQ(selected[1], 1u);
    EXPECT_EQ(selected[2], 2u);
}

TEST(SelectTauIndices, SeedOverloadOnlyConsidersSeedIndices) {
    auto f = MakeTaus({100.f, 100.f, 100.f}, {0.5f, 0.5f, 0.5f}, {6, 6, 6},
                      {6, 6, 6}, {4, 4, 4});
    const auto taus = MakeTauCollection(f);
    AnalyzerCore core;

    const std::vector<std::size_t> seed{2};
    const auto selected =
        core.SelectTauIndices(taus, seed, kMediumTau, 50.f, 2.3f);

    ASSERT_EQ(selected.size(), 1u);
    EXPECT_EQ(selected[0], 2u);
}

TEST(SelectTauIndices, BoundsAreStrict) {
    auto f = MakeTaus({50.f, 100.f}, {0.5f, 2.3f}, {6, 6}, {6, 6}, {4, 4});
    const auto taus = MakeTauCollection(f);
    AnalyzerCore core;

    const auto selected = core.SelectTauIndices(taus, kMediumTau, 50.f, 2.3f);

    EXPECT_TRUE(selected.empty());
}

TEST(SelectTauIndices, EmptyCollectionYieldsEmptyResult) {
    auto f = MakeTaus({}, {}, {}, {}, {});
    const auto taus = MakeTauCollection(f);
    AnalyzerCore core;

    EXPECT_TRUE(core.SelectTauIndices(taus, kMediumTau, 50.f, 2.3f).empty());
}

}

TEST(SelectTauIndices, RejectsDecayModesThePOGDoesNotCalibrate) {
    const std::vector<unsigned char> dms{0, 1, 2, 5, 6, 10, 11};
    const std::size_t n = dms.size();
    auto f = MakeTaus(std::vector<float>(n, 100.f), std::vector<float>(n, 0.5f),
                      std::vector<unsigned char>(n, 6),
                      std::vector<unsigned char>(n, 6),
                      std::vector<unsigned char>(n, 4),
                      std::vector<float>(n, 0.f), dms);
    const auto taus = MakeTauCollection(f);
    AnalyzerCore core;
    TauView::TauID calibrated = kMediumTau;
    calibrated.requireCalibratedDecayMode = true;

    const auto selected = core.SelectTauIndices(taus, calibrated, 50.f, 2.3f);

    ASSERT_EQ(selected.size(), 4u);
    for (const auto index : selected) {
        const unsigned char dm = dms[index];
        EXPECT_TRUE(dm == 0 || dm == 1 || dm == 10 || dm == 11)
            << "decay mode " << static_cast<int>(dm) << " should be rejected";
    }
}

TEST(SelectTauIndices, KeepsUncalibratedDecayModesWhenAsked) {
    const std::vector<unsigned char> dms{0, 5, 6};
    const std::size_t n = dms.size();
    auto f = MakeTaus(std::vector<float>(n, 100.f), std::vector<float>(n, 0.5f),
                      std::vector<unsigned char>(n, 6),
                      std::vector<unsigned char>(n, 6),
                      std::vector<unsigned char>(n, 4),
                      std::vector<float>(n, 0.f), dms);
    const auto taus = MakeTauCollection(f);
    AnalyzerCore core;

    TauView::TauID relaxed = kMediumTau;
    relaxed.requireCalibratedDecayMode = false;

    EXPECT_EQ(core.SelectTauIndices(taus, relaxed, 50.f, 2.3f).size(), 3u);
}
