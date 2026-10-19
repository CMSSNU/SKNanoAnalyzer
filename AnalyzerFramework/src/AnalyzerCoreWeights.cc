#include <AnalyzerFramework/AnalyzerCore.h>
#include "JetView.h"
#include <algorithm>
#include <cmath>
#include <execution>
#include <fstream>
#include <numeric>
#include <stdexcept>
#include <tuple>

void AnalyzerCore::load_modelling_json(const TString &filename) {
  using json = nlohmann::json;

  std::cout << "Loading modelling json file: " << filename.Data() << std::endl;
  std::ifstream file_stream(filename.Data());
  if (!file_stream.is_open()) {
    throw std::runtime_error("Could not open modelling json file: " +
                             std::string(filename.Data()));
  }

  json j;
  file_stream >> j;
  file_stream.close();

  modelling_json = j;

  if (!j.contains("PD")) {
    throw std::runtime_error("modelling_json does not contain 'PD' field");
  }
  std::string pd_key = j["PD"].get<std::string>();

  if (!j.contains("subprocesses")) {
    throw std::runtime_error(
        "modelling_json does not contain 'subprocesses' field");
  }
  const auto &subprocs = j["subprocesses"];

  modelling_patches.clear();

  for (auto it = subprocs.begin(); it != subprocs.end(); ++it) {
    const std::string sub_name = it.key();
    const auto &sub = it.value();

    ModellingPatch mp;

    if (sub.contains("patch_ScaleVariation") &&
        sub["patch_ScaleVariation"].is_array()) {
      for (const auto &v : sub["patch_ScaleVariation"]) {
        if (v.is_null())
          continue;
        mp.patch_ScaleVariation.push_back(static_cast<float>(v.get<double>()));
      }
    }

    if (sub.contains("patch_PSVariation") &&
        sub["patch_PSVariation"].is_array()) {
      for (const auto &v : sub["patch_PSVariation"]) {
        if (v.is_null())
          continue;
        mp.patch_PSVariation.push_back(static_cast<float>(v.get<double>()));
      }
    }

    if (sub.contains("patch_hdamp_up") && !sub["patch_hdamp_up"].is_null())
      mp.patch_hdamp_up = sub["patch_hdamp_up"].get<float>();
    else
      mp.patch_hdamp_up = 0.0f;

    if (sub.contains("patch_hdamp_down") && !sub["patch_hdamp_down"].is_null())
      mp.patch_hdamp_down = sub["patch_hdamp_down"].get<float>();
    else
      mp.patch_hdamp_down = 0.0f;

    if (sub.contains("patch_minnlo") && !sub["patch_minnlo"].is_null())
      mp.patch_minnlo = sub["patch_minnlo"].get<float>();
    else
      mp.patch_minnlo = 0.0f;

    modelling_patches.emplace(sub_name, std::move(mp));
  }

  std::cout << "[modelling] PD = " << pd_key << std::endl;
  std::cout << "[modelling] loaded subprocesses: ";
  for (const auto &kv : modelling_patches) {
    std::cout << kv.first << " ";
  }
  std::cout << std::endl;
}

// https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETOptionalFiltersRun2
bool AnalyzerCore::PassNoiseFilter(const JetViewCollection &Alljets,
                                   const Event &ev, Event::MET_Type met_type) {
  return PassNoiseFilterCommon(Alljets, ev, met_type);
}

bool AnalyzerCore::PassMetFilter(const JetViewCollection &AllJets,
                                 const Event &ev, Event::MET_Type met_type) {
  return PassNoiseFilter(AllJets, ev, met_type);
}

float AnalyzerCore::GetScaleVariation(const MyCorrection::variation &muF_syst,
                                      const MyCorrection::variation &muR_syst) {
  if (nLHEScaleWeight == 0)
    return 1.;
  // (nom, nom) is the central value by definition; it needs no slot, and the
  // 8-entry layout does not have one.
  if (muF_syst == MyCorrection::variation::nom &&
      muR_syst == MyCorrection::variation::nom)
    return 1.f;

  // LHEScaleWeight is a 3x3 grid over (muR, muF) in {0.5, 1, 2} with muR as the
  // outer index: slot = 3 * iR + iF. Producers ship it either complete (9
  // entries) or with the (1, 1) nominal at slot 4 dropped (8 entries), and
  // which one you get is a per-sample property -- every 2024 NanoAODv15 sample
  // checked carries 9. Assuming 8 unconditionally silently returns the nominal
  // for (muF up, muR nom) and reads a neighbouring scale pair for the three
  // combinations above it, so the muF-up nuisance collapses to zero while
  // muR-up quietly mixes in a muF shift.
  const auto axisIndex = [](const MyCorrection::variation &v) -> int {
    return v == MyCorrection::variation::down
               ? 0
               : (v == MyCorrection::variation::nom ? 1 : 2);
  };
  int slot = 3 * axisIndex(muR_syst) + axisIndex(muF_syst);
  if (nLHEScaleWeight == 8 && slot > 4)
    slot -= 1; // nominal slot absent, everything above it shifts down

  if (slot < 0 || slot >= static_cast<int>(nLHEScaleWeight))
    throw SKNano::LogicError(
        "[AnalyzerCore::GetScaleVariation] slot " + std::to_string(slot) +
        " is out of range for nLHEScaleWeight = " +
        std::to_string(nLHEScaleWeight));
  return LHEScaleWeight[slot];
}

float AnalyzerCore::GetPSWeight(const MyCorrection::variation &ISR_syst,
                                const MyCorrection::variation &FSR_syst) {
  //[0] is ISR=2 FSR=1; [1] is ISR=1 FSR=2[2] is ISR=0.5 FSR=1; [3] is ISR=1
  // FSR=0.5;
  if (nPSWeight == 1)
    return 1.;
  if (ISR_syst == MyCorrection::variation::up &&
      FSR_syst == MyCorrection::variation::nom)
    return PSWeight[0];
  else if (ISR_syst == MyCorrection::variation::nom &&
           FSR_syst == MyCorrection::variation::up)
    return PSWeight[1];
  else if (ISR_syst == MyCorrection::variation::down &&
           FSR_syst == MyCorrection::variation::nom)
    return PSWeight[2];
  else if (ISR_syst == MyCorrection::variation::nom &&
           FSR_syst == MyCorrection::variation::down)
    return PSWeight[3];
  else {
    throw SKNano::LogicError(
        "[AnalyzerCore::GetPSWeight] requested variation is not implemented");
  }
}

float AnalyzerCore::GetHT(const JetViewCollection &jets,
                          const std::vector<std::size_t> &indices,
                          const MyCorrection::variation &jesVar,
                          const MyCorrection::variation &jerVar) const {
  if (jesVar != MyCorrection::variation::nom &&
      jerVar != MyCorrection::variation::nom) {
    throw runtime_error("[AnalyzerCore::GetHT] Both JES and JER are "
                        "non-nominal, not supported");
  }

  float ht = 0.f;
  for (const auto idx : indices) {
    if (idx >= jets.size()) {
      throw runtime_error("[AnalyzerCore::GetHT] Index out of range: " +
                          to_string(idx) + " >= " + to_string(jets.size()));
    }

    const auto &jet = jets[idx];
    if (!jet.valid())
      continue;

    if (IsDATA) {
      ht += jet.CorrectedPt();
    } else if (jesVar == MyCorrection::variation::up) {
      ht += jet.JesPtUp();
    } else if (jesVar == MyCorrection::variation::down) {
      ht += jet.JesPtDown();
    } else if (jerVar == MyCorrection::variation::up) {
      ht += jet.SmearedPtUp();
    } else if (jerVar == MyCorrection::variation::down) {
      ht += jet.SmearedPtDown();
    } else {
      ht += jet.SmearedPtNominal();
    }
  }
  return ht;
}

float AnalyzerCore::GetHT(const SelectedJetViewCollection &jets) const {
  float ht = 0.f;
  for (const auto jet : jets)
    ht += jet.Pt();
  return ht;
}

void AnalyzerCore::METType1Propagation(Particle &MET,
                                       RVec<Particle> &original_objects,
                                       RVec<Particle> &corrected_objects) {
  const auto particle_reducer = [](Particle lhs, const Particle &rhs) {
    lhs += rhs;
    return lhs;
  };

  const Particle original_sum =
      std::reduce(std::execution::unseq, original_objects.begin(),
                  original_objects.end(), Particle(), particle_reducer);

  const Particle corrected_sum =
      std::reduce(std::execution::unseq, corrected_objects.begin(),
                  corrected_objects.end(), Particle(), particle_reducer);

  MET += (original_sum - corrected_sum);
}

float AnalyzerCore::GetL1PrefireWeight(MyCorrection::variation syst) {
  if (Run == 3)
    return 1.;
  if (IsDATA)
    return 1.;
  float weight = 1.;
  throw std::logic_error(
      "[TODO] I cannot find L1Prefiring weight in the v15, please update");
  // switch(syst){
  //     case MyCorrection::variation::nom:
  //         weight = L1PreFiringWeight_Nom;
  //         break;
  //     case MyCorrection::variation::up:
  //         weight = L1PreFiringWeight_Up;
  //         break;
  //     case MyCorrection::variation::down:
  //         weight = L1PreFiringWeight_Dn;
  //         break;
  //     default:
  //         exit(ENODATA);
  // }
  return weight;
}
// pdfs
float AnalyzerCore::GetPDFWeight(LHAPDF::PDF *pdf) {
  float pdf1 = pdf->xfxQ2(Generator_id1, Generator_x1, Generator_scalePDF);
  float pdf2 = pdf->xfxQ2(Generator_id2, Generator_x2, Generator_scalePDF);
  return pdf1 * pdf2;
}

float AnalyzerCore::GetPDFReweight() {
  return GetPDFWeight(pdfReweight->NewPDF) / GetPDFWeight(pdfReweight->ProdPDF);
}

float AnalyzerCore::GetPDFReweight(int member) {
  return GetPDFWeight(pdfReweight->PDFErrorSet.at(member)) /
         GetPDFWeight(pdfReweight->ProdPDF);
}

// MC weights
// TODO: Treat MiNNLO and Sherpa weights
float AnalyzerCore::MCweight(bool usesign, bool norm_1invpb) const {
  if (IsDATA)
    return 1.;
  float weight = genWeight;
  if (usesign) {
    if (weight > 0)
      weight = 1.0;
    else if (weight < 0)
      weight = -1.0;
    else
      weight = 0.0;
  }

  if (norm_1invpb) {
    if (usesign)
      weight *= xsec / sumSign;
    else
      weight *= xsec / sumW;
  }
  return weight;
}
