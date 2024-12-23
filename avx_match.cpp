#include "avx_match.hpp"

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

static const __m512i initial_permute =
    _mm512_set_epi16(14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 15,
                     14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
static const __m512i shift_down =
    _mm512_set_epi16(14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 14,
                     13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0);

static const __m512i initial_clone =
    _mm512_set_epi16(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 15,
                     14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);

// RDI, RSI, RDX, RCX, R8, R9
int _find_matches_avx512_16x16(__m256i const &board, __m256i const &candidate,
                               int candidate_width, int candidate_height,
                               __m256i *result_ptr) {

  int num_outer_loops = 16 - candidate_height;
  int num_inner_loops = (16 - candidate_width) / 2;

  auto start = result_ptr;

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
    JA .end_second_result%=
    JNZ .end_first_result%=
      VMOVDQU64 [%[result_ptr]] {%[lower_4_bits_mask]}, zmm1
      ADD %[result_ptr], 32
    .end_first_result%=:

      VMOVDQU64 [%[result_ptr]-32] {%[upper_4_bits_mask]}, zmm1
      ADD %[result_ptr], 32
    .end_second_result%=:

    VPSLLW zmm1, zmm1, 2
  LOOP .inner_loop%=
    VPERMW zmm3 %{%[down_shift_permutation_mask]}%{z}, %[shift_down], zmm3
    VMOVDQA64 zmm1, zmm3
    dec %[num_outer_loops]
jnz .outer_loop%=
   )"
      : [result_ptr] "+r"(result_ptr)
      : [num_inner_loops] "g"(num_inner_loops),
        [num_outer_loops] "r"(num_outer_loops), [board] "r"(&board),
        [candidate] "r"(&candidate), [initial_permute] "v"(initial_permute),
        [shift_down] "v"(shift_down), [initial_clone] "v"(initial_clone),
        [initial_permuate_mask] "Yk"(0b11111111111111101111111111111111),
        [down_shift_permutation_mask] "Yk"(0b11111111111111101111111111111110),
        [initial_shift_mask] "Yk"(0b11111111111111110000000000000000),
        [lower_4_bits_mask] "Yk"(0b00001111),
        [upper_4_bits_mask] "Yk"(0b11110000)

      // [permutation_mask] "Yk"(0b11111111111111101111111111111110),
      // [bit_extract_mask] "Yk"(0b00001111), [idx_array32] "v"(idx_array32),
      // [swap_boards_array] "r"(swap_boards_array)
      : "zmm0", "zmm1", "zmm2", "zmm3", "k1", "cc", "rcx", "memory");
  return result_ptr - start;
}
