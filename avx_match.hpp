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

class BoardMatcher {
public:
  inline BoardMatcher(__m256i board, std::pair<uint8_t, uint8_t> max_xy) : m_board(board), m_max_xy(max_xy) {
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
    per_entry_popcnt[0] = 0;
    for (int c = 1; c < cnt; ++c) {
      per_entry_popcnt[c] = std::popcount(data[c-1]) + per_entry_popcnt[c-1];
    }
  }

  inline uint64_t compress(__m256i match) const {
    uint64_t tmp[4];
    _mm256_mask_compressstoreu_epi8(tmp, mask, match);
    uint64_t result{};
    for (int c = 0; c < cnt; ++c) {
      result |= (_pext_u64(tmp[c], data[c]) << per_entry_popcnt[c]);
    }
    return result;
  }

  __m256i board() const { return m_board; }
  std::pair<uint8_t, uint8_t> max_xy() const { return m_max_xy; }

private:
  __m256i m_board;
  __mmask32 mask;

  std::pair<uint8_t, uint8_t> m_max_xy;
  uint32_t cnt;
  int per_entry_popcnt[4];
  uint64_t data[4];
};

// extracts the canonical representation and the lr flipped representation of a
// polyomino into a bitmask
template <std::size_t N>
BoardMatcher PolyominoToBoardMatcher(const Polyomino<N> &p) {
  __m256i bitmask;
  std::memset(&bitmask, 0, sizeof(bitmask));
  for (auto [x, y] : p.xy_cords) {
    set_bit(bitmask, x, y);
  }
  return BoardMatcher{bitmask, p.max_xy()};
}

struct CandidateMatchBitmask {
  __m256i bitmasks[8];
  std::pair<uint8_t, uint8_t> max_xy[8];
  int cnt;
};

template <std::size_t N>
void PolyominoToMatchBitMask(const Polyomino<N> &p,
                             CandidateMatchBitmask &matcher) {
  std::memset(&matcher, 0, sizeof(matcher));
  for (auto b : p.symmetries()) {
    b = b._align_to_positive_quadrant();
    const auto [xy_max_x, xy_max_y] = b.max_xy();
    matcher.max_xy[matcher.cnt] = {static_cast<uint8_t>(xy_max_x),
                                   static_cast<uint8_t>(xy_max_y)};
    for (auto [x, y] : b.xy_cords) {
      set_bit(matcher.bitmasks[matcher.cnt], x, y);
    }
    matcher.cnt++;
  }
}

std::vector<uint64_t>
find_matches_avx512(BoardMatcher const &board,
                    CandidateMatchBitmask const &candidate);

int _find_matches_avx512_16x16(__m256i const &board, __m256i const &candidate,
                               std::pair<uint8_t, uint8_t> max_xy_board,
                               std::pair<uint8_t, uint8_t> max_xy_candidate,
                               __m256i (&results)[256]);
