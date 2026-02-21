#pragma once

#include "core/algorithms/cfd/cfdfinder/model/expansion/expansion_strategy.h"
#include "core/algorithms/cfd/cfdfinder/model/pattern/entry.h"
#include "core/algorithms/cfd/cfdfinder/types/hyfd_types.h"

namespace algos::cfdfinder {

class ConstantExpansion : public ExpansionStrategy {
public:
    explicit ConstantExpansion(RowsPtr&& compressed_records)
        : ExpansionStrategy(std::move(compressed_records)) {}

    Pattern GenerateNullPattern(BitSet const& attributes) const override;
    void ExpandAndProcess(Pattern parent_pattern, Frontier& frontier, Row const& inverted_pli_rhs,
                          PruningStrategy& pruning_strategy) const override;
};

}  // namespace algos::cfdfinder
