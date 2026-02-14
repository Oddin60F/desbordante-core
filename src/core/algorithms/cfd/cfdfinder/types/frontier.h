#pragma once

#include <set>
#include <unordered_set>
#include <utility>

#include "core/algorithms/cfd/cfdfinder/model/pattern/pattern.h"

namespace algos::cfdfinder {

class Frontier {
private:
    std::multiset<Pattern, std::greater<Pattern>> sorted_index_;
    std::unordered_set<Entries> search_index_;

public:
    Frontier() = default;

    void Emplace(Pattern&& pattern) {
        auto const& entries = pattern.GetEntries();
        search_index_.insert(entries);
        sorted_index_.insert(std::move(pattern));
    }

    Pattern Poll() {
        auto it = sorted_index_.begin();
        Pattern pattern = std::move(sorted_index_.extract(it).value());
        search_index_.erase(pattern.GetEntries());
        return pattern;
    }

    bool Contains(Entries const& entries) const {
        return search_index_.contains(entries);
    }

    bool Empty() const {
        return sorted_index_.empty();
    }

    void Swap(Frontier& other) {
        sorted_index_.swap(other.sorted_index_);
        search_index_.swap(other.search_index_);
    }
};

}  // namespace algos::cfdfinder