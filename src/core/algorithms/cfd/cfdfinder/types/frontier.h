#pragma once

#include <functional>
#include <utility>

#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>

#include "core/algorithms/cfd/cfdfinder/model/pattern/pattern.h"

namespace algos::cfdfinder {

namespace bmi = boost::multi_index;

class Frontier {
    struct ByEntries {};

    struct ByPriority {};

    struct PatternHash {
        using is_transparent = void;

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

    struct FrontierItem {
        Pattern pattern;
        size_t insert_num;
    };

    using HashedIndex =
            bmi::hashed_unique<bmi::tag<ByEntries>,
                               bmi::member<FrontierItem, Pattern, &FrontierItem::pattern>,
                               PatternHash, PatternEqual>;
    using PriorityIndex = bmi::ordered_non_unique<
            bmi::tag<ByPriority>,
            bmi::composite_key<FrontierItem,
                               bmi::member<FrontierItem, Pattern, &FrontierItem::pattern>,
                               bmi::member<FrontierItem, size_t, &FrontierItem::insert_num>>,
            bmi::composite_key_compare<std::greater<Pattern>, std::greater<size_t>>>;

    using FrontierContainer =
            bmi::multi_index_container<FrontierItem, bmi::indexed_by<HashedIndex, PriorityIndex>>;

    FrontierContainer container_;
    size_t next_seq_ = 0;

public:
    void Emplace(Pattern&& pattern) {
        container_.insert(FrontierItem{std::move(pattern), next_seq_++});
    }

    Pattern Poll() {
        auto& idx = container_.get<ByPriority>();
        auto it = idx.begin();
        FrontierItem item = std::move(idx.extract(it).value());
        return std::move(item.pattern);
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
        std::swap(next_seq_, other.next_seq_);
    }
};

}  // namespace algos::cfdfinder
