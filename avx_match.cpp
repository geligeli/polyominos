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
  auto at_xy = [&](int x, int y) -> char & { return out_buffer[y * kNCols + x]; };
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

// alignas(64) const uint16_t idx_array32[32] = {
//     0, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
//     0, 16, 17, 18, 19, 20, 21, 22, 23, 23, 25, 26, 27, 28, 29, 30};

// alignas(64) const uint64_t swap_boards_array[8] = {4, 5, 6, 7, 0, 1, 2, 3};


static const __m512i initial_permute = _mm512_set_epi16(14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
static const __m512i initial_clone = _mm512_set_epi16(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);

// RDI, RSI, RDX, RCX, R8, R9
void _find_matches_avx512_16x16(__m256i const &board, __m256i const &candidate,
                                __m512i &result , void* result_ptr) {
std::cout << result_ptr << std::endl;

  // __m512i initial_permute = _mm512_set_epi16(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
  //                                            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
  // int b=123;
  asm volatile(
      R"(
    // zmm0 board
    // zmm1 candidate
    // zmm2 matching mask
    // r9 = number of results
    // rcx, r8 = loop counters

    VMOVDQA64 ymm0, [%[board]]
    VMOVDQA64 ymm1, [%[candidate]]
    // VPERMW %[result] %{%[initial_permuate_mask]}%{z}, %[initial_permute], zmm0
    VPERMW zmm0 %{%[initial_permuate_mask]}%{z}, %[initial_permute], zmm0
    VPERMW zmm1, %[initial_clone], zmm1
    VPSLLW zmm1 %{%[initial_shift_mask]}, zmm1, 1

    XOR r9, r9
    MOV r8, 8
.outer_loop%=:
    // Inner loop
    MOV ecx, 8
  .inner_loop%=:
    VPTERNLOGD zmm2, zmm1, zmm0, 0x22
    VPTESTMQ k1, zmm2, zmm2
    KTESTB k1, %[lower_4_bits_mask]

    // ZF is set if board has a match
    // zmm1[255:0] is fit
    JNZ .end_first_result%=
      // int 3
      VMOVDQA64 [%[result_ptr]] {%[lower_4_bits_mask]}, zmm1
      ADD %[result_ptr], 32
    .end_first_result%=:

    // CF is set if flipped board has a match
    // zmm1[511:256] is fit
    JAE .end_second_result%=
      VMOVDQA64 [%[result_ptr]-32] {%[upper_4_bits_mask]}, zmm1
      ADD %[result_ptr], 32
    .end_second_result%=:
    VPSLLW zmm1, zmm1, 2
  LOOP .inner_loop%=

    VPERMW zmm1 %{%[down_shift_permutation_mask]}%{z}, %[initial_permute], zmm1
    dec r8
jnz .outer_loop%=

    VMOVDQA64 %[result], zmm3


   )" :[result] "=v"(result),
       [result_ptr] "+r" (result_ptr)
      :
      [board] "r" (&board),
      [candidate] "r"(&candidate),
      [initial_permute] "v" (initial_permute),
      [initial_clone] "v" (initial_clone),
           [initial_permuate_mask] "Yk" (0b11111111111111101111111111111111),
      [down_shift_permutation_mask] "Yk"(0b11111111111111101111111111111110),
              [initial_shift_mask] "Yk" (0b11111111111111110000000000000000),
      [lower_4_bits_mask] "Yk"(0b00001111),
      [upper_4_bits_mask] "Yk"(0b11110000)
      

      // [permutation_mask] "Yk"(0b11111111111111101111111111111110),
      // [bit_extract_mask] "Yk"(0b00001111), [idx_array32] "v"(idx_array32),
      // [swap_boards_array] "r"(swap_boards_array)
      : "zmm0", "zmm1", "zmm2", "k1", "rcx", "r8", "r9", "memory"
      );
  std::cout << result_ptr << std::endl;
}

/*
//  ZMM0 is the shifted candidate
//  ZMM1 is the intersecion mask for board and candidate

     VPERMQ %[board_and_board_flipped], zmm28, %[board_and_board_flipped]
     board_and_board_flipped
     VPTESTMB k2,
     VMOVDQA64 zmm28, [%[swap_boards_array]]
     mov rdx, %[result]
      ebx, r9
 .second_rount%=:
     xor eax, eax // 32 bit line result = 2x16 bit
     xor r10, r10
 .start%=:
     // generate masks for the candidate
     vmovdqa64 ZMM0, %[candidate_12]
     mov r12, 17
     mov ECX, 16
     .pattern_gen%=:
     VPTERNLOGD ZMM1, ZMM0, %[board_and_board_flipped] 0b00100010
     VPTESTMQ k1, ZMM1, ZMM1
     KTESTB k1, %[bit_extract_mask]
     // ZF is set if board has a match
     // CF is set if flipped board has a match
     VPSLLW ZMM0, ZMM0, 2
     LOOP .pattern_gen%=
     // shift pattern 1 row down
     VPERMW %[candidate_12] %{%[permutation_mask]}%{z}, %[idx_array32],
%[candidate_12] mov [rdx], eax add rdx, 4 inc r10 cmp r10, 16 jne .start%= cmp
r9, 1 je .end%= VPERMQ %[board_and_board_flipped], zmm28,
%[board_and_board_flipped] mov rdx, %[result2] mov r9, 1 jmp .second_rount%=
 .end%=:
*/

/*
void _find_matches_avx512_16x16_512(__m256i const& board,
                                    __m256i const& candidate,
                                    __m512i &result, __m512i &result2) {
  asm volatile(R"(
//  ZMM0 is the shifted candidate
//  ZMM1 is the intersecion mask for board and candidate

    VPERMQ %[board_and_board_flipped], zmm28, %[board_and_board_flipped]
    board_and_board_flipped
    VPTESTMB k2,
    VMOVDQA64 zmm28, [%[swap_boards_array]]
    mov rdx, %[result]
     ebx, r9
.second_rount%=:
    xor eax, eax // 32 bit line result = 2x16 bit
    xor r10, r10
.start%=:
    // generate masks for the candidate
    vmovdqa64 ZMM0, %[candidate_12]
    mov r12, 17


    mov ECX, 16
    .pattern_gen%=:
    VPTERNLOGD ZMM1, ZMM0, %[board_and_board_flipped] 0b00100010
    VPTESTMQ k1, ZMM1, ZMM1
    KTESTB k1, %[bit_extract_mask]

    // ZF is set if board has a match
    // CF is set if flipped board has a match

    VPSLLW ZMM0, ZMM0, 2
    LOOP .pattern_gen%=

    // shift pattern 1 row down
    VPERMW %[candidate_12] %{%[permutation_mask]}%{z}, %[idx_array32],
%[candidate_12] mov [rdx], eax add rdx, 4 inc r10 cmp r10, 16 jne .start%=

    cmp r9, 1
    je .end%=

    VPERMQ %[board_and_board_flipped], zmm28, %[board_and_board_flipped]
    mov rdx, %[result2]
    mov r9, 1
    jmp .second_rount%=
.end%=:
  )"
               : [result] "+v"(result), [result2] "+v"(result2)
               :
                 [board] "v"(board),
                 [candidate_12] "v"(candidate_12),
                 [permutation_mask] "Yk"(0b11111111111111101111111111111110),
                 [bit_extract_mask] "Yk"(0b00001111),
                 [idx_array32] "v"(idx_array32),
                 [swap_boards_array] "r"(swap_boards_array)
               :
                  "eax",
                  "k1", "memory");
}
*/

//   asm volatile(R"(
// //  ZMM0 is the shifted candidate
// //  ZMM1 is the intersecion mask for board and candidate

//     VPERMQ %[board_and_board_flipped], zmm28, %[board_and_board_flipped]
//     board_and_board_flipped
//     VPTESTMB k2,
//     VMOVDQA64 zmm28, [%[swap_boards_array]]
//     mov rdx, %[result]
//      ebx, r9
// .second_rount%=:
//     xor eax, eax // 32 bit line result = 2x16 bit
//     xor r10, r10
// .start%=:
//     // generate masks for the candidate
//     vmovdqa64 ZMM0, %[candidate_12]
//     mov r12, 17

//     mov ECX, 16
//     .pattern_gen%=:
//     VPTERNLOGD ZMM1, ZMM0, %[board_and_board_flipped] 0b00100010
//     VPTESTMQ k1, ZMM1, ZMM1
//     KTESTB k1, %[bit_extract_mask]

//     // ZF is set if board has a match
//     // CF is set if flipped board has a match

//     VPSLLW ZMM0, ZMM0, 2
//     LOOP .pattern_gen%=

//     // shift pattern 1 row down
//     VPERMW %[candidate_12] %{%[permutation_mask]}%{z}, %[idx_array32],
//     %[candidate_12] mov [rdx], eax add rdx, 4 inc r10 cmp r10, 16 jne
//     .start%=

//     cmp r9, 1
//     je .end%=

//     VPERMQ %[board_and_board_flipped], zmm28, %[board_and_board_flipped]
//     mov rdx, %[result2]
//     mov r9, 1
//     jmp .second_rount%=
// .end%=:
//   )"
//                : [result] "+v"(result), [result2] "+v"(result2)
//                :
//                  [board] "v"(board),
//                  [candidate_12] "v"(candidate_12),
//                  [permutation_mask] "Yk"(0b11111111111111101111111111111110),
//                  [bit_extract_mask] "Yk"(0b00001111),
//                  [idx_array32] "v"(idx_array32),
//                  [swap_boards_array] "r"(swap_boards_array)
//                :
//               //  "zmm0", "zmm1", "zmm2", "zmm3", "zmm4", "zmm5", "zmm6",
//               "zmm7",
//               //  "zmm8", "zmm9", "zmm10", "zmm11", "zmm12", "zmm13",
//               "zmm14",
//               //  "zmm15", "zmm17",  "rax", "rbx",
//               //  "rcx", "rdx", "r9", "r10", "r11", "r12", "cc",
//                   "eax",
//                   "k1", "memory");

// VMOVDQA64 zmm31, [%[board_and_board_flipped]]
// VMOVDQA64 zmm30, [%[candidate_12]]
// VMOVDQA64 zmm29, [%[idx_array32]]
// VMOVDQA64 zmm28, [%[swap_boards_array]]

// RAX RBX RCX RDX  RSP RBP RDI	RSI
//  R8–R15
// }