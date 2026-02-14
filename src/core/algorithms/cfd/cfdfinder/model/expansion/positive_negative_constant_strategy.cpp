#include "core/algorithms/cfd/cfdfinder/model/expansion/positive_negative_constant_strategy.h"

#include <cstddef>

#include <boost/unordered/unordered_flat_map.hpp>

#include "core/algorithms/cfd/cfdfinder/model/pattern/constant_entry.h"
#include "core/algorithms/cfd/cfdfinder/model/pattern/negative_constant_entry.h"

namespace algos::cfdfinder {
void PositiveNegativeConstantExpansion::ProcessForId(Entries& buffer_entries, size_t replaced_pos,
                                                     Frontier& frontier,
                                                     Row const& inverted_pli_rhs,
                                                     PruningStrategy& pruning_strategy, size_t id,
                                                     std::vector<int>&& valid_constants,
                                                     Cover const& cover) const {
    std::vector<size_t> cluster_representatives = GetClusterRepresentatives(id, cover);

    std::vector<int> final_constants = FilterForSupport(
            std::move(valid_constants), pruning_strategy, cluster_representatives, cover);

    if (final_constants.empty()) {
        return;
    }
    std::vector<std::pair<int, boost::dynamic_bitset<>>> results =
            CalculateCoverMasks(std::move(final_constants), cluster_representatives, cover);

    for (auto const& [constant, cover_mask] : results) {
        auto inverted_cover_mask = ~cover_mask;
        Cover child_cover;
        child_cover.reserve(inverted_cover_mask.count());
        util::ForEachIndex(inverted_cover_mask,
                           [&](size_t cluster_id) { child_cover.push_back(cover[cluster_id]); });

        Entries new_entries = buffer_entries;
        new_entries[replaced_pos].entry = std::make_shared<NegativeConstantEntry>(constant);

        Pattern child(std::move(new_entries), std::move(child_cover), inverted_pli_rhs);

        frontier.Emplace(std::move(child));
    }
}

std::vector<int> PositiveNegativeConstantExpansion::FilterForSupport(
        std::vector<int>&& valid_constants, PruningStrategy const& pruning_strategy,
        std::vector<size_t> const& cluster_representatives, Cover const& cover) const {
    boost::unordered_flat_map<int, size_t> accumulated_support;
    accumulated_support.reserve(valid_constants.size());
    for (int val : valid_constants) {
        accumulated_support[val] = 0;
    }

    for (size_t cluster_id = 0; cluster_id < cover.size(); ++cluster_id) {
        int constant = cluster_representatives[cluster_id];
        if (auto it = accumulated_support.find(constant); it != accumulated_support.end()) {
            it->second += cover[cluster_id].size();
        }
    }
    auto const num_rows = columns_.GetColumn(0).size();
    std::vector<int> final_values;
    final_values.reserve(valid_constants.size());
    for (int val : valid_constants) {
        size_t support = accumulated_support[val];
        if (pruning_strategy.IsPatternWorthConsidering(num_rows - support)) {
            final_values.push_back(val);
        }
    }
    return final_values;
}

void PositiveNegativeConstantExpansion::ExpandAndProcess(Pattern&& parent_pattern,
                                                         Frontier& frontier,
                                                         Row const& inverted_pli_rhs,
                                                         PruningStrategy& pruning_strategy) {
    auto parent_entries = parent_pattern.GetEntries();
    auto copy_parent_entries = std::make_shared<Entries>(parent_pattern.GetEntries());

    for (size_t i = 0; i < parent_entries.size(); ++i) {
        auto const& item = parent_entries[i];
        if (item.entry->IsConstantType()) {
            continue;
        }

        boost::dynamic_bitset<> unique_ids =
                CalculateUniqueConstants(item.id, parent_pattern.GetCover());
        auto valid_pos_constants = FilterValidConstants(
                parent_entries, copy_parent_entries, i, unique_ids, frontier, pruning_strategy,
                [](int val) { return std::make_shared<ConstantEntry>(val); });

        if (!valid_pos_constants.empty()) {
            ConstantExpansion::ProcessForId(
                    parent_entries, i, frontier, inverted_pli_rhs, pruning_strategy, item.id,
                    std::move(valid_pos_constants), parent_pattern.GetCover());
        }

        auto valid_neg_constants = FilterValidConstants(
                parent_entries, copy_parent_entries, i, unique_ids, frontier, pruning_strategy,
                [](int val) { return std::make_shared<NegativeConstantEntry>(val); });

        if (!valid_neg_constants.empty()) {
            PositiveNegativeConstantExpansion::ProcessForId(
                    parent_entries, i, frontier, inverted_pli_rhs, pruning_strategy, item.id,
                    std::move(valid_neg_constants), parent_pattern.GetCover());
        }
    }
}

}  // namespace algos::cfdfinder
