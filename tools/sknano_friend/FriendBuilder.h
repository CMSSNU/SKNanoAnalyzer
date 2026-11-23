#ifndef SKNANO_FRIEND_BUILDER_H
#define SKNANO_FRIEND_BUILDER_H

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

// Builds a friend RNTuple for one central NanoAOD file (docs/FriendInput.md):
// the custom-only fields of a private production, re-ordered to the central
// file's entry order, plus the per-entry SKFriend_status and the SKFriendInfo
// record the framework checks when it attaches the file.
namespace sknano_friend {

struct BuildOptions {
    std::string central;            // central NanoAOD RNTuple file
    std::vector<std::string> aux;   // private-production files covering its lumis
    std::string out;                // friend file to write (published atomically)
    std::string production;         // friend production name, recorded in SKFriendInfo
    std::string ntupleName = "Events";
    // Collections taken from the aux files even though the central file has
    // them too (their rows differ, e.g. an extended GenPart).
    std::vector<std::string> overrides;
    // Collections of the aux files to leave out (e.g. loose duplicates).
    std::vector<std::string> excludePrefixes;
    bool isMC = false;              // read Runs; sum genWeight over status-2 entries
    // The production applies an event filter: an absent event is a rejected
    // one (status 1, no loss assumed) instead of a lost one (status 2).
    bool filtered = false;
    std::string filterNote;         // what the producer's event filter was
    std::string scratch = "/tmp";   // local directory for the stage files
    // Aux entries buffered per stage file.  Memory is dominated by the buffered
    // REntry objects: 5000 keeps a 356-field friend build at 1.6 GB peak RSS,
    // 20000 needed 4.3 GB, for the same wall time.
    std::size_t chunkEntries = 5000;
};

// The Runs table of one aux file: events read (before any filter) and their
// genWeight sums -- the normalisation of a filtered production.
struct RunsSums {
    std::int64_t count = 0;
    double sumw = 0.;
    double sumw2 = 0.;
};

struct BuildReport {
    std::uint64_t centralEntries = 0;
    std::uint64_t nStatus[3] = {0, 0, 0};
    std::uint64_t auxOnly = 0;      // aux events of the central lumis absent from the central file
    std::vector<std::pair<std::uint32_t, std::uint32_t>> missingLumis;
    double missingSumW = 0.;
    double missingSumSign = 0.;
    std::uint64_t missingCount = 0;
    std::vector<std::string> fields;
    std::vector<std::string> checkedCounts; // nX compared between aux and central
    std::vector<std::string> sourceFiles;
    std::map<std::string, RunsSums> auxRuns; // MC only
    bool filtered = false;
    double seconds = 0.;

    nlohmann::json toJson() const;
};

BuildReport BuildFriend(const BuildOptions &options);

struct CheckReport {
    std::uint64_t entries = 0;
    std::uint64_t mismatches = 0;
    std::uint64_t nStatus[3] = {0, 0, 0};
    nlohmann::json info;            // the SKFriendInfo record

    nlohmann::json toJson() const;
};

// Re-reads a friend next to its central file and compares every entry key.
CheckReport CheckFriend(const std::string &friendFile, const std::string &central,
                        const std::string &ntupleName = "Events");

} // namespace sknano_friend

#endif
