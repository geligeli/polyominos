#include "avx_match.hpp"
#include "polyominos.hpp"

#include <gtest/gtest.h>

TEST(AVXTest, TestCppMatcher) {
  auto polyomino_board = PrecomputedPolyminosSet<12>::polyminos()[0];
  auto polyomino_5 = PrecomputedPolyminosSet<5>::polyminos()[0];
  auto patterns = FindMatchPatterns(polyomino_board, polyomino_5);

  for (auto pattern : patterns) {
    ASSERT_EQ(std::popcount(pattern), 5);
  }
  ASSERT_EQ(patterns.size(), 8u);
}


TEST(AVXTest, TestAvxMatcher) {
  auto polyomino_board = PrecomputedPolyminosSet<12>::polyminos()[0];
  auto polyomino_5 = PrecomputedPolyminosSet<5>::polyminos()[0];

  BoardMatcher board = PolyominoToBoardMatcher(polyomino_board);
  CandidateMatchBitmask candidate;
  PolyominoToMatchBitMask(polyomino_5, candidate);
  std::vector<uint64_t> patterns = find_matches_avx512(board, candidate);
  ASSERT_EQ(patterns, FindMatchPatterns(polyomino_board, polyomino_5));
}

TEST(AVXTest, FindMatches_avx512_16x16_512) {
  int outer_idx = 0;
  for (auto polyomino_board : PrecomputedPolyminosSet<12>::polyminos()) {
    int inner_idx = 0;
    for (auto polyomino_5 : PrecomputedPolyminosSet<5>::polyminos()) {
      BoardMatcher board = PolyominoToBoardMatcher(polyomino_board);
      CandidateMatchBitmask candidate;
      PolyominoToMatchBitMask(polyomino_5, candidate);
      std::vector<uint64_t> results_a = find_matches_avx512(board, candidate);
      std::vector<uint64_t> results_b =
          FindMatchPatterns(polyomino_board, polyomino_5);
      SCOPED_TRACE("Outer index: " + std::to_string(outer_idx) +
                   " Inner index: " + std::to_string(inner_idx));
      ASSERT_EQ(results_a, results_b);
      inner_idx++;
    }
    outer_idx++;
  }
}