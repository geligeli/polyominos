#include "avx_match.hpp"
#include "polyominos.hpp"

#include <algorithm>
#include <array>
#include <benchmark/benchmark.h>
#include <bitset>
#include <random>

template <int N> void BM_FindMatchPatterns(benchmark::State &state) {
  static_assert(N<=16);
  auto a = CreateSquare<N>();
  int i = 0;
  const auto p = PrecomputedPolyminosSet<5>::polyminos();

  for (auto _ : state) {
    FindMatchPatterns(a, p[i]);
    ++i;
    i = i % p.size();
  }
}
BENCHMARK(BM_FindMatchPatterns<4>);
BENCHMARK(BM_FindMatchPatterns<8>);
BENCHMARK(BM_FindMatchPatterns<12>);
BENCHMARK(BM_FindMatchPatterns<16>);

template <int N> void BM_FindMatchPatternsAvx(benchmark::State &state) {
  // Perform setup here
  static_assert(N<=16);
  auto a = CreateSquare<N>();
  int i = 0;
  const auto p = PrecomputedPolyminosSet<5>::polyminos();
  BoardMatcher board = PolyominoToBoardMatcher(a);
  
  CandidateMatchBitmask candidate;
  std::vector<CandidateMatchBitmask> candidates(p.size());
  for (int i = 0; i < p.size(); ++i) {
    PolyominoToMatchBitMask(p[i], candidates[i]);
  }
  for (auto _ : state) {
    benchmark::DoNotOptimize(find_matches_avx(board, candidates[i]));
    ++i;
    i = i % p.size();
  }
}
BENCHMARK(BM_FindMatchPatternsAvx<4>);
BENCHMARK(BM_FindMatchPatternsAvx<8>);
BENCHMARK(BM_FindMatchPatternsAvx<12>);
BENCHMARK(BM_FindMatchPatternsAvx<16>);

// Run the benchmark
BENCHMARK_MAIN();
