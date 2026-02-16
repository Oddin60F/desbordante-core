#pragma once

#include <functional>
#include <utility>

#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>

#include "core/algorithms/cfd/cfdfinder/model/pattern/pattern.h"

namespace algos::cfdfinder {

namespace bmi = boost::multi_index;

struct ByEntries {};

struct ByPriority {};

struct PatternHash {
    using is_transparent = void;  // для гетерогенного поиска

    size_t operator()(Pattern const& p) const noexcept {
        return p.GetHash();
    }

    size_t operator()(Entries const& e) const noexcept {
        return std::hash<Entries>{}(e);
    }
};

struct PatternEqual {
    using is_transparent = void;

    bool operator()(Pattern const& a, Pattern const& b) const noexcept {
        return a == b;
    }

    bool operator()(Pattern const& p, Entries const& e) const noexcept {
        return p.GetEntries() == e;
    }

    bool operator()(Entries const& e, Pattern const& p) const noexcept {
        return p.GetEntries() == e;
    }
};

using FrontierContainer = bmi::multi_index_container<
        Pattern,
        bmi::indexed_by<bmi::hashed_unique<bmi::tag<ByEntries>, bmi::identity<Pattern>, PatternHash,
                                           PatternEqual>,
                        bmi::ordered_non_unique<bmi::tag<ByPriority>, bmi::identity<Pattern>,
                                                std::greater<Pattern> > > >;

class Frontier {
    FrontierContainer container_;

public:
    void Emplace(Pattern&& pattern) {
        auto& idx = container_.get<ByEntries>();
        // Вставляем с готовым хэшем из pattern.cached_hash_
        container_.insert(std::move(pattern));
    }

    Pattern Poll() {
        auto& idx = container_.get<ByPriority>();
        auto it = idx.begin();
        return std::move(idx.extract(it).value());
    }

    bool Contains(Entries const& entries) const {
        auto& idx = container_.get<ByEntries>();
        return idx.contains(entries);
    }

    bool Empty() const {
        return container_.empty();
    }

    void Swap(Frontier& other) {
        container_.swap(other.container_);
    }
};

}  // namespace algos::cfdfinder