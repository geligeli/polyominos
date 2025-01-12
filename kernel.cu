#include "kernel.h"

#include <cstdint>
#include <iostream>
#include <stdio.h>

__device__ const uint64_t *
BitmasksForTile::bitmasks_for_tile(uint64_t tile_idx) const {
  return bitmasks + bitmask_offset_for_tile[tile_idx];
}

__device__ uint64_t
BitmasksForTile::num_bitmasks_for_tile(uint64_t tile_idx) const {
  // printf("tile_idx: %d\n", tile_idx);
  return bitmask_offset_for_tile[tile_idx + 1] -
         bitmask_offset_for_tile[tile_idx];
}

__device__ void recusriveSolve(SolvingState &state, uint64_t current_state,
                               int current_index,
                               const BitmasksForTile &bitmasks) {
  // printf("current_index: %d\n", current_index);
  if (current_index == state.num_tiles) {
    state.result = true;
    return;
  }

  // printf("line: %d\n", __LINE__);

  auto tile_idx = state.candidate_tiles[current_index];

  auto *bitmask_offset = bitmasks.bitmasks_for_tile(tile_idx);
  auto num_masks = bitmasks.num_bitmasks_for_tile(tile_idx);

  std::size_t start_offset = 0;
  if (current_index > 0 && state.candidate_tiles[current_index - 1] ==
                               state.candidate_tiles[current_index]) {
    start_offset = state.indices[current_index - 1] + 1;
  }

  // printf("line: %d\n", __LINE__);
  for (std::size_t i = start_offset; i < num_masks; ++i) {
    const auto mask = bitmask_offset[i];
    if ((current_state & mask) != 0) {
      continue;
    }
    state.indices[current_index] = i;
    // printf("line: %d\n", __LINE__);

    // printf("line: %p \n", &state);
    recusriveSolve(state, current_state | mask, current_index + 1, bitmasks);
    if (state.result) {
      return;
    }
  }
  // printf("line: %d\n", __LINE__);
  state.result = false;
}

__global__ void kernel(const BitmasksForTile &bitmasks, SolvingState& state) {

  recusriveSolve(state, 0, 0, bitmasks);
}

void launch(const BitmasksForTile *device_bitmasks, SolvingState* device_state) {

  size_t newStackSize;
  CUDA_CHECK(cudaDeviceGetLimit(&newStackSize, cudaLimitStackSize));
  std::cout << "Old stack size: " << newStackSize << std::endl;
  CUDA_CHECK(cudaDeviceSetLimit(cudaLimitStackSize, 1024*10));
  CUDA_CHECK(cudaDeviceGetLimit(&newStackSize, cudaLimitStackSize));
  std::cout << "New stack size: " << newStackSize << std::endl;

  kernel<<<1, 1>>>(*device_bitmasks, *device_state);
}