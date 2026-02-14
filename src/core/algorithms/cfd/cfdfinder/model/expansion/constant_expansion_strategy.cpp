#include "core/algorithms/cfd/cfdfinder/model/expansion/constant_expansion_strategy.h"

#include <cstddef>

#include "core/algorithms/cfd/cfdfinder/model/pattern/constant_entry.h"
#include "core/algorithms/cfd/cfdfinder/model/pattern/variable_entry.h"
#include "core/util/bitset_utils.h"

namespace algos::cfdfinder {

Pattern ConstantExpansion::GenerateNullPattern(BitSet const& attributes) const {
    Entries entries;
    util::ForEachIndex(attributes, [&entries](size_t attr) {
        entries.emplace_back(attr, std::make_shared<VariableEntry>());
    });

    return Pattern(std::move(entries));
}

std::vector<ExpansionStrategy::Child> ConstantExpansion::GetChildPatterns(
        Pattern const& current_pattern) const {
    std::vector<Child> result;
    auto const& entries = current_pattern.GetEntries();

    for (size_t i = 0; i < entries.size(); ++i) {
        auto const& item = entries[i];
        if (item.entry->IsConstant()) {
            continue;
        }
        for (auto const& cluster : current_pattern.GetCover()) {
            int value = (*compressed_records_)[cluster[0]][item.id];

            result.emplace_back(i, std::make_shared<ConstantEntry>(value));
        }
    }
    return result;
}

}  // namespace algos::cfdfinder
