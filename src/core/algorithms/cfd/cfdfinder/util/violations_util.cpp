#include "core/algorithms/cfd/cfdfinder/util/violations_util.h"

#include <algorithm>
#include <cstddef>
#include <unordered_map>

#include "core/algorithms/fd/hycommon/util/pli_util.h"

namespace algos::cfdfinder::utils {
size_t CalculateViolations(Pattern const& pattern, Row const& inverted_rhs_pli) {
    size_t violations = 0;
    std::unordered_map<size_t, size_t> rhs_cluster_counts;
    for (auto const& cluster : pattern.GetCover()) {
        size_t max_cluster_size = 0;

        for (auto tuple : cluster) {
            auto cluster_id = inverted_rhs_pli[tuple];

            if (hy::PLIUtil::IsSingletonCluster(cluster_id)) continue;

            max_cluster_size = std::max(max_cluster_size, ++rhs_cluster_counts[cluster_id]);
        }
        violations +=
                (max_cluster_size > 0) ? (cluster.size() - max_cluster_size) : (cluster.size() - 1);
        rhs_cluster_counts.clear();
    }

    return violations;
}

std::vector<size_t> CalculateViolations(std::vector<Cluster> const& cover,
                                        Row const& inverted_rhs_pli) {
    std::vector<size_t> violations;
    violations.reserve(cover.size());
    std::unordered_map<size_t, size_t> rhs_cluster_counts;
    for (auto const& cluster : cover) {
        size_t max_cluster_size = 0;

        for (auto tuple : cluster) {
            auto cluster_id = inverted_rhs_pli[tuple];

            if (hy::PLIUtil::IsSingletonCluster(cluster_id)) continue;

            max_cluster_size = std::max(max_cluster_size, ++rhs_cluster_counts[cluster_id]);
        }
        size_t cluster_violations =
                (max_cluster_size > 0) ? (cluster.size() - max_cluster_size) : (cluster.size() - 1);
        violations.push_back(cluster_violations);
        rhs_cluster_counts.clear();
    }

    return violations;
}

}  // namespace algos::cfdfinder::utils
