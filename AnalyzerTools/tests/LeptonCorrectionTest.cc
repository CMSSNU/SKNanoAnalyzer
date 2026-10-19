#include <gtest/gtest.h>

#include <cmath>
#include <cstdlib>
#include <exception>
#include <memory>
#include <string>
#include <vector>

#include "MuonScaRe.h"
#include "MyCorrection.h"
#include "TauTestSupport.h"

using sknano_test::VectorColumn;

namespace {

TEST(MuonCrystalBall, InverseCdfUndoesCdf) {
    const SKNano::MuonCrystalBall cb(0.01, 0.95, 1.6, 4.5);
    for (const double x : {-8., -3., -1.6, -0.5, 0., 0.7, 1.5, 3., 8.})
        EXPECT_NEAR(cb.invcdf(cb.cdf(x)), x, 1e-6) << "x = " << x;
}

// Built once for the suite: MyCorrection opens every file the era declares.
// Skips when the correction area is not reachable.
class LeptonCorrection : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        if (!std::getenv("JSONPOG_REPO_PATH") || !std::getenv("SKNANO_DATA") ||
            !std::getenv("ROCCOR_PATH")) {
            skip_ = "correction environment not set; source setup.sh first";
            return;
        }
        try {
            mc_ = std::make_unique<MyCorrection>("2024", "CDE", "TTLJ_powheg",
                                                 false);
            data_ = std::make_unique<MyCorrection>("2024", "CDE", "Muon", true);
        } catch (const std::exception &e) {
            skip_ = std::string("lepton corrections unavailable: ") + e.what();
            mc_.reset();
            data_.reset();
        }
    }
    static void TearDownTestSuite() {
        mc_.reset();
        data_.reset();
    }
    void SetUp() override {
        if (!mc_)
            GTEST_SKIP() << skip_;
    }

    static std::unique_ptr<MyCorrection> mc_;
    static std::unique_ptr<MyCorrection> data_;
    static std::string skip_;
};

std::unique_ptr<MyCorrection> LeptonCorrection::mc_;
std::unique_ptr<MyCorrection> LeptonCorrection::data_;
std::string LeptonCorrection::skip_;

// ---- MuonScaRe validity window ---------------------------------------------
// Expected values come from the POG C++ reference (jsonpog-integration
// examples/MuonScaRe.cc: pt_scale, pt_scale_var, pt_resol_var) run on the 2024
// MUO muon_scalesmearing.json.gz of snapshot 2026-06-18. Regenerate them when
// the era yml moves to a different MUO snapshot. MyCorrection reads the
// parameters at float precision, hence the relative tolerance.
constexpr double kRelTol = 1e-6;
#define EXPECT_PT(actual, expected) \
    EXPECT_NEAR((actual), (expected), kRelTol * (expected))

struct Lanes {
    MuonScaReState state;
    MyCorrection::MuonMomentumLanes lanes;
};
Lanes Correct(const MyCorrection &corr, int q, float pt, float eta,
              float phi) {
    const MuonScaReState state =
        corr.GetMuonScaleSmearing(q, pt, eta, phi, 14, 123456789ull, 42u);
    return {state, corr.GetMuonScaleSmearingVariations(state, q, eta, phi)};
}

TEST_F(LeptonCorrection, MuonDataScaleStartsAt26AndHasNoUpperBound) {
    const auto below = Correct(*data_, 1, 25.5f, 0.5f, 1.f).lanes;
    EXPECT_EQ(below.nominal, 25.5f);
    EXPECT_PT(Correct(*data_, 1, 26.f, 0.5f, 1.f).lanes.nominal,
              26.020476799426799);
    EXPECT_PT(Correct(*data_, -1, 250.f, -1.25f, -2.5f).lanes.nominal,
              249.86000518770359);
    const auto high = Correct(*data_, 1, 1000.f, 0.5f, 1.f).lanes;
    EXPECT_PT(high.nominal, 971.26511637736098);
    // Data carries no variation of its own.
    EXPECT_EQ(high.scaleUp, high.nominal);
    EXPECT_EQ(high.resDown, high.nominal);
}

TEST_F(LeptonCorrection, MuonMCOutsideSmearWindowKeepsScaleUncertainty) {
    // Below 26 GeV: no scale, no smearing, but the stat scale uncertainty.
    const auto low = Correct(*mc_, -1, 20.f, -1.25f, -2.5f).lanes;
    EXPECT_EQ(low.nominal, 20.f);
    EXPECT_PT(low.scaleUp, 20.000497985848114);
    EXPECT_PT(low.scaleDown, 19.999502014151886);
    EXPECT_EQ(low.resUp, low.nominal);
    EXPECT_EQ(low.resDown, low.nominal);

    // Above 200 GeV: scaled but not smeared.
    const auto high = Correct(*mc_, -1, 250.f, -1.25f, -2.5f);
    EXPECT_EQ(high.state.k, 0.);
    EXPECT_PT(high.lanes.nominal, 250.48865383178176);
    EXPECT_PT(high.lanes.scaleUp, 250.6103083969285);
    EXPECT_PT(high.lanes.scaleDown, 250.36699926663502);
    EXPECT_EQ(high.lanes.resUp, high.lanes.nominal);
    EXPECT_EQ(high.lanes.resDown, high.lanes.nominal);
}

TEST_F(LeptonCorrection, MuonMCInsideSmearWindowMatchesReference) {
    // The draw comes from the file's RandomSmearing node rather than the
    // reference's TRandom3, so the reference variations are evaluated on this
    // code's scaled and smeared pt: (26.020255338753788, 26.051951680637128)
    // at 26 GeV and (199.88338519687858, 200.31881020270939) at 199.5 GeV.
    const auto at26 = Correct(*mc_, 1, 26.f, 0.5f, 1.f);
    EXPECT_PT(at26.state.scaledPt, 26.020255767764539);
    EXPECT_GT(at26.state.k, 0.);
    EXPECT_NE(at26.state.correctedPt, at26.state.scaledPt);
    EXPECT_PT(at26.lanes.scaleUp, 26.052231582040616);
    EXPECT_PT(at26.lanes.scaleDown, 26.05167177923364);
    EXPECT_PT(at26.lanes.resUp, 26.051996695343487);
    EXPECT_PT(at26.lanes.resDown, 26.051906665930769);

    // Scaled to 199.88 GeV, inside the window, so smeared even though the
    // result lands above 200 GeV.
    const auto at200 = Correct(*mc_, 1, 199.5f, 0.5f, 1.f);
    EXPECT_PT(at200.state.scaledPt, 199.88338849011086);
    EXPECT_PT(at200.lanes.nominal, 200.31881020270939);
    EXPECT_PT(at200.lanes.scaleUp, 200.32214446720778);
    EXPECT_PT(at200.lanes.scaleDown, 200.31547593821099);
    EXPECT_PT(at200.lanes.resUp, 200.31942858728041);
    EXPECT_PT(at200.lanes.resDown, 200.31819181813836);
}

// ---- Electron SF evaluation point ------------------------------------------
TEST_F(LeptonCorrection, ElectronSFsClampPtToTheMapEdge) {
    EXPECT_FLOAT_EQ(mc_->GetElectronIDSF("Tight", 0.5f, 9.9f, 0.f),
                    mc_->GetElectronIDSF("Tight", 0.5f, 10.f, 0.f));
    EXPECT_FLOAT_EQ(mc_->GetElectronRECOSF(0.5f, 9.9f, 0.f),
                    mc_->GetElectronRECOSF(0.5f, 10.f, 0.f));
    EXPECT_FLOAT_EQ(
        mc_->GetElectronTriggerSF("HLT_SF_Ele30_MVAiso80ID", 0.5f, 24.9f, 0.f),
        mc_->GetElectronTriggerSF("HLT_SF_Ele30_MVAiso80ID", 0.5f, 25.f, 0.f));
}

TEST_F(LeptonCorrection, ElectronViewSFsUseSignedScEtaAndRawPt) {
    // Track eta and supercluster eta on opposite sides of zero, raw pt below
    // the ID map edge.
    VectorColumn<float> pt{std::vector<float>{9.9f}};
    VectorColumn<float> eta{std::vector<float>{0.5f}};
    VectorColumn<float> phi{std::vector<float>{0.f}};
    VectorColumn<float> scEta{std::vector<float>{-0.5f}};
    ElectronSoA soa;
    soa.pt.bind(&pt);
    soa.eta.bind(&eta);
    soa.phi.bind(&phi);
    soa.scEta.bind(&scEta);
    const ElectronView electron(&soa, 0);

    // The map is not symmetric in eta.
    EXPECT_NE(mc_->GetElectronIDSF("Tight", -0.5f, 40.f, 0.f),
              mc_->GetElectronIDSF("Tight", 0.5f, 40.f, 0.f));
    EXPECT_FLOAT_EQ(mc_->GetElectronIDSF("Tight", electron),
                    mc_->GetElectronIDSF("Tight", -0.5f, 10.f, 0.f));
    EXPECT_FLOAT_EQ(mc_->GetElectronRECOSF(electron),
                    mc_->GetElectronRECOSF(-0.5f, 10.f, 0.f));
    EXPECT_FLOAT_EQ(
        mc_->GetElectronTriggerSF("HLT_SF_Ele30_MVAiso80ID", electron),
        mc_->GetElectronTriggerSF("HLT_SF_Ele30_MVAiso80ID", -0.5f, 25.f,
                                  0.f));
}

} // namespace
