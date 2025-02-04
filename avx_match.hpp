#pragma once
#include "polyominos.hpp"
#include <array>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <string>

#include <immintrin.h>


#ifdef __AVX512VL__
// #define USE_AVX512
#endif

// represent 2 matching patterns in 16x16 grid
std::string BitmaskToString(const __m256i &bitmask);
bool get_bit(const __m256i &bitmask, int x, int y);
void set_bit(__m256i &bitmask, int x, int y);

#ifdef USE_AVX512
bool get_bit(const __m512i &bitmask, int x, int y, bool second_grid);
void set_bit(__m512i &bitmask, int x, int y, bool second_grid);
std::string BitmaskToString(const __m512i &bitmask);
int _find_matches_avx512_16x16(__m256i const &board, __m256i const &candidate,
                               std::pair<uint8_t, uint8_t> max_xy_board,
                               std::pair<uint8_t, uint8_t> max_xy_candidate,
                               __m256i (&results)[256]);
#else
void _mask_compressstoreu_epi8(uint64_t (&output)[4], __mmask32 k, __m256i a);
#endif

class BoardMatcher {
public:
  BoardMatcher(__m256i board, std::pair<uint8_t, uint8_t> max_xy);

  inline uint64_t compress(__m256i match) const {
    uint64_t tmp[4];
#ifdef USE_AVX512
    _mm256_mask_compressstoreu_epi8(tmp, mask, match);
#else
    _mask_compressstoreu_epi8(tmp, mask, match);
#endif
    uint64_t result{};
    for (std::size_t c = 0; c < cnt; ++c) {
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
  // matcher = {};
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

std::vector<uint64_t> find_matches_avx(BoardMatcher const &board,
                                       CandidateMatchBitmask const &candidate);
