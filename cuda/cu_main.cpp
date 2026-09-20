#include "cuda/cuda_datastructures.h"
#include "kernel.h"

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


struct Tile {
  PolyominoIndex polyomino_index;
  std::vector<BitMaskType> masks;
};

auto find_solutions(const auto& tiles, uint8_t max_size, uint8_t board_size) {
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

  std::cout << "Num Bitmasks: " << total_num_masks << std::endl;

  BitmasksForTile* d_bitmask_data = create_bitmasks_for_tile(bitmasks);
  SolvingState* d_solving_state = create_solving_state(max_size, (uint32_t{1} << board_size) - 1);

  int threadsPerBlock = 256;
  int blocksPerGrid = 256;

  ResultsList* d_results = create_results_list(threadsPerBlock*blocksPerGrid, 10, max_size);

  launch(*d_bitmask_data, *d_solving_state, *d_results, threadsPerBlock, blocksPerGrid);

  auto results = read_results(*d_results);

  free_results_list(d_results);
  free_solver_state(d_solving_state);
  free_bitmasks_for_tile(d_bitmask_data);
}

int main() {
  cudaDeviceProp prop;
  int deviceCount;

  cudaGetDeviceCount(&deviceCount);

  for (int i = 0; i < deviceCount; i++) {
    cudaGetDeviceProperties(&prop, i);

    std::cout << "Device " << i << ": " << prop.name << std::endl;
    std::cout << "  SM Count: " << prop.multiProcessorCount << std::endl;
    std::cout << "  Max Threads per SM: " << prop.maxThreadsPerMultiProcessor << std::endl;
    std::cout << "  Warp Size: " << prop.warpSize << std::endl;
    std::cout << "  Max Threads per Block: " << prop.maxThreadsPerBlock << std::endl;
    std::cout << "  Max Blocks in Grid: (" 
              << prop.maxGridSize[0] << ", " 
              << prop.maxGridSize[1] << ", " 
              << prop.maxGridSize[2] << ")" << std::endl;
  }
  return 0;

  auto board = CreateRectangle<5,4>();

  std::array<std::vector<Tile>, std::size(kPrecomputedPolyminosMatchSet)> possible_tiles_per_size;
  BoardMatcher matcher = PolyominoToBoardMatcher(board);
  for (std::size_t i = 0; i < kMaxPolyominoSize; ++i) {
  // int i = 4;
    for (std::size_t j = 0; j < kPrecomputedPolyminosMatchSet[i].size(); ++j) {
      auto result = find_matches_avx(matcher, kPrecomputedPolyminosMatchSet[i][j]);
      if (result.size() > 0) {
        PolyominoIndex idx{i + 1, j};
        Tile tile{idx, std::move(result)};
        possible_tiles_per_size[i].push_back(std::move(tile));
      }
    }
  }
  std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
  find_solutions(possible_tiles_per_size, 5, board.size);
  std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
  std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms" << std::endl;
  return 0;
}
