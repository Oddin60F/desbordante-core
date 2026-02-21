#include "core/algorithms/cfd/cfdfinder/model/expansion/constant_expansion_strategy.h"

#include <cstddef>
#include <memory>

#include <boost/unordered/unordered_flat_map.hpp>

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

void ConstantExpansion::ExpandAndProcess(Pattern parent_pattern, Frontier& frontier,
                                         Row const& inverted_pli_rhs,
                                         PruningStrategy& pruning_strategy) const {
    auto parent_entries = parent_pattern.GetEntries();
    auto const& parent_cover = parent_pattern.GetCover();

    for (size_t i = 0; i < parent_entries.size(); ++i) {
        auto& entry = parent_entries[i].entry;

        if (entry->IsConstant()) {
            continue;
        }
        auto const& id = parent_entries[i].id;
        auto const& column = columns_.GetColumn(id);

        boost::dynamic_bitset<> unique_values(columns_.GetMaxValue(id));
        for (auto const& cluster : parent_cover) {
            int value = column[cluster[0]];
            if (!unique_values.test(value)) {
                unique_values.set(value);
            }
        }

        std::vector<int> valid_values;
        valid_values.reserve(unique_values.count());
        util::ForEachIndex(unique_values, [&](size_t value) {
            std::shared_ptr<Entry> buff_entry = std::make_shared<ConstantEntry>(value);
            std::swap(entry, buff_entry);

            if (frontier.Contains(parent_entries) ||
                !pruning_strategy.ValidForProcessing(parent_entries)) {
                std::swap(entry, buff_entry);
                return;
            }
            std::swap(entry, buff_entry);
            valid_values.push_back(value);
        });

        if (valid_values.empty()) {
            continue;
        }

        boost::unordered_flat_map<int, size_t> support_map;
        support_map.reserve(valid_values.size());
        for (int val : valid_values) {
            support_map[val] = 0;
        }

        std::vector<size_t> cluster_first;
        cluster_first.reserve(parent_cover.size());
        for (auto const& cluster : parent_cover) {
            cluster_first.push_back(column[cluster[0]]);
        }

        for (size_t cluster_id = 0; cluster_id < parent_cover.size(); ++cluster_id) {
            int val = cluster_first[cluster_id];
            auto it = support_map.find(val);
            if (it != support_map.end()) {
                it->second += parent_cover[cluster_id].size();
            }
        }

        std::vector<int> final_values;
        final_values.reserve(valid_values.size());
        for (int val : valid_values) {
            size_t support = support_map[val];
            if (pruning_strategy.IsPatternWorthConsidering(support)) {
                final_values.push_back(val);
            }
        }

        std::vector<std::pair<int, boost::dynamic_bitset<>>> results;
        results.reserve(final_values.size());
        for (int val : final_values) {
            results.emplace_back(val, boost::dynamic_bitset<>(parent_cover.size()));
        }

        std::unordered_map<int, size_t> value_to_idx;
        value_to_idx.reserve(final_values.size());
        for (size_t idx = 0; idx < final_values.size(); ++idx) {
            value_to_idx[final_values[idx]] = idx;
        }

        for (size_t cluster_id = 0; cluster_id < parent_cover.size(); ++cluster_id) {
            int val = cluster_first[cluster_id];
            auto it = value_to_idx.find(val);
            if (it != value_to_idx.end()) {
                size_t idx = it->second;
                std::get<1>(results[idx]).set(cluster_id);
            }
        }

        for (auto&& [constant, cover_mask] : results) {
            Entries new_entries = parent_entries;
            new_entries[i].entry = std::make_shared<ConstantEntry>(constant);

            Pattern child(std::move(new_entries));
            std::vector<Cluster> child_cover;
            child_cover.reserve(cover_mask.count());
            util::ForEachIndex(cover_mask, [&](size_t cluster_id) {
                child_cover.push_back(parent_cover[cluster_id]);
            });

            child.SetCover(std::move(child_cover));
            child.UpdateKeepers(inverted_pli_rhs);
            frontier.Emplace(std::move(child));
        }
    }
}
}  // namespace algos::cfdfinder
