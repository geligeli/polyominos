#include "dl_matrix.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <iostream>
#include <numeric>
#include <random>

TEST(DLMatrix, SingleEntry) {
  std::vector<uint64_t> v = {1};
  DLMatrix dl_matrix(v);
  ASSERT_EQ(dl_matrix.entries.size(), 1u);
  ASSERT_EQ(dl_matrix.entries[0].left, 0);
  ASSERT_EQ(dl_matrix.entries[0].right, 0);
  ASSERT_EQ(dl_matrix.entries[0].up, -1);
  ASSERT_EQ(dl_matrix.entries[0].down, -1);
  ASSERT_EQ(dl_matrix.entries[0].col, 0);
  ASSERT_EQ(dl_matrix.entries[0].row, 0);
}

TEST(DLMatrix, SingleEntry2) {
  std::vector<uint64_t> v = {2};
  DLMatrix dl_matrix(v);
  ASSERT_EQ(dl_matrix.entries.size(), 1u);
  ASSERT_EQ(dl_matrix.entries[0].left, 0);
  ASSERT_EQ(dl_matrix.entries[0].right, 0);
  ASSERT_EQ(dl_matrix.entries[0].up, -1);
  ASSERT_EQ(dl_matrix.entries[0].down, -1);
  ASSERT_EQ(dl_matrix.entries[0].col, 1);
  ASSERT_EQ(dl_matrix.entries[0].row, 0);
}

TEST(DLMatrix, Empty) {
  std::vector<uint64_t> v = {0, 0, 0};
  DLMatrix dl_matrix(v);
  ASSERT_EQ(dl_matrix.entries.size(), 0u);
}

TEST(DLMatrix, Empty2) {
  std::vector<uint64_t> v;
  DLMatrix dl_matrix(v);
  ASSERT_EQ(dl_matrix.entries.size(), 0u);
}

TEST(DLMatrix, OneRow) {
  std::vector<uint64_t> v = {0xf0f0};
  DLMatrix dl_matrix(v);
  ASSERT_EQ(dl_matrix.entries.size(), 8u);
  ASSERT_EQ(dl_matrix.entries[0].left, 7);
  ASSERT_EQ(dl_matrix.entries[0].up, -1);
  ASSERT_EQ(dl_matrix.entries[0].down, -1);

  for (std::size_t i = 0; i < 7; ++i) {
    ASSERT_EQ(dl_matrix.entries[i].right, i + 1);
  }
  for (std::size_t i = 0; i < 8; ++i) {
    ASSERT_EQ(dl_matrix.entries[7].row, 0);
  }

  ASSERT_EQ(dl_matrix.entries[7].right, 0);
  ASSERT_EQ(dl_matrix.entries[0].col, 4);
  ASSERT_EQ(dl_matrix.entries[1].col, 5);
  ASSERT_EQ(dl_matrix.entries[2].col, 6);
  ASSERT_EQ(dl_matrix.entries[3].col, 7);
  ASSERT_EQ(dl_matrix.entries[4].col, 12);
  ASSERT_EQ(dl_matrix.entries[5].col, 13);
  ASSERT_EQ(dl_matrix.entries[6].col, 14);
  ASSERT_EQ(dl_matrix.entries[7].col, 15);
}

TEST(DLMatrix, TwoRows) {
  std::vector<uint64_t> v = {0xf0ff, 0x0f0f};

  DLMatrix dl_matrix(v);

  EXPECT_EQ(dl_matrix.DebugString(), "  FEDCBA9876543210\n"
                                     "0 1111000011111111\n"
                                     "1 0000111100001111\n");
}

TEST(DLMatrix, CoverColumn) {
  std::vector<uint64_t> v = {0xf0ff, 0x0f0f};
  DLMatrix dl_matrix(v);
  EXPECT_EQ(dl_matrix.DebugString(), "  FEDCBA9876543210\n"
                                     "0 1111000011111111\n"
                                     "1 0000111100001111\n");
  dl_matrix.CoverColumn(0);
  EXPECT_EQ(dl_matrix.DebugString(), "  FEDCBA987654321\n");

  dl_matrix.UncoverColum(0);
  EXPECT_EQ(dl_matrix.DebugString(), "  FEDCBA9876543210\n"
                                     "0 1111000011111111\n"
                                     "1 0000111100001111\n");
}

TEST(DLMatrix, SolveCoverProblem) {
  std::vector<uint64_t> v = {
      0b10,
      0b11,
  };
  std::vector<std::size_t> rows;
  DLMatrix dl_matrix(v);
  ASSERT_TRUE(SolveCoverProblem(dl_matrix, rows));
  ASSERT_THAT(rows, testing::ElementsAre(1));
}

TEST(DLMatrix, SolveCoverProblemNoSolution) {
  std::vector<uint64_t> v = {
      0b100,
      0b101,
  };
  std::vector<std::size_t> rows;
  DLMatrix dl_matrix(v);
  ASSERT_FALSE(SolveCoverProblem(dl_matrix, rows));
  ASSERT_TRUE(rows.empty());
}

TEST(DLMatrix, SolveCoverProblemTricky) {
  std::vector<uint64_t> v = {
      0b1001001, 0b1001000, 0b0001101, 0b0010110, 0b0110011, 0b0100001,
  };
  std::vector<std::size_t> rows;
  DLMatrix dl_matrix(v);
  ASSERT_TRUE(SolveCoverProblem(dl_matrix, rows));
  ASSERT_THAT(rows, testing::UnorderedElementsAre(1,3,5));
}

TEST(DLMatrix, ExhaustiveSolveCoverProblem) {
  std::vector<uint64_t> v = {
      0b10,
      0b11,
  };
  std::vector<std::vector<std::size_t>> solutions;
  ExhaustiveSolveCoverProblem(v, solutions);
  ASSERT_EQ(solutions.size(), 1);
  ASSERT_THAT(solutions[0], testing::ElementsAre(1));
}

TEST(DLMatrix, ExhaustiveSolveCoverProblemNoSolution) {
  std::vector<uint64_t> v = {
      0b100,
      0b101,
  };
  std::vector<std::vector<std::size_t>> solutions;
  ExhaustiveSolveCoverProblem(v, solutions);
  ASSERT_TRUE(solutions.empty());
}

TEST(DLMatrix, ExhaustiveSolveCoverProblemTricky) {
  std::vector<uint64_t> v = {
      0b1001001, 0b1001000, 0b0001101, 0b0010110, 0b0110011, 0b0100001,
  };
  std::vector<std::vector<std::size_t>> results;
  ExhaustiveSolveCoverProblem(v, results);
  ASSERT_EQ(results.size(), 1);
  ASSERT_THAT(results[0], testing::UnorderedElementsAre(1,3,5));
}
