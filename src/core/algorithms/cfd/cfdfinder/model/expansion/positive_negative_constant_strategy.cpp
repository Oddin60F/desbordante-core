#include "core/algorithms/cfd/cfdfinder/model/expansion/positive_negative_constant_strategy.h"

#include <cstddef>

#include "core/algorithms/cfd/cfdfinder/model/pattern/constant_entry.h"
#include "core/algorithms/cfd/cfdfinder/model/pattern/negative_constant_entry.h"

namespace algos::cfdfinder {

std::vector<ExpansionStrategy::ReplacedItem> PositiveNegativeConstantExpansion::ExpandPatterns(
        Pattern const& current_pattern) const {
    std::vector<ReplacedItem> result;
    auto const& entries = current_pattern.GetEntries();

    for (size_t i = 0; i < entries.size(); ++i) {
        auto const& item = entries[i];
        if (item.entry->IsConstant()) {
            continue;
        }
        for (auto const& cluster : current_pattern.GetCover()) {
            int value = (*compressed_records_)[cluster[0]][item.id];

            result.emplace_back(i, std::make_shared<ConstantEntry>(value));
            result.emplace_back(i, std::make_shared<NegativeConstantEntry>(value));
        }
    }
    return result;
}
}  // namespace algos::cfdfinder
