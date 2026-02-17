#pragma once

#include <cstddef>
#include <string>
#include <string_view>

#include <boost/dynamic_bitset.hpp>

#include "core/algorithms/cfd/cfdfinder/types/cluster.h"
#include "core/algorithms/cfd/cfdfinder/types/hyfd_types.h"
#include "core/algorithms/cfd/cfdfinder/types/inverted_cluster_maps.h"

namespace algos::cfdfinder {
class Entry {
protected:
    inline static constexpr std::string_view kNullRepresentation = "null";
    inline static constexpr std::string_view kWildCard = "_";

public:
    virtual ~Entry() = default;

    virtual bool Matches(size_t value) const = 0;
    virtual size_t Hash() const = 0;
    virtual bool operator==(Entry const& other) const = 0;
    virtual bool IsConstant() const = 0;
    virtual std::string ToString(InvertedClusterMap const& cluster_map) const = 0;

    virtual std::pair<boost::dynamic_bitset<>, size_t> GetCoverMask(
            std::vector<Cluster> const& parent_cover, Row const& column) const {
        boost::dynamic_bitset<> cover_mask(parent_cover.size());
        size_t new_support = 0;
        for (size_t cluster_id = 0; cluster_id < parent_cover.size(); ++cluster_id) {
            auto const& cluster = parent_cover[cluster_id];
            if (Matches(column[cluster[0]])) {
                cover_mask.set(cluster_id);
                new_support += parent_cover[cluster_id].size();
            }
        }
        return {std::move(cover_mask), new_support};
    }
};

}  // namespace algos::cfdfinder
