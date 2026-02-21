#pragma once

#include <cstddef>
#include <unordered_set>

#include <boost/unordered/unordered_flat_set.hpp>

#include "core/algorithms/cfd/cfdfinder/model/pruning/pruning_strategy.h"

namespace algos::cfdfinder {

class LegacyPruning : public PruningStrategy {
private:
    double min_support_;
    double min_confidence_;
    double cumulative_support_;
    size_t num_tuples_;
    boost::unordered_flat_set<Entries, std::hash<Entries>> visited_;

public:
    LegacyPruning(double min_support, double min_confidence, size_t num_tuples)
        : min_support_(min_support),
          min_confidence_(min_confidence),
          cumulative_support_(0),
          num_tuples_(num_tuples) {}

    void StartNewTableau([[maybe_unused]] Candidate const& candidate) override;

    bool HasEnoughPatterns([[maybe_unused]] std::vector<Pattern> const& tableau) override;

    bool IsPatternWorthConsidering(double new_support) override;

    bool TryAdding(Pattern& pattern) override;

    bool ValidForProcessing(Entries const& entries) override;

    bool ContinueGeneration(PatternTableau const& currentTableau) override;

    std::shared_ptr<PruningStrategy> Clone() const override {
        return std::make_shared<LegacyPruning>(*this);
    }
};
}  // namespace algos::cfdfinder
