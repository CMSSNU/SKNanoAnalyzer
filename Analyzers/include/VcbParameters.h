#ifndef VcbParameters_h
#define VcbParameters_h

#include "unordered_map"
#include <string>
#include "TString.h"
#include "Muon.h"
#include "Electron.h"
#include "Jet.h"
#include "JetTaggingParameter.h"

// Vcb Global
static constexpr Muon::MuonID Muon_Veto_ID = Muon::MuonID::POG_TIGHT;
static constexpr Electron::ElectronID Electron_Veto_ID = Electron::ElectronID::POG_MVAISO_WP80;
static constexpr Muon::MuonID Muon_Veto_Iso = Muon::MuonID::POG_PFISO_TIGHT;
static constexpr Muon::MuonID Muon_Tight_ID = Muon::MuonID::POG_TIGHT;
static constexpr Muon::MuonID Muon_Tight_Iso = Muon::MuonID::POG_PFISO_MEDIUM;
static constexpr Electron::ElectronID Electron_Tight_ID = Electron::ElectronID::POG_MVAISO_WP90;

static constexpr float Muon_Veto_Pt = 10.f;
static constexpr float Muon_Veto_Eta = 2.4;
static constexpr float Muon_Tight_Eta = 2.4;

static constexpr float Electron_Veto_Pt = 15.f;
static constexpr float Electron_Veto_Eta = 2.5;
static constexpr float Electron_Tight_Eta = 2.5;

static inline std::unordered_map<std::string, float> Muon_Tight_Pt = {
    {"2016preVFP", 26.f},
    {"2016postVFP", 26.f},
    {"2017", 29.f},
    {"2018", 29.f},
    {"2022EE", 26.f}};

static inline std::unordered_map<std::string, float> Electron_Tight_Pt = {
    {"2016preVFP", 35.f},
    {"2016postVFP", 35.f},
    {"2017", 37.f},
    {"2018", 37.f},
    {"2022EE", 35.f}};

static constexpr float Jet_Veto_DR = 0.4;

static constexpr Jet::JetID Jet_ID = Jet::JetID::TIGHTLEPVETO;
static constexpr Jet::JetID Jet_PUID = Jet::JetID::PUID_MEDIUM;
static constexpr float Jet_Eta_cut = 2.4;

static inline std::unordered_map<std::string, JetTagging::JetFlavTagger> FlavTagger = {
    {"2016preVFP", JetTagging::JetFlavTagger::DeepJet},
    {"2016postVFP", JetTagging::JetFlavTagger::DeepJet},
    {"2017", JetTagging::JetFlavTagger::DeepJet},
    {"2018", JetTagging::JetFlavTagger::DeepJet},
    {"2022EE", JetTagging::JetFlavTagger::ParT}};

// FH part
static inline std::unordered_map<std::string, int> category_for_training_FH = {
    {"TTJJ_Vcb_powheg", 0},
    {"TTJJ_powheg", 1},
    {"TTJJ_powheg_tthf", 2},
    {"QCD", 3}};

static inline std::unordered_map<std::string, TString> FH_Trigger = {
    {"2016preVFP", "HLT_PFHT450_SixJet40_BTagCSV_p056"},
    {"2016postVFP", "HLT_PFHT450_SixJet40_BTagCSV_p056"},
    {"2017", "HLT_PFHT430_SixPFJet40_PFBTagCSV_1p5"},
    {"2018", "HLT_PFHT450_SixPFJet36_PFBTagDeepCSV_1p59"}};

static inline std::unordered_map<std::string, TString> FH_Trigger_DoubleBTag = {
    {"2016preVFP", "HLT_PFHT400_SixJet30_DoubleBTagCSV_p056"},
    {"2016postVFP", "HLT_PFHT400_SixJet30_DoubleBTagCSV_p056"},
    {"2017", "HLT_PFHT380_SixPFJet32_DoublePFBTagDeepCSV_2p2"},
    {"2018", "HLT_PFHT400_SixPFJet32_DoublePFBTagDeepCSV_2p94"},
    {"2022EE", "HLT_PFHT400_SixPFJet32_DoublePFBTagDeepJet_2p94"}};

static inline std::unordered_map<std::string, TString> FH_Control_Trigger = {
    {"2016preVFP", "HLT_PFHT450"},
    {"2016postVFP", "HLT_PFHT450"},
    {"2017", "HLT_PFHT430"},
    {"2018", "HLT_PFHT450"}
    };

static inline std::unordered_map<std::string, TString> FH_Control_Trigger_DoubleBTag = {
    {"2016preVFP", "HLT_PFHT400"},
    {"2016postVFP", "HLT_PFHT400"},
    {"2017", "HLT_PFHT380"},
    {"2018", "HLT_PFHT400"}};

static inline std::unordered_map<std::string, float> FH_HT_cut = {
    {"2016preVFP", 420.f},
    {"2016postVFP", 420.f},
    {"2017", 400.f},
    {"2018", 420.f},
    {"2022EE", 420.f}
};

static inline std::unordered_map<std::string, float> FH_Jet_Pt_cut = {
    {"2016preVFP", 32.f},
    {"2016postVFP", 32.f},
    {"2017", 34.f},
    {"2018", 34.f},
    {"2022EE", 34.f}
    };

static constexpr JetTagging::JetFlavTaggerWP FH_BTag_WP = JetTagging::JetFlavTaggerWP::Medium;

// SL part
static inline std::unordered_map<std::string, int> category_for_training_SL = {
    {"Vcb", 0},
    {"TT", 1},
    {"TT_tthf", 2},
    {"Others", 3}};
static inline std::unordered_map<std::string, TString> Mu_Trigger = {
    {"2016preVFP", "HLT_IsoMu24"},
    {"2016postVFP", "HLT_IsoMu24"},
    {"2017", "HLT_IsoMu27"},
    {"2018", "HLT_IsoMu24"},
    {"2022EE", "HLT_IsoMu24"}};

static inline std::unordered_map<std::string, TString> Mu_Trigger_SF_Key = {
    {"2016preVFP", "NUM_IsoMu24_or_IsoTkMu24_DEN_CutBasedIdTight_and_PFIsoTight"},
    {"2016postVFP", "NUM_IsoMu24_or_IsoTkMu24_DEN_CutBasedIdTight_and_PFIsoTight"},
    {"2017", "NUM_IsoMu27_DEN_CutBasedIdTight_and_PFIsoTight"},
    {"2018", "NUM_IsoMu24_DEN_CutBasedIdTight_and_PFIsoTight"},
    {"2022EE", "NUM_IsoMu24_DEN_CutBasedIdTight_and_PFIsoTight"}};

static inline std::unordered_map<std::string, TString> Mu_ID_SF_Key = {
    {"2016preVFP", "NUM_TightID_DEN_TrackerMuons"},
    {"2016postVFP", "NUM_TightID_DEN_TrackerMuons"},
    {"2017", "NUM_TightID_DEN_TrackerMuons"},
    {"2018", "NUM_TightID_DEN_TrackerMuons"},
    {"2022EE", "NUM_TightID_DEN_TrackerMuons"}};

static inline std::unordered_map<std::string, TString> Mu_Iso_SF_Key = {
    {"2016preVFP", "NUM_TightRelIso_DEN_TightIDandIPCut"},
    {"2016postVFP", "NUM_TightRelIso_DEN_TightIDandIPCut"},
    {"2017", "NUM_TightRelIso_DEN_TightIDandIPCut"},
    {"2018", "NUM_TightRelIso_DEN_TightIDandIPCut"},
    {"2022EE", "NUM_TightPFIso_DEN_TightID"}};

static inline std::unordered_map<std::string, TString> El_Trigger = {
    {"2016preVFP", "HLT_Ele32_WPTight_Gsf"},
    {"2016postVFP", "HLT_Ele32_WPTight_Gsf"},
    {"2017", "HLT_Ele35_WPTight_Gsf"},
    {"2018", "HLT_Ele35_WPTight_Gsf"},
    {"2022EE", "HLT_Ele30_WPTight_Gsf"}};

static inline std::unordered_map<std::string, TString> El_Trigger_SF_Key = {
    {"2016preVFP", "NUM_Ele27_WPTight_Gsf_DEN_CutBasedIdTight_and_PFIsoTight"},
    {"2016postVFP", "NUM_Ele27_WPTight_Gsf_DEN_CutBasedIdTight_and_PFIsoTight"},
    {"2017", "NUM_Ele32_WPTight_Gsf_DEN_CutBasedIdTight_and_PFIsoTight"},
    {"2018", "NUM_Ele32_WPTight_Gsf_DEN_CutBasedIdTight_and_PFIsoTight"},
    {"2022EE", "HLT_SF_Ele30_MVAiso80ID"}};

static inline std::unordered_map<std::string, TString> El_ID_SF_Key = {
    {"2016preVFP", "wp80iso"},
    {"2016postVFP", "wp80iso"},
    {"2017", "wp80iso"},
    {"2018", "wp80iso"},
    {"2022EE", "wp80iso"}};

static constexpr JetTagging::JetFlavTaggerWP SL_BTag_WP = JetTagging::JetFlavTaggerWP::Medium;

static constexpr float SL_Jet_Pt_cut = 30.f;

// DL part
static constexpr JetTagging::JetFlavTaggerWP DL_BTag_WP = JetTagging::JetFlavTaggerWP::Medium;

static constexpr float DL_Jet_Pt_cut = 30.f;

static inline std::unordered_map<std::string, std::string> Sample_Shorthand = {
    {"TTJJ_powheg", "TTJJ"},
    {"TTJJ_Vcb_powheg", "TTJJ_Vcb"},
    {"TTJJ_powheg_mTop171p5", "TTJJ"},
    {"TTJJ_powheg_mTop173p5", "TTJJ"},
    {"TTJJ_powheg_hDampUp", "TTJJ"},
    {"TTJJ_powheg_hDampDown", "TTJJ"},
    {"TTJJ_powheg_CP5Up", "TTJJ"},
    {"TTJJ_powheg_CP5Down", "TTJJ"},
    {"TTLJ_powheg", "TTLJ"},
    {"TTLJ_Vcb_powheg", "TTLJ_Vcb"},
    {"TTLJ_powheg_mTop171p5", "TTLJ"},
    {"TTLJ_powheg_mTop173p5", "TTLJ"},
    {"TTLJ_powheg_hDampUp", "TTLJ"},
    {"TTLJ_powheg_hDampDown", "TTLJ"},
    {"TTLJ_powheg_CP5Up", "TTLJ"},
    {"TTLJ_powheg_CP5Down", "TTLJ"},
    {"TTLL_powheg", "TTLL"},
    {"TTLL_Vcb_powheg", "TTLL_Vcb"},
    {"TTLL_powheg_mTop171p5", "TTLL"},
    {"TTLL_powheg_mTop173p5", "TTLL"},
    {"TTLL_powheg_hDampUp", "TTLL"},
    {"TTLL_powheg_hDampDown", "TTLL"},
    {"TTLL_powheg_CP5Up", "TTLL"},
    {"TTLL_powheg_CP5Down", "TTLL"},
    {"DYJets", "VJets"},
    {"WJets", "VJets"},
    {"ST_tW_top_Semilep", "ST"},
    {"ST_tW_antitop_Semilep", "ST"},
    {"ST_tW_top_Lep", "ST"},
    {"ST_tW_antitop_Lep", "ST"},
    {"ST_tW_top_Fullyhad", "ST"},
    {"ST_tW_antitop_Fullyhad", "ST"},
    {"ST_tch_top", "ST"},
    {"ST_tch_antitop", "ST"},
    {"ST_sch_antitop_Lep", "ST"},
    {"ST_sch_top_Lep", "ST"},
    {"ttHtobb_M125", "ttV"},
    {"ttHtoNonbb_M125", "ttV"},
    {"ttWtoQQ", "ttV"},
    {"ttZtoQQ", "ttV"},
    {"WWtoLNuQQ", "VV"},
    {"WWto2L2Nu", "VV"},
    {"WWtoLNu2Q", "VV"},
    {"WWto4Q", "VV"},
    {"WZto3LNu", "VV"},
    {"WZto2L2Q", "VV"},
    {"WZtoLNu2Q", "VV"},
    {"ZZto4L", "VV"},
    {"ZZto2Nu2Q", "VV"},
    {"ZZto2L2Nu", "VV"},
    {"ZZto2L2Q", "VV"},
    {"QCDB_HT40to100", "QCDB"},
    {"QCDB_HT100to200", "QCDB"},
    {"QCDB_HT200to400", "QCDB"},
    {"QCDB_HT400to600", "QCDB"},
    {"QCDB_HT600to800", "QCDB"},
    {"QCDB_HT800to1000", "QCDB"},
    {"QCDB_HT1000to1500", "QCDB"},
    {"QCDB_HT1500to2000", "QCDB"},
    {"QCD_HT70to100", "QCD"},
    {"QCD_HT100to200", "QCD"},
    {"QCD_HT200to400", "QCD"},
    {"QCD_HT400to600", "QCD"},
    {"QCD_HT600to800", "QCD"},
    {"QCD_HT800to1000", "QCD"},
    {"QCD_HT1000to1200", "QCD"},
    {"QCD_HT1200to1500", "QCD"},
    {"QCD_HT1500to2000", "QCD"},
};

#endif