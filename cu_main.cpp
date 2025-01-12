#include "kernel.h"

#include "avx_match.hpp"
#include "polyominos.hpp"
#include "puzzle_solver.hpp"

#include <cstdint>
#include <iostream>
#include <numeric>
#include <vector>
#include <map>


struct Tile {
  PolyominoIndex polyomino_index;
  std::vector<BitMaskType> masks;
};


void copy_and_launch(const std::array<std::vector<Tile>, kPrecomputedPolyminosMatchSet.size()>& tiles, const std::vector<PolyominoIndex>& candidates) {
  BitmasksForTile tmp{};

  std::vector<uint64_t> host_bitmask_offset_for_tile;
  std::vector<uint64_t> host_bitmasks;
  host_bitmask_offset_for_tile.push_back(0);


  std::map<PolyominoIndex, std::size_t> candidate_to_index;
  std::size_t j = 0;
  for (std::size_t i =0; i < tiles.size(); ++i) {
    tmp.num_tiles += tiles[i].size();
    for (const auto& bitmask : tiles[i]) {
      tmp.num_bitmasks += bitmask.masks.size();
      host_bitmask_offset_for_tile.push_back(host_bitmask_offset_for_tile.back() + bitmask.masks.size());
      host_bitmasks.insert(host_bitmasks.end(), bitmask.masks.begin(), bitmask.masks.end());
      candidate_to_index[bitmask.polyomino_index] = j;
      ++j;
    }
  }

  CUDA_CHECK(cudaMalloc((void**)&tmp.bitmasks, (1+tmp.num_bitmasks) * sizeof(uint64_t)));
  CUDA_CHECK(cudaMalloc((void**)&tmp.bitmask_offset_for_tile, (1+tmp.num_tiles) * sizeof(uint64_t)));
  CUDA_CHECK(cudaMemcpy(tmp.bitmask_offset_for_tile, host_bitmask_offset_for_tile.data(),  (1+tmp.num_tiles) * sizeof(uint64_t), cudaMemcpyHostToDevice));
  CUDA_CHECK(cudaMemcpy(tmp.bitmasks, host_bitmasks.data(),  (1+tmp.num_bitmasks) * sizeof(uint64_t), cudaMemcpyHostToDevice));


  SolvingState tmp_state(candidates.size());

  std::vector<int> candidate_tiles(candidates.size(),0);
  std::vector<int> indices(candidates.size(),0);
  for (std::size_t i = 0; i < candidates.size(); ++i) {
    candidate_tiles[i] = candidate_to_index[candidates[i]];
  }
  CUDA_CHECK(cudaMemcpy(tmp_state.candidate_tiles, candidate_tiles.data(), tmp_state.num_tiles * sizeof(int), cudaMemcpyHostToDevice));
  CUDA_CHECK(cudaMemcpy(tmp_state.indices, indices.data(), tmp_state.num_tiles * sizeof(int), cudaMemcpyHostToDevice));

  SolvingState* device_state;
  CUDA_CHECK(cudaMalloc((void**)&device_state, sizeof(SolvingState)));
  CUDA_CHECK(cudaMemcpy(device_state, &tmp_state, sizeof(SolvingState), cudaMemcpyHostToDevice));


  BitmasksForTile* device_bitmasks;

  CUDA_CHECK(cudaMalloc((void**)&device_bitmasks, sizeof(BitmasksForTile)));
  CUDA_CHECK(cudaMemcpy(device_bitmasks, &tmp, sizeof(BitmasksForTile), cudaMemcpyHostToDevice));
  
  std::cout << "Launching kernel" << std::endl;

  std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
  launch(device_bitmasks, device_state);
  CUDA_CHECK(cudaDeviceSynchronize());
  std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
  std::cout << "Kernel took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;

  std::cout << "Kernel finished" << std::endl;
  CUDA_CHECK(cudaFree(tmp.bitmasks));
  CUDA_CHECK(cudaFree(tmp.bitmask_offset_for_tile));
  CUDA_CHECK(cudaFree(device_bitmasks)); 
  CUDA_CHECK(cudaFree(device_state));
  CUDA_CHECK(cudaFree(tmp_state.candidate_tiles));

  cudaMemcpy(indices.data(), tmp_state.indices, tmp_state.num_tiles * sizeof(int), cudaMemcpyDeviceToHost);
  CUDA_CHECK(cudaFree(tmp_state.indices));
  for (std::size_t i = 0; i < candidates.size(); ++i) {
    std::cout << "Index " << i << " : " << indices[i] << std::endl;
  }
}

int main() {
  auto board = CreateRectangle<6,5>();
  std::array<std::vector<Tile>, kPrecomputedPolyminosMatchSet.size()> possible_tiles_per_size;

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

  std::vector<PolyominoIndex> candidates;
  
  for (std::size_t i = 0; i < 15; ++i) {
    candidates.push_back({2,0});
  }

  copy_and_launch(possible_tiles_per_size, candidates);

  std::cout << "Done" << std::endl;
  
  return 0;
}
