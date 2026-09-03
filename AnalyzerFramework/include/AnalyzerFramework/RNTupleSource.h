#ifndef SKNANO_RNTUPLESOURCE_H
#define SKNANO_RNTUPLESOURCE_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_set>

namespace SKNano {

// Type-erased bridge between the analyzer's generated branch wrappers and
// ROOT's typed RNTuple views.  Keeping ROOT's RNTuple templates in the .cc
// file avoids exposing their evolving implementation to every analyzer and
// to rootcling.
struct RNTupleSpan {
    const void *data = nullptr;
    std::size_t size = 0;
};

class RNTupleScalarColumn {
public:
    virtual ~RNTupleScalarColumn() = default;
    virtual void read(std::uint64_t entry, void *destination) const = 0;
};

class RNTupleVectorColumn {
public:
    virtual ~RNTupleVectorColumn() = default;
    virtual RNTupleSpan read(std::uint64_t entry) const = 0;
};

// Byte-level read accounting for one input file, summed by the loader over
// the whole chain and published in the performance report.
struct RNTupleReadStats {
    std::uint64_t prefetchBytes = 0;
    std::uint64_t prefetchRanges = 0;
    std::uint64_t prefetchClusters = 0;
    // ROOT's own page-source counters; only populated when metrics are on.
    std::uint64_t readPayloadBytes = 0;
    std::uint64_t readOverheadBytes = 0;
    std::uint64_t readCalls = 0;
    std::uint64_t readVCalls = 0;
    std::uint64_t unzipBytes = 0;
    double readWallSeconds = 0.;
    double unzipWallSeconds = 0.;

    RNTupleReadStats &operator+=(const RNTupleReadStats &other) noexcept {
        prefetchBytes += other.prefetchBytes;
        prefetchRanges += other.prefetchRanges;
        prefetchClusters += other.prefetchClusters;
        readPayloadBytes += other.readPayloadBytes;
        readOverheadBytes += other.readOverheadBytes;
        readCalls += other.readCalls;
        readVCalls += other.readVCalls;
        unzipBytes += other.unzipBytes;
        readWallSeconds += other.readWallSeconds;
        unzipWallSeconds += other.unzipWallSeconds;
        return *this;
    }
};

class RNTupleSource {
public:
    RNTupleSource();
    ~RNTupleSource();
    RNTupleSource(const RNTupleSource &) = delete;
    RNTupleSource &operator=(const RNTupleSource &) = delete;
    RNTupleSource(RNTupleSource &&) noexcept;
    RNTupleSource &operator=(RNTupleSource &&) noexcept;

    // prefetchClusters > 0 starts a page-cache read-ahead thread that reads
    // the pages of the next `prefetchClusters` clusters sequentially ahead of
    // the entry cursor (see noteEntry).  RNTuple's own cluster pool issues one
    // page-sized read at a time, which on a high-latency network mount caps a
    // job at a few MB/s; sequential multi-megabyte reads run at the mount's
    // bandwidth and leave the pages in the kernel page cache for the pool.
    // `prefetchFields` limits the read-ahead to the columns of those
    // top-level fields; empty or null reads the whole cluster span.
    void open(const std::string &ntupleName, const std::string &fileName,
              bool enableMetrics = false, bool enableClusterCache = true,
              unsigned prefetchClusters = 0,
              const std::unordered_set<std::string> *prefetchFields = nullptr);
    void close();
    // Entry cursor for the read-ahead thread; cheap unless a cluster boundary
    // is crossed, where the current set of activated top-level fields is
    // handed over so the read-ahead follows the analyzer's lazy activation.
    // No-op when prefetching is off.
    void noteEntry(std::uint64_t localEntry,
                   const std::unordered_set<std::string> *activeFields = nullptr);
    RNTupleReadStats readStats() const;
    bool isOpen() const noexcept;
    std::uint64_t entries() const;
    bool hasField(const std::string &name) const;
    std::string fieldType(const std::string &name) const;
    const std::string &fileName() const noexcept;

    std::shared_ptr<RNTupleScalarColumn>
    makeScalarColumn(const std::string &name, std::type_index outputType) const;
    std::shared_ptr<RNTupleVectorColumn>
    makeVectorColumn(const std::string &name, std::type_index elementType) const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace SKNano

#endif
