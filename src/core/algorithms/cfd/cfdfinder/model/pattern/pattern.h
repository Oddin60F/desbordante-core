#pragma once

#include <compare>
#include <cstddef>
#include <ranges>
#include <unordered_set>
#include <vector>

#include "core/algorithms/cfd/cfdfinder/model/pattern/pattern_item.h"
#include "core/algorithms/cfd/cfdfinder/types/cluster.h"
#include "core/algorithms/cfd/cfdfinder/types/hyfd_types.h"

namespace algos::cfdfinder {

class Pattern {
private:
    Entries entries_;
    std::vector<Cluster> cover_;
    double support_;
    size_t num_keepers_;
    size_t cached_hash_ = 0;

public:
    explicit Pattern(Entries&& entries) : entries_(std::move(entries)) {
        cached_hash_ = std::hash<Entries>{}(entries_);
    }

    size_t GetHash() const noexcept {
        return cached_hash_;
    }

    Pattern(Pattern&& other) noexcept = default;
    Pattern(Pattern const& other) = default;
    Pattern& operator=(Pattern&& other) noexcept = default;
    Pattern& operator=(Pattern const& other) = default;

    bool operator<(Pattern const& other) const noexcept;

    bool operator==(Pattern const& other) const noexcept {
        return entries_ == other.entries_;
    };

    bool operator>(Pattern const& other) const noexcept {
        return other < *this;
    }

    void UpdateCover(boost::dynamic_bitset<> const& used_rows);
    void UpdateKeepers(Row const& inverted_pli_rhs);
    size_t GetNumCover() const;

    Entries const& GetEntries() const noexcept {
        return entries_;
    }

    double GetSupport() const noexcept {
        return support_;
    }

    void SetSupport(double support) {
        support_ = support;
    }

    double GetConfidence() const {
        auto num_cover = GetNumCover();
        return num_cover == 0 ? 0 : static_cast<double>(num_keepers_) / num_cover;
    }

    std::vector<Cluster> const& GetCover() const noexcept {
        return cover_;
    }

    void SetCover(std::vector<Cluster>&& new_cover) {
        cover_ = std::move(new_cover);

        support_ = GetNumCover();
    }

    size_t GetNumKeepers() const noexcept {
        return num_keepers_;
    }

    void SetNumKeepers(size_t num_keepers) {
        num_keepers_ = num_keepers;
    }
};

}  // namespace algos::cfdfinder

template <>
struct std::hash<algos::cfdfinder::Pattern> {
    size_t operator()(algos::cfdfinder::Pattern const& p) const {
        return p.GetHash();
    }
};
