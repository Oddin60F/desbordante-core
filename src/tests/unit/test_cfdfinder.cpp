#include <boost/algorithm/string/join.hpp>
#include <gtest/gtest.h>

#include "core/algorithms/algo_factory.h"
#include "core/algorithms/cfd/cfdfinder/cfdfinder.h"
#include "core/config/indices/option.h"
#include "core/config/names_and_descriptions.h"
#include "core/config/thread_number/type.h"
#include "tests/common/all_csv_configs.h"

namespace tests {
struct CFDFinderParams {
    using FD = std::string;
    using Tableau = std::vector<std::string>;
    using Excepted_CFD = std::pair<FD, Tableau>;
    algos::StdParamsMap params;
    std::set<Excepted_CFD> excepted_cfds;

    // legacy strategy
    CFDFinderParams(CSVConfig csv_config, algos::cfdfinder::Expansion expansion,
                    algos::cfdfinder::Result result, unsigned int max_lhs, double min_sup,
                    double min_conf, bool is_null_equal_null, config::ThreadNumType thread_num,
                    std::set<Excepted_CFD> excepted)
        : params({{config::names::kCsvConfig, csv_config},
                  {config::names::kMaximumLhs, max_lhs},
                  {config::names::kCfdPruningStrategy, +algos::cfdfinder::Pruning::legacy},
                  {config::names::kCfdMinimumSupport, min_sup},
                  {config::names::kCfdMinimumConfidence, min_conf},
                  {config::names::kEqualNulls, is_null_equal_null},
                  {config::names::kCfdResultStrategy, +result},
                  {config::names::kCfdExpansionStrategy, +expansion},
                  {config::names::kThreads, thread_num}}),
          excepted_cfds(std::move(excepted)) {}

    // support_independent strategy
    CFDFinderParams(CSVConfig csv_config, algos::cfdfinder::Expansion expansion,
                    algos::cfdfinder::Result result, unsigned int max_lhs, double min_conf,
                    double min_support_gain, double max_level_support_drop,
                    unsigned int pattern_threshold, bool is_null_equal_null,
                    config::ThreadNumType thread_num, std::set<Excepted_CFD> excepted)
        : params({{config::names::kCsvConfig, csv_config},
                  {config::names::kMaximumLhs, max_lhs},
                  {config::names::kCfdPruningStrategy,
                   +algos::cfdfinder::Pruning::support_independent},
                  {config::names::kCfdMinimumConfidence, min_conf},
                  {config::names::kMinSupportGain, min_support_gain},
                  {config::names::kMaxLevelSupportDrop, max_level_support_drop},
                  {config::names::kMaxPatterns, pattern_threshold},
                  {config::names::kEqualNulls, is_null_equal_null},
                  {config::names::kCfdResultStrategy, +result},
                  {config::names::kCfdExpansionStrategy, +expansion},
                  {config::names::kThreads, thread_num}}),
          excepted_cfds(std::move(excepted)) {}

    // rhs_filter strategy
    CFDFinderParams(CSVConfig csv_config, algos::cfdfinder::Expansion expansion,
                    algos::cfdfinder::Result result, unsigned int max_lhs, double min_conf,
                    double min_support_gain, double max_level_support_drop,
                    unsigned int pattern_threshold, config::IndicesType rhs_indeces,
                    bool is_null_equal_null, config::ThreadNumType thread_num,
                    std::set<Excepted_CFD> excepted)
        : params({{config::names::kCsvConfig, csv_config},
                  {config::names::kMaximumLhs, max_lhs},
                  {config::names::kCfdPruningStrategy, +algos::cfdfinder::Pruning::rhs_filter},
                  {config::names::kCfdMinimumConfidence, min_conf},
                  {config::names::kMinSupportGain, min_support_gain},
                  {config::names::kMaxLevelSupportDrop, max_level_support_drop},
                  {config::names::kMaxPatterns, pattern_threshold},
                  {config::names::kEqualNulls, is_null_equal_null},
                  {config::names::kRhsIndices, rhs_indeces},
                  {config::names::kCfdResultStrategy, +result},
                  {config::names::kCfdExpansionStrategy, +expansion},
                  {config::names::kThreads, thread_num}}),
          excepted_cfds(std::move(excepted)) {}

    // max_g1 strategy
    CFDFinderParams(CSVConfig csv_config, algos::cfdfinder::Expansion expansion,
                    algos::cfdfinder::Result result, unsigned int max_lhs, double max_g1,
                    bool is_null_equal_null, config::ThreadNumType thread_num,
                    std::set<Excepted_CFD> excepted)
        : params({{config::names::kCsvConfig, csv_config},
                  {config::names::kCfdResultStrategy, +result},
                  {config::names::kMaximumLhs, max_lhs},
                  {config::names::kCfdPruningStrategy, +algos::cfdfinder::Pruning::partial_fd},
                  {config::names::kCfdExpansionStrategy, +expansion},
                  {config::names::kMaximumG1, max_g1},
                  {config::names::kEqualNulls, is_null_equal_null},
                  {config::names::kCfdExpansionStrategy, +expansion},
                  {config::names::kThreads, thread_num}}),
          excepted_cfds(std::move(excepted)) {}
};

// static void CheckEqualityExceptedCFDs(std::set<CFDFinderParams::Excepted_CFD> const& expected,
//                                       std::list<algos::cfdfinder::CFD> const& actual) {
//     ASSERT_EQ(actual.size(), expected.size()) << "count of cfds does not match: expected "
//                                               << expected.size() << ", got " << actual.size();

//     for (auto const& cfd : actual) {
//         auto embeded_fd = cfd.GetEmbeddedFD().ToLongString();

//         std::vector<std::string> patterns;
//         for (auto const& pattern : cfd.GetTableau()) {
//             patterns.push_back(boost::algorithm::join(pattern, "|"));
//         }
//         std::pair<std::string, std::vector<std::string>> expected_cfd = {std::move(embeded_fd),
//                                                                          std::move(patterns)};
//         if (expected.find(expected_cfd) == expected.end()) {
//             FAIL() << "generated cfd not found in expected";
//         }
//     }
//     SUCCEED();
// }

class CFDFinderAlgorithmTest : public ::testing::TestWithParam<CFDFinderParams> {};

TEST_P(CFDFinderAlgorithmTest, Test) {
    std::ofstream file("/home/oddin60/Work/metanome_old/test_c++.txt", std::ios::trunc);
    auto const& p = GetParam();
    auto mp = algos::StdParamsMap(p.params);
    auto algo = algos::CreateAndLoadAlgorithm<algos::cfdfinder::CFDFinder>(mp);
    algo->Execute();
    for (auto const& cfd : algo->CfdList()) {
        file << cfd.ToString();
    }
}

INSTANTIATE_TEST_SUITE_P(
        CFDFinderAdditionalTests, CFDFinderAlgorithmTest,
        ::testing::Values(CFDFinderParams(
                {kNCVOTER,
                 algos::cfdfinder::Expansion::constant,
                 algos::cfdfinder::Result::direct,
                 3333,  // max_lhs
                 1.0,   // min_conf
                 100,
                 50,
                 1000,
                 true,  // is_null_equal_null
                 5,
                 {
                         {"[temp humidity windy play] -> outlook",
                          {"_|high|_|_", "_|_|true|_", "mild|_|_|_", "hot|_|_|_"}},
                         {"[outlook temp humidity play] -> windy",
                          {"_|_|_|yes", "_|mild|_|_", "_|_|normal|_"}},
                         {"[outlook temp play] -> windy", {"_|_|yes", "_|mild|_", "_|cool|_"}},
                 }})));

}  // namespace tests
