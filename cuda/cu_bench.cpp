#include "cuda/cuda_datastructures.h"
#include "cuda/kernel.h"

#include "avx_match.hpp"
#include "polyominos.hpp"
#include "puzzle_solver.hpp"

#include <chrono>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <numeric>
#include <vector>
#include <map>

#include <benchmark/benchmark.h>

struct Tile {
  PolyominoIndex polyomino_index;
  std::vector<BitMaskType> masks;
};

auto find_solutions(const auto& tiles, uint8_t max_size, uint8_t board_size, int threadsPerBlock,
  int blocksPerGrid) {
  std::vector<std::vector<uint32_t>> bitmasks;

  uint64_t total_num_masks = 0;
  bitmasks.reserve(tiles.size());
  for (const auto& tile : tiles) {
    std::vector<uint32_t> tile_bitmasks;
    tile_bitmasks.reserve(tile.size());
    for (const auto& t : tile) {
      for (const auto& mask : t.masks) {
        tile_bitmasks.push_back(mask);
        ++total_num_masks;
      }
    }
    bitmasks.push_back(std::move(tile_bitmasks));
  }

  BitmasksForTile* d_bitmask_data = create_bitmasks_for_tile(bitmasks);
  SolvingState* d_solving_state = create_solving_state(max_size, (uint64_t{1} << board_size) - 1);

  ResultsList* d_results = create_results_list(threadsPerBlock*blocksPerGrid, 10, max_size);

  launch(*d_bitmask_data, *d_solving_state, *d_results, threadsPerBlock, blocksPerGrid);

  auto results = read_results(*d_results);

  free_results_list(d_results);
  free_solver_state(d_solving_state);
  free_bitmasks_for_tile(d_bitmask_data);
}

void BM_CudaSolve(benchmark::State &state) {
  auto board = CreateRectangle<3,2>();
  std::array<std::vector<Tile>, std::size(kPrecomputedPolyminosMatchSet)> possible_tiles_per_size;
  BoardMatcher matcher = PolyominoToBoardMatcher(board);
  for (std::size_t i = 0; i < kMaxPolyominoSize; ++i) {  
    for (std::size_t j = 0; j < kPrecomputedPolyminosMatchSet[i].size(); ++j) {
      auto result = find_matches_avx(matcher, kPrecomputedPolyminosMatchSet[i][j]);
      if (result.size() > 0) {
        PolyominoIndex idx{i + 1, j};
        Tile tile{idx, std::move(result)};
        possible_tiles_per_size[i].push_back(std::move(tile));
      }
    }
  }

  for (auto _ : state) {
    find_solutions(possible_tiles_per_size, 4, board.size, state.range(0), state.range(1));
  }
}

BENCHMARK(BM_CudaSolve)
    ->ArgsProduct({
      {256},
      {256}
      // benchmark::CreateRange(32, 1024, /*multi=*/4),
      // benchmark::CreateRange(1, 1024, /*step=*/4)
    });


// Run the benchmark
BENCHMARK_MAIN();
