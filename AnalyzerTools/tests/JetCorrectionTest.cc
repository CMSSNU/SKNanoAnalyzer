#include <gtest/gtest.h>

#include <cstdlib>
#include <exception>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <yaml-cpp/yaml.h>

#include "MyCorrection.h"

namespace {

// Built once for the suite: MyCorrection opens every file the era declares.
// The factorized cross-check reads fatJet_jerc.json.gz itself, resolved from
// the same era yml (campaign, JME snapshot, FJES tags) MyCorrection uses.
class JetCorrection : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        const char *jsonpog = std::getenv("JSONPOG_REPO_PATH");
        const char *data = std::getenv("SKNANO_DATA");
        if (!jsonpog || !data || !std::getenv("ROCCOR_PATH")) {
            skip_ = "correction environment not set; source setup.sh first";
            return;
        }
        try {
            const YAML::Node yml = YAML::LoadFile(
                std::string(data) + "/2024/Correction/era_config.yml");
            const YAML::Node entry = yml["pog_files"]["jerc_fatjet"];
            std::string snapshot =
                yml["snapshots"]["JME"].as<std::string>(
                    yml["snapshots"]["default"].as<std::string>("latest"));
            snapshot = entry["snapshot"].as<std::string>(snapshot);
            if (const char *tag = std::getenv("SKNANO_CORRECTION_TAG"))
                snapshot = tag;
            fatjet_ = correction::CorrectionSet::from_file(
                std::string(jsonpog) + "/JME/" +
                yml["campaign"].as<std::string>() + "/" + snapshot + "/" +
                entry["name"].as<std::string>());
            fjesTag_[false] = yml["global_tags"]["FJES_MC"].as<std::string>();
            fjesTag_[true] = yml["global_tags"]["FJES_DATA"].as<std::string>();

            mc_ = std::make_unique<MyCorrection>("2024", "CDE", "TTLJ_powheg",
                                                 false);
            data_ = std::make_unique<MyCorrection>("2024", "CDE", "Muon", true);
            mc_->GetFJESUncertainty(0.5f, 300.f);
        } catch (const std::exception &e) {
            skip_ = std::string("jet corrections unavailable: ") + e.what();
            mc_.reset();
            data_.reset();
        }
    }

    static void TearDownTestSuite() {
        mc_.reset();
        data_.reset();
        fatjet_.reset();
    }

    void SetUp() override {
        if (!mc_)
            GTEST_SKIP() << skip_;
    }

    // L1 -> L2 -> L3 (-> residual on data), each level evaluated at the pT
    // the previous ones produced, inputs filled by name.
    static double Factorized(bool isData, double area, double eta, double pt,
                             double phi, double rho, double run) {
        std::vector<std::string> levels = {"L1FastJet", "L2Relative",
                                           "L3Absolute"};
        if (isData)
            levels.push_back("L2L3Residual");
        double product = 1.;
        for (const auto &level : levels) {
            std::string key = fjesTag_[isData];
            key.replace(key.find("######"), 6, level);
            const auto corr = fatjet_->at(key);
            std::vector<correction::Variable::Type> args;
            for (const auto &input : corr->inputs()) {
                const std::string &name = input.name();
                args.emplace_back(name == "JetA"     ? area
                                  : name == "JetEta" ? eta
                                  : name == "JetPt"  ? pt
                                  : name == "Rho"    ? rho
                                  : name == "JetPhi" ? phi
                                                     : run);
            }
            const double sf = corr->evaluate(args);
            product *= sf;
            pt *= sf;
        }
        return product;
    }

    static std::unique_ptr<MyCorrection> mc_;
    static std::unique_ptr<MyCorrection> data_;
    static std::unique_ptr<correction::CorrectionSet> fatjet_;
    static std::map<bool, std::string> fjesTag_;
    static std::string skip_;
};

std::unique_ptr<MyCorrection> JetCorrection::mc_;
std::unique_ptr<MyCorrection> JetCorrection::data_;
std::unique_ptr<correction::CorrectionSet> JetCorrection::fatjet_;
std::map<bool, std::string> JetCorrection::fjesTag_;
std::string JetCorrection::skip_;

TEST_F(JetCorrection, FatJetCompoundEqualsFactorizedProduct) {
    struct Point {
        float eta, pt, phi;
    };
    const Point points[] = {
        {0.5f, 300.f, 0.3f}, {2.2f, 200.f, -1.2f}, {-1.3f, 450.f, 2.5f},
        {-2.8f, 150.f, 1.0f}};
    const float area = 2.0f, rho = 20.f;
    const unsigned int run = 380000;
    for (const auto &p : points) {
        SCOPED_TRACE(testing::Message() << "eta " << p.eta << " pt " << p.pt);
        EXPECT_NEAR(mc_->GetFJESSF(area, p.eta, p.pt, p.phi, rho, run),
                    Factorized(false, area, p.eta, p.pt, p.phi, rho, run),
                    1e-5);
        EXPECT_NEAR(data_->GetFJESSF(area, p.eta, p.pt, p.phi, rho, run),
                    Factorized(true, area, p.eta, p.pt, p.phi, rho, run),
                    1e-5);
    }
}

TEST_F(JetCorrection, TotalSourceSpellingsResolveToTotal) {
    const float ak8 = mc_->GetFJESUncertainty(0.5f, 300.f, "Total");
    const float ak4 = mc_->GetJESUncertainty(0.5f, 300.f, "Total");
    EXPECT_GT(ak8, 0.f);
    EXPECT_GT(ak4, 0.f);
    for (const char *spelling : {"", "total", "TOTAL", "ToTaL"}) {
        SCOPED_TRACE(spelling);
        EXPECT_EQ(mc_->GetFJESUncertainty(0.5f, 300.f, spelling), ak8);
        EXPECT_EQ(mc_->GetJESUncertainty(0.5f, 300.f, spelling), ak4);
    }
    EXPECT_EQ(mc_->GetFJESUncertainty(0.5f, 300.f), ak8);
    EXPECT_EQ(mc_->GetJESUncertainty(0.5f, 300.f), ak4);
    EXPECT_EQ(mc_->GetFJESUncertaintySF(0.5f, 300.f, MyCorrection::variation::up),
              1.f + ak8);
}

TEST_F(JetCorrection, UnknownSourceNamesTheMissingKey) {
    for (const bool fatjet : {false, true}) {
        SCOPED_TRACE(fatjet ? "AK8" : "AK4");
        try {
            if (fatjet)
                mc_->GetFJESUncertainty(0.5f, 300.f, "NoSuchSource");
            else
                mc_->GetJESUncertainty(0.5f, 300.f, "NoSuchSource");
            ADD_FAILURE() << "expected SKNano::ConfigError";
        } catch (const SKNano::ConfigError &e) {
            EXPECT_NE(std::string(e.what()).find("_NoSuchSource_"),
                      std::string::npos)
                << e.what();
        }
    }
}

} // namespace
