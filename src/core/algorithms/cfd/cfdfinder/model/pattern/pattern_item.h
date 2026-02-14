#pragma once
#include <memory>
#include <vector>

#include "core/algorithms/cfd/cfdfinder/model/pattern/entry.h"

namespace algos::cfdfinder {
struct PatternItem {
    size_t id;
    std::shared_ptr<Entry> entry;

    PatternItem(size_t id, std::shared_ptr<Entry> entry) : id(id), entry(std::move(entry)) {}

    bool operator==(PatternItem const& other) const {
        return id == other.id && *entry == *(other.entry);
    }

    bool operator!=(PatternItem const& other) const = default;
};

using Entries = std::vector<PatternItem>;
}  // namespace algos::cfdfinder

template <>
struct std::hash<algos::cfdfinder::Entries> {
    size_t operator()(algos::cfdfinder::Entries const& entries) const {
        size_t seed = 0;
        for (auto const& [id, entry] : entries) {
            seed = seed * 31 + id;
            seed = seed * 31 + entry->Hash();
        }
        return seed;
    }
};
