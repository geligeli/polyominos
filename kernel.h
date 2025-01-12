#pragma once

#include <cstdint>
#include <cuda_runtime_api.h>

#include <thrust/host_vector.h>
#include <thrust/device_vector.h>

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
  int h_num_tiles;
  std::vector<int> h_candidate_tiles;
  std::vector<int> h_indices;
  bool h_result;

  struct {
    int num_tiles;
    int *candidate_tiles;
    int *indices;
  } d;

  // __host__ __device__ SolvingState(int num_tiles_) {
  //   num_tiles = num_tiles_;
  // #ifdef __CUDA_ARCH__
  //   CUDA_CHECK(cudaMalloc((void**)&candidate_tiles, num_tiles * sizeof(int)));
  //   CUDA_CHECK(cudaMalloc((void**)&indices, num_tiles * sizeof(int)));
  // #else
  //   candidate_tiles = (int*)malloc(num_tiles * sizeof(int));
  //   indices = (int*)malloc(num_tiles * sizeof(int));
  // #endif
  // };

  // __host__ __device__ ~SolvingState() {
  // #ifdef __CUDA_ARCH__
  //   CUDA_CHECK(cudaFree(candidate_tiles));
  //   CUDA_CHECK(cudaFree(indices));
  // #else
  //   free(candidate_tiles);
  //   free(indices);
  // #endif
  // }
};

// struct SolvingState {
//   int num_tiles;
//   int *candidate_tiles;
//   int *indices;
//   bool result;
// };


void launch(const BitmasksForTile *device_bitmasks, SolvingState *device_state);