#pragma once

#include <cstdint>
#include <cuda_runtime_api.h>
#include <vector>
#include <span>

struct BitmasksForTile {
  uint32_t num_tiles;
  uint32_t *bitmask_offset_for_tile;

  uint32_t num_bitmasks;
  uint32_t *bitmasks;

  // __host__ __device__ const uint64_t *bitmasks_for_tile(uint64_t tile_idx) const;
  // __host__ __device__ uint64_t num_bitmasks_for_tile(uint64_t tile_idx) const;
};

BitmasksForTile *create_bitmasks_for_tile(const std::vector<std::vector<uint32_t>>& tiles);
void free_bitmasks_for_tile(BitmasksForTile *bitmasks);

struct SolvingState {
  uint32_t goal_bitmask;
  uint8_t num_indices;

  uint16_t* indices;
};

SolvingState *create_solving_state(uint8_t num_indices, uint32_t goal_bitmask);
void free_solver_state(SolvingState *state);

struct ResultsList {
  uint64_t num_results;
  uint64_t max_results_per_thread;

  __host__ __device__ constexpr uint64_t num_threads() const {
    return num_results / max_results_per_thread;
  }

  uint8_t num_indices;

  uint32_t *results;
  uint32_t *results_per_thread;
  uint64_t *ops_per_thread;
};

ResultsList *create_results_list(uint64_t num_threads, uint64_t num_results_per_thread, uint8_t num_indices);
void free_results_list(ResultsList *r);
std::vector<std::vector<uint32_t>> read_results(const ResultsList &r);