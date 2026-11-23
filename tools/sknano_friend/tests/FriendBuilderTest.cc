#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <random>
#include <string>
#include <typeindex>
#include <unistd.h>
#include <vector>

#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleWriter.hxx>
#include <ROOT/RVec.hxx>
#include <TFile.h>

#include <AnalyzerFramework/RNTupleSource.h>

#include "FriendBuilder.h"

// A central file sorted by event number, two aux files in scrambled order with
// one lumi never processed, one event filtered away and one aux-only event;
// the friend must come out aligned with the central file and readable through
// the framework's RNTupleSource.
namespace {

namespace fs = std::filesystem;
using ROOT::VecOps::RVec;

constexpr std::uint32_t kRun = 1;

struct Event {
    std::uint32_t lumi;
    std::uint64_t event;
};

float Weight(std::uint64_t event) { return event % 7 == 0 ? -1.f : 1.f; }
float LambdaPt(std::uint64_t event) { return 0.5f * static_cast<float>(event); }

std::vector<Event> CentralEvents() {
    std::vector<Event> events;
    for (std::uint64_t e = 100; e < 120; ++e)
        events.push_back({10 + static_cast<std::uint32_t>((e - 100) / 5), e});
    return events; // lumis 10..13, five events each, sorted by event
}

void WriteCentral(const std::string &path) {
    auto model = ROOT::RNTupleModel::Create();
    auto run = model->MakeField<std::uint32_t>("run");
    auto lumi = model->MakeField<std::uint32_t>("luminosityBlock");
    auto event = model->MakeField<std::uint64_t>("event");
    auto genWeight = model->MakeField<float>("genWeight");
    auto nJet = model->MakeField<std::uint32_t>("nJet");
    auto jetPt = model->MakeField<RVec<float>>("Jet_pt");
    auto nGenPart = model->MakeField<std::uint32_t>("nGenPart");
    auto genPt = model->MakeField<RVec<float>>("GenPart_pt");
    auto writer = ROOT::RNTupleWriter::Recreate(std::move(model), "Events", path);
    for (const auto &e : CentralEvents()) {
        *run = kRun;
        *lumi = e.lumi;
        *event = e.event;
        *genWeight = Weight(e.event);
        *jetPt = {1.f * e.event, 2.f * e.event};
        *nJet = 2;
        *genPt = {10.f};
        *nGenPart = 1;
        writer->Fill();
    }
}

struct AuxSpec {
    std::vector<std::uint32_t> lumis;        // lumis this file processed
    std::vector<Event> events;               // in scrambled order
    std::uint64_t jetMismatchEvent = 0;      // write three jets for this event
    std::int64_t filtered = 0;               // events read but not written (Runs.genEventCount)
};

void WriteAux(const std::string &path, const AuxSpec &spec) {
    {
        auto model = ROOT::RNTupleModel::Create();
        auto run = model->MakeField<std::uint32_t>("run");
        auto lumi = model->MakeField<std::uint32_t>("luminosityBlock");
        auto event = model->MakeField<std::uint64_t>("event");
        auto nJet = model->MakeField<std::uint32_t>("nJet");
        auto jetPt = model->MakeField<RVec<float>>("Jet_pt");
        auto jetPull = model->MakeField<RVec<float>>("Jet_pullY");
        auto nGenPart = model->MakeField<std::uint32_t>("nGenPart");
        auto genPt = model->MakeField<RVec<float>>("GenPart_pt");
        auto nLambda = model->MakeField<std::uint32_t>("nLambda");
        auto lambdaPt = model->MakeField<RVec<float>>("Lambda_pt");
        auto loose = model->MakeField<RVec<float>>("LambdaLoose_pt");
        auto writer = ROOT::RNTupleWriter::Recreate(std::move(model), "Events", path);
        for (const auto &e : spec.events) {
            *run = kRun;
            *lumi = e.lumi;
            *event = e.event;
            const std::size_t jets = e.event == spec.jetMismatchEvent ? 3 : 2;
            *jetPt = RVec<float>(jets, 1.f);
            *jetPull = RVec<float>(jets, 0.25f);
            *nJet = jets;
            *genPt = {20.f, 30.f};
            *nGenPart = 2;
            *lambdaPt = {LambdaPt(e.event)};
            *nLambda = 1;
            *loose = {-1.f};
            writer->Fill();
        }
    }
    auto model = ROOT::RNTupleModel::Create();
    auto run = model->MakeField<std::uint32_t>("run");
    auto lumi = model->MakeField<std::uint32_t>("luminosityBlock");
    std::unique_ptr<TFile> file(TFile::Open(path.c_str(), "UPDATE"));
    auto writer = ROOT::RNTupleWriter::Append(std::move(model), "LuminosityBlocks", *file);
    for (const auto l : spec.lumis) {
        *run = kRun;
        *lumi = l;
        writer->Fill();
    }
    writer.reset();
    auto runsModel = ROOT::RNTupleModel::Create();
    auto runsRun = runsModel->MakeField<std::uint32_t>("run");
    auto genEventCount = runsModel->MakeField<std::int64_t>("genEventCount");
    auto genEventSumw = runsModel->MakeField<double>("genEventSumw");
    auto genEventSumw2 = runsModel->MakeField<double>("genEventSumw2");
    auto runsWriter = ROOT::RNTupleWriter::Append(std::move(runsModel), "Runs", *file);
    *runsRun = kRun;
    *genEventCount = static_cast<std::int64_t>(spec.events.size()) + spec.filtered;
    *genEventSumw = static_cast<double>(*genEventCount);
    *genEventSumw2 = static_cast<double>(*genEventCount);
    runsWriter->Fill();
    runsWriter.reset();
    file->Close();
}

class FriendBuilderTest : public ::testing::Test {
protected:
    void SetUp() override {
        dir_ = fs::temp_directory_path() /
               ("sknano-friend-builder-" + std::to_string(::getpid()));
        std::error_code error;
        fs::create_directories(dir_, error);
        if (error)
            GTEST_SKIP() << "no writable temporary directory: " << error.message();
        central_ = (dir_ / "central.root").string();
        WriteCentral(central_);

        // aux0: lumis 10, 11 and an unrelated lumi 50; event 103 filtered out,
        // event 999 only here.  aux1: lumi 13.  Lumi 12 is nowhere.
        std::vector<Event> first, second;
        for (const auto &e : CentralEvents()) {
            if (e.lumi == 12 || e.event == 103)
                continue;
            (e.lumi == 13 ? second : first).push_back(e);
        }
        first.push_back({10, 999});
        first.push_back({50, 5000});
        std::mt19937 rng(7);
        std::shuffle(first.begin(), first.end(), rng);
        std::shuffle(second.begin(), second.end(), rng);
        aux0_ = (dir_ / "aux0.root").string();
        aux1_ = (dir_ / "aux1.root").string();
        WriteAux(aux0_, {{10, 11, 50}, first});
        WriteAux(aux1_, {{13}, second});
        first_ = first;
    }
    void TearDown() override {
        std::error_code error;
        fs::remove_all(dir_, error);
    }
    sknano_friend::BuildOptions Options() const {
        sknano_friend::BuildOptions options;
        options.central = central_;
        options.aux = {aux0_, aux1_};
        options.out = (dir_ / "out" / "friend.root").string();
        options.production = "test";
        options.overrides = {"GenPart"};
        options.excludePrefixes = {"LambdaLoose"};
        options.isMC = true;
        options.scratch = dir_.string();
        options.chunkEntries = 3; // several stage files per aux
        return options;
    }
    static std::vector<float> Read(const SKNano::RNTupleVectorColumn &column,
                                   std::uint64_t entry) {
        const auto span = column.read(entry);
        const auto *data = static_cast<const float *>(span.data);
        return {data, data + span.size};
    }

    fs::path dir_;
    std::string central_, aux0_, aux1_;
    std::vector<Event> first_;
};

// Lumi 12 (events 110..114) and event 103 (lumi 10) are not in the production.
TEST_F(FriendBuilderTest, BuildsAnAlignedFriendWithStatusAndNormalisation) {
    const auto options = Options();
    const auto report = sknano_friend::BuildFriend(options);
    EXPECT_EQ(report.centralEntries, 20U);
    EXPECT_EQ(report.nStatus[0], 14U);
    EXPECT_EQ(report.nStatus[1], 0U);
    EXPECT_EQ(report.nStatus[2], 6U);
    EXPECT_EQ(report.auxOnly, 1U);
    EXPECT_EQ(report.missingLumis,
              (std::vector<std::pair<std::uint32_t, std::uint32_t>>{{kRun, 10U}, {kRun, 12U}}));
    EXPECT_EQ(report.missingCount, 6U);
    float sumW = 0.f, sumSign = 0.f;
    for (std::uint64_t e : {103, 110, 111, 112, 113, 114}) {
        sumW += Weight(e);
        sumSign += Weight(e) > 0 ? 1.f : -1.f;
    }
    EXPECT_FLOAT_EQ(report.missingSumW, sumW);
    EXPECT_FLOAT_EQ(report.missingSumSign, sumSign);
    EXPECT_EQ(report.fields, (std::vector<std::string>{
        "GenPart_pt", "Jet_pullY", "Lambda_pt", "nGenPart", "nLambda"}));
    EXPECT_EQ(report.checkedCounts, std::vector<std::string>{"nJet"});
    EXPECT_FALSE(fs::exists(options.out + ".tmp"));

    const auto check = sknano_friend::CheckFriend(options.out, central_);
    EXPECT_EQ(check.mismatches, 0U);
    EXPECT_EQ(check.nStatus[0], 14U);
    EXPECT_EQ(check.info["production"], "test");
    EXPECT_EQ(check.info["missing_lumis"], nlohmann::json({10, 12}));

    SKNano::RNTupleSource source;
    source.open("Events", central_);
    source.addFriend(options.out);
    EXPECT_FALSE(source.hasField("LambdaLoose_pt"));
    auto lambdaPt = source.makeVectorColumn("Lambda_pt", std::type_index(typeid(float)));
    auto genPt = source.makeVectorColumn("GenPart_pt", std::type_index(typeid(float)));
    auto pull = source.makeVectorColumn("Jet_pullY", std::type_index(typeid(float)));
    auto nLambda = source.makeScalarColumn("nLambda", std::type_index(typeid(unsigned int)));
    const auto events = CentralEvents();
    for (std::uint64_t i = 0; i < events.size(); ++i) {
        ASSERT_NO_THROW(source.noteEntry(i));
        const auto status = source.friendStatus(i);
        unsigned int count = 99;
        nLambda->read(i, &count);
        if (events[i].lumi == 12 || events[i].event == 103) {
            EXPECT_EQ(status, 2);
        } else {
            EXPECT_EQ(status, 0);
            EXPECT_EQ(Read(*lambdaPt, i), std::vector<float>{LambdaPt(events[i].event)});
            EXPECT_EQ(Read(*genPt, i), (std::vector<float>{20.f, 30.f}));
            EXPECT_EQ(Read(*pull, i), (std::vector<float>{0.25f, 0.25f}));
            EXPECT_EQ(count, 1U);
            continue;
        }
        EXPECT_TRUE(Read(*lambdaPt, i).empty());
        EXPECT_TRUE(Read(*genPt, i).empty());
        EXPECT_EQ(count, 0U);
    }
}

TEST_F(FriendBuilderTest, LumiSplitOverAuxFilesIsJoinedWhenAllPartsAreGiven) {
    // Lumi 11 (events 105..109) split: 105..106 stay in aux0, 107..109 move
    // to aux2; both files list lumi 11 in LuminosityBlocks, as NanoAOD does.
    std::vector<Event> kept, moved;
    for (const auto &e : first_)
        (e.lumi == 11 && e.event >= 107 ? moved : kept).push_back(e);
    ASSERT_EQ(moved.size(), 3U);
    const auto part0 = (dir_ / "split0.root").string();
    const auto part2 = (dir_ / "split2.root").string();
    WriteAux(part0, {{10, 11, 50}, kept});
    WriteAux(part2, {{11}, moved});

    auto options = Options();
    options.aux = {part0, aux1_, part2};
    const auto report = sknano_friend::BuildFriend(options);
    EXPECT_EQ(report.nStatus[0], 14U);
    EXPECT_EQ(report.nStatus[2], 6U);
    EXPECT_EQ(sknano_friend::CheckFriend(options.out, central_).mismatches, 0U);

    // Without the second part its events are simply lost.
    options.aux = {part0, aux1_};
    options.out = (dir_ / "out" / "partial.root").string();
    const auto partial = sknano_friend::BuildFriend(options);
    EXPECT_EQ(partial.nStatus[0], 11U);
    EXPECT_EQ(partial.nStatus[2], 9U);
}

TEST_F(FriendBuilderTest, DataDropsEveryLumiThatLostAnEvent) {
    auto options = Options();
    options.isMC = false;
    const auto report = sknano_friend::BuildFriend(options);
    EXPECT_EQ(report.nStatus[0], 10U); // lumi 10 lost event 103, so all of lumi 10 goes
    EXPECT_EQ(report.nStatus[2], 10U);
    EXPECT_EQ(report.missingLumis,
              (std::vector<std::pair<std::uint32_t, std::uint32_t>>{{kRun, 10U}, {kRun, 12U}}));
    EXPECT_EQ(report.missingSumW, 0.);
}

TEST_F(FriendBuilderTest, FilteredProductionMarksRejectedEventsAndRecordsRuns) {
    // Same events, but aux0 read one more than it wrote (event 103 rejected)
    // and the lumi-12 file read five and wrote none.
    const auto f0 = (dir_ / "filtered0.root").string();
    const auto f1 = (dir_ / "filtered1.root").string();
    std::vector<Event> second;
    for (const auto &e : CentralEvents())
        if (e.lumi == 13)
            second.push_back(e);
    WriteAux(f0, {{10, 11, 50}, first_, 0, 1});
    WriteAux(f1, {{12, 13}, second, 0, 5});
    auto options = Options();
    options.aux = {f0, f1};
    options.filtered = true;
    const auto report = sknano_friend::BuildFriend(options);
    EXPECT_EQ(report.nStatus[0], 14U);
    EXPECT_EQ(report.nStatus[1], 6U);
    EXPECT_EQ(report.nStatus[2], 0U);
    EXPECT_EQ(report.missingSumW, 0.);
    ASSERT_EQ(report.auxRuns.size(), 2U);
    EXPECT_EQ(report.auxRuns.at(f0).count, static_cast<std::int64_t>(first_.size()) + 1);
    EXPECT_EQ(report.auxRuns.at(f1).count, static_cast<std::int64_t>(second.size()) + 5);
    EXPECT_EQ(report.toJson()["filtered"], true);

    // The declaration is checked against the Runs table both ways.
    options.filtered = false;
    options.out = (dir_ / "out" / "undeclared.root").string();
    EXPECT_THROW(sknano_friend::BuildFriend(options), std::runtime_error);
    auto unfiltered = Options();
    unfiltered.filtered = true;
    unfiltered.out = (dir_ / "out" / "overdeclared.root").string();
    EXPECT_THROW(sknano_friend::BuildFriend(unfiltered), std::runtime_error);
}

TEST_F(FriendBuilderTest, DuplicateEventAcrossAuxFilesIsFatal) {
    auto options = Options();
    const auto dup = (dir_ / "dup.root").string();
    WriteAux(dup, {{11}, {{11, 106}}}); // 106 is already in aux0
    options.aux.push_back(dup);
    EXPECT_THROW(sknano_friend::BuildFriend(options), std::runtime_error);
}

TEST_F(FriendBuilderTest, ExtensionRowCountMismatchIsFatal) {
    auto options = Options();
    const auto bad = (dir_ / "bad0.root").string();
    WriteAux(bad, {{10, 11, 50}, first_, 101});
    options.aux[0] = bad;
    EXPECT_THROW(sknano_friend::BuildFriend(options), std::runtime_error);
}

} // namespace
