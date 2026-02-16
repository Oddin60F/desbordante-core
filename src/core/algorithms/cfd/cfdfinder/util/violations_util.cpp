#include "core/algorithms/cfd/cfdfinder/util/violations_util.h"

#include <algorithm>
#include <cstddef>
#include <hash_table8.hpp>  // уже есть, используем emhash8::HashMap
// #include <unordered_map>   // можно убрать

#include "core/algorithms/fd/hycommon/util/pli_util.h"

namespace algos::cfdfinder::utils {
size_t CalculateViolations(Pattern const& pattern, Row const& inverted_rhs_pli) {
    size_t violations = 0;
    for (auto const& cluster : pattern.GetCover()) {
        // Заменяем std::unordered_map на emhash8::HashMap
        emhash8::HashMap<size_t, size_t> rhs_cluster_counts;
        rhs_cluster_counts.reserve(cluster.size());  // если метод есть — оставляем
        size_t max_cluster_size = 0;

        for (auto tuple : cluster) {
            auto cluster_id = inverted_rhs_pli[tuple];
            if (hy::PLIUtil::IsSingletonCluster(cluster_id)) continue;
            // operator[] работает аналогично std::unordered_map
            max_cluster_size = std::max(max_cluster_size, ++rhs_cluster_counts[cluster_id]);
        }
        violations +=
                (max_cluster_size > 0) ? (cluster.size() - max_cluster_size) : (cluster.size() - 1);
        // clear() не нужен, объект уничтожится после итерации
    }
    return violations;
}
}  // namespace algos::cfdfinder::utils