#pragma once

#include <list>
#include <memory>
#include <vector>

#include "core/algorithms/cfd/cfdfinder/model/expansion/expansion_strategy.h"
#include "core/algorithms/cfd/cfdfinder/types/inverted_cluster_maps.h"

namespace algos::cfdfinder {

class RangePatternExpansion : public ExpansionStrategy {
private:
    using SortedClustersId = std::vector<ClusterId>;
    using SortedClustersIdPtr = std::shared_ptr<SortedClustersId>;
    std::vector<SortedClustersIdPtr> sorted_clusters_ids_;

public:
    RangePatternExpansion(InvertedClusterMaps const& inverted_cluster_maps,
                          RowsPtr&& compressed_records);

    Pattern GenerateNullPattern(BitSet const& attributes) const override;
    void ExpandAndProcess(Pattern&& parent_pattern, Frontier& frontier, Row const& inverted_pli_rhs,
                          PruningStrategy& pruning_strategy) override;
};

}  // namespace algos::cfdfinder
