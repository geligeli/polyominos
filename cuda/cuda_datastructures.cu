#include "cuda/cuda_datastructures.h"

#include <iostream>

#include <cstdint>
#include <numeric>
#include <sys/types.h>

#include "cuda/cuda_check.h"

BitmasksForTile *create_bitmasks_for_tile(const std::vector<std::vector<uint32_t>>& tiles) {
    BitmasksForTile bitmasks;
    bitmasks.num_tiles = tiles.size();
    bitmasks.num_bitmasks = std::accumulate(tiles.begin(), tiles.end(), 0,
                                            [](int sum, const auto &tile) {
                                                return sum + tile.size();
                                            });

    CUDA_CHECK(cudaMalloc(&bitmasks.bitmask_offset_for_tile, sizeof(bitmasks.bitmask_offset_for_tile[0])*(bitmasks.num_tiles + 1)));
    CUDA_CHECK(cudaMalloc(&bitmasks.bitmasks, sizeof(bitmasks.bitmasks[0])*bitmasks.num_bitmasks));

    std::vector<uint32_t> tmp_bitmask_offset_for_tile(bitmasks.num_tiles + 1);
    tmp_bitmask_offset_for_tile[0] = 0;
    for (std::size_t i = 0; i < bitmasks.num_tiles; ++i) {
        tmp_bitmask_offset_for_tile[i + 1] = tmp_bitmask_offset_for_tile[i] + tiles[i].size();
    }
    CUDA_CHECK(cudaMemcpy(bitmasks.bitmask_offset_for_tile, tmp_bitmask_offset_for_tile.data(), sizeof(bitmasks.bitmask_offset_for_tile[0])*(bitmasks.num_tiles + 1), cudaMemcpyHostToDevice));

    auto* bitmask_ptr = bitmasks.bitmasks;
    for (std::size_t i = 0; i < tiles.size(); ++i) {
        CUDA_CHECK(cudaMemcpy(bitmask_ptr, tiles[i].data(), sizeof(bitmasks.bitmasks[0])*tiles[i].size(), cudaMemcpyHostToDevice));
        bitmask_ptr += tiles[i].size();
    }

    BitmasksForTile *device_bitmasks;
    CUDA_CHECK(cudaMalloc(&device_bitmasks, sizeof(BitmasksForTile)));
    CUDA_CHECK(cudaMemcpy(device_bitmasks, &bitmasks, sizeof(BitmasksForTile), cudaMemcpyHostToDevice));
    return device_bitmasks;
}

void free_bitmasks_for_tile(BitmasksForTile *bitmasks) {
    BitmasksForTile tmp_bitmasks;
    CUDA_CHECK(cudaMemcpy(&tmp_bitmasks, bitmasks, sizeof(BitmasksForTile), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaFree(tmp_bitmasks.bitmasks));
    CUDA_CHECK(cudaFree(tmp_bitmasks.bitmask_offset_for_tile));
    CUDA_CHECK(cudaFree(bitmasks));
}

SolvingState *create_solving_state(uint8_t num_indices, uint32_t goal_bitmask) {
    SolvingState state;
    state.num_indices = num_indices;
    state.goal_bitmask = goal_bitmask;

    CUDA_CHECK(cudaMalloc(&state.indices, sizeof(state.indices[0])*num_indices));
    SolvingState *device_state;
    CUDA_CHECK(cudaMalloc(&device_state, sizeof(SolvingState)));
    CUDA_CHECK(cudaMemcpy(device_state, &state, sizeof(SolvingState), cudaMemcpyHostToDevice));
    return device_state;
}

void free_solver_state(SolvingState *state) {
    SolvingState tmp_state;
    CUDA_CHECK(cudaMemcpy(&tmp_state, state, sizeof(SolvingState), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaFree(tmp_state.indices));
    CUDA_CHECK(cudaFree(state));
}

ResultsList *create_results_list(uint64_t num_threads, uint64_t num_results_per_thread, uint8_t num_indices) {
    ResultsList results;
    results.num_indices = num_indices;
    results.num_results = num_threads*num_results_per_thread;
    results.max_results_per_thread = num_results_per_thread;

    CUDA_CHECK(cudaMalloc(&results.results, sizeof(results.results[0])*results.num_results*num_indices));
    CUDA_CHECK(cudaMalloc(&results.results_per_thread, sizeof(results.results_per_thread[0])*num_threads));
    CUDA_CHECK(cudaMalloc(&results.ops_per_thread, sizeof(results.ops_per_thread[0])*num_threads));
    

    ResultsList *device_results;
    CUDA_CHECK(cudaMalloc(&device_results, sizeof(ResultsList)));
    CUDA_CHECK(cudaMemcpy(device_results, &results, sizeof(ResultsList), cudaMemcpyHostToDevice));
    return device_results;
}

std::vector<std::vector<uint32_t>> read_results(const ResultsList &r) {
    ResultsList tmp_results;
    CUDA_CHECK(cudaMemcpy(&tmp_results, &r, sizeof(ResultsList), cudaMemcpyDeviceToHost));

    std::vector<uint32_t> results_per_thread(tmp_results.num_threads());
    CUDA_CHECK(cudaMemcpy(results_per_thread.data(), tmp_results.results_per_thread, sizeof(tmp_results.results_per_thread[0])*tmp_results.num_threads(), cudaMemcpyDeviceToHost));
    std::vector<uint64_t> ops_per_thread(tmp_results.num_threads());
    CUDA_CHECK(cudaMemcpy(ops_per_thread.data(), tmp_results.ops_per_thread, sizeof(uint64_t)*tmp_results.num_threads(), cudaMemcpyDeviceToHost));

    auto total_results_found = std::accumulate(results_per_thread.begin(), results_per_thread.end(), uint64_t{0});
    // std::cout << "total_results_found: " << total_results_found << std::endl;
    auto total_ops = std::accumulate(ops_per_thread.begin(), ops_per_thread.end(), uint64_t{0});
    // std::cout << "total_ops: " << total_ops << std::endl;

    uint64_t data_size = tmp_results.num_results*tmp_results.num_indices;

    std::vector<uint32_t> data(data_size);
    CUDA_CHECK(cudaMemcpy(data.data(), tmp_results.results, sizeof(tmp_results.results[0])*data_size, cudaMemcpyDeviceToHost));

    std::vector<std::vector<uint32_t>> results;
    for (std::size_t i = 0; i < tmp_results.num_results; ++i) {
        if (data[i*tmp_results.num_indices] == 0xffffffff) {
            continue;
        }
        std::vector<uint32_t> tmp(tmp_results.num_indices);
        for (std::size_t j = 0; j < tmp_results.num_indices; ++j) {
            tmp[j] = data[i*tmp_results.num_indices + j];
        }
        results.push_back(std::move(tmp));
    }

    return results;
}

void free_results_list(ResultsList *r) {
    ResultsList tmp_results;
    CUDA_CHECK(cudaMemcpy(&tmp_results, r, sizeof(ResultsList), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaFree(tmp_results.ops_per_thread));
    CUDA_CHECK(cudaFree(tmp_results.results_per_thread));
    CUDA_CHECK(cudaFree(tmp_results.results));
    CUDA_CHECK(cudaFree(r));
}