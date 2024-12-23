#include "avx_match.hpp"
#include "polyominos.hpp"

#include <gtest/gtest.h>

TEST(AVXTest, BoardBitMaskConversion) {
  // clang-format off
  Polyomino<7> polyomino_7 = {{
    { {0, 0},
      {0, 1},
      {0, 2},
      {0, 3}, {1, 3}, {2, 3}, {3, 3} }
  }};
  // clang-format on

  __m256i bm;
  PolyominoToBitMask(polyomino_7, bm);
  std::string expected = "1000000000000000\n"
                         "1000000000000000\n"
                         "1000000000000000\n"
                         "1111000000000000\n"
                         "0000000000000000\n"
                         "0000000000000000\n"
                         "0000000000000000\n"
                         "0000000000000000\n"
                         "0000000000000000\n"
                         "0000000000000000\n"
                         "0000000000000000\n"
                         "0000000000000000\n"
                         "0000000000000000\n"
                         "0000000000000000\n"
                         "0000000000000000\n"
                         "0000000000000000\n";
  EXPECT_EQ(BitmaskToString(bm), expected);
}

// TEST(AVXTest, CandidateBitMaskConversion) {
//   // clang-format off
//   Polyomino<9> polyomino_9 = {{
//     { {0, 0}, //{1, 0}, {2, 0}, {3, 0},
//       {0, 1}, //{1, 1}, {2, 1}, {3, 1},
//       {0, 2}, {1, 2}, {2, 2}, {3, 2},
//       {0, 3}, {1, 3}, /* {2, 3},*/ {3, 3} }
//   }};
//   // clang-format on

//   MatchBitmask bm;
//   PolyominoToMatchBitMask(polyomino_9, bm);
//   std::string expected = "1000000000000000|0011000000000000\n"
//                          "1000000000000000|0010000000000000\n"
//                          "1111000000000000|0011000000000000\n"
//                          "1101000000000000|1111000000000000\n"
//                          "0000000000000000|0000000000000000\n"
//                          "0000000000000000|0000000000000000\n"
//                          "0000000000000000|0000000000000000\n"
//                          "0000000000000000|0000000000000000\n"
//                          "0000000000000000|0000000000000000\n"
//                          "0000000000000000|0000000000000000\n"
//                          "0000000000000000|0000000000000000\n"
//                          "0000000000000000|0000000000000000\n"
//                          "0000000000000000|0000000000000000\n"
//                          "0000000000000000|0000000000000000\n"
//                          "0000000000000000|0000000000000000\n"
//                          "0000000000000000|0000000000000000\n";
//   std::string expected2 = "1011000000000000|1111000000000000\n"
//                           "1111000000000000|1100000000000000\n"
//                           "0001000000000000|0100000000000000\n"
//                           "0001000000000000|1100000000000000\n"
//                           "0000000000000000|0000000000000000\n"
//                           "0000000000000000|0000000000000000\n"
//                           "0000000000000000|0000000000000000\n"
//                           "0000000000000000|0000000000000000\n"
//                           "0000000000000000|0000000000000000\n"
//                           "0000000000000000|0000000000000000\n"
//                           "0000000000000000|0000000000000000\n"
//                           "0000000000000000|0000000000000000\n"
//                           "0000000000000000|0000000000000000\n"
//                           "0000000000000000|0000000000000000\n"
//                           "0000000000000000|0000000000000000\n"
//                           "0000000000000000|0000000000000000\n";
//    EXPECT_EQ(BitmaskToString(bm.bitmask_rot_0_90), expected);
//    EXPECT_EQ(BitmaskToString(bm.bitmask_rot_180_270), expected2);
// }

TEST(AVXTest, FindMatches_avx512_16x16_512) {
  // clang-format off
  Polyomino<16> polyomino_9 = {{
    { {0, 0}, {1, 0}, {2, 0}, {3, 0},
      {0, 1}, {1, 1}, {2, 1}, {3, 1},
      {0, 2}, {1, 2}, {2, 2}, {3, 2},
      {0, 3}, {1, 3}, {2, 3}, {3, 3} }
  }};
  // clang-format on

  Polyomino<5> polyomino_5 = {{{
      {0, 0},
      {0, 1},
      {0, 2},
      {1, 2},
      {2, 2},
  }}};

  __m256i board_bm;
  __m256i candidate_bm;
  PolyominoToBitMask(polyomino_9, board_bm);
  PolyominoToBitMask(polyomino_5, candidate_bm);
  // __m512i result, result2;
  // __m512i result;

  const auto [x_max, y_max] = polyomino_5.max_xy();
   __m256i resultsPtrs[256];
  int num_matches =
      _find_matches_avx512_16x16(board_bm, candidate_bm, x_max, y_max, resultsPtrs);

  BoardCompression comp(board_bm);



  for (int i = 0; i < num_matches; ++i) {
    std::cout << BitmaskToString(resultsPtrs[i]) << std::endl;
    std::cout << std::bitset<64>(comp.compress(resultsPtrs[i])) << std::endl;
    
  }
}

// TEST(AVXTest, BitMaskFailedBackCovnersion) {
//   // clang-format off
//   Polyomino<16> polyomino_16 = {{
//     { {0, 0}, {1, 0}, {2, 0}, {3, 0},
//       {0, 1}, {1, 1}, {2, 1}, {3, 1},
//       {0, 2}, {1, 2}, {2, 2}, {3, 2},
//       {0, 3}, {1, 3}, {2, 3}, {3, 3} }
//   }};
//   // clang-format on
//   auto bm = PolyominoToBitmask(polyomino_16);
//   bm.bits[0] = 0;
//   auto r = BitmaskToPolymino(bm);
//   ASSERT_FALSE(r.has_value());
// }

// TEST(AVXTest, MoveLeft) {
//   // clang-format off
//   Polyomino<16> polyomino_16 = {{
//     { {0, 0}, {1, 0}, {2, 0}, {3, 0},
//       {0, 1}, {1, 1}, {2, 1}, {3, 1},
//       {0, 2}, {1, 2}, {2, 2}, {3, 2},
//       {0, 3}, {1, 3}, {2, 3}, {3, 3} }
//   }};
//   auto bm16 = PolyominoToBitmask(polyomino_16);
//   Polyomino<16> polyomino_16_shifted = {{
//     { {1, 0}, {2, 0}, {3, 0}, {4, 0},
//       {1, 1}, {2, 1}, {3, 1}, {4, 1},
//       {1, 2}, {2, 2}, {3, 2}, {4, 2},
//       {1, 3}, {2, 3}, {3, 3} , {4, 3}}
//   }};
//   // clang-format on
//   move_left(bm16);
//   auto val = BitmaskToPolymino(bm16);
//   ASSERT_TRUE(val.has_value());
//   ASSERT_EQ(polyomino_16_shifted, val.value());
// }

// TEST(AVXTest, MoveDown) {
//   // clang-format off
//   Polyomino<16> polyomino_16 = {{
//     { {0, 0}, {1, 0}, {2, 0}, {3, 0},
//       {0, 1}, {1, 1}, {2, 1}, {3, 1},
//       {0, 2}, {1, 2}, {2, 2}, {3, 2},
//       {0, 3}, {1, 3}, {2, 3}, {3, 3} }
//   }};
//   auto bm16 = PolyominoToBitmask(polyomino_16);
//   Polyomino<16> polyomino_16_down = {{
//     { {0, 1}, {1, 1}, {2, 1}, {3, 1},
//       {0, 2}, {1, 2}, {2, 2}, {3, 2},
//       {0, 3}, {1, 3}, {2, 3}, {3, 3},
//       {0, 4}, {1, 4}, {2, 4}, {3, 4} }
//   }};
//   // clang-format on
//   move_down(bm16);
//   auto val = BitmaskToPolymino(bm16);
//   ASSERT_TRUE(val.has_value());
//   ASSERT_EQ(polyomino_16_down, val.value());
// }

// TEST(AVXTest, CandidateInBoard) {
//   // clang-format off
//   Polyomino<16> board = {{
//     { {0, 0}, {1, 0}, {2, 0}, {3, 0},
//       {0, 1}, {1, 1}, {2, 1}, {3, 1},
//       {0, 2}, {1, 2}, {2, 2}, {3, 2},
//       {0, 3}, {1, 3}, {2, 3}, {3, 3} }
//   }};

//   // Polyomino matching_candidate { {1, 1}, {2, 1}, {1, 2}, {2, 2}, };
//   Polyomino<4> candidate = {{
//      {{1, 1}, {2, 1}, {1, 2}, {2, 2}}
//   }};

//   auto bm_board = PolyominoToBitmask(board);
//   auto bm_candidate = PolyominoToBitmask(candidate);

//   ASSERT_TRUE(candidate_inside_board(bm_board, bm_candidate));
//   move_down(bm_candidate); // at border
//   ASSERT_TRUE(candidate_inside_board(bm_board, bm_candidate));
//   move_down(bm_candidate); // past border
//   ASSERT_FALSE(candidate_inside_board(bm_board, bm_candidate));
//   move_down(bm_candidate);
//   ASSERT_FALSE(candidate_inside_board(bm_board, bm_candidate));
//   move_down(bm_candidate);
//   ASSERT_FALSE(candidate_inside_board(bm_board, bm_candidate));
// }

// TEST(AVXTest, CandidateInBoard2) {
//   // clang-format off
//   Polyomino<16> board = {{
//     { {0, 0}, {1, 0}, {2, 0}, {3, 0},
//       {0, 1}, {1, 1}, {2, 1}, {3, 1},
//       {0, 2}, {1, 2}, {2, 2}, {3, 2},
//       {0, 3}, {1, 3}, {2, 3}, {3, 3} }
//   }};

//   // Polyomino matching_candidate { {1, 1}, {2, 1}, {1, 2}, {2, 2}, };
//   Polyomino<4> candidate = {{
//      {{1, 1}, {2, 1}, {1, 2}, {2, 2}}
//   }};

//   auto bm_board = PolyominoToBitmask(board);
//   auto bm_candidate = PolyominoToBitmask(candidate);

//   ASSERT_TRUE(candidate_inside_board(bm_board, bm_candidate));
//   move_left(bm_candidate); // at border
//   ASSERT_TRUE(candidate_inside_board(bm_board, bm_candidate));
//   move_left(bm_candidate); // past border
//   ASSERT_FALSE(candidate_inside_board(bm_board, bm_candidate));
//   move_left(bm_candidate);
//   ASSERT_FALSE(candidate_inside_board(bm_board, bm_candidate));
//   move_left(bm_candidate);
//   ASSERT_FALSE(candidate_inside_board(bm_board, bm_candidate));
// }

// TEST(AVXTest, FindMatches) {
//   // clang-format off
//   Polyomino<16> board = {{
//     { {0, 0}, {1, 0}, {2, 0}, {3, 0},
//       {0, 1}, {1, 1}, {2, 1}, {3, 1},
//       {0, 2}, {1, 2}, {2, 2}, {3, 2},
//       {0, 3}, {1, 3}, {2, 3}, {3, 3} }
//   }};

//   // Polyomino matching_candidate { {1, 1}, {2, 1}, {1, 2}, {2, 2}, };
//   Polyomino<4> candidate = {{
//      {{1, 1}, {2, 1}, {1, 2}, {2, 2}}
//   }};

//   auto bm_board = PolyominoToBitmask(board);
//   auto bm_candidate = PolyominoToBitmask(candidate);

//   auto matches = find_matches(bm_board, bm_candidate);

//   std::cout << BitmaskToString(matches) << std::endl;

// }

// TEST(Polyomino, Rotate) {
//   // clang-format off
//   Polyomino<4> candidate = {{
//      {{1, 1}, {2, 1}, {1, 2}, {2, 2}}
//   }};

//   // clang-format on
//   const auto s = candidate.rotate_90();

//   std::cout << (int)s.xy_cords[0].first << " " << (int)s.xy_cords[0].second
//   << std::endl; std::cout << (int)s.xy_cords[1].first << " " <<
//   (int)s.xy_cords[1].second << std::endl; std::cout <<
//   (int)s.xy_cords[2].first << " " << (int)s.xy_cords[2].second << std::endl;

//   auto mask = PolyominoToBitmask(s);

//   std::cout << BitmaskToString(mask) << std::endl;
// }

// TEST(AVXTest, FindMatches2) {
//   // clang-format off
//   Polyomino<16> board = {{
//     { {0, 0}, {1, 0}, {2, 0}, {3, 0},
//       {0, 1}, {1, 1}, {2, 1}, {3, 1},
//       {0, 2}, {1, 2}, {2, 2}, {3, 2},
//       {0, 3}, {1, 3}, {2, 3}, {3, 3} }
//   }};

//   Polyomino<4> candidate = {{
//      {{1, 1}, {2, 1}, {1, 2}, {2, 2}}
//   }};
//   // clang-format on

//   std::array<Bitmask<4>, 8> symmetries;
//   const auto s = candidate.symmetries();
//   for (int i = 0; i < 8; i++) {
//     symmetries[i] = PolyominoToBitmask(s[i]._align_to_positive_quadrant());
//   }

//   auto bm_board = PolyominoToBitmask(board);

//   for (int i = 0; i < 8; i++) {
//     auto matches = find_matches2(bm_board, symmetries[i]);
//     std::cout << BitmaskToString(symmetries[i]) << std::endl;
//     std::cout << BitmaskToString(matches) << std::endl;
//     std::cout << "----" << std::endl;
//   }
// }
