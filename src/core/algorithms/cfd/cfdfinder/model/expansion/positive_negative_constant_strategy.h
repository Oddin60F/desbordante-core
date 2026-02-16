#pragma once
#include <list>

#include "core/algorithms/cfd/cfdfinder/model/expansion/constant_expansion_strategy.h"
#include "core/algorithms/cfd/cfdfinder/types/hyfd_types.h"

namespace algos::cfdfinder {
class PositiveNegativeConstantExpansion : public ConstantExpansion {
private:
public:
    explicit PositiveNegativeConstantExpansion(RowsPtr&& compressed_records)
        : ConstantExpansion(std::move(compressed_records)) {}

    std::vector<ReplacedItem> ExpandPatterns(Pattern const& current_pattern) const override;
};

}  // namespace algos::cfdfinder
