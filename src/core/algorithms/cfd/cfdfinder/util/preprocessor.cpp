#include "algorithms/cfd/cfdfinder/util/preprocessor.h"

namespace algos::cfdfinder {
using namespace hy::util;

std::tuple<PLIs, Columns, Rows, std::vector<ClusterId>> Preprocess(
        ColumnLayoutRelationData* relation) {
    PLIs plis = BuildPLIs(relation);

    auto og_mapping = SortAndGetMapping(plis);

    auto const inverted_plis = BuildInvertedPlis(plis);

    auto pli_records = BuildRecordRepresentation(inverted_plis);

    return std::make_tuple(std::move(plis), std::move(inverted_plis), std::move(pli_records),
                           std::move(og_mapping));
}

boost::dynamic_bitset<> RestoreAgreeSet(boost::dynamic_bitset<> const& as,
                                        std::vector<ClusterId> const& og_mapping, size_t num_cols) {
    boost::dynamic_bitset<> mapped_as(num_cols);
    for (size_t i = as.find_first(); i != boost::dynamic_bitset<>::npos; i = as.find_next(i)) {
        mapped_as.set(og_mapping[i]);
    }
    return mapped_as;
}

}  // namespace algos::cfdfinder