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


class CrossProductIteratorTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}

    // Helper function to convert iterator results to set for comparison
    template<typename T>
    std::set<std::vector<T>> iteratorToSet(std::vector<std::vector<T>>& input) {
        auto [begin, end] = make_cross_product_iterator(input);
        return std::set<std::vector<T>>(begin, end);
    }
};

TEST_F(CrossProductIteratorTest, EmptyInput) {
    std::vector<std::vector<int>> empty_input;
    auto [begin, end] = make_cross_product_iterator(empty_input);
    EXPECT_EQ(begin, end);
}

TEST_F(CrossProductIteratorTest, SingleEmptyVector) {
    std::vector<std::vector<int>> input = {{}};
    auto [begin, end] = make_cross_product_iterator(input);
    EXPECT_EQ(begin, end);
}

TEST_F(CrossProductIteratorTest, OneVectorWithOneElement) {
    std::vector<std::vector<int>> input = {{1}};
    auto [begin, end] = make_cross_product_iterator(input);
    
    std::vector<std::vector<int>> expected = {{1}};
    std::set<std::vector<int>> result = iteratorToSet(input);
    EXPECT_EQ(result, std::set<std::vector<int>>(expected.begin(), expected.end()));
}

TEST_F(CrossProductIteratorTest, TwoVectorsBasicCase) {
    std::vector<std::vector<int>> input = {{1, 2}, {3, 4}};
    std::set<std::vector<int>> result = iteratorToSet(input);
    
    std::set<std::vector<int>> expected = {
        {1, 3}, {1, 4},
        {2, 3}, {2, 4}
    };
    EXPECT_EQ(result, expected);
}

TEST_F(CrossProductIteratorTest, ThreeVectorsComprehensive) {
    std::vector<std::vector<int>> input = {{1, 2}, {3, 4}, {5, 6}};
    std::set<std::vector<int>> result = iteratorToSet(input);
    
    std::set<std::vector<int>> expected = {
        {1, 3, 5}, {1, 3, 6},
        {1, 4, 5}, {1, 4, 6},
        {2, 3, 5}, {2, 3, 6},
        {2, 4, 5}, {2, 4, 6}
    };
    EXPECT_EQ(result, expected);
}

TEST_F(CrossProductIteratorTest, DifferentSizeVectors) {
    std::vector<std::vector<int>> input = {{1}, {2, 3}, {4, 5, 6}};
    std::set<std::vector<int>> result = iteratorToSet(input);
    
    std::set<std::vector<int>> expected = {
        {1, 2, 4}, {1, 2, 5}, {1, 2, 6},
        {1, 3, 4}, {1, 3, 5}, {1, 3, 6}
    };
    EXPECT_EQ(result, expected);
}

TEST_F(CrossProductIteratorTest, StringType) {
    std::vector<std::vector<std::string>> input = {
        {"a", "b"},
        {"x", "y"}
    };
    std::set<std::vector<std::string>> result = iteratorToSet(input);
    
    std::set<std::vector<std::string>> expected = {
        {"a", "x"}, {"a", "y"},
        {"b", "x"}, {"b", "y"}
    };
    EXPECT_EQ(result, expected);
}

TEST_F(CrossProductIteratorTest, IteratorOperations) {
    std::vector<std::vector<int>> input = {{1, 2}, {3, 4}};
    auto [begin, end] = make_cross_product_iterator(input);
    
    // Test prefix increment
    auto it = begin;
    std::vector<int> first_val = *it;
    ++it;
    std::vector<int> second_val = *it;
    EXPECT_NE(first_val, second_val);
    
    // Test postfix increment
    std::vector<int> third_val = *it;
    it++;
    std::vector<int> fourth_val = *it;
    EXPECT_NE(third_val, fourth_val);
    
    // Test dereferencing
    it = begin;
    auto value = *it;
    EXPECT_EQ(value.size(), 2);
}

TEST_F(CrossProductIteratorTest, ExceptionOnEndDereference) {
    std::vector<std::vector<int>> input = {{1}};
    auto [begin, end] = make_cross_product_iterator(input);
    EXPECT_THROW(*end, std::out_of_range);
}

TEST_F(CrossProductIteratorTest, ParallelUnorderedExecution) {
    std::vector<std::vector<int>> input = {{1, 2, 3}, {4, 5}, {6, 7}};
    auto [begin, end] = make_cross_product_iterator(input);
    
    // Create a set to store results with mutex for thread safety
    std::set<std::vector<int>> result_set;
    std::mutex mutex;
    
    // Use parallel unordered execution
    std::for_each(std::execution::par_unseq, begin, end,
        [&](const std::vector<int>& combination) {
            std::lock_guard<std::mutex> lock(mutex);
            result_set.insert(combination);
        }
    );
    
    // Create expected set
    std::set<std::vector<int>> expected = {
        {1, 4, 6}, {1, 4, 7},
        {1, 5, 6}, {1, 5, 7},
        {2, 4, 6}, {2, 4, 7},
        {2, 5, 6}, {2, 5, 7},
        {3, 4, 6}, {3, 4, 7},
        {3, 5, 6}, {3, 5, 7}
    };
    
    EXPECT_EQ(result_set, expected);
}

TEST_F(CrossProductIteratorTest, MultipleIterations) {
    std::vector<std::vector<int>> input = {{1, 2}, {3, 4}};
    auto [begin, end] = make_cross_product_iterator(input);
    
    // First iteration
    std::vector<std::vector<int>> first_pass;
    for (auto it = begin; it != end; ++it) {
        first_pass.push_back(*it);
    }
    
    // Second iteration
    std::vector<std::vector<int>> second_pass;
    for (auto it = begin; it != end; ++it) {
        second_pass.push_back(*it);
    }
    
    EXPECT_EQ(first_pass, second_pass);
}

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
    std::vector<std::size_t> result = getCombinationIndices(100, 3, 2);
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
        auto result = getCombinationIndices(N, K, index);
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
        auto result = getCombinationIndices(N, K, index);
        ASSERT_EQ(result, kSubSets[index]) << "Index: " << index << " N: " << N << " K: " << K;
    }
}

TEST(BionomialCoeff, SubSetsSequence) {
    const uint64_t N = 40;
    const uint64_t K = 4;
    auto seq = SubSetsSequence(N, K);
    uint64_t idx = 0;
    for (const auto& subset : seq) {
        SCOPED_TRACE("Index: " + std::to_string(idx));
        ASSERT_EQ(subset.size(), K);
        std::vector<std::size_t> expected = getCombinationIndices(N, K, idx);
        ASSERT_EQ(subset, expected);
        ++idx;
    }
}

TEST(BionomialCoeff, DecreaseKSubsets) {
    std::vector<std::size_t> indices = { 39, 2,1,0 };
    DecreaseKSubsets(indices);
    std::vector<std::size_t> expected = { 38, 37, 36, 35 };
    ASSERT_EQ(indices, expected);
}

TEST(BionomialCoeff, SubSetsSequenceDecrease) {
    const uint64_t N = 40;
    const uint64_t K = 4;
    auto seq = SubSetsSequence(N, K);
    uint64_t idx = binomialCoeff(N, K) - 1;

    for (auto it = seq.end(); --it != seq.begin(); --idx) {
        SCOPED_TRACE("Index: " + std::to_string(idx));
        std::vector<std::size_t> expected = getCombinationIndices(N, K, idx);
        ASSERT_EQ(*it, expected);
    }
}


TEST(BionomialCoeff, UseSubsetSequenceAsRange) {
    const uint64_t N = 100000;
    const uint64_t K = 1;
    auto seq = SubSetsSequence(N, K);
    uint64_t idx = 0;
    for (const auto& subset : seq) {
        ASSERT_EQ(subset[0], idx);
        ++idx;
    }
}
