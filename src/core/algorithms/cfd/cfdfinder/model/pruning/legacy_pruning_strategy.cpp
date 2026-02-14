#include "core/algorithms/cfd/cfdfinder/model/pruning/legacy_pruning_strategy.h"

#include <list>
#include <memory>
#include <ranges>

#include "core/algorithms/cfd/cfdfinder/candidate.h"
#include "core/algorithms/cfd/cfdfinder/model/pattern/pattern.h"
#include "core/algorithms/cfd/cfdfinder/model/pattern/pattern_tableau.h"
#include "core/algorithms/cfd/cfdfinder/model/pattern/variable_entry.h"

namespace algos::cfdfinder {

void LegacyPruning::StartNewTableau([[maybe_unused]] Candidate const& candidate) {
    cumulative_support_ = 0;
    visited_.clear();
}

void LegacyPruning::AddPattern(Pattern const& pattern) {
    cumulative_support_ += pattern.GetSupport();
}

void LegacyPruning::ExpandPattern(Pattern const& pattern) {
    visited_.insert(pattern.GetEntries());
}

bool LegacyPruning::HasEnoughPatterns([[maybe_unused]] std::vector<Pattern> const& tableau) {
    return cumulative_support_ >= min_support_ * num_tuples_;
}

bool LegacyPruning::IsPatternWorthConsidering(double new_support) {
    return new_support > 0;
}

bool LegacyPruning::IsPatternWorthAdding(Pattern const& pattern) {
    return pattern.GetConfidence() >= min_confidence_;
}

bool LegacyPruning::ValidForProcessing(Entries const& entries) {
    size_t count = 0;
    bool yes = true;
    for (size_t i = 0; i < entries.size(); ++i) {
        if (entries[i].entry->IsConstant()) {
            ++count;
            if (!yes) {
                continue;
            }
            auto new_entries = entries;
            new_entries[i].entry = std::make_shared<VariableEntry>();
            yes = visited_.contains(new_entries);
        }
    }

    PatternDebugController::Plus(count);
    return yes;
}

// bool LegacyPruning::ValidForProcessing(Pattern const& child) {
//     auto entries = child.GetEntries();
//     std::shared_ptr<Entry> variable_entry = std::make_shared<VariableEntry>();
//     std::shared_ptr<Entry> buff_entry;
//     bool yes = true;
//     size_t count = 0;
//     for (size_t i = 0; i < entries.size(); ++i) {
//         if (entries[i].entry->IsConstant()) {
//             ++count;

//             if (!yes) {
//                 continue;
//             }
//             buff_entry = std::move(entries[i].entry);
//             entries[i].entry = std::move(variable_entry);
//             yes = visited_.contains(entries);
//             variable_entry = std::move(entries[i].entry);
//             entries[i].entry = std::move(buff_entry);
//         }
//     }
//     PatternDebugController::Plus(count);
//     return yes;
// }
bool LegacyPruning::ContinueGeneration(PatternTableau const& currentTableau) {
    return currentTableau.GetSupport() >= min_support_ &&
           currentTableau.GetConfidence() >= min_confidence_;
}

}  // namespace algos::cfdfinder
