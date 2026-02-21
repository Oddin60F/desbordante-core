#include "core/algorithms/cfd/cfdfinder/model/pattern/pattern.h"

#include <algorithm>
#include <numeric>
#include <tuple>
#include <unordered_set>

#include "core/algorithms/cfd/cfdfinder/util/violations_util.h"

namespace algos::cfdfinder {

void Pattern::UpdateCover(boost::dynamic_bitset<> const& used_rows) {
    for (auto& cluster : cover_) {
        std::erase_if(cluster, [&used_rows](int element) { return used_rows.test(element); });
    }

    std::erase_if(cover_, [](auto const& c) { return c.empty(); });

    support_ = static_cast<double>(GetNumCover());
}

void Pattern::UpdateKeepers(Row const& inverted_pli_rhs) {
    num_keepers_ = support_ - utils::CalculateViolations(*this, inverted_pli_rhs);
}

size_t Pattern::GetNumCover() const {
    return std::accumulate(cover_.begin(), cover_.end(), 0u,
                           [](size_t sum, Cluster const& cluster) { return sum + cluster.size(); });
}

bool Pattern::operator<(Pattern const& other) const noexcept {
    if (support_ != other.support_) {
        return support_ < other.support_;
    }
    if (num_keepers_ != other.num_keepers_) {
        return num_keepers_ < other.num_keepers_;
    }
    for (size_t i = 0; i < entries_.size(); i++) {
        auto const& this_entry = entries_[i].entry;
        auto const& other_entry = other.entries_[i].entry;
        int cmp = other_entry->CompareTo(*this_entry);
        if (cmp != 0) {
            return cmp < 0 ? true : false;
        }
    }
    return 0;
}
}  // namespace algos::cfdfinder
