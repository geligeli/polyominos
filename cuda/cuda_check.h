#pragma once

#include <cstdio>
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
