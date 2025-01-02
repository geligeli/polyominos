#include "dl_matrix.hpp"

#include "gmock/gmock.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iostream>
#include <numeric>
#include <random>
#include <ranges>

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


/*
TEST(DLMatrix, RemoveRestoreRow) {
  std::vector<uint64_t> v = {0xffff, 0xaaaa, 0x8888, 0x8080};
  DLMatrix dl_matrix(v);

  ASSERT_EQ(dl_matrix.DebugString(), "  FEDCBA9876543210\n"
                                     "0 1111111111111111\n"
                                     "1 1010101010101010\n"
                                     "2 1000100010001000\n"
                                     "3 1000000010000000\n");

  dl_matrix.DetachRow(0);
  dl_matrix.DetachRow(17);
  ASSERT_EQ(dl_matrix.DebugString(), "  FEDCBA9876543210\n"
                                     "2 1000100010001000\n"
                                     "3 1000000010000000\n");
  dl_matrix.RestoreRow(17);
  dl_matrix.RestoreRow(0);
  ASSERT_EQ(dl_matrix.DebugString(), "  FEDCBA9876543210\n"
                                     "0 1111111111111111\n"
                                     "1 1010101010101010\n"
                                     "2 1000100010001000\n"
                                     "3 1000000010000000\n");
}

TEST(DLMatrix, RemoveRestoreRowFuzzing) {
  std::vector<uint64_t> v = {0xffff, 0xaaaa, 0x8888, 0x8080};
  DLMatrix dl_matrix(v);

  ASSERT_EQ(dl_matrix.DebugString(), "  FEDCBA9876543210\n"
                                     "0 1111111111111111\n"
                                     "1 1010101010101010\n"
                                     "2 1000100010001000\n"
                                     "3 1000000010000000\n");
  std::vector<std::size_t> row_indices = {0, 16, 24, 28};

  std::random_device rd;
  std::mt19937 g(rd());

  for (int i = 0; i < 100; ++i) {
    std::shuffle(row_indices.begin(), row_indices.end(), g);
    std::size_t num_elements =
        std::uniform_int_distribution<>(1, row_indices.size())(g);
    for (auto r : row_indices | std::views::take(num_elements)) {
      dl_matrix.DetachRow(r);
    }
    for (auto r :
         row_indices | std::views::take(num_elements) | std::views::reverse) {
      dl_matrix.RestoreRow(r);
    }

    ASSERT_EQ(dl_matrix.DebugString(), "  FEDCBA9876543210\n"
                                       "0 1111111111111111\n"
                                       "1 1010101010101010\n"
                                       "2 1000100010001000\n"
                                       "3 1000000010000000\n");
  }
}

TEST(DLMatrix, RemoveRelatedElements) {
  std::vector<uint64_t> v = {
      0b110011,
      0b001100,
      0b001001,
  };
  DLMatrix dl_matrix(v);
  ASSERT_EQ(dl_matrix.DebugString(), "  543210\n"
                                     "0 110011\n"
                                     "1 001100\n"
                                     "2 001001\n");
  std::vector<DLMatrix::PtrType> rows_removed;
  dl_matrix.DetachRelatedEntries(4, rows_removed);
  ASSERT_EQ(dl_matrix.DebugString(), "  5410\n"
                                     "0 1111\n");
  dl_matrix.RestoreRelatedEntries(4, rows_removed);
  ASSERT_EQ(dl_matrix.DebugString(), "  543210\n"
                                     "0 110011\n"
                                     "1 001100\n"
                                     "2 001001\n");
}

TEST(DLMatrix, RemoveAll) {
  std::vector<uint64_t> v = {
      0b110011,
      0b001100,
      0b001001,
  };
  DLMatrix dl_matrix(v);
  ASSERT_EQ(dl_matrix.DebugString(), "  543210\n"
                                     "0 110011\n"
                                     "1 001100\n"
                                     "2 001001\n");
  std::vector<DLMatrix::PtrType> rows_removed;
  dl_matrix.DetachRelatedEntries(4, rows_removed);
  ASSERT_EQ(dl_matrix.DebugString(), "  5410\n"
                                     "0 1111\n");

  std::vector<DLMatrix::PtrType> rows_removed2;
  dl_matrix.DetachRelatedEntries(0, rows_removed2);
  ASSERT_EQ(dl_matrix.DebugString(), "  \n");

  dl_matrix.ForEachColumn([&](DLMatrix::ColumnIndex col) {
    ASSERT_EQ(dl_matrix.col_headers[col].col_size, 0) << "Column: " << col;
  });
  ASSERT_EQ(dl_matrix.initial_col_header, -1);
  dl_matrix.RestoreRelatedEntries(0, rows_removed2);
  dl_matrix.RestoreRelatedEntries(4, rows_removed);
  ASSERT_EQ(dl_matrix.DebugString(), "  543210\n"
                                     "0 110011\n"
                                     "1 001100\n"
                                     "2 001001\n");
}

TEST(DLMatrix, ImpossibleToSolve) {
  std::vector<uint64_t> v = {
      0b100,
      0b101,
  };
  DLMatrix dl_matrix(v);
  ASSERT_EQ(dl_matrix.status(), DLMatrix::Status::NO_SOLUTION);
}

TEST(DLMatrix, SimpleSolve) {
  std::vector<uint64_t> v = {
      0b10,
      0b11,
  };
  DLMatrix dl_matrix(v);
  ASSERT_EQ(dl_matrix.status(), DLMatrix::Status::OK);

  std::vector<DLMatrix::PtrType> rows_removed;
  dl_matrix.DetachRelatedEntries(0, rows_removed);
  ASSERT_EQ(dl_matrix.num_columns, 1);
  ASSERT_EQ(dl_matrix.status(), DLMatrix::Status::NO_SOLUTION);
  dl_matrix.RestoreRelatedEntries(0, rows_removed);
  ASSERT_EQ(dl_matrix.status(), DLMatrix::Status::OK);

  dl_matrix.DetachRelatedEntries(1, rows_removed);
  ASSERT_EQ(dl_matrix.status(), DLMatrix::Status::SOLVED);
}
*/

TEST(DLMatrix, SolveCoverProblem) {
  std::vector<uint64_t> v = {
      0b10,
      0b11,
  };
  std::vector<std::size_t> rows;
  ASSERT_TRUE(SolveCoverProblem(v, rows));
  ASSERT_THAT(rows, testing::ElementsAre(1));
}

TEST(DLMatrix, SolveCoverProblemNoSolution) {
  std::vector<uint64_t> v = {
      0b100,
      0b101,
  };
  std::vector<std::size_t> rows;
  ASSERT_FALSE(SolveCoverProblem(v, rows));
  ASSERT_TRUE(rows.empty());
}

TEST(DLMatrix, SolveCoverProblemTricky) {
  std::vector<uint64_t> v = {
      0b1001001, 0b1001000, 0b0001101, 0b0010110, 0b0110011, 0b0100001,
  };
  std::vector<std::size_t> rows;
  ASSERT_TRUE(SolveCoverProblem(v, rows));
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
