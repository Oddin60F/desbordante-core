#pragma once

#include <cstddef>
#include <utility>
#include <vector>

#include "core/algorithms/cfd/cfdfinder/model/expansion/expansion_strategy.h"
#include "core/algorithms/cfd/cfdfinder/model/pattern/entry.h"
#include "core/algorithms/cfd/cfdfinder/model/pruning/pruning_strategy.h"
#include "core/algorithms/cfd/cfdfinder/types/hyfd_types.h"

namespace algos::cfdfinder {

class ConstantExpansion : public ExpansionStrategy {
protected:
    boost::dynamic_bitset<> CalculateUniqueConstants(size_t column_id, Cover const& cover) const;
    virtual std::vector<int> FilterForSupport(std::vector<int>&& valid_constants,
                                              PruningStrategy const& pruning_strategy,
                                              std::vector<size_t> const& cluster_representatives,
                                              Cover const& cover) const;

    std::vector<std::pair<int, boost::dynamic_bitset<>>> CalculateCoverMasks(
            std::vector<int>&& processed_values, std::vector<size_t> const& cluster_representatives,
            Cover const& cover) const;

    virtual void ProcessForId(Entries& buffer_entries, size_t replaced_index, Frontier& frontier,
                              Row const& inverted_pli_rhs, PruningStrategy& pruning_strategy,
                              size_t id, std::vector<int>&& valid_constants,
                              Cover const& cover) const;

    Entries GenerateNullEntries(BitSet const& attributes) const override;

    template <typename EntryCreator>
    std::vector<int> FilterValidConstants(Entries& entries_buffer,
                                          std::shared_ptr<Entries> const& parent_entries,
                                          size_t replaced_index,
                                          boost::dynamic_bitset<> const& constants,
                                          Frontier const& frontier,
                                          PruningStrategy& pruning_strategy,
                                          EntryCreator&& create_entry) const {
        std::vector<int> valid_constants;
        util::ForEachIndex(constants, [&](size_t constant) {
            std::shared_ptr<Entry> new_entry = create_entry(constant);
            std::shared_ptr<Entry>& original_entry = entries_buffer[replaced_index].entry;

            std::swap(original_entry, new_entry);
            PruningStrategy::ValidationContext ctx{entries_buffer, replaced_index, original_entry,
                                                   parent_entries};

            if (pruning_strategy.ValidForProcessing(std::move(ctx))) {
                valid_constants.push_back(constant);
            }

            std::swap(original_entry, new_entry);
        });

        return valid_constants;
    }

public:
    explicit ConstantExpansion(RowsPtr&& compressed_records)
        : ExpansionStrategy(std::move(compressed_records)) {}

    void ExpandAndProcess(Pattern&& parent_pattern, Frontier& frontier, Row const& inverted_pli_rhs,
                          PruningStrategy& pruning_strategy) override;
};

}  // namespace algos::cfdfinder
