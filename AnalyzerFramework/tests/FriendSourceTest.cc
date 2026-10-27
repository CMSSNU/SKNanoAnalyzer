#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <typeindex>
#include <unistd.h>
#include <vector>

#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleWriter.hxx>
#include <ROOT/RVec.hxx>
#include <TFile.h>

#include <AnalyzerFramework/RNTupleSource.h>

#include "AnalysisException.h"

// Round trip of the friend contract through RNTupleSource alone: BranchManager
// and the typed handles only ever call hasField/fieldType/make*Column, so what
// is checked here is exactly what an analyzer sees.
namespace {

namespace fs = std::filesystem;
using ROOT::VecOps::RVec;

constexpr std::uint64_t kEntries = 4;

struct FriendSpec {
    std::uint64_t entries = kEntries;
    std::vector<std::string> overrides{"GenPart"};
    std::vector<std::uint8_t> status{0, 0, 1, 2};
    bool breakKeyAtEntry2 = false;
    bool carryJetPt = false; // a duplicate that is not an override
    std::uint32_t formatVersion = SKNano::kFriendFormatVersion;
    std::uint64_t centralEntries = kEntries;
};

void WritePrimary(const std::string &path) {
    auto model = ROOT::RNTupleModel::Create();
    auto run = model->MakeField<std::uint32_t>("run");
    auto lumi = model->MakeField<std::uint32_t>("luminosityBlock");
    auto event = model->MakeField<std::uint64_t>("event");
    auto nJet = model->MakeField<std::uint32_t>("nJet");
    auto jetPt = model->MakeField<RVec<float>>("Jet_pt");
    auto nGenPart = model->MakeField<std::uint32_t>("nGenPart");
    auto genPt = model->MakeField<RVec<float>>("GenPart_pt");
    auto writer = ROOT::RNTupleWriter::Recreate(std::move(model), "Events", path);
    for (std::uint64_t i = 0; i < kEntries; ++i) {
        *run = 1;
        *lumi = 10 + static_cast<std::uint32_t>(i / 2);
        *event = 100 + i;
        *jetPt = {10.f + i, 20.f + i};
        *nJet = jetPt->size();
        *genPt = {1.f + i};
        *nGenPart = genPt->size();
        writer->Fill();
    }
}

void WriteFriend(const std::string &path, const FriendSpec &spec) {
    {
        auto model = ROOT::RNTupleModel::Create();
        auto run = model->MakeField<std::uint32_t>("run");
        auto lumi = model->MakeField<std::uint32_t>("luminosityBlock");
        auto event = model->MakeField<std::uint64_t>("event");
        auto status = model->MakeField<std::uint8_t>(SKNano::kFriendStatusField);
        auto nLambda = model->MakeField<std::uint32_t>("nLambda");
        auto lambdaPt = model->MakeField<RVec<float>>("Lambda_pt");
        auto nGenPart = model->MakeField<std::uint32_t>("nGenPart");
        auto genPt = model->MakeField<RVec<float>>("GenPart_pt");
        std::shared_ptr<RVec<float>> jetPt;
        if (spec.carryJetPt)
            jetPt = model->MakeField<RVec<float>>("Jet_pt");
        auto writer = ROOT::RNTupleWriter::Recreate(std::move(model), "Events", path);
        for (std::uint64_t i = 0; i < spec.entries; ++i) {
            *run = 1;
            *lumi = 10 + static_cast<std::uint32_t>(i / 2);
            *event = 100 + i + (spec.breakKeyAtEntry2 && i == 2 ? 1000 : 0);
            *status = i < spec.status.size() ? spec.status[i] : 0;
            if (*status == 0) {
                *lambdaPt = {5.f + i};
                *genPt = {1.f + i, 2.f + i, 3.f + i}; // more rows than the primary
            } else {
                lambdaPt->clear();
                genPt->clear();
            }
            *nLambda = lambdaPt->size();
            *nGenPart = genPt->size();
            if (jetPt)
                *jetPt = {0.f};
            writer->Fill();
        }
    }
    auto model = ROOT::RNTupleModel::Create();
    auto version = model->MakeField<std::uint32_t>("formatVersion");
    auto overrides = model->MakeField<std::vector<std::string>>("overrides");
    auto centralEntries = model->MakeField<std::uint64_t>("centralEntries");
    std::unique_ptr<TFile> file(TFile::Open(path.c_str(), "UPDATE"));
    auto writer = ROOT::RNTupleWriter::Append(std::move(model),
                                              SKNano::kFriendInfoNTuple, *file);
    *version = spec.formatVersion;
    *overrides = spec.overrides;
    *centralEntries = spec.centralEntries;
    writer->Fill();
    writer.reset();
    file->Close();
}

class FriendSourceTest : public ::testing::Test {
protected:
    void SetUp() override {
        dir_ = fs::temp_directory_path() /
               ("sknano-friend-" + std::to_string(::getpid()));
        std::error_code error;
        fs::create_directories(dir_, error);
        if (error)
            GTEST_SKIP() << "no writable temporary directory: " << error.message();
        primary_ = (dir_ / "primary.root").string();
        WritePrimary(primary_);
    }
    void TearDown() override {
        std::error_code error;
        fs::remove_all(dir_, error);
    }
    std::string friendFile(const std::string &name, const FriendSpec &spec) {
        const auto path = (dir_ / name).string();
        WriteFriend(path, spec);
        return path;
    }
    static std::vector<float> Read(const SKNano::RNTupleVectorColumn &column,
                                   std::uint64_t entry) {
        const auto span = column.read(entry);
        const auto *data = static_cast<const float *>(span.data);
        return {data, data + span.size};
    }

    fs::path dir_;
    std::string primary_;
};

TEST_F(FriendSourceTest, FriendFieldsAreServedNextToPrimaryOnes) {
    SKNano::RNTupleSource source;
    source.open("Events", primary_);
    source.addFriend(friendFile("friend.root", FriendSpec{}));
    ASSERT_EQ(source.friendCount(), 1U);

    EXPECT_TRUE(source.hasField("Jet_pt"));
    EXPECT_TRUE(source.hasField("Lambda_pt"));
    EXPECT_TRUE(source.hasField("nLambda"));
    EXPECT_EQ(source.fieldType("Lambda_pt"), "ROOT::VecOps::RVec<float>");

    auto jetPt = source.makeVectorColumn("Jet_pt", std::type_index(typeid(float)));
    auto lambdaPt = source.makeVectorColumn("Lambda_pt", std::type_index(typeid(float)));
    auto nLambda = source.makeScalarColumn("nLambda", std::type_index(typeid(unsigned int)));
    for (std::uint64_t entry = 0; entry < kEntries; ++entry) {
        source.noteEntry(entry);
        EXPECT_EQ(Read(*jetPt, entry), (std::vector<float>{10.f + entry, 20.f + entry}));
        unsigned int count = 99;
        nLambda->read(entry, &count);
        if (entry < 2) {
            EXPECT_EQ(Read(*lambdaPt, entry), std::vector<float>{5.f + entry});
            EXPECT_EQ(count, 1U);
        } else {
            EXPECT_TRUE(Read(*lambdaPt, entry).empty());
            EXPECT_EQ(count, 0U);
        }
    }
}

TEST_F(FriendSourceTest, DeclaredOverrideReplacesThePrimaryCollection) {
    SKNano::RNTupleSource source;
    source.open("Events", primary_);
    source.addFriend(friendFile("friend.root", FriendSpec{}));
    auto genPt = source.makeVectorColumn("GenPart_pt", std::type_index(typeid(float)));
    auto nGenPart = source.makeScalarColumn("nGenPart", std::type_index(typeid(unsigned int)));
    EXPECT_EQ(Read(*genPt, 1), (std::vector<float>{2.f, 3.f, 4.f}));
    unsigned int count = 0;
    nGenPart->read(1, &count);
    EXPECT_EQ(count, 3U);
}

TEST_F(FriendSourceTest, ReservedFieldsStayWithThePrimary) {
    SKNano::RNTupleSource source;
    source.open("Events", primary_);
    source.addFriend(friendFile("friend.root", FriendSpec{}));
    EXPECT_FALSE(source.hasField(SKNano::kFriendStatusField));
    EXPECT_TRUE(source.hasField("event"));
    auto event = source.makeScalarColumn("event", std::type_index(typeid(ULong64_t)));
    ULong64_t value = 0;
    event->read(3, &value);
    EXPECT_EQ(value, 103U);
}

TEST_F(FriendSourceTest, StatusFollowsTheFriendColumn) {
    SKNano::RNTupleSource source;
    source.open("Events", primary_);
    EXPECT_EQ(source.friendStatus(0), 0);
    source.addFriend(friendFile("friend.root", FriendSpec{}));
    EXPECT_EQ(source.friendStatus(0), 0);
    EXPECT_EQ(source.friendStatus(2), 1);
    EXPECT_EQ(source.friendStatus(3), 2);
}

TEST_F(FriendSourceTest, EntryCountMismatchIsRejectedOnAttach) {
    FriendSpec spec;
    spec.entries = kEntries - 1;
    spec.centralEntries = kEntries - 1;
    SKNano::RNTupleSource source;
    source.open("Events", primary_);
    EXPECT_THROW(source.addFriend(friendFile("short.root", spec)), SKNano::ConfigError);
    EXPECT_EQ(source.friendCount(), 0U);
}

TEST_F(FriendSourceTest, MisalignedKeyIsCaughtAtThatEntry) {
    FriendSpec spec;
    spec.breakKeyAtEntry2 = true;
    SKNano::RNTupleSource source;
    source.open("Events", primary_);
    source.addFriend(friendFile("broken.root", spec));
    EXPECT_NO_THROW(source.noteEntry(0));
    EXPECT_NO_THROW(source.noteEntry(1));
    EXPECT_THROW(source.noteEntry(2), SKNano::ConfigError);
}

TEST_F(FriendSourceTest, UndeclaredDuplicateIsRejected) {
    FriendSpec spec;
    spec.carryJetPt = true;
    SKNano::RNTupleSource source;
    source.open("Events", primary_);
    EXPECT_THROW(source.addFriend(friendFile("dup.root", spec)), SKNano::ConfigError);
}

TEST_F(FriendSourceTest, UnknownFormatVersionIsRejected) {
    FriendSpec spec;
    spec.formatVersion = SKNano::kFriendFormatVersion + 1;
    SKNano::RNTupleSource source;
    source.open("Events", primary_);
    EXPECT_THROW(source.addFriend(friendFile("v2.root", spec)), SKNano::ConfigError);
}

} // namespace
