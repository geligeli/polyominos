#include "avx_match.hpp"

#include <cstdint>
#include <cstring>
#include <immintrin.h>

bool get_bit(const __m256i &bitmask, int x, int y) {
  const auto bit_index = y * 16 + x;
  const auto int64_idx = (bit_index) / 64;
  return bitmask[int64_idx] & (int64_t{1} << (bit_index % 64));
}

void set_bit(__m256i &bitmask, int x, int y) {
  const auto bit_index = y * 16 + x;
  const auto int64_idx = (bit_index) / 64;
  bitmask[int64_idx] |= (int64_t{1} << (bit_index % 64));
}

std::string BitmaskToString(const __m256i &bitmask) {
  constexpr int kNCols = 16 + 1;
  std::string out_buffer(16 * kNCols, ' ');
  // 17 columns
  auto at_xy = [&](int x, int y) -> char & {
    return out_buffer[y * kNCols + x];
  };
  auto at_xy_grid = [&](int x, int y) -> char & { return at_xy(x, y); };
  auto at_xy_bitmask = [&](int x, int y) -> bool {
    const auto bit_index = y * 16 + x;
    const auto int64_idx = (bit_index) / 64;
    return bitmask[int64_idx] & (int64_t{1} << (bit_index % 64));
  };
  for (int y = 0; y < 16; y++) {
    at_xy(16, y) = '\n';
  }
  for (int y = 0; y < 16; y++) {
    for (int x = 0; x < 16; x++) {
      at_xy_bitmask(x, y) ? at_xy_grid(x, y) = '1' : at_xy_grid(x, y) = '0';
    }
  }
  return out_buffer;
};

#ifdef USE_AVX512

bool get_bit(const __m512i &bitmask, int x, int y, bool second_grid) {
  const auto bit_index = y * 16 + x;
  const auto int64_idx = (bit_index) / 64 + (second_grid ? 4 : 0);
  return bitmask[int64_idx] & (int64_t{1} << (bit_index % 64));
}

void set_bit(__m512i &bitmask, int x, int y, bool second_grid) {
  const auto bit_index = y * 16 + x;
  const auto int64_idx = (bit_index) / 64 + (second_grid ? 4 : 0);
  bitmask[int64_idx] |= (int64_t{1} << (bit_index % 64));
}

// represent 2 matching patterns in 16x16 grid
std::string BitmaskToString(const __m512i &bitmask) {
  std::string out_buffer(16 * 16 * 2 + 32, ' ');
  // 34 columns
  auto at_xy = [&](int x, int y) -> char & { return out_buffer[y * 34 + x]; };
  auto at_xy_grid1 = [&](int x, int y) -> char & { return at_xy(x, y); };
  auto at_xy_grid2 = [&](int x, int y) -> char & { return at_xy(x + 17, y); };
  auto at_xy_bitmask1 = [&](int x, int y) -> bool {
    const auto bit_index = y * 16 + x;
    const auto int64_idx = (bit_index) / 64;
    return bitmask[int64_idx] & (int64_t{1} << (bit_index % 64));
  };
  auto at_xy_bitmask2 = [&](int x, int y) -> bool {
    const auto bit_index = y * 16 + x;
    const auto int64_idx = 4 + (bit_index) / 64;
    return bitmask[int64_idx] & (int64_t{1} << (bit_index % 64));
  };
  for (int y = 0; y < 16; y++) {
    at_xy(16, y) = '|';
    at_xy(16 + 16 + 1, y) = '\n';
  }
  for (int y = 0; y < 16; y++) {
    for (int x = 0; x < 16; x++) {
      at_xy_bitmask2(x, y) ? at_xy_grid2(x, y) = '1' : at_xy_grid2(x, y) = '0';
      at_xy_bitmask1(x, y) ? at_xy_grid1(x, y) = '1' : at_xy_grid1(x, y) = '0';
    }
  }
  return out_buffer;
};

static const __m512i initial_permute =
    _mm512_set_epi16(14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 15,
                     14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);

static const __m512i shift_down = _mm512_set_epi16(
    14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 30, 29, 28, 27, 26, 25,
    24, 23, 22, 21, 20, 19, 18, 17, 16, 16);
static const uint64_t shift_down_mask = 0xfffefffe;

static const __m512i initial_clone =
    _mm512_set_epi16(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 15,
                     14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);

int _find_matches_avx512_16x16(__m256i const &board, __m256i const &candidate,
                               std::pair<uint8_t, uint8_t> max_xy_board,
                               std::pair<uint8_t, uint8_t> max_xy_candidate,
                               __m256i (&results)[256]) {
  if (max_xy_candidate.first > max_xy_board.first) {
    return 0;
  }
  if (max_xy_candidate.second > max_xy_board.second) {
    return 0;
  }

  // uint32_t num_outer_loops = 16 - max_xy_candidate.second;
  // uint32_t num_inner_loops = (16 - max_xy_candidate.first) / 2;
  uint32_t num_outer_loops = max_xy_board.second - max_xy_candidate.second + 1;
  uint32_t num_inner_loops =
      (max_xy_board.first - max_xy_candidate.first) / 2 + 1;

  __m256i *result_ptr = &results[0];

  asm volatile(
      R"(
    // zmm0 board
    // zmm1 candidate for inner loop
    // zmm2 matching mask
    // zmm3 candidate for outer loop
    // rcx = loop counters

    VMOVDQA64 ymm0, [%[board]]
    VMOVDQA64 ymm1, [%[candidate]]

    VPERMW zmm0, %[initial_clone], zmm0
    VPERMW zmm1, %[initial_clone], zmm1
    VPSLLW zmm1 %{%[initial_shift_mask]}, zmm1, 1
    VMOVDQA64 zmm3, zmm1

.outer_loop%=:
    // Inner loop
    MOV ecx, %[num_inner_loops]
  .inner_loop%=:
    VPTERNLOGD zmm2, zmm1, zmm0, 0x44
    VPTESTMQ k1, zmm2, zmm2

    KTESTB %[lower_4_bits_mask], k1
    // ZF is set if board has a match
    // zmm1[255:0] is fit
    // CF is set if flipped board has a match
    // zmm1[511:256] is fit
    JA .end_write_results%=
    
    // jump if zf is 0
    JNZ .write_second_result%=
      VMOVDQU64 [%[result_ptr]] %{%[lower_4_bits_mask]}, zmm1
      ADD %[result_ptr], 32
    KTESTB %[lower_4_bits_mask], k1
    // jump if cf is 0
    JAE .end_write_results%=
    .write_second_result%=: 
      VMOVDQU64 [%[result_ptr]-32] %{%[upper_4_bits_mask]}, zmm1
      ADD %[result_ptr], 32
    .end_write_results%=:

    VPSLLW zmm1, zmm1, 2
  LOOP .inner_loop%=
    VPERMW zmm3 %{%[shift_down_mask]}%{z}, %[shift_down], zmm3
    VMOVDQA64 zmm1, zmm3
    dec %[num_outer_loops]
jnz .outer_loop%=
   )"
      : [result_ptr] "+r"(result_ptr)
      : [num_inner_loops] "r"(num_inner_loops),
        [num_outer_loops] "r"(num_outer_loops), [board] "r"(&board),
        [candidate] "r"(&candidate), [initial_permute] "v"(initial_permute),
        [shift_down] "v"(shift_down), [initial_clone] "v"(initial_clone),
        [initial_permuate_mask] "Yk"(0b11111111111111101111111111111111),
        [shift_down_mask] "Yk"(shift_down_mask),
        [initial_shift_mask] "Yk"(0b11111111111111110000000000000000),
        [lower_4_bits_mask] "Yk"(0b00001111),
        [upper_4_bits_mask] "Yk"(0b11110000)
      : "zmm0", "zmm1", "zmm2", "zmm3", "k1", "cc", "ecx", "al", "memory");
  return result_ptr - &results[0];
}

#else

void _mask_compressstoreu_epi8(uint64_t (&output)[4], __mmask32 k, __m256i a) {
  std::array<uint8_t, 32> a_array;
  std::array<uint8_t, 32> out_array = {};

  std::memcpy(&a_array, &a, sizeof(a_array));
  uint8_t *src_addr = a_array.data();
  uint8_t *dst_addr = out_array.data();
  while (k != 0) {
    if (k & 1) {
      *dst_addr = *src_addr;
      ++dst_addr;
    }
    ++src_addr;
    k >>= 1;
  }
  std::memcpy(output, out_array.data(), sizeof(output));
}

namespace {

int _find_matches_avx2(__m256i const &board, __m256i const &candidate,
                       std::pair<uint8_t, uint8_t> max_xy_board,
                       std::pair<uint8_t, uint8_t> max_xy_candidate,
                       __m256i (&results)[256]) {
  if (max_xy_candidate.first > max_xy_board.first) {
    return 0;
  }
  if (max_xy_candidate.second > max_xy_board.second) {
    return 0;
  }

  uint32_t num_outer_loops = max_xy_board.second - max_xy_candidate.second + 1;
  uint32_t num_inner_loops = max_xy_board.first - max_xy_candidate.first + 1;

  int num_results = 0;

  constexpr auto shift_left = [](__m256i a) -> __m256i {
    std::array<uint16_t, 16> working_set;   
    std::memcpy(&working_set, &a, sizeof(a));
    for (int i=0; i<16; i++) {
        working_set[i] <<=1;
    }
    return std::bit_cast<__m256i>(working_set);
};


  constexpr auto shift_down = [](__m256i a) -> __m256i{
    std::array<uint16_t, 16> src;
    std::array<uint16_t, 16> result;
    result[0]=0;
    std::memcpy(&src, &a, sizeof(a));
    for (int i=1; i<16; ++i) {
      result[i] = src[i-1];
    }
    return std::bit_cast<__m256i>(result);
  };
  
  __m256i c = candidate;
  for (uint32_t i = 0; i < num_outer_loops; ++i) {
    __m256i c_inner = c;
    for (uint32_t j = 0; j < num_inner_loops; ++j) {
      if ((c_inner[0] & ~board[0]) == 0 &&
          (c_inner[1] & ~board[1]) == 0 &&
          (c_inner[2] & ~board[2]) == 0 &&
          (c_inner[3] & ~board[3]) == 0) {
        results[num_results++] = c_inner;
      }
      c_inner=shift_left(c_inner);
    }
    c = shift_down(c);
  }
  return num_results;
}

} // namespace

#endif

std::vector<uint64_t>
find_matches_avx(BoardMatcher const &board,
                    CandidateMatchBitmask const &candidate) {
  auto board_max_xy = board.max_xy();
  std::vector<uint64_t> results;
  for (int i = 0; i < candidate.cnt; ++i) {
    __m256i tmp[256];

#ifdef USE_AVX512
    auto num_matches =
        _find_matches_avx512_16x16(board.board(), candidate.bitmasks[i],
                                   board_max_xy, candidate.max_xy[i], tmp);
#else
    auto num_matches =_find_matches_avx2(board.board(), candidate.bitmasks[i],
                                   board_max_xy, candidate.max_xy[i], tmp);
#endif

    for (int j = 0; j < num_matches; ++j) {
      results.push_back(board.compress(tmp[j]));
    }
  }
  std::sort(results.begin(), results.end());
  results.erase(std::unique(results.begin(), results.end()), results.end());
  return results;
}


BoardMatcher::BoardMatcher(__m256i board, std::pair<uint8_t, uint8_t> max_xy)
    : m_board(board), m_max_xy(max_xy) {
#ifdef USE_AVX512
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
#else
  std::array<uint8_t, 32> a = std::bit_cast<std::array<uint8_t, 32>>(board);
  mask = 0;
  for (int i = 0; i < 32; ++i) {
    mask |= a[i] > 0 ? 1 << i : 0;
  }
  cnt = std::popcount((uint64_t)board[0]) + std::popcount((uint64_t)board[1]) +
        std::popcount((uint64_t)board[2]) + std::popcount((uint64_t)board[3]);
  cnt = (cnt + 7) / 8;
  std::memset(data, 0, cnt * 8);
  _mask_compressstoreu_epi8(data, mask, board);
#endif
  
  per_entry_popcnt[0] = 0;
  for (std::size_t c = 1; c < cnt; ++c) {
    per_entry_popcnt[c] = std::popcount(data[c - 1]) + per_entry_popcnt[c - 1];
  }
}
