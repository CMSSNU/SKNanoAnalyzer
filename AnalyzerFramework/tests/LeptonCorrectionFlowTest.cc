#include <gtest/gtest.h>

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <memory>
#include <string>
#include <vector>

#include <AnalyzerFramework/AnalyzerCore.h>

#include "TauTestSupport.h"

using sknano_test::VectorColumn;

namespace {

// Two electrons with identical kinematics, so only the electron index tells
// their smearing draws apart.
struct ElectronColumns {
    VectorColumn<float> pt{std::vector<float>{40.f, 40.f}};
    VectorColumn<float> eta{std::vector<float>{0.5f, 0.5f}};
    VectorColumn<float> phi{std::vector<float>{1.f, 1.f}};
    VectorColumn<float> scEta{std::vector<float>{0.5f, 0.5f}};
    VectorColumn<float> r9{std::vector<float>{0.95f, 0.95f}};
    VectorColumn<unsigned char> seedGain{std::vector<unsigned char>{12, 12}};
};

// Binds the columns and the two AnalyzerCore stages for one (run, lumi, event).
void BindElectrons(ElectronSoA &soa, ElectronColumns &columns,
                   AnalyzerCore &core, std::uint64_t run, std::uint64_t lumi,
                   std::uint64_t event) {
    soa.pt.bind(&columns.pt);
    soa.eta.bind(&columns.eta);
    soa.phi.bind(&columns.phi);
    soa.scEta.bind(&columns.scEta);
    soa.r9.bind(&columns.r9);
    soa.seedGain.bind(&columns.seedGain);
    soa.populateMomentum = [&soa, &core, run, lumi, event] {
        core.PopulateElectronMomentum(soa, run, lumi, event);
    };
    soa.populateMomentumVariations = [&soa, &core, run] {
        core.PopulateElectronMomentumVariations(soa, run);
    };
}

class ElectronSmearing : public ::testing::Test {
protected:
    void SetUp() override {
        if (!std::getenv("JSONPOG_REPO_PATH") || !std::getenv("SKNANO_DATA") ||
            !std::getenv("ROCCOR_PATH"))
            GTEST_SKIP() << "correction environment not set; source setup.sh first";
        try {
            core_.myCorr = new MyCorrection("2024", "CDE", "TTLJ_powheg", false);
        } catch (const std::exception &e) {
            GTEST_SKIP() << "electron corrections unavailable: " << e.what();
        }
        core_.IsDATA = false;
    }
    AnalyzerCore core_; // owns and deletes myCorr
};

TEST_F(ElectronSmearing, NominalAndSmearVariationsShareOneDraw) {
    ElectronColumns columns;
    ElectronSoA soa;
    BindElectrons(soa, columns, core_, 386000, 120, 987654321);
    const ElectronView electron(&soa, 0);

    const float nominal = electron.Pt();
    EXPECT_TRUE(soa.smearUpPt.empty()); // the nominal stage stops there
    EXPECT_TRUE(soa.scaleUpPt.empty());
    EXPECT_NE(nominal, 40.f);

    // Each lane is the raw pt smeared with its own width and the one draw.
    const float draw = soa.smearDraw[0];
    const auto expected = [&](MyCorrection::variation syst) {
        return 40.f * (1.f + draw * core_.myCorr->GetElectronSmearWidth(
                                        40.f, 0.95f, 0.5f, syst));
    };
    EXPECT_FLOAT_EQ(nominal, expected(MyCorrection::variation::nom));
    EXPECT_FLOAT_EQ(electron.SmearUpPt(), expected(MyCorrection::variation::up));
    EXPECT_FLOAT_EQ(electron.SmearDownPt(),
                    expected(MyCorrection::variation::down));
}

TEST_F(ElectronSmearing, DrawIsDeterministicPerEventAndElectron) {
    ElectronColumns columnsA, columnsB, columnsC;
    ElectronSoA a, b, c;
    BindElectrons(a, columnsA, core_, 386000, 120, 987654321);
    BindElectrons(b, columnsB, core_, 386000, 120, 987654321);
    BindElectrons(c, columnsC, core_, 386000, 120, 987654322);

    EXPECT_EQ(ElectronView(&a, 0).Pt(), ElectronView(&b, 0).Pt());
    EXPECT_EQ(ElectronView(&a, 1).SmearUpPt(), ElectronView(&b, 1).SmearUpPt());
    // Identical kinematics, different index or event: different draw.
    EXPECT_NE(a.smearDraw[0], a.smearDraw[1]);
    EXPECT_NE(ElectronView(&a, 0).Pt(), ElectronView(&c, 0).Pt());
}

TEST_F(ElectronSmearing, DataVariationsEqualTheNominal) {
    MyCorrection *mc = core_.myCorr;
    core_.myCorr = new MyCorrection("2024", "CDE", "EGamma", true);
    delete mc;
    core_.IsDATA = true;
    ElectronColumns columns;
    ElectronSoA soa;
    BindElectrons(soa, columns, core_, 386000, 120, 987654321);
    const ElectronView electron(&soa, 0);
    EXPECT_EQ(electron.ScaleUpPt(), electron.Pt());
    EXPECT_EQ(electron.SmearDownPt(), electron.Pt());
}

// ---- MET propagation -------------------------------------------------------
struct MuonColumns {
    VectorColumn<float> pt{std::vector<float>{30.f, 50.f}};
    VectorColumn<float> phi{std::vector<float>{0.f, 1.5f}};
};

void BindMuons(MuonSoA &soa, MuonColumns &columns) {
    soa.pt.bind(&columns.pt);
    soa.phi.bind(&columns.phi);
    soa.populateMomentum = [&soa] {
        soa.miniAODPt = {30.f, 50.f};
        soa.correctedPt = {31.f, 49.f};
    };
    soa.populateMomentumVariations = [&soa] {
        soa.momentumScaleUp = {32.f, 49.5f};
        soa.momentumScaleDown = {30.5f, 48.5f};
        soa.momentumResUp = soa.momentumResDown = {31.f, 49.f};
    };
}

TEST(LeptonMETPropagation, SubtractsCorrectionAlongLeptonPhi) {
    MuonColumns columns;
    auto soa = std::make_shared<MuonSoA>();
    BindMuons(*soa, columns);
    const MuonViewCollection muons(soa);
    AnalyzerCore core;

    Particle met;
    met.SetXYZM(10., -5., 0., 0.);
    ASSERT_TRUE(core.PropagateMuonCorrToMET(muons, met));
    EXPECT_NEAR(met.Px(), 10. - (1. * std::cos(0.) - 1. * std::cos(1.5)), 1e-5);
    EXPECT_NEAR(met.Py(), -5. - (1. * std::sin(0.) - 1. * std::sin(1.5)), 1e-5);

    Particle metUp;
    metUp.SetXYZM(10., -5., 0., 0.);
    core.PropagateMuonCorrToMET(muons, metUp, MyCorrection::variation::up);
    EXPECT_NEAR(metUp.Px(), 10. - (2. * std::cos(0.) - 0.5 * std::cos(1.5)),
                1e-5);
    EXPECT_NEAR(metUp.Py(), -5. - (2. * std::sin(0.) - 0.5 * std::sin(1.5)),
                1e-5);

    EXPECT_THROW(core.PropagateMuonCorrToMET(muons, metUp,
                                             MyCorrection::variation::up,
                                             MyCorrection::variation::down),
                 std::runtime_error);
    EXPECT_THROW(core.PropagateElectronCorrToMET(ElectronViewCollection(), met,
                                                 MyCorrection::variation::down,
                                                 MyCorrection::variation::up),
                 std::runtime_error);
}

} // namespace
