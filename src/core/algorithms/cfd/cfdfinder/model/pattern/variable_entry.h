#pragma once

#include <cstddef>
#include <string>

#include "core/algorithms/cfd/cfdfinder/model/pattern/entry.h"

namespace algos::cfdfinder {

class VariableEntry final : public Entry {
public:
    inline bool Matches([[maybe_unused]] size_t value) const override final {
        return true;
    }

    bool operator==(Entry const& other) const override final {
        return dynamic_cast<VariableEntry const*>(&other) != nullptr;
    }

    bool operator!=(Entry const& other) const {
        return !(*this == other);
    }

    size_t Hash() const override {
        return 0x9e3779b9;
    }

    bool IsConstant() const override {
        return false;
    }

    std::string ToString([[maybe_unused]] InvertedClusterMap const& cluster_map) const override {
        return std::string(kWildCard);
    }

    std::pair<boost::dynamic_bitset<>, size_t> GetCoverMask(
            std::vector<Cluster> const& parent_cover,
            std::vector<size_t> const& column) const override {
        boost::dynamic_bitset<> valid_cover_mask(parent_cover.size());
        size_t new_support = 0;
        for (size_t clusted_id = 0; clusted_id < parent_cover.size(); ++clusted_id) {
            if (VariableEntry::Matches(column[clusted_id])) {
                valid_cover_mask.set(clusted_id);
                new_support += parent_cover[clusted_id].size();
            }
        }

        return {std::move(valid_cover_mask), new_support};
    }
};
}  // namespace algos::cfdfinder
