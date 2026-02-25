#pragma once

#include <vector>

#include "core/algorithms/cfd/cfdfinder/types/hyfd_types.h"

namespace algos::cfdfinder {
class ColumnRecords {
private:
    std::vector<std::vector<model::ColumnIndex>> columns_;
    std::vector<model::ColumnIndex> max_values_;

public:
    explicit ColumnRecords(RowsPtr&& rows) {
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
}  // namespace algos::cfdfinder
