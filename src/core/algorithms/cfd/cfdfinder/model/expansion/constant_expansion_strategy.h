#pragma once

#include <list>

#include "core/algorithms/cfd/cfdfinder/model/expansion/expansion_strategy.h"
#include "core/algorithms/cfd/cfdfinder/model/pattern/entry.h"
#include "core/algorithms/cfd/cfdfinder/types/hyfd_types.h"

namespace algos::cfdfinder {

class ConstantExpansion : public ExpansionStrategy {
protected:
    RowsPtr compressed_records_;

public:
    explicit ConstantExpansion(RowsPtr&& compressed_records)
        : compressed_records_(std::move(compressed_records)) {}

    Pattern GenerateNullPattern(BitSet const& attributes) const override;
    std::vector<Child> GetChildPatterns(Pattern const& current_pattern) const override;
};

}  // namespace algos::cfdfinder
