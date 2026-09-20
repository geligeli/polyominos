load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cuda//cuda:defs.bzl", "cuda_library")

# No rdc (relocatable device code) on the cuda_librarys: all device code is in
# kernel.cu, so nothing is linked across translation units on the device side.
# It could not be turned on as is, either: under -fcuda-rdc clang derives every
# translation unit's module id from the same constant, so two rdc libraries in
# one binary collide in the device link step.

# Force-included into every CUDA compile (CUDA_COPTS). NVIDIA's host_defines.h
# defines __noinline__ as a macro, which breaks libstdc++'s
# __attribute__((__noinline__)). clang knows __noinline__ as a keyword in CUDA
# mode, so dropping the macro loses nothing.
cc_library(
    name = "undef_noinline",
    hdrs = ["undef_noinline.h"],
)

CUDA_COPTS = [
    "-std=c++20",
    "-include",
    "cuda/undef_noinline.h",
]

cc_library(
    name = "cuda_check",
    hdrs = ["cuda_check.h"],
)

cuda_library(
    name = "cuda_datastructures",
    srcs = ["cuda_datastructures.cu"],
    hdrs = ["cuda_datastructures.h"],
    copts = CUDA_COPTS,
    deps = [
        ":cuda_check",
        ":undef_noinline",
    ],
)

cuda_library(
    name = "kernel",
    srcs = ["kernel.cu"],
    hdrs = ["kernel.h"],
    copts = CUDA_COPTS,
    deps = [
        ":cuda_check",
        ":cuda_datastructures",
        ":undef_noinline",
    ],
)

cc_binary(
    name = "cu_main",
    srcs = ["cu_main.cpp"],
    deps = [
        ":kernel",
        "//:avx_match",
        "//:polyominos",
        "//:puzzle_solver",
    ],
)

cc_binary(
    name = "cu_bench",
    srcs = [
        "cu_bench.cpp",
    ],
    deps = [
        ":kernel",
        "//:avx_match",
        "//:polyominos",
        "//:puzzle_solver",
        "@google_benchmark//:benchmark",
    ],
)
