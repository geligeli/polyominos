#include <gtest/gtest.h>
#include <cstdint>
#include <array>

#include <immintrin.h>


void my_avx_function(uint64_t* ptr1, uint64_t* ptr2, uint64_t* result) {
  __m256i a = _mm256_load_epi64(ptr1);
  __m256i b = _mm256_load_epi64(ptr2);
  __m256i c = _mm256_xor_si256(a, b);
  _mm256_store_epi64(result, c);
}

TEST(AVXTest, Test1) {
  uint64_t ptr1[4] = {1, 2, 3, 4};
  uint64_t ptr2[4] = {5, 6, 7, 8};
  std::array<uint64_t, 4> result;
  my_avx_function(ptr1, ptr2, &result);
  std::array<uint64_t, 4> expected = {1 ^ 5, 2 xor 6, 3 ^ 7, 4 ^ 8};
  EXPECT_EQ(result, expected);
}