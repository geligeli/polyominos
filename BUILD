cc_library(
    name = "loggers",
    srcs = ["loggers.cpp"],
    hdrs = ["loggers.hpp"],
)

cc_library(
    name = "partition_function",
    hdrs = ["partition_function.hpp"],
    srcs = ["partition_function.cpp"],
)

cc_library(
    name = "polyominos",
    hdrs = ["polyominos.hpp"],
    deps = [
        ":loggers",
        ":partition_function",
    ],
)

cc_binary(
    name = "puzzle_maker",
    srcs = [
        "puzzle_maker.cpp",
    ],
    deps = [
        ":loggers",
        ":partition_function",
        ":polyominos",
        ":avx_match",
    ],
)

cc_library(
    name = "avx_match",
    hdrs = [
        "avx_match.hpp",
    ],
    srcs = [
        "avx_match.cpp",
    ],
    deps = [
        ":polyominos",
    ],
)

cc_binary(
    name = "avx_match_test",
    srcs = [
        "avx_match_test.cpp",
    ],
    deps = [
        ":polyominos",
        ":avx_match",
        "@googletest//:gtest_main",
    ],
)

cc_binary(
    name = "bench",
    srcs = [
        "bench.cpp",
    ],
    deps = [
        ":avx_match",
        ":polyominos",
        "@google_benchmark//:benchmark",
    ],
)
