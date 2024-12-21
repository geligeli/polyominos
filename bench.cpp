#include "avx_match.hpp"
#include "polyominos.hpp"

#include <algorithm>
#include <array>
#include <benchmark/benchmark.h>
#include <bitset>
#include <random>

// template <int N, int K>
// inline std::vector<int64_t>
// FindMatchPatterns2(const Polyomino<N> &board,
//                    const Polyomino<K> &tile) noexcept {
//   static_assert(N > K);
//   std::vector<int64_t> tile_masks;
//   const auto [x_max, y_max] = board.max_xy();
//   tile_masks.reserve((x_max + 1) * (y_max + 1) * 8);
//   int c = 0;
//   for (const auto t : tile.symmetries()) {
//     for (int8_t dx = 0; dx <= x_max; ++dx) {
//       for (int8_t dy = 0; dy <= y_max; ++dy) {
//         auto mask = TileFitBitMask(board, t, dx, dy);

//         if (mask != 0) {
//           ++c;
//           // tile_masks.push_back(mask);
//         }
//       }
//     }
//   }
//   tile_masks.reserve(c);
//   std::sort(tile_masks.begin(), tile_masks.end());
//   tile_masks.erase(std::unique(tile_masks.begin(), tile_masks.end()),
//                    tile_masks.end());
//   return tile_masks;
// }

// template <int N, int K>
// __attribute__((always_inline)) inline constexpr int64_t
// TileFitBitMask2(const Polyomino<N> &board, const Polyomino<K> &tile, int8_t dx,
//                 int8_t dy) noexcept {
//   static_assert(N > K);
//   int64_t result = 0;
//   int num_matches = 0;
//   int64_t tile_mask = 1;

//   for (const auto &xy : tile.xy_cords) {
//     if (!board.has_coord({xy.first + dx, xy.second + dy})) {
//       return 0;
//     }
//   }

//   for (const auto &xy : board.xy_cords) {
//     if (tile.has_coord({xy.first - dx, xy.second - dy})) {
//       result |= tile_mask;
//       ++num_matches;
//     }
//     tile_mask <<= 1;
//   }
//   return (num_matches == K) ? result : 0;
// }

// template <int MAX_X, int MAX_Y, int K>
// auto CreateMatchPatterns(const Polyomino<K> &pattern) {
//   static_assert(MAX_X >= K);
//   static_assert(MAX_Y >= K);
//   std::vector<std::bitset<MAX_X * MAX_Y>> tile_masks;
//   tile_masks.resize(8 * (MAX_X - K + 1) * (MAX_Y - K + 1));

// #if 0
//   const auto [p_max_x, p_max_y] = pattern.max_xy();
//   int idx = 1;
//   for (int8_t dy = 0; dy <= MAX_Y - K; ++dy) {
//     for (int8_t dx = 0; dx <= MAX_X - K; ++dx) {
//       for (const auto [x, y] : pattern.xy_cords) {
//         tile_masks[idx  ][(          y + dy) * MAX_X +           x + dx]=true;
//         tile_masks[idx+1][(          y + dy) * MAX_X + p_max_x - x + dx]=true;
//         tile_masks[idx+2][(p_max_y - y + dy) * MAX_X + p_max_x - x + dx]=true;
//         tile_masks[idx+3][(p_max_y - y + dy) * MAX_X +           x + dx]=true;
//         tile_masks[idx+4][(          x + dy) * MAX_X +           y + dx]=true;
//         tile_masks[idx+5][(          x + dy) * MAX_X + p_max_y - y + dx]=true;
//         tile_masks[idx+6][(p_max_x - x + dy) * MAX_X + p_max_y - y + dx]=true;
//         tile_masks[idx+7][(p_max_x - x + dy) * MAX_X +           y + dx]=true;
//       }
//       idx += 8;
//     }
//   }
// #else
//   int idx = 0;
//   for (auto t : pattern.symmetries()) {
//     t = t._align_to_positive_quadrant();
//     for (int8_t dy = 0; dy <= MAX_Y - K; ++dy) {
//       for (int8_t dx = 0; dx <= MAX_X - K; ++dx) {
//         for (const auto [x, y] : t.xy_cords) {
//           tile_masks[idx][(y + dy) * MAX_X + x + dx] = true;
//         }
//         ++idx;
//       }
//     }
//   }
// #endif
//   return tile_masks;
// }

// static void BM_CreateMatchPatterns(benchmark::State &state) {
//   // Perform setup here
//   Polyomino<12> b;

//   int i = 0;
//   std::generate(b.xy_cords.begin(), b.xy_cords.end(), [&]() {
//     return std::pair<int8_t, int8_t>{0, i++};
//   });

//   for (auto _ : state) {
//     benchmark::DoNotOptimize(CreateMatchPatterns<16, 16>(b));
//   }
// }
// BENCHMARK(BM_CreateMatchPatterns);

static void BM_FindMatchPatterns(benchmark::State &state) {
  // Perform setup here
  Polyomino<16> a;
  Polyomino<8> b;

  int i = 0;

  std::generate_n(a.xy_cords.begin(), 16, [&]() {
    return std::pair<int8_t, int8_t>{0, ++i};
  });
  i = 0;
  std::generate_n(b.xy_cords.begin(), 8, [&]() {
    return std::pair<int8_t, int8_t>{0, ++i};
  });

  for (auto _ : state) {
    FindMatchPatterns(a, b);
  }
}
// BENCHMARK(BM_FindMatchPatterns);

static void BM_FindMatchPatternsAvx(benchmark::State &state) {
  // Perform setup here
  Polyomino<16> a;
  Polyomino<8> b;

  int i = 0;

  std::generate_n(a.xy_cords.begin(), 16, [&]() {
    return std::pair<int8_t, int8_t>{0, ++i};
  });
  i = 0;
  std::generate_n(b.xy_cords.begin(), 8, [&]() {
    return std::pair<int8_t, int8_t>{0, ++i};
  });

  for (auto _ : state) {
    FindMatchPatternsAvx(a, b);
  }
}
BENCHMARK(BM_FindMatchPatternsAvx);



// static void BM_AvxMatch3(benchmark::State &state) {
//   __m512i board;
//   __m512i candidate;
//   __m512i candidate2;
//   __m512i result;
//   __m512i result2;

  
//   for (auto _ : state) {    
//     //  for (int i = 0; i < 8; i++) {
//       // benchmark::DoNotOptimize(find_matches2(bm_board, symmetries[i]));
      
//       _find_matches_avx512_16x16_512(&board, &candidate, &result);
//       _find_matches_avx512_16x16_512(&board, &candidate2, &result2);
//       // benchmark::DoNotOptimize();
//     //  }
//   }
// }
// BENCHMARK(BM_AvxMatch3);



// BENCHMARK(BM_FindMatchPatterns);
// BENCHMARK(BM_PolyominoToBitmap);
// BENCHMARK(BM_AvxGenerateBitmask);
// BENCHMARK(BM_AvxShift);


// Run the benchmark
BENCHMARK_MAIN();
