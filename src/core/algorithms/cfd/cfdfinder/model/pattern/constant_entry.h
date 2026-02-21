#pragma once

#include <cstddef>
#include <string>

#include <boost/functional/hash.hpp>

#include "core/algorithms/cfd/cfdfinder/model/pattern/entry.h"

namespace algos::cfdfinder {

class ConstantEntry final : public Entry {
private:
    size_t constant_;

public:
    explicit ConstantEntry(size_t constant) : constant_(constant) {}

    inline bool Matches(size_t value) const override final {
        return constant_ == value;
    }

    bool operator==(Entry const& other) const override final {
        auto const* other_constant = dynamic_cast<ConstantEntry const*>(&other);
        return other_constant != nullptr && constant_ == other_constant->constant_;
    }

    bool operator!=(Entry const& other) const {
        return !(*this == other);
    }

    size_t Hash() const override {
        return boost::hash_value(constant_);
    }

    size_t GetConstant() const {
        return constant_;
    }

    bool IsConstant() const override {
        return true;
    }

    int GetTypeRank() const override {
        return 1;
    }

    int CompareTo(Entry const& other) const override {
        int rank = GetTypeRank();
        int other_rank = other.GetTypeRank();
        if (rank != other_rank) return rank - other_rank;

        auto const& other_constant = static_cast<ConstantEntry const&>(other);
        if (constant_ < other_constant.constant_) return -1;
        if (constant_ > other_constant.constant_) return 1;
        return 0;
    }

    std::string ToString(InvertedClusterMap const& cluster_map) const override {
        std::string value = cluster_map.at(constant_);

        return !value.empty() ? value : std::string(kNullRepresentation);
    }
};
}  // namespace algos::cfdfinder
