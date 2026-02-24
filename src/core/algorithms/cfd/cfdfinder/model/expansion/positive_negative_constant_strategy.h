#pragma once
#include <list>

#include "core/algorithms/cfd/cfdfinder/model/expansion/constant_expansion_strategy.h"
#include "core/algorithms/cfd/cfdfinder/types/hyfd_types.h"

namespace algos::cfdfinder {
class PositiveNegativeConstantExpansion : public ConstantExpansion {
private:
    void ProcessForId(Entries& buffer_entries, size_t replaced_index, Frontier& frontier,
                      std::vector<size_t> const& cluster_violations,
                      PruningStrategy& pruning_strategy, size_t id,
                      std::vector<int>&& valid_constants,
                      std::vector<Cluster> const& cover) const override;
    std::vector<int> FilterForSupport(std::vector<int>&& valid_constants,
                                      PruningStrategy const& pruning_strategy,
                                      std::vector<size_t> const& cluster_representatives,
                                      std::vector<Cluster> const& cover) const override;

public:
    explicit PositiveNegativeConstantExpansion(RowsPtr&& compressed_records)
        : ConstantExpansion(std::move(compressed_records)) {}

    void ExpandAndProcess(Pattern&& parent_pattern, Frontier& frontier, Row const& inverted_pli_rhs,
                          PruningStrategy& pruning_strategy) override;
};

}  // namespace algos::cfdfinder
