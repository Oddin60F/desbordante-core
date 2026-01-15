#pragma once

#include <cstddef>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "core/algorithms/cfd/cfdfinder/model/pruning/pruning_strategy.h"
#include "core/config/thread_number/type.h"

namespace algos::cfdfinder {

class SupportIndependentPruning : public PruningStrategy {
private:
    class ThreadSafeSupportMap {
    private:
        std::unordered_map<Candidate, double> support_map_;
        mutable std::shared_mutex mutex_;

    public:
        void SetSupport(Candidate const& candidate, double support) {
            std::unique_lock<std::shared_mutex> lock(mutex_);
            support_map_[candidate] = support;
        }

        double GetSupport(Candidate const& candidate) const {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            auto it = support_map_.find(candidate);
            if (it != support_map_.end()) {
                return it->second;
            }
            return 0.0;
        }

        bool Contains(Candidate const& candidate) const {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            return support_map_.contains(candidate);
        }
    };

    void SetSupport(Candidate const& candidate, double support) {
        std::visit(
                [&](auto& map) {
                    if constexpr (std::is_same_v<std::decay_t<decltype(map)>,
                                                 std::shared_ptr<ThreadSafeSupportMap>>) {
                        map->SetSupport(candidate, support);
                    } else {
                        map[candidate] = support;
                    }
                },
                support_map_);
    }

    double GetSupport(Candidate const& candidate) const {
        return std::visit(
                [&](auto const& map) -> double {
                    if constexpr (std::is_same_v<std::decay_t<decltype(map)>,
                                                 std::shared_ptr<ThreadSafeSupportMap>>) {
                        return map->GetSupport(candidate);
                    } else {
                        auto it = map.find(candidate);
                        return it != map.end() ? it->second : 0.0;
                    }
                },
                support_map_);
    }

    bool Contains(Candidate const& candidate) const {
        return std::visit(
                [&](auto const& map) -> bool {
                    if constexpr (std::is_same_v<std::decay_t<decltype(map)>,
                                                 std::shared_ptr<ThreadSafeSupportMap>>) {
                        return map->Contains(candidate);
                    } else {
                        return map.contains(candidate);
                    }
                },
                support_map_);
    }

    using SupportMap = std::variant<std::unordered_map<Candidate, double>,
                                    std::shared_ptr<ThreadSafeSupportMap>>;
    size_t max_patterns_;
    double min_support_gain_;
    double min_confidence_;
    double max_level_support_drop_;
    bool insufficient_support_gain_;
    SupportMap support_map_;
    std::unordered_set<Entries> visited_;

protected:
    Candidate current_candidate_;

public:
    SupportIndependentPruning(size_t pattern_threshold, double min_support_gain,
                              double max_level_support_drop, double min_confidence,
                              config::ThreadNumType thread_num = 1);

    void StartNewTableau(Candidate const& candidate) override;
    void AddPattern([[maybe_unused]] Pattern const& pattern) override;
    void ExpandPattern([[maybe_unused]] Pattern const& pattern) override;
    void ProcessChild(Pattern& child) override;
    bool HasEnoughPatterns(std::vector<Pattern> const& tableau) override;
    bool IsPatternWorthConsidering(Pattern const& pattern) override;
    bool IsPatternWorthAdding(Pattern const& pattern) override;
    bool ValidForProcessing(Pattern const& child) override;
    bool ContinueGeneration(PatternTableau const& current_tableau) override;

    std::shared_ptr<PruningStrategy> Clone() const override {
        return std::make_shared<SupportIndependentPruning>(*this);
    }
};

}  // namespace algos::cfdfinder
