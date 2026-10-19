#include <gtest/gtest.h>

#include <AnalyzerFramework/AnalyzerCore.h>

// RNTuples are always written under their flattened top-level name
// ("dir/name" -> "dir_name"), so two Book() calls whose paths look unrelated
// before flattening can still collide on the same output key -- and so can
// an RNTuple and a top-level histogram. ValidateTreePath/ValidateHistogramPath
// must catch that at Book time; see AnalyzerCoreOutput.cc and
// /tmp/pr_review/rnt/dup.C for the ROOT-level reproduction (no exception,
// just stacked key cycles).

TEST(OutputPathCollision, FlattenedRNTuplePathsCollide) {
    AnalyzerCore core;
    core.Output().Book("A/B_C");
    EXPECT_THROW(core.Output().Book("A_B/C"), SKNano::ConfigError);
}

TEST(OutputPathCollision, RNTupleCollidesWithExistingTopLevelHistogram) {
    AnalyzerCore core;
    core.BookHist1D("Central_SR", 1, 0., 1.);
    EXPECT_THROW(core.Output().Book("Central/SR"), SKNano::ConfigError);
}

TEST(OutputPathCollision, HistogramCollidesWithExistingRNTuple) {
    AnalyzerCore core;
    core.Output().Book("Central/SR");
    EXPECT_THROW(core.BookHist1D("Central_SR", 1, 0., 1.), SKNano::ConfigError);
}

TEST(OutputPathCollision, UnrelatedPathsBookFine) {
    AnalyzerCore core;
    EXPECT_NO_THROW(core.Output().Book("A/B_C"));
    EXPECT_NO_THROW(core.Output().Book("A_B_D"));
    EXPECT_NO_THROW(core.BookHist1D("Central_SR", 1, 0., 1.));
}
