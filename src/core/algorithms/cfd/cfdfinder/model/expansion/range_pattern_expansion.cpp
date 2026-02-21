#include "core/algorithms/cfd/cfdfinder/model/expansion/range_pattern_expansion.h"

#include <algorithm>
#include <cstddef>
#include <ranges>
#include <utility>

#include "core/algorithms/cfd/cfdfinder/model/pattern/range_entry.h"
#include "core/util/bitset_utils.h"

namespace algos::cfdfinder {

RangePatternExpansion::RangePatternExpansion(InvertedClusterMaps const& inverted_cluster_maps,
                                             RowsPtr&& compressed_records)
    : ExpansionStrategy(std::move(compressed_records)) {
    sorted_clusters_ids_.reserve(inverted_cluster_maps.size());
    for (size_t i = 0; i < inverted_cluster_maps.size(); ++i) {
        std::vector<std::pair<ClusterId, AttributeValue>> mappings(inverted_cluster_maps[i].begin(),
                                                                   inverted_cluster_maps[i].end());

        std::ranges::sort(mappings, [](auto const& a, auto const& b) {
            if (b.second.empty()) {
                return false;
            }
            if (a.second.empty()) {
                return true;
            }
            return a.second < b.second;
        });

        std::vector<size_t> sorted_cluster_ids(mappings.size());
        std::ranges::transform(mappings, sorted_cluster_ids.begin(),
                               [](auto const& entry) { return entry.first; });

        sorted_clusters_ids_.push_back(
                std::make_shared<SortedClustersId>(std::move(sorted_cluster_ids)));
    }
}

Pattern RangePatternExpansion::GenerateNullPattern(BitSet const& attributes) const {
    Entries entries;
    util::ForEachIndex(attributes, [&](size_t attr) {
        auto const& clusters = sorted_clusters_ids_.at(attr);
        entries.emplace_back(attr, std::make_shared<RangeEntry>(clusters, 0, clusters->size() - 1));
    });

    return Pattern(std::move(entries));
}

void RangePatternExpansion::ExpandAndProcess(Pattern parent_pattern, Frontier& frontier,
                                             Row const& inverted_pli_rhs,
                                             PruningStrategy& pruning_strategy) const {
    std::vector<std::pair<size_t, std::shared_ptr<Entry>>> replased;
    replased.reserve(parent_pattern.GetEntries().size() * 2);

    for (size_t i = 0; i < parent_pattern.GetEntries().size(); ++i) {
        auto range_entry =
                static_cast<RangeEntry const*>(parent_pattern.GetEntries()[i].entry.get());

        auto lentry = std::static_pointer_cast<RangeEntry>(range_entry->Clone());
        if (lentry->IncreaseLowerBound()) {
            replased.emplace_back(i, std::move(lentry));
        }

        auto rentry = std::static_pointer_cast<RangeEntry>(range_entry->Clone());
        if (rentry->DecreaseUpperBound()) {
            replased.emplace_back(i, std::move(rentry));
        }
    }

    auto parent_entries = parent_pattern.GetEntries();
    std::vector<std::pair<size_t, std::shared_ptr<Entry>>> valid_pairs;
    valid_pairs.reserve(replased.size());

    for (auto&& [id, entry] : replased) {
        std::swap(parent_entries[id].entry, entry);
        if (!pruning_strategy.ValidForProcessing(parent_entries) ||
            frontier.Contains(parent_entries)) {
            std::swap(parent_entries[id].entry, entry);
            continue;
        }
        std::swap(parent_entries[id].entry, entry);

        valid_pairs.emplace_back(id, std::move(entry));
    }

    if (valid_pairs.empty()) {
        return;
    }

    auto const& parent_cover = parent_pattern.GetCover();

    std::unordered_map<size_t, std::vector<int>> cluster_first_cache;
    for (auto const& [id, _] : valid_pairs) {
        if (cluster_first_cache.contains(id)) continue;
        auto const& column = columns_.GetColumn(parent_entries[id].id);
        std::vector<int> cf;
        cf.reserve(parent_cover.size());
        for (auto const& cluster : parent_cover) {
            cf.push_back(column[cluster[0]]);
        }
        cluster_first_cache[id] = std::move(cf);
    }

    for (auto& [id, new_entry] : valid_pairs) {
        auto child_entries = parent_entries;
        child_entries[id].entry = new_entry;

        auto const& cluster_first = cluster_first_cache[id];
        std::vector<Cluster> child_cover;
        size_t support = 0;
        for (size_t cluster_id = 0; cluster_id < parent_cover.size(); ++cluster_id) {
            if (new_entry->Matches(cluster_first[cluster_id])) {
                child_cover.push_back(parent_cover[cluster_id]);
                support += parent_cover[cluster_id].size();
            }
        }

        if (!pruning_strategy.IsPatternWorthConsidering(support)) {
            continue;
        }

        Pattern child(std::move(child_entries));
        child.SetCover(std::move(child_cover));
        child.UpdateKeepers(inverted_pli_rhs);
        frontier.Emplace(std::move(child));
    }
}

}  // namespace algos::cfdfinder
