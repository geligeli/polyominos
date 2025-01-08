#include "combinatorics.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <algorithm>
#include <cstdint>
#include <execution>
#include <iterator>
#include <mutex>
#include <set>
#include <string>
#include <vector>

TEST(BionomialCoeff, Test) {
    EXPECT_EQ(binomialCoeff(5, 2), 10);
    EXPECT_EQ(binomialCoeff(10, 3), 120);
    EXPECT_EQ(binomialCoeff(10, 10), 1);
    EXPECT_EQ(binomialCoeff(10, 0), 1);
    EXPECT_EQ(binomialCoeff(10, 11), 0);

    EXPECT_EQ(binomialCoeff(100, 5), 75287520);
    EXPECT_EQ(binomialCoeff(369, 5), 55479552948);
}

TEST(BionomialCoeff, getCombinationIndicesOne) {
    std::vector<std::size_t> result = getCombinationIndices(3, 2);
    std::vector<std::size_t> expected = {3, 2, 0};
    EXPECT_EQ(result, expected);
}

TEST(BionomialCoeff, getCombinationIndicesMultiple) {
    auto generateKSubsets = [](uint64_t n, uint64_t k) {
        std::vector<std::vector<uint64_t>> result;
        std::vector<bool> indices(n, false);
        std::fill(indices.begin(), indices.begin() + k, true);
        do {
            std::vector<uint64_t> subset;
            for (uint64_t i = 0; i < n; ++i) {
                if (indices[n-i-1]) {
                    subset.push_back(n-i-1);
                }
            }
            result.push_back(subset);
        } while (std::prev_permutation(indices.begin(), indices.end()));
        std::sort(result.begin(), result.end());
        return result;
    };

    
    uint64_t N = 5;
    uint64_t K = 2;

    auto kSubSets = generateKSubsets(N, K);
    ASSERT_EQ(binomialCoeff(N, K), kSubSets.size());
    for (uint64_t index=0; index < binomialCoeff(N, K); ++index) {
        auto result = getCombinationIndices(K, index);
        ASSERT_EQ(result, kSubSets[index]) << "Index: " << index << " N: " << N << " K: " << K;
    }

    N = 100;
    K = 3;

    // 2 1 0
    // 3 1 0
    // 3 2 0
    // 3 2 1

    kSubSets = generateKSubsets(N, K);
    ASSERT_EQ(binomialCoeff(N, K), kSubSets.size());
    for (uint64_t index=0; index < binomialCoeff(N, K); ++index) {
        auto result = getCombinationIndices(K, index);
        ASSERT_EQ(result, kSubSets[index]) << "Index: " << index << " N: " << N << " K: " << K;
    }
}

TEST(SubSets, SubSetsRange) {
    const uint64_t N = 40;
    const uint64_t K = 4;
    auto seq = SubSetsRange(N, K);
    uint64_t idx = 0;
    for (const auto& subset : seq) {
        SCOPED_TRACE("Index: " + std::to_string(idx));
        ASSERT_EQ(subset.size(), K);
        std::vector<std::size_t> expected = getCombinationIndices(K, idx);
        ASSERT_EQ(subset, expected);
        ++idx;
    }
}

TEST(SubSets, DecreaseKSubsets) {
    std::vector<std::size_t> indices = { 39, 2,1,0 };
    DecreaseKSubsets(indices);
    std::vector<std::size_t> expected = { 38, 37, 36, 35 };
    ASSERT_EQ(indices, expected);
}

TEST(SubSets, SubSetsRangeDecrease) {
    const uint64_t N = 40;
    const uint64_t K = 4;
    auto seq = SubSetsRange(N, K);
    uint64_t idx = binomialCoeff(N, K) - 1;

    for (auto it = seq.end(); --it != seq.begin(); --idx) {
        SCOPED_TRACE("Index: " + std::to_string(idx));
        std::vector<std::size_t> expected = getCombinationIndices(K, idx);
        ASSERT_EQ(*it, expected);
    }
}

TEST(SubSets, UseSubsetSequenceAsRange) {
    const uint64_t N = 100000;
    const uint64_t K = 1;
    auto seq = SubSetsRange(N, K);
    uint64_t idx = 0;
    for (const auto& subset : seq) {
        ASSERT_EQ(subset[0], idx);
        ++idx;
    }
}

TEST(SubSets, SubSetsRangeProduct) {
    SubSetsRangeProductRange ssxp({
        SubSetsRange(3, 2),
        SubSetsRange(2, 1)
    });

    std::vector<std::vector<uint64_t>> result;

    for (const auto& subset : ssxp) {
        result.push_back(subset);
    }

    ASSERT_EQ(result.size(), 6);
    ASSERT_THAT(result[0], testing::ElementsAre(1,0,0));
    ASSERT_THAT(result[1], testing::ElementsAre(2,0,0));
    ASSERT_THAT(result[2], testing::ElementsAre(2,1,0));
    ASSERT_THAT(result[3], testing::ElementsAre(1,0,1));
    ASSERT_THAT(result[4], testing::ElementsAre(2,0,1));
    ASSERT_THAT(result[5], testing::ElementsAre(2,1,1));
}


TEST(MultiSets, getMultiSetCombinationIndices) {
    std::vector<uint64_t> result = getMultiSetCombinationIndices(3, 2);
    std::vector<uint64_t> expected = {1,1,0};
    EXPECT_EQ(result, expected);
}

TEST(MultiSets, incAndIndexAgree) {
  std::vector<uint64_t> expected = {0,0,0};

  for (uint64_t i = 0; i < multichoose(5,3); ++i) {
    ASSERT_EQ(expected, getMultiSetCombinationIndices(expected.size(), i)) << "Index: " << i;
    IncreaseKMultiSet(expected);
  }
}


TEST(MultiSets, decAndIndexAgree) {
  std::vector<uint64_t> expected = {4,4,4};
  for (uint64_t i = multichoose(5,3); --i > 0;) {
    ASSERT_EQ(expected, getMultiSetCombinationIndices(expected.size(), i)) << "Index: " << i;
    DecreaseKMultiSet(expected);
  }
}


// size=35
// i=6 size=107
// i=7 size=361
// i=8 size=1200
// size=2 count=3
// size=5 count=1
// size=9 count=2
// partition=9 9 5 2 2 2 
// indices=1 0 0 2 1 0 
// geli@f7ff21508446:/workspaces/polyominos$ bazel run -c dbg --copt=-O0 //:puzzle_maker
// INFO: Analyzed target //:puzzle_maker (0 packages loaded, 0 targets configured).
// INFO: Found 1 target...
// Target //:puzzle_maker up-to-date:
//   bazel-bin/puzzle_maker
// INFO: Elapsed time: 3.418s, Critical Path: 3.36s
// INFO: 3 processes: 1 internal, 2 linux-sandbox.
// INFO: Build completed successfully, 3 total actions
// INFO: Running command line: bazel-bin/puzzle_maker
// i=0 size=1
// i=1 size=1
// i=2 size=2
// i=3 size=5
// i=4 size=12
// i=5 size=35
// i=6 size=107
// i=7 size=361
// i=8 size=1200
// size=2 count=1
// size=5 count=12
// size=9 count=1200
// partition=9 9 5 2 2 2 
// indices=1 0 0 2 1 0 