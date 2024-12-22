#pragma once
#include "polyominos.hpp"
#include <array>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <string>

#include <immintrin.h>

// represent 2 matching patterns in 16x16 grid
std::string BitmaskToString(const __m512i &bitmask);
std::string BitmaskToString(const __m256i &bitmask);

bool get_bit(const __m256i &bitmask, int x, int y);
void set_bit(__m256i &bitmask, int x, int y);
bool get_bit(const __m512i &bitmask, int x, int y, bool second_grid);
void set_bit(__m512i &bitmask, int x, int y, bool second_grid);

struct MatchBitmask {
  __m512i bitmask_rot_0_90;
  __m512i bitmask_rot_180_270;
};

// extracts the canonical representation and the lr flipped representation of a
// polyomino into a bitmask
template <std::size_t N>
void PolyominoToBitMask(const Polyomino<N> &p, __m256i &bitmask) {
  std::memset(&bitmask, 0, sizeof(bitmask));
  for (auto [x, y] : p.xy_cords) {
    set_bit(bitmask, x, y);
  }
}

template <std::size_t N>
void PolyominoToMatchBitMask(const Polyomino<N> &p, MatchBitmask &bitmask) {
  std::memset(&bitmask, 0, sizeof(bitmask));
  for (auto [x, y] : p.xy_cords) {
    set_bit(bitmask.bitmask_rot_0_90, x, y, false);
  }
  for (auto [x, y] : p.rotate_90()._align_to_positive_quadrant().xy_cords) {
    set_bit(bitmask.bitmask_rot_0_90, x, y, true);
  }
  for (auto [x, y] : p.rotate_180()._align_to_positive_quadrant().xy_cords) {
    set_bit(bitmask.bitmask_rot_180_270, x, y, false);
  }
  for (auto [x, y] : p.rotate_270()._align_to_positive_quadrant().xy_cords) {
    set_bit(bitmask.bitmask_rot_180_270, x, y, true);
  }
}

__attribute__((noinline)) void
_find_matches_avx512_16x16(__m256i const &board, __m256i const &candidate,
                           __m512i &result , void* result_ptrs);

// void _find_matches_avx512_16x16_512(__m512i const& board_and_board_flipped,
//                                     __m512i const& candidate_12,
//                                     __m512i &result, __m512i &result2);

// template <std::size_t N, std::size_t K>
// inline std::vector<int64_t>
// FindMatchPatternsAvx(const Polyomino<N> &board,
//                      const Polyomino<K> &tile) noexcept {
//   BoardMatchBitmask bm_board;
//   PolyominoToBoardBitMask(board, bm_board);
//   MatchBitmask bm_tile;
//   PolyominoToMatchBitMask(tile, bm_tile);
//   __m512i result1, result2, result3, result4;

//   _find_matches_avx512_16x16_512(bm_board.board_and_board_flipped,
//   bm_tile.bitmask_rot_0_90,    result1, result2);
//   _find_matches_avx512_16x16_512(bm_board.board_and_board_flipped,
//   bm_tile.bitmask_rot_180_270, result3, result4);

//   // std::vector<int64_t> result;
//   // result.reserve(16*16*8); // 8 possible symmetries on a 16x16 grid
//   return {};
// }