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

struct ReplacedEntries {
    std::shared_ptr<Entries const> parent_entries;
    PatternItem replaced_entry;

    bool operator==(ReplacedEntries const& other) const {
        if (parent_entries->size() != other.parent_entries->size()) return false;
        for (size_t i = 0; i < parent_entries->size(); ++i) {
            auto const& this_item = (*parent_entries)[i];
            auto const& other_item = (*other.parent_entries)[i];
            if (this_item.id != other_item.id) return false;

            auto const& this_entry =
                    (i == replaced_entry.id) ? *replaced_entry.entry : *this_item.entry;
            auto const& other_entry = (i == other.replaced_entry.id) ? *other.replaced_entry.entry
                                                                     : *other_item.entry;
            if (this_entry != other_entry) return false;
        }
        return true;
    }

    bool operator!=(ReplacedEntries const& other) const = default;
};
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

template <>
struct std::hash<algos::cfdfinder::ReplacedEntries> {
    size_t operator()(algos::cfdfinder::ReplacedEntries const& r) const {
        size_t seed = 0;
        auto const& entries = *r.parent_entries;
        for (size_t i = 0; i < entries.size(); ++i) {
            auto const& item = entries[i];
            seed = seed * 31 + item.id;
            if (i == r.replaced_entry.id) {
                seed = seed * 31 + r.replaced_entry.entry->Hash();
            } else {
                seed = seed * 31 + item.entry->Hash();
            }
        }
        return seed;
    }
};
