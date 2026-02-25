#pragma once
#include <memory>
#include <vector>

#include <boost/functional/hash.hpp>

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

    bool operator<(PatternItem const& other) const {
        return *entry < *other.entry;
    }
};

using Entries = std::vector<PatternItem>;

struct ReplacedEntries {
private:
    size_t compute_hash() const {
        size_t seed = 0;
        auto const& entries = *parent_entries;
        for (size_t i = 0; i < entries.size(); ++i) {
            if (i == replaced_entry.id) {
                boost::hash_combine(seed, replaced_entry.entry->Hash());
            } else {
                boost::hash_combine(seed, entries[i].entry->Hash());
            }
        }
        return seed;
    }

public:
    std::shared_ptr<Entries const> parent_entries;
    PatternItem const replaced_entry;
    size_t const cached_hash_;

    ReplacedEntries(std::shared_ptr<Entries const> p, PatternItem r)
        : parent_entries(std::move(p)),
          replaced_entry(std::move(r)),
          cached_hash_(compute_hash()) {}

    bool operator==(ReplacedEntries const& other) const {
        auto const& this_entries = *parent_entries;
        auto const& other_entries = *other.parent_entries;

        if (this_entries.size() != other_entries.size()) {
            return false;
        }

        for (size_t i = 0; i < this_entries.size(); ++i) {
            auto const& this_item = this_entries[i];
            auto const& other_item = other_entries[i];
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
        for (auto const& [_, entry] : entries) {
            boost::hash_combine(seed, entry->Hash());
        }
        return seed;
    }
};

template <>
struct std::hash<algos::cfdfinder::ReplacedEntries> {
    size_t operator()(algos::cfdfinder::ReplacedEntries const& r) const {
        return r.cached_hash_;
    }
};
