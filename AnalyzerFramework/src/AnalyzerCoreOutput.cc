#include <AnalyzerFramework/AnalyzerCore.h>
#include <Compression.h>
#include <ROOT/RNTupleWriteOptions.hxx>
#include <ROOT/RNTupleWriter.hxx>
#include <ROOT/RNTupleReader.hxx>
#include <ROOT/RDataFrame.hxx>
#include <ROOT/RSnapshotOptions.hxx>
#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <functional>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

class RNTupleOutputState {
public:
  struct FieldBinding {
    std::type_index type;
    const void *address;
    std::function<void(ROOT::Detail::RRawPtrWriteEntry &)> bind;
  };

  RNTupleOutputState(AnalyzerCore::RNTupleOutputProfile profile_,
                     unsigned int compressionThreads_)
      : model(ROOT::RNTupleModel::CreateBare()), profile(profile_),
        compressionThreads(compressionThreads_) {}

  std::unique_ptr<ROOT::RNTupleModel> model;
  std::unique_ptr<ROOT::Detail::RRawPtrWriteEntry> entry;
  std::unique_ptr<ROOT::RNTupleWriter> writer;
  std::unordered_map<std::string, FieldBinding> fields;
  AnalyzerCore::RNTupleOutputProfile profile;
  unsigned int compressionThreads;
  std::uint64_t entries = 0;
};

namespace {

bool outputPathsConflict(std::string_view first, std::string_view second) {
  if (first == second)
    return true;
  const auto isParent = [](std::string_view parent, std::string_view child) {
    return child.size() > parent.size() &&
           child.compare(0, parent.size(), parent) == 0 &&
           child[parent.size()] == '/';
  };
  return isParent(first, second) || isParent(second, first);
}

std::pair<TDirectory *, std::string>
resolveRNTupleDirectory(TFile &file, const std::string &path) {
  const auto slash = path.find_last_of('/');
  if (slash == std::string::npos)
    return {&file, path};
  const std::string directoryName = path.substr(0, slash);
  const std::string objectName = path.substr(slash + 1);
  TDirectory *directory = file.GetDirectory(directoryName.c_str());
  if (!directory)
    directory = file.mkdir(directoryName.c_str());
  if (!directory)
    throw SKNano::LogicError("[AnalyzerCore::BookRNTuple] cannot create "
                             "output directory '" +
                             directoryName + "'");
  return {directory, objectName};
}

void ensureRNTupleWriter(RNTupleOutputState &state, const std::string &path,
                         TFile *outfile) {
  if (state.writer)
    return;
  if (!outfile || !outfile->IsOpen())
    throw SKNano::LogicError(
        "[AnalyzerCore::BookRNTuple] output file is not open");
  state.model->Freeze();
  state.entry = state.model->CreateRawPtrWriteEntry();
  for (const auto &[unused, field] : state.fields) {
    static_cast<void>(unused);
    field.bind(*state.entry);
  }

  ROOT::RNTupleWriteOptions options;
  // Level 1 keeps the fast LZ4 path; levels above 1 switch ROOT to LZ4HC,
  // which showed up at ~2% of the event loop in perf for little size gain.
  options.SetCompression(ROOT::RCompressionSetting::EAlgorithm::kLZ4, 1);
  const std::size_t bufferSize =
      state.profile == AnalyzerCore::RNTupleOutputProfile::Fast
          ? 64U * 1024U * 1024U
          : 4U * 1024U * 1024U;
  options.SetApproxZippedClusterSize(bufferSize);
  options.SetPageBufferBudget(bufferSize);
  options.SetMaxUnzippedClusterSize(4U * bufferSize);
  options.SetUseBufferedWrite(true);
  options.SetEnablePageChecksums(true);
  options.SetUseImplicitMT(
      state.compressionThreads > 1
          ? ROOT::RNTupleWriteOptions::EImplicitMT::kOn
          : ROOT::RNTupleWriteOptions::EImplicitMT::kOff);

  auto [directory, name] = resolveRNTupleDirectory(*outfile, path);
  state.writer = ROOT::RNTupleWriter::Append(
      std::move(state.model), name, *directory, options);
}

} // namespace

void AnalyzerCore::SetOutfilePath(const TString &outpath) {
  if (outfile)
    throw SKNano::LogicError(
        "[AnalyzerCore::SetOutfilePath] output file is already configured");
  outputFinalPath_ = outpath.Data();
  if (outputFinalPath_.empty())
    throw SKNano::ConfigError(
        "[AnalyzerCore::SetOutfilePath] output path is empty");
  outputPartialPath_ = outputFinalPath_ + ".partial";
  outfile = new TFile(outputPartialPath_.c_str(), "RECREATE");
  if (!outfile || outfile->IsZombie())
    throw SKNano::LogicError(
        "[AnalyzerCore::SetOutfilePath] cannot create partial output '" +
        outputPartialPath_ + "'");
  SetErrorReportPath(outputFinalPath_ + ".errors.jsonl");
  SetPerformanceReportPath(outputFinalPath_ + ".performance.json");
}

void AnalyzerCore::SetOutputThreads(unsigned int threads) {
  if (threads == 0)
    throw SKNano::ConfigError(
        "[AnalyzerCore::SetOutputThreads] thread count must be positive");
  if (!rntupleOutputs_.empty())
    throw SKNano::LogicError(
        "[AnalyzerCore::SetOutputThreads] configure threads before outputs");
  outputCompressionThreads_ = threads;
  if (threads > 1)
    ROOT::EnableImplicitMT(threads);
}

void AnalyzerCore::FillHist(std::string_view histname, float value,
                            float weight, int n_bin, float x_min, float x_max) {
  BookHist1D(histname, n_bin, x_min, x_max).Fill(value, weight);
}

void AnalyzerCore::FillHist(std::string_view histname, float value,
                            float weight, int n_bin, float *xbins) {
  BookHist1D(histname, n_bin, xbins).Fill(value, weight);
}

void AnalyzerCore::FillHist(std::string_view histname, float value_x,
                            float value_y, float weight, int n_binx,
                            float x_min, float x_max, int n_biny, float y_min,
                            float y_max) {
  BookHist2D(histname, n_binx, x_min, x_max, n_biny, y_min, y_max)
      .Fill(value_x, value_y, weight);
}

void AnalyzerCore::FillHist(std::string_view histname, float value_x,
                            float value_y, float weight, int n_binx,
                            float *xbins, int n_biny, float *ybins) {
  BookHist2D(histname, n_binx, xbins, n_biny, ybins)
      .Fill(value_x, value_y, weight);
}

void AnalyzerCore::FillHist(std::string_view histname, float value_x,
                            float value_y, float value_z, float weight,
                            int n_binx, float x_min, float x_max, int n_biny,
                            float y_min, float y_max, int n_binz, float z_min,
                            float z_max) {
  BookHist3D(histname, n_binx, x_min, x_max, n_biny, y_min, y_max, n_binz,
             z_min, z_max)
      .Fill(value_x, value_y, value_z, weight);
}

void AnalyzerCore::FillHist(std::string_view histname, float value_x,
                            float value_y, float value_z, float weight,
                            int n_binx, float *xbins, int n_biny, float *ybins,
                            int n_binz, float *zbins) {
  BookHist3D(histname, n_binx, xbins, n_biny, ybins, n_binz, zbins)
      .Fill(value_x, value_y, value_z, weight);
}

AnalyzerCore::RNTupleHandle
AnalyzerCore::BookRNTuple(const TString &ntupleName,
                          RNTupleOutputProfile profile) {
  const std::string name(ntupleName.Data());
  auto found = rntupleOutputs_.find(name);
  if (found == rntupleOutputs_.end()) {
    ValidateTreePath(name);
    rntupleOutputs_.emplace(
        name, std::make_shared<RNTupleOutputState>(
                  profile, outputCompressionThreads_));
  } else if (found->second->profile != profile) {
    throw SKNano::ConfigError(
        "[AnalyzerCore::BookRNTuple] conflicting output profile for '" +
        name + "'");
  }
  return RNTupleHandle(this, name);
}

AnalyzerCore::RNTupleHandle
AnalyzerCore::OutputRNTuple(const TString &ntupleName) {
  const std::string name(ntupleName.Data());
  if (rntupleOutputs_.find(name) == rntupleOutputs_.end())
    throw SKNano::LogicError("[AnalyzerCore::OutputRNTuple] RNTuple " + name +
                             " not found");
  return RNTupleHandle(this, name);
}

AnalyzerCore::RNTupleHandle
AnalyzerCore::OutputRegistry::Book(std::string_view name,
                                   RNTupleOutputProfile profile) const {
  return owner_->BookRNTuple(std::string(name), profile);
}

AnalyzerCore::RNTupleHandle
AnalyzerCore::OutputRegistry::Get(std::string_view name) const {
  return owner_->OutputRNTuple(std::string(name));
}

void AnalyzerCore::RNTupleHandle::RequireValid() const {
  if (!owner_)
    throw SKNano::LogicError("[RNTupleHandle] empty handle access");
}

void AnalyzerCore::RNTupleHandle::Fill() const {
  RequireValid();
  owner_->FillRNTuple(ntupleName_);
}

std::uint64_t AnalyzerCore::RNTupleHandle::GetEntries() const {
  RequireValid();
  return owner_->GetRNTupleEntries(ntupleName_);
}

AnalyzerCore::RNTupleHandle &
AnalyzerCore::RNTupleHandle::Set(const TString &name, float value) {
  RequireValid();
  owner_->SetRNTupleValue(ntupleName_, name, value);
  return *this;
}

AnalyzerCore::RNTupleHandle &
AnalyzerCore::RNTupleHandle::Set(const TString &name, double value) {
  RequireValid();
  owner_->SetRNTupleValue(ntupleName_, name, value);
  return *this;
}

AnalyzerCore::RNTupleHandle &
AnalyzerCore::RNTupleHandle::Set(const TString &name, int value) {
  RequireValid();
  owner_->SetRNTupleValue(ntupleName_, name, value);
  return *this;
}

AnalyzerCore::RNTupleHandle &
AnalyzerCore::RNTupleHandle::Set(const TString &name, bool value) {
  RequireValid();
  owner_->SetRNTupleValue(ntupleName_, name, value);
  return *this;
}

void AnalyzerCore::SetRNTupleValue(const TString &ntupleName,
                                    const TString &fieldName, float value) {
  const std::string key = std::string(ntupleName.Data()) + "/" +
                          std::string(fieldName.Data());
  auto [slot, inserted] = scalar_float_storage.try_emplace(key);
  if (inserted) {
    slot->second = std::make_unique<float>();
    OutputRNTuple(ntupleName).Field(fieldName.Data(), *slot->second);
  }
  *slot->second = value;
}

void AnalyzerCore::SetRNTupleValue(const TString &ntupleName,
                                    const TString &fieldName, double value) {
  SetRNTupleValue(ntupleName, fieldName, static_cast<float>(value));
}

void AnalyzerCore::SetRNTupleValue(const TString &ntupleName,
                                    const TString &fieldName, int value) {
  const std::string key = std::string(ntupleName.Data()) + "/" +
                          std::string(fieldName.Data());
  auto [slot, inserted] = scalar_int_storage.try_emplace(key);
  if (inserted) {
    slot->second = std::make_unique<int>();
    OutputRNTuple(ntupleName).Field(fieldName.Data(), *slot->second);
  }
  *slot->second = value;
}

void AnalyzerCore::SetRNTupleValue(const TString &ntupleName,
                                    const TString &fieldName, bool value) {
  const std::string key = std::string(ntupleName.Data()) + "/" +
                          std::string(fieldName.Data());
  auto [slot, inserted] = scalar_bool_storage.try_emplace(key);
  if (inserted) {
    slot->second = std::make_unique<bool>();
    OutputRNTuple(ntupleName).Field(fieldName.Data(), *slot->second);
  }
  *slot->second = value;
}

void AnalyzerCore::RegisterRNTupleField(
    const std::string &ntupleName, const std::string &fieldName,
    std::type_index type, const void *address,
    std::function<void(ROOT::RNTupleModel &)> addField,
    std::function<void(ROOT::Detail::RRawPtrWriteEntry &)> bindField) {
  auto output = rntupleOutputs_.find(ntupleName);
  if (output == rntupleOutputs_.end())
    throw SKNano::LogicError("[AnalyzerCore::RNTupleHandle] RNTuple " +
                             ntupleName + " not found");
  if (fieldName.empty())
    throw SKNano::ConfigError(
        "[AnalyzerCore::RNTupleHandle] field name is empty");

  auto &state = *output->second;
  const auto found = state.fields.find(fieldName);
  if (found != state.fields.end()) {
    if (found->second.type != type || found->second.address != address)
      throw SKNano::ConfigError(
          "[RNTupleHandle::Field] incompatible duplicate field '" +
          ntupleName + "/" + fieldName + "'");
    return;
  }
  if (state.writer)
    throw SKNano::LogicError(
        "[RNTupleHandle::Field] cannot add field after the first Fill for '" +
        ntupleName + "'");

  addField(*state.model);
  state.fields.emplace(fieldName, RNTupleOutputState::FieldBinding{
                                      type, address, std::move(bindField)});
}

void AnalyzerCore::FillRNTuple(const std::string &ntupleName) {
  auto output = rntupleOutputs_.find(ntupleName);
  if (output == rntupleOutputs_.end())
    throw SKNano::LogicError("[AnalyzerCore::FillRNTuple] RNTuple " +
                             ntupleName + " not found");
  auto &state = *output->second;
  ensureRNTupleWriter(state, ntupleName, outfile);
  state.writer->Fill(*state.entry);
  ++state.entries;
}

std::uint64_t
AnalyzerCore::GetRNTupleEntries(const std::string &ntupleName) const {
  const auto output = rntupleOutputs_.find(ntupleName);
  if (output == rntupleOutputs_.end())
    throw SKNano::LogicError("[AnalyzerCore::GetRNTupleEntries] RNTuple " +
                             ntupleName + " not found");
  return output->second->entries;
}

bool AnalyzerCore::HasRNTupleOutput(std::string_view name) const noexcept {
  return rntupleOutputs_.find(std::string(name)) != rntupleOutputs_.end();
}

void AnalyzerCore::SnapshotSelectedInput(std::vector<Long64_t> entries,
                                         std::string outputName) {
  if (inputFiles.empty())
    throw SKNano::LogicError(
        "[AnalyzerCore::SnapshotSelectedInput] no input files configured");
  if (outputName.empty())
    throw SKNano::ConfigError(
        "[AnalyzerCore::SnapshotSelectedInput] output name is empty");
  if (!selectedInputOutputName_.empty())
    throw SKNano::LogicError(
        "[AnalyzerCore::SnapshotSelectedInput] a skim is already queued");
  ValidateTreePath(outputName);
  std::sort(entries.begin(), entries.end());
  entries.erase(std::unique(entries.begin(), entries.end()), entries.end());
  selectedInputEntries_ = std::move(entries);
  selectedInputOutputName_ = std::move(outputName);
}

void AnalyzerCore::FinalizeRNTuples() {
  for (auto &[name, output] : rntupleOutputs_) {
    // Dynamically named category outputs acquire their schema on first use.
    // Do not persist a zero-column placeholder: another shard may contain the
    // real schema for the same category, which would make the files
    // unmergeable.
    if (output->fields.empty())
      continue;
    ensureRNTupleWriter(*output, name, outfile);
    std::cout << "[AnalyzerCore::WriteHist] Writing RNTuple: " << name
              << " (" << output->entries << " entries)" << std::endl;
    output->writer.reset();
    output->entry.reset();
  }
}

AnalyzerCore::HistogramGroup AnalyzerCore::Hists(std::string_view prefix) {
  std::string normalized(prefix);
  while (!normalized.empty() && normalized.front() == '/')
    normalized.erase(normalized.begin());
  while (!normalized.empty() && normalized.back() == '/')
    normalized.pop_back();
  return HistogramGroup(this, std::move(normalized));
}

void AnalyzerCore::HistogramGroup::RequireValid() const {
  if (!owner_)
    throw SKNano::LogicError("[HistogramGroup] empty group access");
}

std::string
AnalyzerCore::HistogramGroup::Resolve(std::string_view name) const {
  RequireValid();
  if (name.empty())
    throw SKNano::ConfigError("[HistogramGroup] histogram name is empty");
  std::string relative(name);
  while (!relative.empty() && relative.front() == '/')
    relative.erase(relative.begin());
  return prefix_.empty() ? relative : prefix_ + "/" + relative;
}

AnalyzerCore::HistogramGroup
AnalyzerCore::HistogramGroup::Group(std::string_view child) const {
  return HistogramGroup(owner_, Resolve(child));
}

Hist1DHandle AnalyzerCore::HistogramGroup::Book1D(
    std::string_view name, int bins, double minimum, double maximum) const {
  return owner_->BookHist1D(Resolve(name), bins, minimum, maximum);
}

Hist1DHandle AnalyzerCore::HistogramGroup::Book1D(
    std::string_view name, int bins, const float *edges) const {
  return owner_->BookHist1D(Resolve(name), bins, edges);
}

Hist2DHandle AnalyzerCore::HistogramGroup::Book2D(
    std::string_view name, int binsX, double minX, double maxX, int binsY,
    double minY, double maxY) const {
  return owner_->BookHist2D(Resolve(name), binsX, minX, maxX, binsY, minY,
                            maxY);
}

Hist2DHandle AnalyzerCore::HistogramGroup::Book2D(
    std::string_view name, int binsX, const float *edgesX, int binsY,
    const float *edgesY) const {
  return owner_->BookHist2D(Resolve(name), binsX, edgesX, binsY, edgesY);
}

Hist3DHandle AnalyzerCore::HistogramGroup::Book3D(
    std::string_view name, int binsX, double minX, double maxX, int binsY,
    double minY, double maxY, int binsZ, double minZ, double maxZ) const {
  return owner_->BookHist3D(Resolve(name), binsX, minX, maxX, binsY, minY,
                            maxY, binsZ, minZ, maxZ);
}

Hist3DHandle AnalyzerCore::HistogramGroup::Book3D(
    std::string_view name, int binsX, const float *edgesX, int binsY,
    const float *edgesY, int binsZ, const float *edgesZ) const {
  return owner_->BookHist3D(Resolve(name), binsX, edgesX, binsY, edgesY,
                            binsZ, edgesZ);
}

void AnalyzerCore::HistogramGroup::Fill(std::string_view name, float value,
                                        float weight, int bins, float minimum,
                                        float maximum) const {
  Book1D(name, bins, minimum, maximum).Fill(value, weight);
}

void AnalyzerCore::HistogramGroup::Fill(std::string_view name, float value,
                                        float weight, int bins,
                                        float *edges) const {
  Book1D(name, bins, edges).Fill(value, weight);
}

void AnalyzerCore::HistogramGroup::Fill(
    std::string_view name, float x, float y, float weight, int binsX,
    float minX, float maxX, int binsY, float minY, float maxY) const {
  Book2D(name, binsX, minX, maxX, binsY, minY, maxY).Fill(x, y, weight);
}

void AnalyzerCore::HistogramGroup::Fill(std::string_view name, float x,
                                        float y, float weight, int binsX,
                                        float *edgesX, int binsY,
                                        float *edgesY) const {
  Book2D(name, binsX, edgesX, binsY, edgesY).Fill(x, y, weight);
}

void AnalyzerCore::HistogramGroup::Fill(
    std::string_view name, float x, float y, float z, float weight, int binsX,
    float minX, float maxX, int binsY, float minY, float maxY, int binsZ,
    float minZ, float maxZ) const {
  Book3D(name, binsX, minX, maxX, binsY, minY, maxY, binsZ, minZ, maxZ)
      .Fill(x, y, z, weight);
}

void AnalyzerCore::HistogramGroup::Fill(
    std::string_view name, float x, float y, float z, float weight, int binsX,
    float *edgesX, int binsY, float *edgesY, int binsZ, float *edgesZ) const {
  Book3D(name, binsX, edgesX, binsY, edgesY, binsZ, edgesZ)
      .Fill(x, y, z, weight);
}

void AnalyzerCore::ValidateTreePath(std::string_view treeName) const {
  if (treeName.empty())
    throw SKNano::ConfigError("[AnalyzerCore::BookRNTuple] name is empty");
  for (const auto &[otherNtuple, unused] : rntupleOutputs_) {
    static_cast<void>(unused);
    if (outputPathsConflict(treeName, otherNtuple))
      throw SKNano::ConfigError(
          "[AnalyzerCore::BookRNTuple] output path conflicts with RNTuple '" +
          otherNtuple + "'");
  }
  const auto checkHistograms = [&](const auto &histograms) {
    for (const auto &[histogram, unused] : histograms) {
      static_cast<void>(unused);
      if (outputPathsConflict(treeName, histogram))
        throw SKNano::ConfigError(
            "[AnalyzerCore::BookRNTuple] output path conflicts with histogram '" +
            histogram + "'");
    }
  };
  checkHistograms(histmap1d);
  checkHistograms(histmap2d);
  checkHistograms(histmap3d);
}

void AnalyzerCore::ValidateHistogramPath(
    std::string_view histogramName) const {
  if (histogramName.empty())
    throw SKNano::ConfigError(
        "[AnalyzerCore::BookHist] histogram name is empty");
  for (const auto &[ntupleName, unused] : rntupleOutputs_) {
    static_cast<void>(unused);
    if (outputPathsConflict(histogramName, ntupleName))
      throw SKNano::ConfigError(
          "[AnalyzerCore::BookHist] output path conflicts with RNTuple '" +
          ntupleName + "'");
  }
}

void AnalyzerCore::WriteHist() {
  if (outputFinalized_)
    throw SKNano::LogicError(
        "[AnalyzerCore::WriteHist] output was already finalized");
  if (!outfile || !outfile->IsOpen())
    throw SKNano::LogicError(
        "[AnalyzerCore::WriteHist] output file is not configured");
  const int compression_level = 1;
  const int compression_algorithm = ROOT::RCompressionSetting::EAlgorithm::kLZ4;
  cout << "[AnalyzerCore::WriteHist] Writing histograms to "
       << outfile->GetName() << endl;
  cout << "[AnalyzerCore::WriteHist] Set compression algorithm to LZ4 with "
          "level "
       << compression_level << endl;
  outfile->SetCompressionAlgorithm(compression_algorithm);
  outfile->SetCompressionLevel(compression_level);
  FinalizeRNTuples();
  std::vector<std::pair<std::string, TH1 *>> sorted_histograms1d(
      histmap1d.begin(), histmap1d.end());
  std::vector<std::pair<std::string, TH2 *>> sorted_histograms2d(
      histmap2d.begin(), histmap2d.end());
  std::vector<std::pair<std::string, TH3 *>> sorted_histograms3d(
      histmap3d.begin(), histmap3d.end());
  std::sort(
      sorted_histograms1d.begin(), sorted_histograms1d.end(),
      [](const std::pair<std::string, TH1 *> &a,
         const std::pair<std::string, TH1 *> &b) { return a.first < b.first; });
  std::sort(
      sorted_histograms2d.begin(), sorted_histograms2d.end(),
      [](const std::pair<std::string, TH2 *> &a,
         const std::pair<std::string, TH2 *> &b) { return a.first < b.first; });
  std::sort(
      sorted_histograms3d.begin(), sorted_histograms3d.end(),
      [](const std::pair<std::string, TH3 *> &a,
         const std::pair<std::string, TH3 *> &b) { return a.first < b.first; });
  for (const auto &pair : sorted_histograms1d) {
    const string &histname = pair.first;
    TH1 *hist = pair.second;
    cout << "[AnalyzerCore::WriteHist] Writing 1D histogram: " << histname
         << endl;
    // Split the directory and name
    // e.g. "dir1/dir2/histname" -> "dir1/dir2", "histname"
    // e.g. "histname" -> "", "histname"
    size_t last_slash = histname.find_last_of('/');
    string this_prefix, this_name;
    last_slash == string::npos ? this_prefix = ""
                               : this_prefix = histname.substr(0, last_slash);
    last_slash == string::npos ? this_name = histname
                               : this_name = histname.substr(last_slash + 1);

    TDirectory *this_dir = outfile->GetDirectory(this_prefix.c_str());
    if (!this_dir)
      outfile->mkdir(this_prefix.c_str());
    outfile->cd(this_prefix.c_str());
    hist->Write(this_name.c_str());
  }
  for (const auto &pair : sorted_histograms2d) {
    const string &histname = pair.first;
    cout << "[AnalyzerCore::WriteHist] Writing 2D histogram: " << histname
         << endl;
    TH2 *hist = pair.second;
    // Split the directory and name
    // e.g. "dir1/dir2/histname" -> "dir1/dir2", "histname"
    // e.g. "histname" -> "", "histname"
    size_t last_slash = histname.find_last_of('/');
    string this_prefix, this_name;
    last_slash == string::npos ? this_prefix = ""
                               : this_prefix = histname.substr(0, last_slash);
    last_slash == string::npos ? this_name = histname
                               : this_name = histname.substr(last_slash + 1);

    TDirectory *this_dir = outfile->GetDirectory(this_prefix.c_str());
    if (!this_dir)
      outfile->mkdir(this_prefix.c_str());
    outfile->cd(this_prefix.c_str());
    hist->Write(this_name.c_str());
  }
  for (const auto &pair : sorted_histograms3d) {
    const string &histname = pair.first;
    cout << "[AnalyzerCore::WriteHist] Writing 3D histogram: " << histname
         << endl;
    TH3 *hist = pair.second;
    // Split the directory and name
    // e.g. "dir1/dir2/histname" -> "dir1/dir2", "histname"
    // e.g. "histname" -> "", "histname"
    size_t last_slash = histname.find_last_of('/');
    string this_prefix, this_name;
    last_slash == string::npos ? this_prefix = ""
                               : this_prefix = histname.substr(0, last_slash);
    last_slash == string::npos ? this_name = histname
                               : this_name = histname.substr(last_slash + 1);

    TDirectory *this_dir = outfile->GetDirectory(this_prefix.c_str());
    if (!this_dir)
      outfile->mkdir(this_prefix.c_str());
    outfile->cd(this_prefix.c_str());
    hist->Write(this_name.c_str());
  }
  cout << "[AnalyzerCore::WriteHist] Writing histograms done" << endl;
  outfile->Close();

  if (!selectedInputOutputName_.empty()) {
    auto selectedEntries =
        std::make_shared<const std::vector<Long64_t>>(selectedInputEntries_);
    ROOT::RDataFrame input(inputDatasetName, inputFiles);
    auto selected = input.Filter(
        [selectedEntries](ULong64_t entry) {
          return std::binary_search(selectedEntries->begin(),
                                    selectedEntries->end(),
                                    static_cast<Long64_t>(entry));
        },
        {"rdfentry_"});
    auto columns = input.GetColumnNames();
    columns.erase(std::remove_if(columns.begin(), columns.end(),
                                 [](const std::string &name) {
                                   return !name.empty() && name.front() == '#';
                                 }),
                  columns.end());

    ROOT::RDF::RSnapshotOptions options;
    options.fMode = "UPDATE";
    options.fOutputFormat = ROOT::RDF::ESnapshotOutputFormat::kRNTuple;
    options.fCompressionAlgorithm =
        ROOT::RCompressionSetting::EAlgorithm::kLZ4;
    options.fCompressionLevel = 1;
    options.fApproxZippedClusterSize = 64U * 1024U * 1024U;
    options.fMaxUnzippedClusterSize = 256U * 1024U * 1024U;
    options.fEnablePageChecksums = true;
    selected.Snapshot(selectedInputOutputName_, outputPartialPath_, columns,
                      options);
  }

  {
    std::unique_ptr<TFile> validation(
        TFile::Open(outputPartialPath_.c_str(), "READ"));
    if (!validation || validation->IsZombie())
      throw SKNano::LogicError(
          "[AnalyzerCore::WriteHist] cannot reopen partial output '" +
          outputPartialPath_ + "'");
    for (const auto &[name, output] : rntupleOutputs_) {
      if (output->fields.empty())
        continue;
      auto reader = ROOT::RNTupleReader::Open(name, outputPartialPath_);
      if (reader->GetNEntries() != output->entries)
        throw SKNano::LogicError(
            "[AnalyzerCore::WriteHist] RNTuple entry validation failed for '" +
            name + "'");
    }
    if (!selectedInputOutputName_.empty()) {
      auto reader = ROOT::RNTupleReader::Open(selectedInputOutputName_,
                                              outputPartialPath_);
      if (reader->GetNEntries() != selectedInputEntries_.size())
        throw SKNano::LogicError(
            "[AnalyzerCore::WriteHist] skim entry validation failed for '" +
            selectedInputOutputName_ + "'");
    }
  }

  if (std::rename(outputPartialPath_.c_str(), outputFinalPath_.c_str()) != 0)
    throw SKNano::LogicError(
        "[AnalyzerCore::WriteHist] cannot publish output '" +
        outputFinalPath_ + "': " + std::strerror(errno));
  outputFinalized_ = true;
  cout << "[AnalyzerCore::WriteHist] Published output to " << outputFinalPath_
       << endl;
}
