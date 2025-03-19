#pragma once

#include "algorithms/fd/hycommon/types.h"
#include "fd/hycommon/preprocessor.h"

namespace algos::cfdfinder {
using namespace hy;
std::tuple<PLIs, Columns, Rows, std::vector<ClusterId>> Preprocess(
        ColumnLayoutRelationData* relation);
boost::dynamic_bitset<> RestoreAgreeSet(boost::dynamic_bitset<> const& as,
                                        std::vector<ClusterId> const& og_mapping, size_t num_cols);
}  // namespace algos::cfdfinder
