#pragma once

#include <vector>

#include "core/algorithms/cfd/cfdfinder/candidate.h"
#include "core/algorithms/cfd/cfdfinder/model/pattern/pattern.h"
#include "core/algorithms/cfd/cfdfinder/model/pattern/pattern_tableau.h"

namespace algos::cfdfinder {
class PruningStrategy {
public:
    struct ValidationContext {
        Entries& entries_buffer;
        size_t replaced_index;
        std::shared_ptr<Entry> const& new_entry;
        std::shared_ptr<Entries> const& parent_entries;
    };

    virtual ~PruningStrategy() = default;
    virtual void StartNewTableau(Candidate const& candidate) = 0;
    virtual bool HasEnoughPatterns(std::vector<Pattern> const& tableau) = 0;
    virtual bool IsPatternWorthConsidering(double new_support) const = 0;
    virtual bool TryAdding(Pattern& pattern) = 0;
    virtual bool ValidForProcessing(ValidationContext const& entries) = 0;
    virtual bool ContinueGeneration(PatternTableau const& currentTableau) = 0;
    virtual std::shared_ptr<PruningStrategy> Clone() const = 0;
};
}  // namespace algos::cfdfinder
