#include "combinatorics.hpp"

#include "gtest/gtest.h"
#include <algorithm>
#include <execution>
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


class MultisetIteratorTest : public ::testing::Test {
protected:
    void verify_multiset_properties(const std::vector<int>& multiset, size_t n, int range_size) {
        // Verify size
        ASSERT_EQ(multiset.size(), n);
        
        // Verify elements are in range
        for (int val : multiset) {
            ASSERT_GE(val, 0);
            ASSERT_LT(val, range_size);
        }
        
        // Verify non-decreasing order
        ASSERT_TRUE(std::is_sorted(multiset.begin(), multiset.end()));
    }
};

TEST_F(MultisetIteratorTest, BasicConstruction) {
    MultisetIterator<int> begin(3, 2);
    MultisetIterator<int> end(3, 2, true);
    
    EXPECT_NE(begin, end);
    
    auto first = *begin;
    EXPECT_EQ(first.size(), 3);
    EXPECT_EQ(first, (std::vector<int>{0, 0, 0}));
}

TEST_F(MultisetIteratorTest, IterationSequence) {
    MultisetRange<int> range(3, 2);  // n=3, range=[0,1]
    
    std::vector<std::vector<int>> expected = {
        {0, 0, 0},
        {0, 0, 1},
        {0, 1, 1},
        {1, 1, 1}
    };
    
    size_t count = 0;
    for (const auto& multiset : range) {
        ASSERT_LT(count, expected.size());
        EXPECT_EQ(multiset, expected[count]);
        verify_multiset_properties(multiset, 3, 2);
        ++count;
    }
    
    EXPECT_EQ(count, expected.size());
}

TEST_F(MultisetIteratorTest, RandomAccess) {
    auto range = make_multiset_range(3, 2);
    auto it = range.begin();
    
    // Test +=
    it += 2;
    EXPECT_EQ(*it, (std::vector<int>{0, 1, 1}));
    
    // Test -=
    it -= 1;
    EXPECT_EQ(*it, (std::vector<int>{0, 0, 1}));
    
    // Test +
    auto it2 = it + 2;
    EXPECT_EQ(*it2, (std::vector<int>{1, 1, 1}));
    
    // Test -
    auto it3 = it2 - 2;
    EXPECT_EQ(*it3, (std::vector<int>{0, 0, 1}));
    
    // Test difference
    EXPECT_EQ(it2 - it, 2);
    EXPECT_EQ(it - it3, 0);
}

TEST_F(MultisetIteratorTest, Comparison) {
    auto range = make_multiset_range(3, 2);
    auto it1 = range.begin();
    auto it2 = range.begin();
    auto end = range.end();
    
    EXPECT_EQ(it1, it2);
    EXPECT_NE(it1, end);
    
    ++it2;
    EXPECT_LT(it1, it2);
    EXPECT_GT(it2, it1);
    EXPECT_LE(it1, it2);
    EXPECT_GE(it2, it1);
}

TEST_F(MultisetIteratorTest, LargerRange) {
    MultisetRange<int> range(4, 3);  // n=4, range=[0,1,2]
    
    std::set<std::vector<int>> unique_multisets;
    size_t count = 0;
    
    for (const auto& multiset : range) {
        verify_multiset_properties(multiset, 4, 3);
        unique_multisets.insert(multiset);
        ++count;
    }
    
    // Verify all multisets are unique
    EXPECT_EQ(unique_multisets.size(), count);
    
    // Verify total count matches combination with repetition formula
    // For n=4, r=3: (n+r-1)!/(n!*(r-1)!) = 15
    EXPECT_EQ(count, 15);
}

TEST_F(MultisetIteratorTest, EdgeCases) {
    // Test single element
    auto range1 = make_multiset_range(1, 2);
    std::vector<std::vector<int>> expected1 = {{0}, {1}};
    EXPECT_EQ(std::vector<std::vector<int>>(range1.begin(), range1.end()), expected1);
    
    // Test single choice
    auto range2 = make_multiset_range(3, 1);
    std::vector<std::vector<int>> expected2 = {{0, 0, 0}};
    EXPECT_EQ(std::vector<std::vector<int>>(range2.begin(), range2.end()), expected2);
}

TEST_F(MultisetIteratorTest, BoundaryRandomAccess) {
    auto range = make_multiset_range(3, 2);
    auto it = range.begin();
    
    // Test jumping beyond end
    it += 10;
    EXPECT_EQ(it, range.end());
    
    // Test jumping backwards from end
    it -= 2;
    EXPECT_EQ(*it, (std::vector<int>{0, 1, 1}));
    
    // Test jumping to beginning
    it = range.end();
    it -= 4;
    EXPECT_EQ(it, range.begin());
}

TEST_F(MultisetIteratorTest, Iteration) {
    auto range = make_multiset_range(3, 2);
    auto it = range.begin();
    auto end = range.end();
    
    // Test post-increment
    auto old_it = it++;
    EXPECT_EQ(*old_it, (std::vector<int>{0, 0, 0}));
    EXPECT_EQ(*it, (std::vector<int>{0, 0, 1}));
    
    // Test pre-increment
    auto& ref = ++it;
    EXPECT_EQ(&ref, &it);
    EXPECT_EQ(*it, (std::vector<int>{0, 1, 1}));
    
    // Test post-decrement
    old_it = it--;
    EXPECT_EQ(*old_it, (std::vector<int>{0, 1, 1}));
    EXPECT_EQ(*it, (std::vector<int>{0, 0, 1}));
    
    // Test pre-decrement
    auto& ref2 = --it;
    EXPECT_EQ(&ref2, &it);
    EXPECT_EQ(*it, (std::vector<int>{0, 0, 0}));
}