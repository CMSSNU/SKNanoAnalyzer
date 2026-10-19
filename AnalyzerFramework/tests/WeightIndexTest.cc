#include <gtest/gtest.h>

#include <AnalyzerFramework/AnalyzerCore.h>

namespace {

using V = MyCorrection::variation;

TEST(ScaleVariationIndex, NineEntriesIsTheFullGrid) {
    // muR outer, muF inner: slot = 3 * iR + iF with down/nom/up = 0/1/2.
    EXPECT_EQ(AnalyzerCore::ScaleVariationIndex(V::down, V::down, 9), 0u);
    EXPECT_EQ(AnalyzerCore::ScaleVariationIndex(V::up, V::down, 9), 2u);
    EXPECT_EQ(AnalyzerCore::ScaleVariationIndex(V::down, V::nom, 9), 3u);
    EXPECT_EQ(AnalyzerCore::ScaleVariationIndex(V::nom, V::nom, 9), 4u);
    EXPECT_EQ(AnalyzerCore::ScaleVariationIndex(V::up, V::nom, 9), 5u);
    EXPECT_EQ(AnalyzerCore::ScaleVariationIndex(V::nom, V::up, 9), 7u);
    EXPECT_EQ(AnalyzerCore::ScaleVariationIndex(V::up, V::up, 9), 8u);
}

TEST(ScaleVariationIndex, EightEntriesSkipTheNominal) {
    EXPECT_EQ(AnalyzerCore::ScaleVariationIndex(V::down, V::nom, 8), 3u);
    EXPECT_EQ(AnalyzerCore::ScaleVariationIndex(V::up, V::nom, 8), 4u);
    EXPECT_EQ(AnalyzerCore::ScaleVariationIndex(V::nom, V::up, 8), 6u);
    EXPECT_EQ(AnalyzerCore::ScaleVariationIndex(V::up, V::up, 8), 7u);
    EXPECT_THROW(AnalyzerCore::ScaleVariationIndex(V::nom, V::nom, 8),
                 SKNano::LogicError);
}

TEST(ScaleVariationIndex, RejectsUnknownLengths) {
    for (std::size_t n : {0u, 1u, 4u, 7u, 10u, 18u, 44u})
        EXPECT_THROW(AnalyzerCore::ScaleVariationIndex(V::up, V::nom, n),
                     SKNano::LogicError)
            << n;
}

TEST(PSWeightIndex, FollowsTheNanoAODLayout) {
    EXPECT_EQ(AnalyzerCore::PSWeightIndex(V::up, V::nom, 4), 0u);
    EXPECT_EQ(AnalyzerCore::PSWeightIndex(V::nom, V::up, 4), 1u);
    EXPECT_EQ(AnalyzerCore::PSWeightIndex(V::down, V::nom, 4), 2u);
    EXPECT_EQ(AnalyzerCore::PSWeightIndex(V::nom, V::down, 4), 3u);
}

TEST(PSWeightIndex, RejectsBothVariedNominalAndUnknownLengths) {
    EXPECT_THROW(AnalyzerCore::PSWeightIndex(V::up, V::up, 4),
                 SKNano::LogicError);
    EXPECT_THROW(AnalyzerCore::PSWeightIndex(V::nom, V::nom, 4),
                 SKNano::LogicError);
    for (std::size_t n : {0u, 1u, 3u, 5u, 44u})
        EXPECT_THROW(AnalyzerCore::PSWeightIndex(V::up, V::nom, n),
                     SKNano::LogicError)
            << n;
}

} // namespace
