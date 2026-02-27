#include "core/algorithms/cfd/cfdfinder/model/expansion/constant_expansion_strategy.h"

#include <cstddef>
#include <memory>

#include <boost/unordered/unordered_flat_map.hpp>

#include "core/algorithms/cfd/cfdfinder/model/pattern/constant_entry.h"
#include "core/algorithms/cfd/cfdfinder/model/pattern/variable_entry.h"
#include "core/util/bitset_utils.h"

namespace algos::cfdfinder {

Entries ConstantExpansion::GenerateNullEntries(BitSet const& attributes) const {
    Entries null_entries;
    util::ForEachIndex(attributes, [&](size_t attr) {
        null_entries.emplace_back(attr, std::make_shared<VariableEntry>());
    });

    return null_entries;
}

boost::dynamic_bitset<> ConstantExpansion::CalculateUniqueConstants(size_t column_id,
                                                                    Cover const& cover) const {
    boost::dynamic_bitset<> unique_constants(columns_.GetMaxValue(column_id));
    auto const& column = columns_.GetColumn(column_id);

    for (auto const& cluster : cover) {
        unique_constants.set(column[cluster[0]]);
    }

    return unique_constants;
}

std::vector<int> ConstantExpansion::FilterForSupport(
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

    std::vector<int> final_values;
    final_values.reserve(valid_constants.size());
    for (int val : valid_constants) {
        size_t support = accumulated_support[val];
        if (pruning_strategy.IsPatternWorthConsidering(support)) {
            final_values.push_back(val);
        }
    }
    return final_values;
}

std::vector<std::pair<int, boost::dynamic_bitset<>>> ConstantExpansion::CalculateCoverMasks(
        std::vector<int>&& processed_values, std::vector<size_t> const& cluster_representatives,
        Cover const& cover) const {
    boost::unordered_flat_map<int, size_t> value_to_idx;
    value_to_idx.reserve(processed_values.size());
    for (size_t idx = 0; idx < processed_values.size(); ++idx) {
        value_to_idx[processed_values[idx]] = idx;
    }

    std::vector<std::pair<int, boost::dynamic_bitset<>>> cover_masks;
    cover_masks.reserve(processed_values.size());
    for (int val : processed_values) {
        cover_masks.emplace_back(val, boost::dynamic_bitset<>(cover.size()));
    }

    for (size_t cluster_id = 0; cluster_id < cover.size(); ++cluster_id) {
        int val = cluster_representatives[cluster_id];
        auto it = value_to_idx.find(val);
        if (it != value_to_idx.end()) {
            size_t idx = it->second;
            std::get<1>(cover_masks[idx]).set(cluster_id);
        }
    }
    return cover_masks;
}

void ConstantExpansion::ProcessForId(Entries& buffer_entries, size_t replaced_pos,
                                     Frontier& frontier, Row const& inverted_pli_rhs,
                                     PruningStrategy& pruning_strategy, size_t id,
                                     std::vector<int>&& valid_constants, Cover const& cover) const {
    auto cluster_representatives = GetClusterRepresentatives(id, cover);

    std::vector<int> final_constants = FilterForSupport(
            std::move(valid_constants), pruning_strategy, cluster_representatives, cover);

    if (final_constants.empty()) {
        return;
    }
    std::vector<std::pair<int, boost::dynamic_bitset<>>> results =
            CalculateCoverMasks(std::move(final_constants), cluster_representatives, cover);

    for (auto const& [constant, cover_mask] : results) {
        Cover child_cover;
        child_cover.reserve(cover_mask.count());
        util::ForEachIndex(cover_mask,
                           [&](size_t cluster_id) { child_cover.push_back(cover[cluster_id]); });

        Entries new_entries = buffer_entries;
        new_entries[replaced_pos].entry = std::make_shared<ConstantEntry>(constant);

        Pattern child(std::move(new_entries), std::move(child_cover), inverted_pli_rhs);

        frontier.Emplace(std::move(child));
    }
}

void ConstantExpansion::ExpandAndProcess(Pattern&& parent_pattern, Frontier& frontier,
                                         Row const& inverted_pli_rhs,
                                         PruningStrategy& pruning_strategy) {
    auto entries_buffer = parent_pattern.GetEntries();
    auto copy_parent_entries = std::make_shared<Entries>(parent_pattern.GetEntries());

    for (size_t i = 0; i < entries_buffer.size(); ++i) {
        auto const& item = entries_buffer[i];
        if (item.entry->IsConstantType()) {
            continue;
        }

        boost::dynamic_bitset<> unique_ids =
                CalculateUniqueConstants(item.id, parent_pattern.GetCover());
        std::vector<int> valid_values = FilterValidConstants<int>(
                entries_buffer, copy_parent_entries, i, unique_ids, frontier, pruning_strategy,
                [](int val) { return std::make_shared<ConstantEntry>(val); });

        if (valid_values.empty()) {
            continue;
        }

        ProcessForId(entries_buffer, i, frontier, inverted_pli_rhs, pruning_strategy, item.id,
                     std::move(valid_values), parent_pattern.GetCover());
    }
}
}  // namespace algos::cfdfinder
