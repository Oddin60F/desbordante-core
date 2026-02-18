#pragma once

#include <list>
#include <utility>

#include <boost/dynamic_bitset.hpp>

#include "core/algorithms/cfd/cfdfinder/model/pattern/pattern.h"
#include "core/algorithms/cfd/cfdfinder/model/pattern/pattern_tableau.h"

namespace algos::cfdfinder {

class ExpansionStrategy {
protected:
public:
    using BitSet = boost::dynamic_bitset<>;
    using ReplacedItem = std::pair<size_t, std::shared_ptr<Entry>>;
    virtual ~ExpansionStrategy() = default;
    virtual Pattern GenerateNullPattern(BitSet const& attributes) const = 0;
    virtual std::vector<ReplacedItem> ExpandPattern(Pattern const& pattern) const = 0;
};

}  // namespace algos::cfdfinder
