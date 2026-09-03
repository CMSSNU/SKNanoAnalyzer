#include <AnalyzerFramework/SKNanoLoader.h>
#include <algorithm>
#include <cstdlib>
#include <exception>
#include <iomanip>
using json = nlohmann::json;

namespace {

std::string summaryPathFor(const std::string &jsonlPath)
{
    if (jsonlPath.size() >= 6 &&
        jsonlPath.compare(jsonlPath.size() - 6, 6, ".jsonl") == 0)
        return jsonlPath.substr(0, jsonlPath.size() - 6) + ".summary.json";
    return jsonlPath + ".summary.json";
}

template <typename BranchT, typename ValueT>
void tryReadBranch(const BranchT &branch, ValueT &value)
{
    try
    {
        if (branch.valid())
            value = static_cast<ValueT>(branch);
    }
    catch (...)
    {
    }
}

} // namespace

SKNanoLoader::SKNanoLoader()
{
    MaxEvent = -1;
    NSkipEvent = 0;
    LogEvery = 1000;
    IsDATA = false;
    DataStream = "";
    MCSample = "";
    SetEra("2018");
    xsec = 1.;
    sumW = 1.;
    sumSign = 1.;
    Userflags.clear();
}

SKNanoLoader::~SKNanoLoader()
{
    // RNTuple column views must be released while their reader is still alive.
    branchManager.clear();
}

void SKNanoLoader::SetRNTupleName(TString name)
{
    if (!inputFiles.empty())
        throw SKNano::ConfigError(
            "[SKNanoLoader] SetRNTupleName() must precede AddFile()");
    inputDatasetName = name.Data();
}

int SKNanoLoader::AddFile(TString filename)
{
    const std::string path = filename.Data();
    if (path.empty())
        return 0;
    inputFiles.push_back(path);
    rntupleFileRanges.clear();
    rntupleTotalEntries = -1;
    return 1;
}

void SKNanoLoader::PrepareRNTupleFiles()
{
    if (rntupleTotalEntries >= 0)
        return;
    rntupleFileRanges.clear();
    Long64_t offset = 0;
    for (const auto &fileName : inputFiles) {
        SKNano::RNTupleSource probe;
        probe.open(inputDatasetName, fileName, false, false);
        const auto entries = probe.entries();
        if (entries > static_cast<std::uint64_t>(
                          std::numeric_limits<Long64_t>::max() - offset))
            throw SKNano::ConfigError(
                "[SKNanoLoader] RNTuple chain entry count overflows Long64_t");
        const Long64_t end = offset + static_cast<Long64_t>(entries);
        rntupleFileRanges.push_back({fileName, offset, end});
        offset = end;
    }
    rntupleTotalEntries = offset;
}

Long64_t SKNanoLoader::GetInputEntries()
{
    PrepareRNTupleFiles();
    return rntupleTotalEntries;
}

void SKNanoLoader::OpenRNTupleFile(std::size_t index)
{
    if (index >= rntupleFileRanges.size())
        throw SKNano::LogicError("[SKNanoLoader] invalid RNTuple file index");
    auto next = std::make_unique<SKNano::RNTupleSource>();
    // The branches activated so far are the columns worth reading ahead; on
    // the first file nothing is active yet and the whole cluster span is read.
    next->open(inputDatasetName, rntupleFileRanges[index].fileName,
               rntupleMetrics, rntupleClusterCache, rntuplePrefetchClusters,
               &branchManager.getActiveBranches());
    // attachRNTuple clears views into the previous reader before the old
    // RNTupleSource is destroyed by the unique_ptr move below.
    branchManager.attachRNTuple(next.get());
    AccumulateRNTupleReadStats();
    rntupleSource = std::move(next);
    currentRNTupleFileIndex = index;
    currentFileNumber = static_cast<int>(index);
    currentFileGlobalBegin = rntupleFileRanges[index].begin;
    currentFileGlobalEnd = rntupleFileRanges[index].end;
    ResetBranchStates();
    ValidateExecutionPlanRNTuple();
    performanceTelemetry.addCounter("rntuple_file_opens");
}

bool SKNanoLoader::PrepareEntry(Long64_t globalEntry)
{
    PrepareRNTupleFiles();
    if (globalEntry < 0 || globalEntry >= rntupleTotalEntries)
        return false;
    if (currentRNTupleFileIndex >= rntupleFileRanges.size() ||
        globalEntry < currentFileGlobalBegin ||
        globalEntry >= currentFileGlobalEnd) {
        const auto it = std::upper_bound(
            rntupleFileRanges.begin(), rntupleFileRanges.end(), globalEntry,
            [](Long64_t entry, const RNTupleFileRange &range) {
                return entry < range.end;
            });
        if (it == rntupleFileRanges.end())
            return false;
        OpenRNTupleFile(static_cast<std::size_t>(
            std::distance(rntupleFileRanges.begin(), it)));
    }
    currentEntry = globalEntry;
    currentLocalEntry = globalEntry - currentFileGlobalBegin;
    ++eventEpoch;
    rntupleSource->noteEntry(static_cast<std::uint64_t>(currentLocalEntry),
                             &branchManager.getActiveBranches());
    performanceTelemetry.addCounter("rntuple_local_entry_advances");
    return true;
}

void SKNanoLoader::ValidateExecutionPlanRNTuple() const
{
    if (!hasExecutionPlan || !rntupleSource)
        return;
    const std::string context = rntupleSource->fileName() + ":" +
                                inputDatasetName;
    for (const auto &column : executionPlan.columns()) {
        if (!rntupleSource->hasField(column.name)) {
            if (column.requirement == SKNano::PlanRequirement::Required)
                throw SKNano::ConfigError(
                    "[ExecutionPlan] required RNTuple field '" + column.name +
                    "' is missing in " + context);
            continue;
        }
        const std::string type = rntupleSource->fieldType(column.name);
        const bool vector = type.find("RVec<") != std::string::npos ||
                            type.find("vector<") != std::string::npos;
        if ((column.cardinality == SKNano::PlanCardinality::Scalar && vector) ||
            (column.cardinality == SKNano::PlanCardinality::Vector && !vector))
            throw SKNano::ConfigError(
                "[ExecutionPlan] RNTuple field '" + column.name +
                "' has incompatible cardinality in " + context);
    }
}

void SKNanoLoader::Loop()
{
    if (GetInputEntries() <= 0)
        return;

    Long64_t totalEntries = GetInputEntries();
    Long64_t targetEntries = totalEntries;
    if (MaxEvent > 0)
        targetEntries = std::min(targetEntries, static_cast<Long64_t>(MaxEvent));

    Long64_t skipEntries = std::clamp<Long64_t>(NSkipEvent, 0, targetEntries);
    Long64_t entriesToProcess = targetEntries - skipEntries;

    if (entriesToProcess <= 0)
    {
        cout << "[SKNanoLoader::Loop] Nothing to process after applying skip/max settings" << endl;
        return;
    }

    auto startTime = std::chrono::steady_clock::now();
    cout << "[SKNanoLoader::Loop] Event Loop Started" << endl;
    rntupleReadStats = SKNano::RNTupleReadStats{};
    performanceEventsProcessed = 0;
    performanceTelemetry.setMetadata("analyzer", analyzerName);
    performanceTelemetry.setMetadata("era", DataEra.Data());
    performanceTelemetry.startRun();

    currentFileNumber = -1;
    currentEntry = -1;
    currentLocalEntry = -1;
    currentFileGlobalBegin = -1;
    currentFileGlobalEnd = -1;
    currentRNTupleFileIndex = std::numeric_limits<std::size_t>::max();
    branchManager.bindEntrySource(&currentLocalEntry);
    branchManager.bindEpochSource(&eventEpoch);

    Long64_t processed = 0;

    int next_log_percent = 0;
    const int log_step_percent = 5;                        // 5% 진행될 때마다 로깅
    auto last_log_time = startTime;
    const auto log_step_time = std::chrono::seconds(60);   // 또는 60초가 지날 때마다 로깅

    for (Long64_t globalEntry = 0; globalEntry < targetEntries; ++globalEntry)
    {
        {
            auto headerPhase = performanceTelemetry.measure("header_io");
            if (!PrepareEntry(globalEntry))
                break;
        }

        if (globalEntry < skipEntries)
            continue;

        int current_percent = (entriesToProcess > 0) ? (processed * 100) / entriesToProcess : 100;

        if (current_percent >= next_log_percent || (processed % 1000 == 0))
        {
            auto currentTime = std::chrono::steady_clock::now();
            
            if (current_percent >= next_log_percent || (currentTime - last_log_time) >= log_step_time)
            {
                std::chrono::duration<double> elapsedTime = currentTime - startTime;
                double timePerEvent = processed > 0 ? elapsedTime.count() / static_cast<double>(processed) : 0.0;
                Long64_t remainingEvents = std::max(entriesToProcess - processed, 0LL);
                double estimatedRemaining = remainingEvents * timePerEvent;

                cout << "[SKNanoLoader::Loop] Progress: " << std::setw(3) << current_percent << "% "
                     << "(" << (skipEntries + processed) << " / " << targetEntries << ") "
                     << "| Elapsed: " << std::fixed << std::setprecision(1) << elapsedTime.count() << "s "
                     << "| ETA: " << std::fixed << std::setprecision(1) << estimatedRemaining << "s" << endl;

                next_log_percent = ((current_percent / log_step_percent) + 1) * log_step_percent;
                last_log_time = currentTime; // 마지막 로그 시간 갱신
            }
        }

        bool skipCurrentEvent = false;
        try
        {
            if (eventArena)
                eventArena->reset();
            auto eventPhase = performanceTelemetry.measure("event_total");
            executeEvent();
        }
        catch (const SKNano::AnalysisException &e)
        {
            const auto context = BuildFailureContext();
            RecordFailure(context, SKNano::ErrorCategoryName(e.category()),
                          e.what(), "SKNano::AnalysisException");
            const bool tooManyErrors =
                maxEventErrors >= 0 && eventErrorCount > maxEventErrors;
            if (failurePolicy == SKNano::FailurePolicy::FailFast ||
                !e.eventLocal() || tooManyErrors)
            {
                WriteFailureSummary();
                WritePerformanceSummary();
                throw;
            }
            skipCurrentEvent = true;
        }
        catch (const std::exception &e)
        {
            const auto context = BuildFailureContext();
            RecordFailure(context, "Unknown", e.what(), "std::exception");
            WriteFailureSummary();
            WritePerformanceSummary();
            throw;
        }
        catch (...)
        {
            const auto context = BuildFailureContext();
            RecordFailure(context, "Unknown", "non-std exception", "unknown");
            WriteFailureSummary();
            WritePerformanceSummary();
            throw;
        }
        ++processed;
        performanceEventsProcessed = processed;
        if (processed >= entriesToProcess)
            break;
        if (skipCurrentEvent)
            continue;
    }

    auto endTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsedTime = endTime - startTime;
    
    cout << "[SKNanoLoader::Loop] Progress: 100% (" << targetEntries << " / " << targetEntries << ") "
         << "| Event Loop Finished in " << std::fixed << std::setprecision(2) << elapsedTime.count() << "s" << endl;
    WriteFailureSummary();
    WritePerformanceSummary();
}

void SKNanoLoader::WritePerformanceSummary()
{
    if (!performanceTelemetry.enabled())
        return;
    performanceTelemetry.setCounter("events_processed", performanceEventsProcessed);
    performanceTelemetry.setCounter("event_errors", eventErrorCount);
    performanceTelemetry.setCounter("active_branches", branchManager.getActiveBranches().size());
    performanceTelemetry.setMetadata("input_format", "rntuple");
    // RNTuple reads bypass TFile, so the byte accounting comes from the
    // page source (ROOT metrics, when enabled) and the read-ahead thread.
    SKNano::RNTupleReadStats total = rntupleReadStats;
    if (rntupleSource)
        total += rntupleSource->readStats();
    performanceTelemetry.setCounter("rntuple_read_payload_bytes",
                                    static_cast<double>(total.readPayloadBytes));
    performanceTelemetry.setCounter("rntuple_read_overhead_bytes",
                                    static_cast<double>(total.readOverheadBytes));
    performanceTelemetry.setCounter("rntuple_read_calls",
                                    static_cast<double>(total.readCalls));
    performanceTelemetry.setCounter("rntuple_readv_calls",
                                    static_cast<double>(total.readVCalls));
    performanceTelemetry.setCounter("rntuple_unzip_bytes",
                                    static_cast<double>(total.unzipBytes));
    performanceTelemetry.setCounter("rntuple_read_wall_seconds",
                                    total.readWallSeconds);
    performanceTelemetry.setCounter("rntuple_unzip_wall_seconds",
                                    total.unzipWallSeconds);
    performanceTelemetry.setCounter("prefetch_bytes",
                                    static_cast<double>(total.prefetchBytes));
    performanceTelemetry.setCounter("prefetch_ranges",
                                    static_cast<double>(total.prefetchRanges));
    performanceTelemetry.setCounter("prefetch_clusters",
                                    static_cast<double>(total.prefetchClusters));
    performanceTelemetry.setCounter("prefetch_window_clusters",
                                    rntuplePrefetchClusters);
    performanceTelemetry.setCounter("input_files", inputFiles.size());
    performanceTelemetry.writeJson();
}

void SKNanoLoader::AccumulateRNTupleReadStats()
{
    if (rntupleSource)
        rntupleReadStats += rntupleSource->readStats();
}

void SKNanoLoader::Init()
{
    cout << "[SKNanoLoader::Init] Initializing. Era = " << DataEra << " Run =  " << Run << endl;
    if (const char *report = std::getenv("SKNANO_PERFORMANCE_REPORT")) {
        SetPerformanceReportPath(report);
        rntupleMetrics = true;
    }
    // SKNANO_INPUT_PREFETCH=0 turns the input read-ahead off, 1 keeps the
    // default window, N>1 sets the window in clusters.
    if (const char *prefetch = std::getenv("SKNANO_INPUT_PREFETCH")) {
        const long value = std::strtol(prefetch, nullptr, 10);
        if (value <= 0)
            rntuplePrefetchClusters = 0;
        else if (value > 1)
            rntuplePrefetchClusters = static_cast<unsigned>(value);
        cout << "[SKNanoLoader::Init] input prefetch window = "
             << rntuplePrefetchClusters << " cluster(s)" << endl;
    }
    if (GetInputEntries() == 0)
    {
        cout << "[SKNanoLoader::Init] No entries in the RNTuple" << endl;
        throw SKNano::ConfigError(
            "[SKNanoLoader::Init] no entries in the RNTuple");
    }

    branchManager.clear();
    branchManager.bindEntrySource(&currentLocalEntry);
    branchManager.bindEpochSource(&eventEpoch);
    RegisterBranches();
    ResetBranchStates();

    // Attach the first concrete source before analyzer initialization so
    // typed required/optional handles validate the physical schema up front.
    PrepareRNTupleFiles();
    if (rntupleFileRanges.empty())
        throw SKNano::ConfigError(
            "[SKNanoLoader::Init] no RNTuple input files");
    OpenRNTupleFile(0);
    ResetBranchStates();

    TriggerMap.clear();

    string json_path = string(getenv("SKNANO_DATA")) + "/" + DataEra.Data() + "/Trigger/HLT_Path.json";
    ifstream json_file(json_path);
    if (json_file.is_open())
    {
        cout << "[SKNanoLoader::Init] Loading HLT Paths in " << json_path << endl;
        json j;
        json_file >> j;
        for (auto &[key, value] : j.items())
        {
            auto info = std::make_unique<TriggerInfo>();
            info->lumi = value["lumi"];

            if (key == "Full")
            {
                info->alwaysTrue = true;
                TriggerMap.emplace(key, std::move(info));
                continue;
            }

            if (!branchManager.available(key))
            {
                cout << "[SKNanoLoader::Init] " << key
                     << " field not in RNTuple - skipped" << endl;
                continue;
            }

            info->hlt = &branchManager.getOrCreateScalar<Bool_t>(key);
            TriggerMap.emplace(key, std::move(info));
        }
    }
    else
    {
        cerr << "[SKNanoLoader::Init] Cannot open " << json_path << endl;
    }
}

bool SKNanoLoader::lookupTrigger(const std::string &name,
                                  SKNano::TriggerDecision &decision) const
{
    if (triggerDecisionCacheEntry != currentEntry) {
        triggerDecisionCache.clear();
        triggerDecisionCacheEntry = currentEntry;
    }
    const auto cached = triggerDecisionCache.find(name);
    if (cached != triggerDecisionCache.end()) {
        decision = cached->second;
        return true;
    }

    const auto it = TriggerMap.find(TString(name));
    if (it == TriggerMap.end() || !it->second)
        return false;

    const TriggerInfo &info = *it->second;
    decision.lumi = info.lumi;
    decision.pass = info.alwaysTrue ||
                    (info.hlt && info.hlt->valid() &&
                     static_cast<bool>(*info.hlt));
    triggerDecisionCache.emplace(name, decision);
    return true;
}


void SKNanoLoader::RegisterBranches()
{
#include <generated_branch_register.inc>
}

void SKNanoLoader::ResetBranchStates()
{
    branchManager.resetAll();
#include <generated_branch_reset.inc>
}

SKNano::FailureContext SKNanoLoader::BuildFailureContext() const
{
    SKNano::FailureContext context;
    context.entry = currentEntry;
    context.localEntry = currentLocalEntry;
    context.treeNumber = currentFileNumber;
    context.analyzer = analyzerName;
    context.sample = MCSample.Data();
    context.dataStream = DataStream.Data();
    context.era = DataEra.Data();
    context.period = DataPeriod.Data();
    context.campaign = Campaign.Data();
    context.systematic = CurrentSystematicName();

    if (rntupleSource) {
        context.inputFile = rntupleSource->fileName();
    }

    tryReadBranch(RunNumber, context.run);
    tryReadBranch(luminosityBlock, context.lumi);
    tryReadBranch(event, context.event);
    return context;
}

void SKNanoLoader::RecordFailure(const SKNano::FailureContext &context,
                                 const std::string &category,
                                 const std::string &message,
                                 const std::string &exceptionType)
{
    ++eventErrorCount;
    ++errorCountsByCategory[category];

    json record = {
        {"category", category},
        {"exception_type", exceptionType},
        {"message", message},
        {"entry", context.entry},
        {"local_entry", context.localEntry},
        {"tree_number", context.treeNumber},
        {"input_file", context.inputFile},
        {"run", context.run},
        {"lumi", context.lumi},
        {"event", context.event},
        {"analyzer", context.analyzer},
        {"sample", context.sample},
        {"data_stream", context.dataStream},
        {"era", context.era},
        {"period", context.period},
        {"campaign", context.campaign},
        {"systematic", context.systematic},
        {"failure_policy", SKNano::FailurePolicyName(failurePolicy)},
        {"event_error_count", eventErrorCount},
        {"max_event_errors", maxEventErrors},
    };

    cerr << "[SKNanoLoader::Loop] Event failure: " << record.dump() << endl;

    if (errorReportPath.empty())
        return;
    std::ofstream out(errorReportPath, std::ios::app);
    if (!out)
    {
        cerr << "[SKNanoLoader::RecordFailure] Cannot open error report "
             << errorReportPath << endl;
        return;
    }
    out << record.dump() << '\n';
}

void SKNanoLoader::WriteFailureSummary() const
{
    if (errorReportPath.empty())
        return;

    json byCategory = json::object();
    for (const auto &kv : errorCountsByCategory)
        byCategory[kv.first] = kv.second;

    json summary = {
        {"failure_policy", SKNano::FailurePolicyName(failurePolicy)},
        {"event_error_count", eventErrorCount},
        {"max_event_errors", maxEventErrors},
        {"errors_by_category", byCategory},
        {"analyzer", analyzerName},
        {"sample", MCSample.Data()},
        {"data_stream", DataStream.Data()},
        {"era", DataEra.Data()},
        {"period", DataPeriod.Data()},
        {"campaign", Campaign.Data()},
    };

    const std::string summaryPath = summaryPathFor(errorReportPath);
    std::ofstream out(summaryPath);
    if (!out)
    {
        cerr << "[SKNanoLoader::WriteFailureSummary] Cannot open error summary "
             << summaryPath << endl;
        return;
    }
    out << summary.dump(2) << '\n';
}
