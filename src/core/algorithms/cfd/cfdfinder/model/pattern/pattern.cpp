#include "core/algorithms/cfd/cfdfinder/model/pattern/pattern.h"

#include <algorithm>
#include <numeric>
#include <tuple>
#include <unordered_set>

#include "core/algorithms/cfd/cfdfinder/util/violations_util.h"

namespace algos::cfdfinder {

double Pattern::UpdateCover(std::unordered_set<int> const& used_rows) {
    for (auto& cluster : cover_) {
        std::erase_if(cluster, [&used_rows](int element) { return used_rows.contains(element); });

        if (PatternDebugController::IsDebugEnabled()) {
            std::ranges::sort(cluster);
        }
    }

    std::erase_if(cover_, [](auto const& c) { return c.empty(); });
    if (PatternDebugController::IsDebugEnabled()) {
        std::ranges::sort(cover_);
    }

    support_ = static_cast<double>(GetNumCover());
    return support_;
}

void Pattern::UpdateKeepers(Row const& inverted_pli_rhs) {
    num_keepers_ = support_ - utils::CalculateViolations(*this, inverted_pli_rhs);
}

size_t Pattern::GetNumCover() const {
    return std::accumulate(cover_.begin(), cover_.end(), 0u,
                           [](size_t sum, Cluster const& cluster) { return sum + cluster.size(); });
}

bool Pattern::operator<(Pattern const& other) const noexcept {
    if (PatternDebugController::IsDebugEnabled()) {
        return std::tie(support_, num_keepers_, number_) <
               std::tie(other.support_, other.num_keepers_, other.number_);
    } else {
        return std::tie(support_, num_keepers_) < std::tie(other.support_, other.num_keepers_);
    }
}
}  // namespace algos::cfdfinder
