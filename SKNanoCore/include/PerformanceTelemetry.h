#ifndef SKNanoCore_PerformanceTelemetry_h
#define SKNanoCore_PerformanceTelemetry_h

#include <chrono>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#if defined(__unix__) || defined(__APPLE__)
#include <sys/resource.h>
#endif

namespace SKNano {

class PerformanceTelemetry {
public:
    struct PhaseStats {
        double seconds = 0.;
        std::uint64_t calls = 0;
    };

    class ScopedPhase {
    public:
        ScopedPhase() = default;
        explicit ScopedPhase(PhaseStats *stats_)
            : stats(stats_), start(std::chrono::steady_clock::now()) {}
        ScopedPhase(const ScopedPhase &) = delete;
        ScopedPhase &operator=(const ScopedPhase &) = delete;
        ScopedPhase(ScopedPhase &&other) noexcept
            : stats(other.stats), start(other.start) {
            other.stats = nullptr;
        }
        ScopedPhase &operator=(ScopedPhase &&other) noexcept {
            if (this != &other) {
                finish();
                stats = other.stats;
                start = other.start;
                other.stats = nullptr;
            }
            return *this;
        }
        ~ScopedPhase() { finish(); }

    private:
        void finish() {
            if (!stats)
                return;
            stats->seconds += std::chrono::duration<double>(
                std::chrono::steady_clock::now() - start).count();
            ++stats->calls;
            stats = nullptr;
        }
        // Points into phases_; std::map nodes never move, and startRun()
        // resets the values in place rather than clearing the map, so a slot
        // stays valid for the lifetime of the telemetry object.
        PhaseStats *stats = nullptr;
        std::chrono::steady_clock::time_point start{};
    };

    void setEnabled(bool value) noexcept { enabled_ = value; }
    bool enabled() const noexcept { return enabled_; }
    void setOutputPath(std::string path) { outputPath_ = std::move(path); }
    const std::string &outputPath() const noexcept { return outputPath_; }

    void startRun() {
        if (!enabled_)
            return;
        for (auto &phase : phases_)
            phase.second = PhaseStats{};
        for (auto &counter : counters_)
            counter.second = 0.;
        wallStart_ = std::chrono::steady_clock::now();
        cpuStart_ = std::clock();
        running_ = true;
        wallSeconds_ = 0.;
        cpuSeconds_ = 0.;
    }

    void stopRun() {
        if (!enabled_ || !running_)
            return;
        wallSeconds_ = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - wallStart_).count();
        cpuSeconds_ = static_cast<double>(std::clock() - cpuStart_) /
                      static_cast<double>(CLOCKS_PER_SEC);
        running_ = false;
    }

    // Heterogeneous lookups: a string literal at the call site never
    // materializes a std::string, which matters at the per-candidate sites
    // that fire tens of millions of times per job.
    ScopedPhase measure(std::string_view name) {
        return enabled_ ? ScopedPhase(phaseSlot(name)) : ScopedPhase();
    }
    PhaseStats *phaseSlot(std::string_view name) {
        auto found = phases_.find(name);
        if (found == phases_.end())
            found = phases_.emplace(std::string(name), PhaseStats{}).first;
        return &found->second;
    }

    void setCounter(std::string name, double value) {
        if (enabled_)
            counters_[std::move(name)] = value;
    }
    void addCounter(std::string_view name, double value = 1.) {
        if (!enabled_)
            return;
        auto found = counters_.find(name);
        if (found == counters_.end())
            found = counters_.emplace(std::string(name), 0.).first;
        found->second += value;
    }
    void setMetadata(std::string name, std::string value) {
        if (enabled_)
            metadata_[std::move(name)] = std::move(value);
    }

    const std::map<std::string, PhaseStats, std::less<>> &phases() const noexcept {
        return phases_;
    }
    const std::map<std::string, double, std::less<>> &counters() const noexcept {
        return counters_;
    }

    void writeJson() {
        if (!enabled_ || outputPath_.empty())
            return;
        stopRun();
        std::ofstream output(outputPath_);
        if (!output)
            throw std::runtime_error(
                "[PerformanceTelemetry] cannot write " + outputPath_);
        output << std::setprecision(17);
        output << "{\n  \"schema_version\": 1,\n";
        output << "  \"wall_seconds\": " << wallSeconds_ << ",\n";
        output << "  \"cpu_seconds\": " << cpuSeconds_ << ",\n";
        const auto events = counters_.find("events_processed");
        const double eventCount = events == counters_.end() ? 0. : events->second;
        output << "  \"events_per_second\": "
               << (wallSeconds_ > 0. ? eventCount / wallSeconds_ : 0.)
               << ",\n";
        output << "  \"cpu_seconds_per_event\": "
               << (eventCount > 0. ? cpuSeconds_ / eventCount : 0.)
               << ",\n";
        output << "  \"peak_rss_kib\": " << peakRssKiB() << ",\n";
        output << "  \"metadata\": {";
        writeStringMap(output, metadata_);
        output << "\n  },\n  \"counters\": {";
        writeNumberMap(output, counters_);
        output << "\n  },\n  \"phases\": {";
        bool first = true;
        for (const auto &item : phases_) {
            output << (first ? "\n" : ",\n") << "    \""
                   << escape(item.first) << "\": {\"seconds\": "
                   << item.second.seconds << ", \"calls\": "
                   << item.second.calls << "}";
            first = false;
        }
        if (!first)
            output << '\n';
        output << "  }\n}\n";
    }

private:
    static long peakRssKiB() noexcept {
#if defined(__unix__) || defined(__APPLE__)
        struct rusage usage {};
        if (getrusage(RUSAGE_SELF, &usage) == 0) {
#if defined(__APPLE__)
            return usage.ru_maxrss / 1024;
#else
            return usage.ru_maxrss;
#endif
        }
#endif
        return -1;
    }

    static std::string escape(const std::string &value) {
        std::string result;
        result.reserve(value.size());
        for (const char character : value) {
            switch (character) {
            case '\\': result += "\\\\"; break;
            case '"': result += "\\\""; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += character; break;
            }
        }
        return result;
    }

    static void writeStringMap(
        std::ostream &output,
        const std::map<std::string, std::string> &values) {
        bool first = true;
        for (const auto &item : values) {
            output << (first ? "\n" : ",\n") << "    \""
                   << escape(item.first) << "\": \"" << escape(item.second)
                   << '"';
            first = false;
        }
        if (!first)
            output << '\n';
    }

    static void writeNumberMap(
        std::ostream &output,
        const std::map<std::string, double, std::less<>> &values) {
        bool first = true;
        for (const auto &item : values) {
            output << (first ? "\n" : ",\n") << "    \""
                   << escape(item.first) << "\": " << item.second;
            first = false;
        }
        if (!first)
            output << '\n';
    }

    bool enabled_ = false;
    bool running_ = false;
    std::string outputPath_;
    std::chrono::steady_clock::time_point wallStart_{};
    std::clock_t cpuStart_ = 0;
    double wallSeconds_ = 0.;
    double cpuSeconds_ = 0.;
    std::map<std::string, PhaseStats, std::less<>> phases_;
    std::map<std::string, double, std::less<>> counters_;
    std::map<std::string, std::string> metadata_;
};

} // namespace SKNano

#endif
