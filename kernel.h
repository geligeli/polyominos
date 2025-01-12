#pragma once

#include <cstdint>
#include <cuda_runtime_api.h>

#define CUDA_CHECK(expr)                                                       \
  do {                                                                         \
    cudaError_t err = (expr);                                                  \
    if (err != cudaSuccess) {                                                  \
      fprintf(stderr,                                                          \
              "CUDA Error Code  : %d\n     Error String: %s\n%s:%d\n", err,   \
              cudaGetErrorString(err), __FILE__, __LINE__);                    \
      exit(err);                                                               \
    }                                                                          \
  } while (0)

struct BitmasksForTile {
  uint64_t num_tiles;
  uint64_t *bitmask_offset_for_tile;

  uint64_t num_bitmasks;
  uint64_t *bitmasks;
  __device__ const uint64_t *bitmasks_for_tile(uint64_t tile_idx) const;
  __device__ uint64_t num_bitmasks_for_tile(uint64_t tile_idx) const;
};

struct SolvingState {
  int num_tiles;

  int *candidate_tiles;
  int *indices;

  bool result;
};

void launch(const BitmasksForTile *device_bitmasks, SolvingState *device_state);