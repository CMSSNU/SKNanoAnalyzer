#ifndef SKNanoLoader_h
#define SKNanoLoader_h

#include <cstddef>
#include <iostream>
#include <cstdint>
#include <fstream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
using namespace std;

#include "TROOT.h"
#include "TFile.h"
#include "TString.h"
#include "TRandom3.h"
#include "ROOT/RVec.hxx"
#include "ROOT/RDataFrame.hxx"
#include <nlohmann/json.hpp>
#include "AnalysisException.h"
#include <AnalyzerFramework/BranchManager.h>
#include "CounterRNG.h"
#include "FailureContext.h"
#include "FailurePolicy.h"
#include "ExecutionPlan.h"
#include "EventArena.h"
#include "PerformanceTelemetry.h"
#include "TriggerDecision.h"
#include <AnalyzerFramework/Triggerinfo.h>
#include "ViewColumns.h"
using namespace ROOT::VecOps;

class SKNanoLoader : public SKNano::TriggerDecisionProvider
{
public:
    SKNanoLoader();
    virtual ~SKNanoLoader();

    virtual void SetRNTupleName(TString name);
    virtual int AddFile(TString filename);
    Long64_t GetInputEntries();
    void RegisterBranches();
    void ResetBranchStates();

    long MaxEvent, NSkipEvent;
    int LogEvery;
    bool IsDATA;
    TString DataStream;
    TString MCSample;
    TString Campaign;
    // bool IsFastSim;
    int DataYear;
    TString DataEra;
    TString DataPeriod;
    int Run;
    float xsec, sumW, sumSign;
    RVec<TString> Userflags;

    virtual void Init();
    virtual void Loop();
    virtual void executeEvent() {};
    virtual std::string CurrentSystematicName() const { return ""; }
    bool lookupTrigger(const std::string &name,
                       SKNano::TriggerDecision &decision) const override;
    std::uint64_t triggerEpoch() const noexcept override { return eventEpoch; }

    template <typename T>
    ColumnHandle<T> GetColumnHandle(
        const std::string &name,
        ColumnRequirement requirement = ColumnRequirement::Required) {
        return branchManager.getColumnHandle<T>(name, requirement);
    }

    template <typename T>
    ScalarHandle<T> GetScalarHandle(
        const std::string &name,
        ColumnRequirement requirement = ColumnRequirement::Required) {
        return branchManager.getScalarHandle<T>(name, requirement);
    }

    virtual void SetEra(TString era)
    {
        DataEra = era;
        DataYear = TString(era(0, 4)).Atoi();
        // 2016, 2017, 2018 are Run2
        if (DataYear == 2016 or DataYear == 2017 or DataYear == 2018)
            Run = 2;
        else if (DataYear == 2022 or DataYear == 2023 or DataYear == 2024 or DataYear == 2025)
            Run = 3;
    }
    virtual void SetPeriod(TString period) { DataPeriod=period; }
    virtual void SetCampaign(TString campaign) { Campaign=campaign; }

    void SetAnalyzerName(const std::string &name) { analyzerName = name; }
    void SetFailurePolicy(SKNano::FailurePolicy policy) { failurePolicy = policy; }
    void SetFailurePolicy(const std::string &policy) { failurePolicy = SKNano::ParseFailurePolicy(policy); }
    void SetMaxEventErrors(int maxErrors) { maxEventErrors = maxErrors; }
    void SetErrorReportPath(const std::string &path) { errorReportPath = path; }
    void EnablePerformanceTelemetry(bool enable = true) {
        performanceTelemetry.setEnabled(enable);
    }
    void SetPerformanceReportPath(const std::string &path) {
        performanceTelemetry.setOutputPath(path);
        performanceTelemetry.setEnabled(true);
    }
    SKNano::PerformanceTelemetry::ScopedPhase
    MeasurePerformancePhase(std::string_view name) {
        return performanceTelemetry.measure(name);
    }
    void AddPerformanceCounter(std::string_view name, double value = 1.) {
        performanceTelemetry.addCounter(name, value);
    }
    void SetExecutionPlan(SKNano::ExecutionPlan plan) {
        executionPlan = std::move(plan);
        hasExecutionPlan = true;
    }
    const SKNano::ExecutionPlan *GetExecutionPlan() const noexcept {
        return hasExecutionPlan ? &executionPlan : nullptr;
    }
    void SetRngMode(SKNano::RngMode mode) noexcept { rngMode = mode; }
    SKNano::RngMode GetRngMode() const noexcept { return rngMode; }
    void EnableEventArena(std::size_t initialBytes = 8 * 1024 * 1024) {
        eventArena = std::make_unique<SKNano::EventArena>(initialBytes);
    }
    bool HasEventArena() const noexcept { return static_cast<bool>(eventArena); }
    SKNano::EventArena &GetEventArena() {
        if (!eventArena)
            throw SKNano::LogicError(
                "[SKNanoLoader] EventArena was not enabled");
        return *eventArena;
    }

    virtual TString GetEra() const { return DataEra; }
    virtual TString GetCampaign() const { return Campaign; }
    virtual int GetYear() const { return DataYear; }
    Long64_t CurrentEntry() const { return currentEntry; }
    Long64_t CurrentLocalEntry() const { return currentLocalEntry; }

    BranchManager branchManager; //!
    TriggerMap_t TriggerMap;
    mutable SKNano::TriggerDecisionMap triggerDecisionCache; //!
    mutable Long64_t triggerDecisionCacheEntry = -1;
    Long64_t currentEntry = -1;
    Long64_t currentLocalEntry = -1;
    Long64_t currentFileGlobalBegin = -1;
    Long64_t currentFileGlobalEnd = -1;
    std::uint64_t eventEpoch = 0;
    int currentFileNumber = -1;
    std::string analyzerName = "SKNanoLoader";
    SKNano::FailurePolicy failurePolicy = SKNano::FailurePolicy::FailFast;
    int maxEventErrors = 1;
    int eventErrorCount = 0;
    std::string errorReportPath;
    std::unordered_map<std::string, int> errorCountsByCategory;
    SKNano::PerformanceTelemetry performanceTelemetry; //!
    SKNano::ExecutionPlan executionPlan; //!
    bool hasExecutionPlan = false;
    SKNano::RngMode rngMode = SKNano::RngMode::StrictLegacy;
    std::unique_ptr<SKNano::EventArena> eventArena; //!

    std::string inputDatasetName = "Events";
    std::vector<std::string> inputFiles;
    struct RNTupleFileRange {
        std::string fileName;
        Long64_t begin = 0;
        Long64_t end = 0;
    };
    std::vector<RNTupleFileRange> rntupleFileRanges; //!
    std::unique_ptr<SKNano::RNTupleSource> rntupleSource; //!
    std::size_t currentRNTupleFileIndex =
        std::numeric_limits<std::size_t>::max();
    Long64_t rntupleTotalEntries = -1;
    bool rntupleClusterCache = true;
    // ROOT's own RNTuple metrics wrap every page read in RNTupleTimer, which
    // costs two clock() syscalls per read.  The framework's phase telemetry is
    // independent of it, so this only turns on for an explicit
    // SKNANO_PERFORMANCE_REPORT request.
    bool rntupleMetrics = false;
    // Page-cache read-ahead window in clusters for the input files (see
    // RNTupleSource::open). 0 disables it. Overridden by SKNANO_INPUT_PREFETCH.
    unsigned rntuplePrefetchClusters = 2;
    SKNano::RNTupleReadStats rntupleReadStats; //!

    Long64_t performanceEventsProcessed = 0;

protected:
    bool PrepareEntry(Long64_t globalEntry);
    void PrepareRNTupleFiles();
    void OpenRNTupleFile(std::size_t index);
    void ValidateExecutionPlanRNTuple() const;
    SKNano::FailureContext BuildFailureContext() const;
    void RecordFailure(const SKNano::FailureContext &context,
                       const std::string &category,
                       const std::string &message,
                       const std::string &exceptionType);
    void WriteFailureSummary() const;
    void WritePerformanceSummary();
    void AccumulateRNTupleReadStats();

#include <generated_loader_api.inc>
};

#endif
