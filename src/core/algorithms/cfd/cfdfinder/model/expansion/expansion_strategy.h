#pragma once

#include <utility>
#include <vector>

#include <boost/dynamic_bitset.hpp>

#include "core/algorithms/cfd/cfdfinder/model/pattern/pattern.h"
#include "core/algorithms/cfd/cfdfinder/model/pattern/pattern_tableau.h"
#include "core/algorithms/cfd/cfdfinder/model/pruning/pruning_strategy.h"
#include "core/algorithms/cfd/cfdfinder/types/frontier.h"

namespace algos::cfdfinder {

class ExpansionStrategy {
protected:
    class ColumnRecords {
    private:
        std::vector<std::vector<model::ColumnIndex>> columns_;
        std::vector<model::ColumnIndex> max_values_;

    public:
        ColumnRecords(RowsPtr&& rows) {
            size_t num_rows = rows->size();
            size_t num_cols = (*rows)[0].size();

            columns_.resize(num_cols);
            max_values_.reserve(num_cols);

            for (size_t col = 0; col < num_cols; ++col) {
                columns_[col].reserve(num_rows);
                model::ColumnIndex max_val = 0;

                for (size_t row = 0; row < num_rows; ++row) {
                    auto val = (*rows)[row][col];
                    columns_[col].push_back(val);
                    max_val = std::max(max_val, val);
                }
                max_values_.push_back(max_val + 1);
            }
        }

        std::vector<model::ColumnIndex> const& GetColumn(size_t col) const {
            return columns_[col];
        }

        model::ColumnIndex GetMaxValue(size_t col) const {
            return max_values_[col];
        }
    };

    template <typename Candidate, typename EntryCreator>
    std::vector<Candidate> FilterValidConstants(Entries& entries_buffer,
                                                std::shared_ptr<Entries> const& parent_entries,
                                                size_t replaced_index,
                                                std::vector<Candidate> const& constants,
                                                Frontier const& frontier,
                                                PruningStrategy& pruning_strategy,
                                                EntryCreator&& create_entry) const {
        std::vector<Candidate> valid_constants;
        for (auto constant : constants) {
            std::shared_ptr<Entry> new_entry = create_entry(constant);
            std::shared_ptr<Entry>& original_entry = entries_buffer[replaced_index].entry;

            std::swap(original_entry, new_entry);
            PruningStrategy::ValidationContext ctx{entries_buffer, replaced_index,
                                                   create_entry(constant), parent_entries};

            if (pruning_strategy.ValidForProcessing(ctx) && !frontier.Contains(entries_buffer)) {
                valid_constants.push_back(constant);
            }

            std::swap(original_entry, new_entry);
        }
        return valid_constants;
    }

    std::vector<size_t> GetClusterRepresentatives(size_t id,
                                                  std::vector<Cluster> const& cover) const {
        auto const& column = columns_.GetColumn(id);

        std::vector<size_t> cluster_representatives;
        cluster_representatives.reserve(cover.size());
        for (auto const& cluster : cover) {
            cluster_representatives.push_back(column[cluster[0]]);
        }
        return cluster_representatives;
    }

    ColumnRecords columns_;

public:
    using BitSet = boost::dynamic_bitset<>;

    ExpansionStrategy(RowsPtr&& rows) : columns_(std::move(rows)) {}

    virtual ~ExpansionStrategy() = default;
    virtual Pattern GenerateNullPattern(BitSet const& attributes) const = 0;
    virtual void ExpandAndProcess(Pattern&& parent_pattern, Frontier& frontier,
                                  Row const& inverted_pli_rhs,
                                  PruningStrategy& pruning_strategy) = 0;
};

}  // namespace algos::cfdfinder
