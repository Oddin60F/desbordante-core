#pragma once

#include <cstddef>
#include <string>
#include <string_view>

#include <boost/functional/hash.hpp>

#include "core/algorithms/cfd/cfdfinder/model/pattern/entry.h"

namespace algos::cfdfinder {

class NegativeConstantEntry final : public Entry {
private:
    inline static constexpr std::string_view kNegationSign = "¬";
    size_t constant_;

public:
    explicit NegativeConstantEntry(size_t constant) : constant_(constant) {}

    inline bool Matches(size_t value) const override final {
        return constant_ != value;
    }

    bool operator==(Entry const& other) const override final {
        auto const* other_constant = dynamic_cast<NegativeConstantEntry const*>(&other);
        return other_constant != nullptr && constant_ == other_constant->constant_;
    }

    size_t Hash() const override {
        return boost::hash_value(constant_) * 31;
    }

    size_t GetConstant() const {
        return constant_;
    }

    bool IsConstant() const override {
        return true;
    }

    std::string ToString(InvertedClusterMap const& cluster_map) const override {
        std::string value = cluster_map.at(constant_);

        return std::string(kNegationSign) +
               (!value.empty() ? value : std::string(kNullRepresentation));
    }

    std::pair<boost::dynamic_bitset<>, size_t> GetCoverMask(
            std::vector<Cluster> const& parent_cover,
            std::vector<size_t> const& column) const override {
        boost::dynamic_bitset<> valid_cover_mask(parent_cover.size());
        size_t new_support = 0;
        for (size_t clusted_id = 0; clusted_id < parent_cover.size(); ++clusted_id) {
            if (NegativeConstantEntry::Matches(column[clusted_id])) {
                valid_cover_mask.set(clusted_id);
                new_support += parent_cover[clusted_id].size();
            }
        }

        return {std::move(valid_cover_mask), new_support};
    }
};
}  // namespace algos::cfdfinder
