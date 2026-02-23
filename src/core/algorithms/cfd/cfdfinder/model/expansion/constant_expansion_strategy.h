#pragma once

#include "core/algorithms/cfd/cfdfinder/model/expansion/expansion_strategy.h"
#include "core/algorithms/cfd/cfdfinder/model/pattern/entry.h"
#include "core/algorithms/cfd/cfdfinder/types/hyfd_types.h"

namespace algos::cfdfinder {

class ConstantExpansion : public ExpansionStrategy {
protected:
    std::vector<int> CalculateUniqueConstants(size_t column_id,
                                              std::vector<Cluster> const& cover) const;
    virtual std::vector<int> FilterForSupport(std::vector<int>&& valid_constants,
                                              PruningStrategy const& pruning_strategy,
                                              std::vector<size_t> const& cluster_representatives,
                                              std::vector<Cluster> const& cover) const;

    std::vector<std::pair<int, boost::dynamic_bitset<>>> CalculateCoverMasks(
            std::vector<int>&& processed_values, std::vector<size_t> const& cluster_representatives,
            std::vector<Cluster> const& cover) const;

    virtual void ProcessForId(Entries& buffer_entries, size_t replaced_index, Frontier& frontier,
                              Row const& inverted_pli_rhs, PruningStrategy& pruning_strategy,
                              size_t id, std::vector<int>&& valid_constants,
                              std::vector<Cluster> const& cover) const;

public:
    explicit ConstantExpansion(RowsPtr&& compressed_records)
        : ExpansionStrategy(std::move(compressed_records)) {}

    Pattern GenerateNullPattern(BitSet const& attributes) const override;
    void ExpandAndProcess(Pattern&& parent_pattern, Frontier& frontier, Row const& inverted_pli_rhs,
                          PruningStrategy& pruning_strategy) override;
};

}  // namespace algos::cfdfinder
