#include "core/algorithms/cfd/cfdfinder/model/pruning/support_independent_strategy.h"

#include <algorithm>

#include "core/algorithms/cfd/cfdfinder/candidate.h"
#include "core/algorithms/cfd/cfdfinder/model/pattern/pattern.h"
#include "core/algorithms/cfd/cfdfinder/model/pattern/pattern_tableau.h"
#include "core/algorithms/cfd/cfdfinder/util/lhs_utils.h"

namespace algos::cfdfinder {

SupportIndependentPruning::SupportIndependentPruning(size_t pattern_threshold,
                                                     double min_support_gain,
                                                     double max_level_support_drop,
                                                     double min_confidence,
                                                     config::ThreadNumType thread_num)
    : max_patterns_(pattern_threshold),
      min_support_gain_(min_support_gain),
      min_confidence_(min_confidence),
      max_level_support_drop_(max_level_support_drop),
      insufficient_support_gain_(false) {
    if (thread_num > 1) {
        support_map_ = std::make_shared<ThreadSafeSupportMap>();
    } else {
        support_map_ = std::unordered_map<Candidate, double>();
    }
}

void SupportIndependentPruning::StartNewTableau(Candidate const& candidate) {
    insufficient_support_gain_ = false;
    current_candidate_ = candidate;
    visited_.clear();
}

bool SupportIndependentPruning::HasEnoughPatterns(std::vector<Pattern> const& tableau) {
    return insufficient_support_gain_ || (tableau.size() >= max_patterns_);
}

bool SupportIndependentPruning::IsPatternWorthConsidering(double new_support) const {
    return new_support >= min_support_gain_;
}

bool SupportIndependentPruning::TryAdding(Pattern& pattern) {
    if (pattern.GetSupport() < min_support_gain_) {
        insufficient_support_gain_ = true;
        return false;
    }
    if (pattern.GetConfidence() < min_confidence_) {
        return false;
    }

    visited_.clear();
    return true;
}

bool SupportIndependentPruning::ValidForProcessing(ValidationContext const& params) {
    ReplacedEntries replaces_entries{params.parent_entries,
                                     {params.replaced_index, params.new_entry}};
    return visited_.insert(std::move(replaces_entries)).second;
}

bool SupportIndependentPruning::ContinueGeneration(PatternTableau const& current_tableau) {
    if (current_tableau.GetPatterns().empty() || current_tableau.GetSupport() == 0) {
        return false;
    }

    SetSupport(current_candidate_, current_tableau.GetSupport());
    double max_support = 0.0;

    for (auto&& superset : utils::GenerateLhsSupersets(current_candidate_.lhs_)) {
        Candidate parent(std::move(superset), current_candidate_.rhs_);

        if (Contains(parent)) {
            double support = GetSupport(parent);
            max_support = std::max(max_support, support);
        }
    }

    return (max_support - max_level_support_drop_) <= current_tableau.GetSupport();
}

}  // namespace algos::cfdfinder
