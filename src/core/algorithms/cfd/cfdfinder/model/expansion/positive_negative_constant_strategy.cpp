#include "core/algorithms/cfd/cfdfinder/model/expansion/positive_negative_constant_strategy.h"

#include <cstddef>

#include "core/algorithms/cfd/cfdfinder/model/pattern/constant_entry.h"
#include "core/algorithms/cfd/cfdfinder/model/pattern/negative_constant_entry.h"

namespace algos::cfdfinder {

void PositiveNegativeConstantExpansion::ExpandAndProcess(Pattern parent_pattern, Frontier& frontier,
                                                         Row const& inverted_pli_rhs,
                                                         PruningStrategy& pruning_strategy) const {
    auto parent_entries = parent_pattern.GetEntries();
    auto const& parent_cover = parent_pattern.GetCover();

    for (auto& [id, entry] : parent_entries) {
        if (entry->IsConstant()) {
            continue;
        }
        auto const& column = columns_.GetColumn(id);

        std::vector<int> unique_values;
        std::unordered_set<int> seen;

        unique_values.reserve(parent_cover.size());
        for (auto const& cluster : parent_cover) {
            int value = column[cluster[0]];
            if (seen.insert(value).second) {
                unique_values.push_back(value);
            }
        }
        std::vector<int> valid_values;
        valid_values.reserve(unique_values.size());
        for (auto value : unique_values) {
            std::shared_ptr<Entry> buff_entry = std::make_shared<ConstantEntry>(value);
            std::swap(entry, buff_entry);
            // ||frontier.Contains(parent_entries)
            if (!pruning_strategy.ValidForProcessing(parent_entries) ||
                frontier.Contains(parent_entries)) {
                std::swap(entry, buff_entry);
                continue;
            }
            std::swap(entry, buff_entry);
            valid_values.push_back(value);
        }

        if (valid_values.empty()) {
            continue;
        }

        {
            std::unordered_map<int, size_t> support_map;
            support_map.reserve(valid_values.size());
            for (int val : valid_values) {
                support_map[val] = 0;  // инициализируем нулевой поддержкой
            }

            std::vector<size_t> cluster_first;
            cluster_first.reserve(parent_cover.size());
            for (auto const& cluster : parent_cover) {
                cluster_first.push_back(column[cluster[0]]);
            }

            for (size_t cluster_id = 0; cluster_id < parent_cover.size(); ++cluster_id) {
                int val = cluster_first[cluster_id];
                auto it = support_map.find(val);
                if (it != support_map.end()) {
                    it->second += parent_cover[cluster_id].size();  // накапливаем поддержку
                }
            }

            // Шаг 2. Фильтрация по поддержке (IsPatternWorthConsidering)
            std::vector<int> final_values;
            final_values.reserve(valid_values.size());
            for (int val : valid_values) {
                size_t support = support_map[val];  // поддержка уже посчитана
                if (pruning_strategy.IsPatternWorthConsidering(support)) {
                    final_values.push_back(val);
                }
            }

            // Шаг 3. Создание results только для отфильтрованных значений
            std::vector<std::pair<int, boost::dynamic_bitset<>>> results;
            results.reserve(final_values.size());
            for (int val : final_values) {
                results.emplace_back(val, boost::dynamic_bitset<>(parent_cover.size()));
            }

            // Шаг 4. Построение отображения значение → индекс в results
            std::unordered_map<int, size_t> value_to_idx;
            value_to_idx.reserve(final_values.size());
            for (size_t idx = 0; idx < final_values.size(); ++idx) {
                value_to_idx[final_values[idx]] = idx;
            }
            for (size_t cluster_id = 0; cluster_id < parent_cover.size(); ++cluster_id) {
                int val = cluster_first[cluster_id];
                auto it = value_to_idx.find(val);
                if (it != value_to_idx.end()) {
                    size_t idx = it->second;
                    std::get<1>(results[idx]).set(cluster_id);  // устанавливаем соответствующий бит
                }
            }

            // Шаг 6. Обработка финальных results (создание потомков)
            for (auto [constant, cover_mask] : results) {
                std::shared_ptr<Entry> buff_entry = std::make_shared<ConstantEntry>(constant);
                std::swap(entry, buff_entry);

                auto new_entries = parent_entries;
                Pattern child(std::move(new_entries));
                std::vector<Cluster> child_cover;
                child_cover.reserve(cover_mask.count());
                util::ForEachIndex(cover_mask, [&](size_t cluster_id) {
                    child_cover.push_back(parent_cover[cluster_id]);
                });

                child.SetCover(std::move(child_cover));
                child.UpdateKeepers(inverted_pli_rhs);
                frontier.Emplace(std::move(child));
                std::swap(entry, buff_entry);
            }
        }

        {
            std::unordered_map<int, size_t> support_map;
            support_map.reserve(valid_values.size());
            for (int val : valid_values) {
                support_map[val] = 0;  // инициализируем нулевой поддержкой
            }

            std::vector<size_t> cluster_first;
            cluster_first.reserve(parent_cover.size());
            for (auto const& cluster : parent_cover) {
                cluster_first.push_back(column[cluster[0]]);
            }

            for (size_t cluster_id = 0; cluster_id < parent_cover.size(); ++cluster_id) {
                int val = cluster_first[cluster_id];
                auto it = support_map.find(val);
                if (it != support_map.end()) {
                    it->second += parent_cover[cluster_id].size();  // накапливаем поддержку
                }
            }

            // Шаг 2. Фильтрация по поддержке (IsPatternWorthConsidering)
            std::vector<int> final_values;
            final_values.reserve(valid_values.size());
            for (int val : valid_values) {
                size_t support = support_map[val];  // поддержка уже посчитана
                if (pruning_strategy.IsPatternWorthConsidering(columns_.GetColumn(0).size() -
                                                               support)) {
                    final_values.push_back(val);
                }
            }

            // Шаг 3. Создание results только для отфильтрованных значений
            std::vector<std::pair<int, boost::dynamic_bitset<>>> results;
            results.reserve(final_values.size());
            for (int val : final_values) {
                results.emplace_back(val, boost::dynamic_bitset<>(parent_cover.size()));
            }

            // Шаг 4. Построение отображения значение → индекс в results
            std::unordered_map<int, size_t> value_to_idx;
            value_to_idx.reserve(final_values.size());
            for (size_t idx = 0; idx < final_values.size(); ++idx) {
                value_to_idx[final_values[idx]] = idx;
            }
            for (size_t cluster_id = 0; cluster_id < parent_cover.size(); ++cluster_id) {
                int val = cluster_first[cluster_id];
                auto it = value_to_idx.find(val);
                if (it != value_to_idx.end()) {
                    size_t idx = it->second;
                    std::get<1>(results[idx]).set(cluster_id);  // устанавливаем соответствующий бит
                }
            }

            // Шаг 6. Обработка финальных results (создание потомков)
            for (auto&& [constant, cover_mask] : results) {
                std::shared_ptr<Entry> buff_entry =
                        std::make_shared<NegativeConstantEntry>(constant);
                std::swap(entry, buff_entry);

                auto new_entries = parent_entries;
                Pattern child(std::move(new_entries));
                std::vector<Cluster> child_cover;
                child_cover.reserve(cover_mask.count());
                util::ForEachIndex(~cover_mask, [&](size_t cluster_id) {
                    child_cover.push_back(parent_cover[cluster_id]);
                });

                child.SetCover(std::move(child_cover));
                child.UpdateKeepers(inverted_pli_rhs);
                frontier.Emplace(std::move(child));
                std::swap(entry, buff_entry);
            }
        }
    }
}

}  // namespace algos::cfdfinder
