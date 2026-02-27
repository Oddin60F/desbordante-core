#pragma once

#include <utility>
#include <vector>

#include <boost/dynamic_bitset.hpp>

#include "core/algorithms/cfd/cfdfinder/model/pattern/pattern.h"
#include "core/algorithms/cfd/cfdfinder/model/pattern/pattern_tableau.h"
#include "core/algorithms/cfd/cfdfinder/model/pruning/pruning_strategy.h"
#include "core/algorithms/cfd/cfdfinder/types/columns.h"
#include "core/algorithms/cfd/cfdfinder/types/frontier.h"
#include "core/algorithms/cfd/cfdfinder/util/lhs_utils.h"

namespace algos::cfdfinder {

class ExpansionStrategy {
protected:
    using BitSet = boost::dynamic_bitset<>;
    using Cover = std::vector<Cluster>;

    ColumnRecords columns_;

    std::vector<size_t> GetClusterRepresentatives(size_t id, Cover const& cover) const {
        auto const& column = columns_.GetColumn(id);

        std::vector<size_t> cluster_representatives;
        cluster_representatives.reserve(cover.size());
        for (auto const& cluster : cover) {
            cluster_representatives.push_back(column[cluster[0]]);
        }
        return cluster_representatives;
    }

    virtual Entries GenerateNullEntries(BitSet const& lhs_attributes) const = 0;

public:
    explicit ExpansionStrategy(RowsPtr&& rows) : columns_(std::move(rows)) {}

    Pattern GenerateNullPattern(BitSet const& lhs_attributes, model::PLI const* lhs_pli,
                                Row const& inverted_pli_rhs) const {
        auto null_entries = GenerateNullEntries(lhs_attributes);
        auto null_cover = utils::EnrichPLI(lhs_pli, columns_.GetColumn(0).size());

        Pattern null_pattern(std::move(null_entries), std::move(null_cover), inverted_pli_rhs);
        return null_pattern;
    }

    virtual ~ExpansionStrategy() = default;

    virtual void ExpandAndProcess(Pattern&& parent_pattern, Frontier& frontier,
                                  Row const& inverted_pli_rhs,
                                  PruningStrategy& pruning_strategy) = 0;
};

}  // namespace algos::cfdfinder
