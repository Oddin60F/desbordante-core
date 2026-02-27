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
                std::make_shared<SortedClustersId const>(std::move(sorted_cluster_ids)));
    }
}

Entries RangePatternExpansion::GenerateNullEntries(BitSet const& attributes) const {
    Entries null_entries;
    util::ForEachIndex(attributes, [&](size_t attr) {
        auto const& clusters = sorted_clusters_ids_.at(attr);
        null_entries.emplace_back(attr,
                                  std::make_shared<RangeEntry>(clusters, 0, clusters->size() - 1));
    });

    return null_entries;
}

void RangePatternExpansion::ExpandAndProcess(Pattern&& parent_pattern, Frontier& frontier,
                                             Row const& inverted_pli_rhs,
                                             PruningStrategy& pruning_strategy) {
    auto parent_entries = parent_pattern.GetEntries();
    auto copy_parent_entries = std::make_shared<Entries>(parent_pattern.GetEntries());

    for (size_t i = 0; i < parent_entries.size(); ++i) {
        std::vector<std::shared_ptr<Entry>> replased;
        auto const& item = parent_entries[i];
        auto range_entry = static_cast<RangeEntry const*>(item.entry.get());

        auto lentry = std::static_pointer_cast<RangeEntry>(range_entry->Clone());
        if (lentry->IncreaseLowerBound()) {
            replased.push_back(std::move(lentry));
        }

        auto rentry = std::static_pointer_cast<RangeEntry>(range_entry->Clone());
        if (rentry->DecreaseUpperBound()) {
            replased.push_back(std::move(rentry));
        }

        std::vector<std::shared_ptr<Entry>> valid_entries;

        for (auto&& range_entry : replased) {
            std::shared_ptr<Entry> new_entry = range_entry;
            std::shared_ptr<Entry>& original_entry = parent_entries[i].entry;

            std::swap(original_entry, new_entry);
            PruningStrategy::ValidationContext ctx{parent_entries, i, original_entry,
                                                   copy_parent_entries};
            //&& !frontier.Contains(entries_buffer)
            if (pruning_strategy.ValidForProcessing(std::move(ctx))) {
                valid_entries.push_back(range_entry);
            }

            std::swap(original_entry, new_entry);
        }

        if (valid_entries.empty()) {
            continue;
        }

        auto const& cover = parent_pattern.GetCover();
        std::vector<size_t> first_vals = GetClusterRepresentatives(item.id, cover);

        for (auto&& new_entry : valid_entries) {
            size_t support = 0;
            boost::dynamic_bitset<> cover_mask(cover.size());
            auto* range_new = static_cast<RangeEntry*>(new_entry.get());
            size_t low = range_new->GetLowerBound();
            size_t high = range_new->GetUpperBound();

            for (size_t cluster_id = 0; cluster_id < cover.size(); ++cluster_id) {
                size_t val = first_vals[cluster_id];

                if (val >= low && val <= high) {
                    support += cover[cluster_id].size();
                    cover_mask.set(cluster_id);
                }
            }

            if (!pruning_strategy.IsPatternWorthConsidering(support)) continue;

            Cover child_cover;
            child_cover.reserve(cover_mask.count());
            util::ForEachIndex(cover_mask, [&](size_t cluster_id) {
                child_cover.push_back(cover[cluster_id]);
            });

            Entries child_entries = parent_entries;
            child_entries[i].entry = new_entry;

            Pattern child(std::move(child_entries), std::move(child_cover), inverted_pli_rhs);
            frontier.Emplace(std::move(child));
        }
    }
}

}  // namespace algos::cfdfinder
