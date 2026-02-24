#pragma once

#include "core/algorithms/cfd/cfdfinder/model/pattern/entry.h"
#include "core/algorithms/cfd/cfdfinder/model/pattern/pattern.h"
#include "core/algorithms/cfd/cfdfinder/types/cluster.h"
#include "core/algorithms/cfd/cfdfinder/types/hyfd_types.h"

namespace algos::cfdfinder::utils {
size_t CalculateViolations(Pattern const& pattern, Row const& inverted_rhs_pli);
std::vector<size_t> CalculateViolations(std::vector<Cluster> const& cover,
                                        Row const& inverted_rhs_pli);
}  // namespace algos::cfdfinder::utils
