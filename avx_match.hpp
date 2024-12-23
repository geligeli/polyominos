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

class BoardCompression {
public:
  inline BoardCompression(__m256i board) {
    asm(
        R"(
      VPTESTMB %[mask], %[board], %[board]
      KMOVD %[cnt], %[mask]
      POPCNT %[cnt], %[cnt]
    )"
        : [mask] "=Yk"(mask), [cnt] "=r"(cnt)
        : [board] "v"(board)
        :);
    cnt = (cnt + 7) / 8;
    std::memset(data, 0, cnt * 8);
    _mm256_mask_compressstoreu_epi8(data, mask, board);
    // std::cout << cnt << std::endl;
    for (int c = 0; c < cnt; ++c) {
      // std::cout << std::bitset<64>(data[c]) << std::endl;
      per_entry_popcnt[c] = std::popcount(data[c]);
    }
  }

  inline uint64_t compress(__m256i match) {
    uint64_t tmp[4];
    _mm256_mask_compressstoreu_epi8(tmp, mask, match);
    uint64_t result = _pext_u64(tmp[0], data[0]);
    for (int c = 1; c < cnt; ++c) {
      result <<= 1;
      result |= _pext_u64(tmp[0], data[0]);
    }
    return result;
  }

private:
  __mmask32 mask;
  int cnt;
  int per_entry_popcnt[4];
  uint64_t data[4];
};

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

int
_find_matches_avx512_16x16(__m256i const &board, __m256i const &candidate,
                           int candidate_width, int candidate_height,
                           __m256i *resultsPtrs);

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