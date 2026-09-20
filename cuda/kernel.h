#pragma once

#include "cuda/cuda_datastructures.h"
#include <cstdint>
#include <cuda_runtime_api.h>

void launch(const BitmasksForTile &device_bitmasks,
            SolvingState &device_state,
            ResultsList &device_results,
            int threadsPerBlock,
            int blocksPerGrid);