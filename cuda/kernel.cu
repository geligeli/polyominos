#include "cuda/cuda_check.h"
#include "cuda/kernel.h"

#include <cstdint>
#include <iostream>
#include <stdio.h>

__device__ void recusriveSolve(SolvingState &state, uint32_t current_state,
                               uint32_t start_bitmask_index,
                               uint8_t recursive_depth, ResultsList &results,
                               const BitmasksForTile &bitmasks) {
  
  if (state.goal_bitmask == current_state) {    
    const auto thread_id = blockDim.x * blockIdx.x + threadIdx.x;
    auto& result_id = results.results_per_thread[thread_id];

    // if (result_id < results.max_results_per_thread) {
    //   auto *result_ptr = results.results;
    //   result_ptr += (results.max_results_per_thread * thread_id + result_id) * results.num_indices;
    //   for (uint8_t i = 0; i < recursive_depth; ++i) {
    //     result_ptr[i] = state.indices[i];
    //   }
    // }

    ++result_id;

    // found a solution
    return;
  }

  if (recursive_depth == 5){ //state.num_indices) {
    return;
  }

  auto start_shift = 0;
  auto step_size = 1;
  if (recursive_depth == 0) {
    start_shift = threadIdx.x;
    step_size = blockDim.x;
  } else if (recursive_depth == 1) {
    start_shift = blockIdx.x;
    step_size = gridDim.x;
  }

  for (auto i = start_bitmask_index + start_shift; i < bitmasks.num_bitmasks; i += step_size) {
    const auto mask = bitmasks.bitmasks[i];
    if ((current_state & mask) != 0) {
      continue;
    }
    // state.indices[recursive_depth] = i;
    recusriveSolve(state, current_state | mask, i + 1, recursive_depth + 1, results, bitmasks);
  }
}

__global__ void kernel(const BitmasksForTile &device_bitmasks,
                       SolvingState &device_state, ResultsList &results) {
  const auto thread_id = blockDim.x * blockIdx.x + threadIdx.x;
  results.results_per_thread[thread_id] = 0;
  auto* result_ptr = results.results + results.max_results_per_thread * thread_id * results.num_indices;
  for (int j = 0; j<results.max_results_per_thread; ++j) {
    for (int i = 0; i < device_state.num_indices; ++i) {
      result_ptr[i] = 0xffffffff;
    }
    result_ptr += results.num_indices;
  }
  recusriveSolve(device_state, 0, 0, 0, results, device_bitmasks);
}

void launch(const BitmasksForTile &device_bitmasks,
            SolvingState &device_state,
            ResultsList &results,
            int threadsPerBlock,
            int blocksPerGrid) {
  CUDA_CHECK(cudaDeviceSetLimit(cudaLimitStackSize, 1024 * 5));
  kernel<<<blocksPerGrid, threadsPerBlock>>>(device_bitmasks, device_state, results);
  CUDA_CHECK(cudaDeviceSynchronize());
  CUDA_CHECK(cudaGetLastError());
}