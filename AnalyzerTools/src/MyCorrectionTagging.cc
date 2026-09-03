#include "MyCorrection.h"
#include <algorithm>
#include <cmath>
#include <execution>
#include <numeric>
#include <unordered_set>
#include <vector>

// Heavy flavor tagging
void MyCorrection::SetTaggingParam(JetTagging::JetFlavTagger tagger,
                                   JetTagging::JetFlavTaggerWP wp) {
  global_tagger = tagger;
  global_wp = wp;
  global_taggerStr = JetTagging::GetTaggerCorrectionLibStr(tagger);
  global_wpStr = JetTagging::GetTaggerCorrectionWPStr(wp);
  global_wpValuesKey = global_taggerStr + "_wp_values";
}

float MyCorrection::GetBTaggingWP() const {
  try {
    const auto &cset =
        cachedBTaggingWP.get(cset_btagging, global_wpValuesKey.c_str());
    return safeEvaluate(cset, "GetBTaggingWP", {global_wpStr});
  } catch (const exception &e) {
    cerr << "[MyCorrection::GetBTaggingWP] Failed to evaluate WP '"
         << global_wpStr << "' for tagger '" << global_taggerStr << "'" << endl;
    throw;
  }
}

float MyCorrection::GetBTaggingWP(JetTagging::JetFlavTagger tagger,
                                  JetTagging::JetFlavTaggerWP wp) const {
  // Convert enumerations to strings
  const string this_taggerStr =
      JetTagging::GetTaggerCorrectionLibStr(tagger).Data();
  const string this_wpStr = JetTagging::GetTaggerCorrectionWPStr(wp).Data();

  try {
    correction::Correction::Ref cset =
        cset_btagging->at(this_taggerStr + "_wp_values");
    return safeEvaluate(cset, "GetBTaggingWP", {this_wpStr});
  } catch (const exception &e) {
    cerr << "[MyCorrection::GetBTaggingWP] Failed to evaluate WP '" << this_wpStr
         << "' for tagger '" << this_taggerStr << "'" << endl;
    throw;
  }
}

float MyCorrection::GetBTaggingEff(const float eta, const float pt,
                                   const int flav,
                                   JetTagging::JetFlavTagger tagger,
                                   JetTagging::JetFlavTaggerWP wp,
                                   const variation syst) {
  const string this_taggerStr =
      JetTagging::GetTaggerCorrectionLibStr(tagger).Data();
  const string this_wpStr = JetTagging::GetTaggerCorrectionWPStr(wp).Data();
  auto cset = cset_btagging_eff->at(this_taggerStr);
  return safeEvaluate(cset, "GetBTaggingSF",
                      {getSystString_BTV(syst), this_wpStr, flav, fabs(eta),
                       pt});
}

float MyCorrection::GetBTaggingSF(const float eta, const float pt,
                                  const int flav,
                                  JetTagging::JetFlavTagger tagger,
                                  JetTagging::JetFlavTaggerWP wp,
                                  const variation syst,
                                  const TString &method) const {
  const string this_taggerStr =
      JetTagging::GetTaggerCorrectionLibStr(tagger).Data();
  const string this_wpStr = JetTagging::GetTaggerCorrectionWPStr(wp).Data();
  // BTV splits the fixed-WP measurement by flavour: heavy flavour is measured
  // in ttbar+mujets ("comb"), light in a negative-tag sample ("light"), and
  // the two live in separate corrections with the same signature.
  const string suffix =
      method.Length() > 0 ? string(method.Data()) : (flav == 0 ? "light" : "comb");
  try {
    correction::Correction::Ref cset =
        cset_btagging->at(this_taggerStr + "_" + suffix);
    return safeEvaluate(cset, "GetBTaggingSF",
                        {getSystString_BTV(syst), this_wpStr, flav, fabs(eta),
                         pt});
  } catch (const exception &e) {
    cerr << "[MyCorrection::GetBTaggingSF] Failed to evaluate '"
         << this_taggerStr << "_" << suffix << "' wp '" << this_wpStr
         << "' flav " << flav << endl;
    throw;
  }
}

pair<float, float> MyCorrection::GetCTaggingWP() const {
  try {
    const auto &cset =
        cachedCTaggingWP.get(cset_ctagging, global_wpValuesKey.c_str());
    const float valCvB =
        safeEvaluate(cset, "GetCTaggingWP", {global_wpStr, "CvB"});
    const float valCvL =
        safeEvaluate(cset, "GetCTaggingWP", {global_wpStr, "CvL"});
    return make_pair(valCvB, valCvL);
  } catch (const exception &e) {
    cerr << "[MyCorrection::GetCTaggingWP] Failed to evaluate WP '"
         << global_wpStr << "' for tagger '" << global_taggerStr << "'" << endl;
    throw;
  }
}

pair<float, float>
MyCorrection::GetCTaggingWP(JetTagging::JetFlavTagger tagger,
                            JetTagging::JetFlavTaggerWP wp) const {
  const string this_taggerStr =
      JetTagging::GetTaggerCorrectionLibStr(tagger).Data();
  const string this_wpStr = JetTagging::GetTaggerCorrectionWPStr(wp).Data();

  try {
    correction::Correction::Ref cset =
        cset_ctagging->at(this_taggerStr + "_wp_values");
    const float valCvB = safeEvaluate(cset, "GetCTaggingWP", {this_wpStr, "CvB"});
    const float valCvL = safeEvaluate(cset, "GetCTaggingWP", {this_wpStr, "CvL"});
    return make_pair(valCvB, valCvL);
  } catch (const exception &e) {
    cerr << "[MyCorrection::GetCTaggingWP] WP '" << this_wpStr
         << "' not found for tagger '" << this_taggerStr << "'" << endl;
    throw;
  }
}

float MyCorrection::GetCTaggingEff(const float eta, const float pt,
                                   const int flav,
                                   JetTagging::JetFlavTagger tagger,
                                   JetTagging::JetFlavTaggerWP wp,
                                   const variation syst) {
  // No c-tagging efficiency map is produced for the supported eras yet; the
  // body that used to follow this return was dead and read the *b*-tagging
  // file.  Restore it together with the ctagging_eff entry in the era yml.
  static_cast<void>(eta);
  static_cast<void>(pt);
  static_cast<void>(flav);
  static_cast<void>(tagger);
  static_cast<void>(wp);
  static_cast<void>(syst);
  return 1.;
}

float MyCorrection::GetCTaggingR(const float nTrueInt, const float HT,
                                 const JetTagging::JetFlavTagger tagger,
                                 const TString &processName,
                                 const TString &ttBarCategory,
                                 const TString &syst_str) const {
  string this_taggerStr = JetTagging::GetTaggerCorrectionLibStr(tagger).Data();
  if (processName != "")
    this_taggerStr += "_" + processName;
  else
    this_taggerStr += (string("_") + Sample.Data());
  auto cset = cset_ctagging_R->at(this_taggerStr);
  return safeEvaluate(cset, "GetTopPtReweight",
                      {syst_str.Data(), ttBarCategory.Data(), nTrueInt, HT});
}
