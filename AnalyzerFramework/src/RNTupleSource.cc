#include <AnalyzerFramework/RNTupleSource.h>

#include "AnalysisException.h"

#include <ROOT/RNTupleDescriptor.hxx>
#include <ROOT/RNTupleMetrics.hxx>
#include <ROOT/RNTupleReadOptions.hxx>
#include <ROOT/RNTupleReader.hxx>
#include <ROOT/RNTupleTypes.hxx>
#include <ROOT/RNTupleView.hxx>
#include <ROOT/RVec.hxx>

#include <algorithm>
#include <atomic>
#include <limits>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <typeindex>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace SKNano {
namespace {

template <typename DiskT, typename OutputT = DiskT>
class ScalarColumn final : public RNTupleScalarColumn {
public:
    ScalarColumn(ROOT::RNTupleReader &reader, const std::string &name)
        : view_(reader.GetView<DiskT>(name)) {}

    void read(std::uint64_t entry, void *destination) const override {
        *static_cast<OutputT *>(destination) =
            static_cast<OutputT>(view_(entry));
    }

private:
    mutable ROOT::RNTupleView<DiskT> view_;
};

template <typename T>
class VectorColumn final : public RNTupleVectorColumn {
public:
    VectorColumn(ROOT::RNTupleReader &reader, const std::string &name)
        : view_(reader.GetView<ROOT::VecOps::RVec<T>>(name)) {}

    RNTupleSpan read(std::uint64_t entry) const override {
        const auto &values = view_(entry);
        return {values.data(), values.size()};
    }

private:
    mutable ROOT::RNTupleView<ROOT::VecOps::RVec<T>> view_;
};

// RVec<bool> deliberately does not expose a bool* suitable for the analyzer's
// byte-contiguous Bool_t contract. Materialize only this specialization; all
// arithmetic vector fields remain zero-copy views into RNTuple pages.
class BoolVectorColumn final : public RNTupleVectorColumn {
public:
    BoolVectorColumn(ROOT::RNTupleReader &reader, const std::string &name)
        : view_(reader.GetView<ROOT::VecOps::RVec<bool>>(name)) {}

    RNTupleSpan read(std::uint64_t entry) const override {
        const auto &values = view_(entry);
        bytes_.resize(values.size());
        std::transform(values.begin(), values.end(), bytes_.begin(),
                       [](bool value) { return static_cast<std::uint8_t>(value); });
        return {bytes_.data(), bytes_.size()};
    }

private:
    mutable ROOT::RNTupleView<ROOT::VecOps::RVec<bool>> view_;
    mutable std::vector<std::uint8_t> bytes_;
};

template <typename T>
bool Is(std::type_index type) {
    return type == std::type_index(typeid(T));
}

template <typename T>
std::shared_ptr<RNTupleScalarColumn>
MakeOrdinaryScalar(ROOT::RNTupleReader &reader, const std::string &name) {
    return std::make_shared<ScalarColumn<T>>(reader, name);
}

template <typename OutputT>
std::shared_ptr<RNTupleScalarColumn>
MakeCardinalityScalar(ROOT::RNTupleReader &reader, const std::string &name,
                      const std::string &fieldType) {
    if (fieldType.find("uint64_t") != std::string::npos)
        return std::make_shared<
            ScalarColumn<ROOT::RNTupleCardinality<std::uint64_t>, OutputT>>(
            reader, name);
    return std::make_shared<
        ScalarColumn<ROOT::RNTupleCardinality<std::uint32_t>, OutputT>>(
        reader, name);
}

bool IsCardinality(const std::string &fieldType) {
    return fieldType.find("RNTupleCardinality") != std::string::npos;
}

// Page-cache warmer.  Reads the byte ranges of upcoming clusters with large
// sequential pread() calls on a private file descriptor so that the RNTuple
// cluster pool's page-sized reads hit the kernel page cache instead of the
// network mount.  It never hands data to the reader: correctness is entirely
// ROOT's, this only changes the timing of the underlying I/O.
class InputPrefetcher {
public:
    struct Range {
        std::uint64_t offset = 0;
        std::uint64_t size = 0;
    };
    // Pages of one cluster, grouped by the top-level field they belong to
    // (index into `fields`); the whole-span extent covers every column.
    struct Cluster {
        std::uint64_t firstEntry = 0;
        std::uint64_t entries = 0;
        std::vector<std::vector<Range>> pagesByField;
        Range span;
    };
    struct Layout {
        std::vector<std::string> fields; // top-level field names
        std::vector<Cluster> clusters;
    };

    InputPrefetcher(const std::string &path, Layout layout, unsigned window,
                    std::unordered_set<std::string> activeFields)
        : layout_(std::move(layout)), window_(window) {
        setActiveFields(std::move(activeFields));
        auto &clusters_ = layout_.clusters;
        if (clusters_.empty())
            return;
        fd_ = ::open(path.c_str(), O_RDONLY);
        if (fd_ < 0) {
            std::cerr << "[RNTupleSource] prefetch disabled: cannot open "
                      << path << std::endl;
            return;
        }
#ifdef POSIX_FADV_SEQUENTIAL
        ::posix_fadvise(fd_, 0, 0, POSIX_FADV_SEQUENTIAL);
#endif
        clusterEnd_ = clusters_.front().firstEntry + clusters_.front().entries;
        thread_ = std::thread([this] { run(); });
    }

    ~InputPrefetcher() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stop_ = true;
        }
        wake_.notify_all();
        if (thread_.joinable())
            thread_.join();
        if (fd_ >= 0)
            ::close(fd_);
    }

    // The analyzer activates fields lazily, so the set of columns worth
    // reading ahead grows during the first file; the loader hands over the
    // current set at every cluster boundary and the thread resolves it when it
    // warms the next cluster.
    void setActiveFields(std::unordered_set<std::string> names) {
        std::vector<bool> mask(layout_.fields.size(), false);
        std::size_t hits = 0;
        for (std::size_t index = 0; index < layout_.fields.size(); ++index) {
            if (names.count(layout_.fields[index])) {
                mask[index] = true;
                ++hits;
            }
        }
        std::lock_guard<std::mutex> lock(mutex_);
        activeMask_ = std::move(mask);
        activeCount_ = hits;
        activeNames_ = names.size();
    }

    void noteEntry(std::uint64_t entry,
                   const std::unordered_set<std::string> *activeFields) {
        if (fd_ < 0 || entry < clusterEnd_)
            return;
        const auto &clusters_ = layout_.clusters;
        // Sequential loops cross one boundary at a time; a jump lands on the
        // right cluster after a few steps either way.
        std::size_t index = current_;
        while (index + 1 < clusters_.size() &&
               entry >= clusters_[index].firstEntry + clusters_[index].entries)
            ++index;
        current_ = index;
        clusterEnd_ = clusters_[index].firstEntry + clusters_[index].entries;
        if (activeFields && activeFields->size() != activeNames_)
            setActiveFields(*activeFields);
        {
            std::lock_guard<std::mutex> lock(mutex_);
            target_ = index;
        }
        wake_.notify_one();
    }

    std::uint64_t bytes() const noexcept { return bytes_.load(); }
    std::uint64_t ranges() const noexcept { return ranges_.load(); }
    std::uint64_t clustersWarmed() const noexcept { return warmed_.load(); }

private:
    static constexpr std::size_t kChunkBytes = 4U * 1024U * 1024U;

    void run() {
        std::vector<char> buffer(kChunkBytes);
        const auto &clusters_ = layout_.clusters;
        std::size_t next = 0;
        std::size_t lastTarget = std::numeric_limits<std::size_t>::max();
        for (;;) {
            std::size_t target = 0;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                wake_.wait(lock, [&] { return stop_ || target_ != lastTarget; });
                if (stop_)
                    return;
                target = target_;
            }
            lastTarget = target;
            const std::size_t last =
                std::min(target + window_, clusters_.size() - 1);
            for (std::size_t index = std::max(next, target); index <= last;
                 ++index) {
                if (stop_.load())
                    return;
                warm(clusters_[index], buffer);
                next = index + 1;
                ++warmed_;
            }
        }
    }

    // Ranges of one cluster for the current active set: the pages of the
    // active fields, sorted and coalesced; the whole span while no field is
    // active yet (start of the first file).
    std::vector<Range> rangesFor(const Cluster &cluster) {
        std::vector<bool> mask;
        std::size_t active = 0;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            mask = activeMask_;
            active = activeCount_;
        }
        if (active == 0)
            return {cluster.span};
        std::vector<Range> pages;
        for (std::size_t field = 0; field < mask.size(); ++field) {
            if (!mask[field] || field >= cluster.pagesByField.size())
                continue;
            const auto &list = cluster.pagesByField[field];
            pages.insert(pages.end(), list.begin(), list.end());
        }
        std::sort(pages.begin(), pages.end(),
                  [](const Range &lhs, const Range &rhs) {
                      return lhs.offset < rhs.offset;
                  });
        // Coalesce pages that sit within a megabyte of each other: the gap
        // is cheaper to stream through than a separate request.
        constexpr std::uint64_t kMaxGap = 1024U * 1024U;
        std::vector<Range> ranges;
        for (const auto &page : pages) {
            if (!ranges.empty()) {
                auto &last = ranges.back();
                const std::uint64_t lastEnd = last.offset + last.size;
                if (page.offset <= lastEnd + kMaxGap) {
                    last.size =
                        std::max(lastEnd, page.offset + page.size) - last.offset;
                    continue;
                }
            }
            ranges.push_back(page);
        }
        return ranges;
    }

    void warm(const Cluster &cluster, std::vector<char> &buffer) {
        for (const auto &range : rangesFor(cluster)) {
            std::uint64_t offset = range.offset;
            std::uint64_t remaining = range.size;
            while (remaining > 0) {
                if (stop_.load())
                    return;
                const std::size_t chunk = static_cast<std::size_t>(
                    std::min<std::uint64_t>(remaining, buffer.size()));
                const ssize_t got = ::pread(fd_, buffer.data(), chunk,
                                            static_cast<off_t>(offset));
                if (got <= 0)
                    break;
                offset += static_cast<std::uint64_t>(got);
                remaining -= static_cast<std::uint64_t>(got);
                bytes_ += static_cast<std::uint64_t>(got);
            }
            ++ranges_;
        }
    }

    Layout layout_;
    unsigned window_ = 0;
    int fd_ = -1;
    // Active-field mask, shared with the I/O thread under mutex_.
    std::vector<bool> activeMask_;
    std::size_t activeCount_ = 0;
    std::size_t activeNames_ = 0;
    // Main-thread cursor state; only noteEntry() touches these.
    std::size_t current_ = 0;
    std::uint64_t clusterEnd_ = 0;
    // Shared with the I/O thread.
    std::mutex mutex_;
    std::condition_variable wake_;
    std::size_t target_ = 0;
    std::atomic<bool> stop_{false};
    std::atomic<std::uint64_t> bytes_{0};
    std::atomic<std::uint64_t> ranges_{0};
    std::atomic<std::uint64_t> warmed_{0};
    std::thread thread_;
};

// Page table for the read-ahead thread from a descriptor snapshot taken once
// per file: for every cluster, the pages of every top-level field (all of its
// columns, sub-fields included) and the whole-cluster extent.
InputPrefetcher::Layout
BuildPrefetchLayout(const ROOT::RNTupleDescriptor &descriptor) {
    InputPrefetcher::Layout layout;
    // physical column id -> top-level field index
    std::unordered_map<ROOT::DescriptorId_t, std::size_t> columnField;
    std::function<void(const ROOT::RFieldDescriptor &, std::size_t)> collect =
        [&](const ROOT::RFieldDescriptor &field, std::size_t index) {
            for (const auto &column : descriptor.GetColumnIterable(field))
                columnField.emplace(column.GetPhysicalId(), index);
            for (const auto &child : descriptor.GetFieldIterable(field))
                collect(child, index);
        };
    for (const auto &field : descriptor.GetTopLevelFields()) {
        layout.fields.push_back(field.GetFieldName());
        collect(field, layout.fields.size() - 1);
    }

    // The entry-indexed cluster lookup is not public API; the iterable is,
    // and clusters are few enough (tens per file) to sort by first entry.
    std::vector<const ROOT::RClusterDescriptor *> ordered;
    for (const auto &cluster : descriptor.GetClusterIterable())
        ordered.push_back(&cluster);
    std::sort(ordered.begin(), ordered.end(),
              [](const auto *lhs, const auto *rhs) {
                  return lhs->GetFirstEntryIndex() < rhs->GetFirstEntryIndex();
              });

    for (const auto *clusterPtr : ordered) {
        const auto &cluster = *clusterPtr;
        InputPrefetcher::Cluster out;
        out.firstEntry = cluster.GetFirstEntryIndex();
        out.entries = cluster.GetNEntries();
        if (out.entries == 0)
            continue;
        out.pagesByField.resize(layout.fields.size());
        std::uint64_t begin = std::numeric_limits<std::uint64_t>::max();
        std::uint64_t end = 0;
        for (const auto &[columnId, fieldIndex] : columnField) {
            if (!cluster.ContainsColumn(columnId))
                continue;
            for (const auto &page : cluster.GetPageRange(columnId).GetPageInfos()) {
                const auto &locator = page.GetLocator();
                if (locator.GetType() != ROOT::RNTupleLocator::ELocatorType::kTypeFile)
                    continue;
                const InputPrefetcher::Range range{
                    locator.GetPosition<std::uint64_t>(),
                    locator.GetNBytesOnStorage()};
                out.pagesByField[fieldIndex].push_back(range);
                begin = std::min(begin, range.offset);
                end = std::max(end, range.offset + range.size);
            }
        }
        if (end <= begin)
            continue;
        out.span = {begin, end - begin};
        layout.clusters.push_back(std::move(out));
    }
    return layout;
}

std::uint64_t MetricInt(const ROOT::RNTupleReader &reader, const char *name) {
    const auto *counter = reader.GetMetrics().GetCounter(name);
    return counter ? static_cast<std::uint64_t>(std::max<std::int64_t>(
                         0, counter->GetValueAsInt()))
                   : 0;
}

} // namespace

class RNTupleSource::Impl {
public:
    std::unique_ptr<ROOT::RNTupleReader> reader;
    std::unique_ptr<InputPrefetcher> prefetcher;
    bool metrics = false;
    std::string file;
    // Top-level field name -> on-disk type name, snapshotted once per file.
    // RNTupleReader::GetDescriptor() clones the descriptor under a lock and
    // FindFieldId() is a linear scan over every top-level field, so neither may
    // be called from the per-entry read path.
    std::unordered_map<std::string, std::string> fieldTypes;

    void snapshotFields() {
        fieldTypes.clear();
        if (!reader)
            return;
        const auto &descriptor = reader->GetDescriptor();
        for (const auto &field : descriptor.GetTopLevelFields())
            fieldTypes.emplace(field.GetFieldName(), field.GetTypeName());
    }
};

RNTupleSource::RNTupleSource() : impl_(std::make_unique<Impl>()) {}
RNTupleSource::~RNTupleSource() = default;
RNTupleSource::RNTupleSource(RNTupleSource &&) noexcept = default;
RNTupleSource &RNTupleSource::operator=(RNTupleSource &&) noexcept = default;

void RNTupleSource::open(const std::string &ntupleName,
                         const std::string &fileName, bool enableMetrics,
                         bool enableClusterCache, unsigned prefetchClusters,
                         const std::unordered_set<std::string> *prefetchFields) {
    ROOT::RNTupleReadOptions options;
    options.SetEnableMetrics(enableMetrics);
    options.SetClusterCache(enableClusterCache
        ? ROOT::RNTupleReadOptions::EClusterCache::kOn
        : ROOT::RNTupleReadOptions::EClusterCache::kOff);
    try {
        auto reader = ROOT::RNTupleReader::Open(ntupleName, fileName, options);
        impl_->reader = std::move(reader);
        impl_->file = fileName;
        impl_->metrics = enableMetrics;
        impl_->snapshotFields();
    } catch (const std::exception &error) {
        throw SKNano::ConfigError("[RNTupleSource] cannot open '" + ntupleName +
                                  "' in " + fileName + ": " + error.what());
    }
    impl_->prefetcher.reset();
    if (prefetchClusters == 0)
        return;
    // Remote protocols have no page cache to warm; only plain paths qualify.
    if (fileName.find("://") != std::string::npos &&
        fileName.compare(0, 7, "file://") != 0)
        return;
    const std::string path =
        fileName.compare(0, 7, "file://") == 0 ? fileName.substr(7) : fileName;
    try {
        auto layout = BuildPrefetchLayout(impl_->reader->GetDescriptor());
        if (!layout.clusters.empty())
            impl_->prefetcher = std::make_unique<InputPrefetcher>(
                path, std::move(layout), prefetchClusters,
                prefetchFields ? *prefetchFields
                               : std::unordered_set<std::string>{});
    } catch (const std::exception &error) {
        std::cerr << "[RNTupleSource] prefetch disabled for " << fileName
                  << ": " << error.what() << std::endl;
    }
}

void RNTupleSource::close() {
    impl_->prefetcher.reset();
    impl_->reader.reset();
    impl_->file.clear();
    impl_->fieldTypes.clear();
}

void RNTupleSource::noteEntry(
    std::uint64_t localEntry,
    const std::unordered_set<std::string> *activeFields) {
    if (impl_->prefetcher)
        impl_->prefetcher->noteEntry(localEntry, activeFields);
}

RNTupleReadStats RNTupleSource::readStats() const {
    RNTupleReadStats stats;
    if (impl_->prefetcher) {
        stats.prefetchBytes = impl_->prefetcher->bytes();
        stats.prefetchRanges = impl_->prefetcher->ranges();
        stats.prefetchClusters = impl_->prefetcher->clustersWarmed();
    }
    if (impl_->reader && impl_->metrics) {
        const auto &reader = *impl_->reader;
        stats.readPayloadBytes =
            MetricInt(reader, "RNTupleReader.RPageSourceFile.szReadPayload");
        stats.readOverheadBytes =
            MetricInt(reader, "RNTupleReader.RPageSourceFile.szReadOverhead");
        stats.readCalls = MetricInt(reader, "RNTupleReader.RPageSourceFile.nRead");
        stats.readVCalls = MetricInt(reader, "RNTupleReader.RPageSourceFile.nReadV");
        stats.unzipBytes = MetricInt(reader, "RNTupleReader.RPageSourceFile.szUnzip");
        stats.readWallSeconds = static_cast<double>(MetricInt(
            reader, "RNTupleReader.RPageSourceFile.timeWallRead")) * 1e-9;
        stats.unzipWallSeconds = static_cast<double>(MetricInt(
            reader, "RNTupleReader.RPageSourceFile.timeWallUnzip")) * 1e-9;
    }
    return stats;
}

bool RNTupleSource::isOpen() const noexcept {
    return static_cast<bool>(impl_->reader);
}

std::uint64_t RNTupleSource::entries() const {
    if (!impl_->reader)
        throw SKNano::LogicError("[RNTupleSource] no RNTuple is open");
    return impl_->reader->GetNEntries();
}

bool RNTupleSource::hasField(const std::string &name) const {
    if (!impl_->reader)
        return false;
    return impl_->fieldTypes.find(name) != impl_->fieldTypes.end();
}

std::string RNTupleSource::fieldType(const std::string &name) const {
    if (!impl_->reader)
        throw SKNano::LogicError("[RNTupleSource] no RNTuple is open");
    const auto entry = impl_->fieldTypes.find(name);
    if (entry == impl_->fieldTypes.end())
        return {};
    return entry->second;
}

const std::string &RNTupleSource::fileName() const noexcept {
    return impl_->file;
}

std::shared_ptr<RNTupleScalarColumn>
RNTupleSource::makeScalarColumn(const std::string &name,
                                std::type_index outputType) const {
    if (!impl_->reader)
        throw SKNano::LogicError("[RNTupleSource] no RNTuple is open");
    if (!hasField(name))
        throw SKNano::ConfigError("[RNTupleSource] missing field '" + name +
                                  "' in " + impl_->file);

    auto &reader = *impl_->reader;
    const auto diskType = fieldType(name);
    try {
        if (IsCardinality(diskType)) {
            if (Is<int>(outputType))
                return MakeCardinalityScalar<int>(reader, name, diskType);
            if (Is<unsigned int>(outputType))
                return MakeCardinalityScalar<unsigned int>(reader, name, diskType);
            if (Is<short>(outputType))
                return MakeCardinalityScalar<short>(reader, name, diskType);
            if (Is<unsigned short>(outputType))
                return MakeCardinalityScalar<unsigned short>(reader, name, diskType);
            if (Is<unsigned char>(outputType))
                return MakeCardinalityScalar<unsigned char>(reader, name, diskType);
            if (Is<Long64_t>(outputType))
                return MakeCardinalityScalar<Long64_t>(reader, name, diskType);
            if (Is<ULong64_t>(outputType))
                return MakeCardinalityScalar<ULong64_t>(reader, name, diskType);
        } else {
            if (Is<float>(outputType)) return MakeOrdinaryScalar<float>(reader, name);
            if (Is<double>(outputType)) return MakeOrdinaryScalar<double>(reader, name);
            if (Is<int>(outputType)) return MakeOrdinaryScalar<int>(reader, name);
            if (Is<unsigned int>(outputType)) return MakeOrdinaryScalar<unsigned int>(reader, name);
            if (Is<bool>(outputType)) return MakeOrdinaryScalar<bool>(reader, name);
            if (Is<short>(outputType)) return MakeOrdinaryScalar<short>(reader, name);
            if (Is<unsigned short>(outputType)) return MakeOrdinaryScalar<unsigned short>(reader, name);
            if (Is<unsigned char>(outputType)) return MakeOrdinaryScalar<unsigned char>(reader, name);
            if (Is<Long64_t>(outputType)) return MakeOrdinaryScalar<Long64_t>(reader, name);
            if (Is<ULong64_t>(outputType)) return MakeOrdinaryScalar<ULong64_t>(reader, name);
        }
    } catch (const std::exception &error) {
        throw SKNano::ConfigError("[RNTupleSource] scalar field '" + name +
                                  "' has incompatible type " + diskType +
                                  " in " + impl_->file + ": " + error.what());
    }
    throw SKNano::ConfigError("[RNTupleSource] unsupported scalar mapping for field '" +
                              name + "' (disk type " + diskType + ")");
}

std::shared_ptr<RNTupleVectorColumn>
RNTupleSource::makeVectorColumn(const std::string &name,
                                std::type_index elementType) const {
    if (!impl_->reader)
        throw SKNano::LogicError("[RNTupleSource] no RNTuple is open");
    if (!hasField(name))
        throw SKNano::ConfigError("[RNTupleSource] missing field '" + name +
                                  "' in " + impl_->file);
    try {
        auto &reader = *impl_->reader;
        if (Is<float>(elementType)) return std::make_shared<VectorColumn<float>>(reader, name);
        if (Is<double>(elementType)) return std::make_shared<VectorColumn<double>>(reader, name);
        if (Is<int>(elementType)) return std::make_shared<VectorColumn<int>>(reader, name);
        if (Is<unsigned int>(elementType)) return std::make_shared<VectorColumn<unsigned int>>(reader, name);
        if (Is<bool>(elementType)) return std::make_shared<BoolVectorColumn>(reader, name);
        if (Is<short>(elementType)) return std::make_shared<VectorColumn<short>>(reader, name);
        if (Is<unsigned short>(elementType)) return std::make_shared<VectorColumn<unsigned short>>(reader, name);
        if (Is<unsigned char>(elementType)) return std::make_shared<VectorColumn<unsigned char>>(reader, name);
        if (Is<Long64_t>(elementType)) return std::make_shared<VectorColumn<Long64_t>>(reader, name);
        if (Is<ULong64_t>(elementType)) return std::make_shared<VectorColumn<ULong64_t>>(reader, name);
    } catch (const std::exception &error) {
        throw SKNano::ConfigError("[RNTupleSource] vector field '" + name +
                                  "' has incompatible type " + fieldType(name) +
                                  " in " + impl_->file + ": " + error.what());
    }
    throw SKNano::ConfigError("[RNTupleSource] unsupported vector mapping for field '" +
                              name + "' (disk type " + fieldType(name) + ")");
}

} // namespace SKNano
