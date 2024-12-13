cc_library(
    name = "loggers",
    srcs = ["loggers.cpp"],
    hdrs = ["loggers.hpp"],
)

cc_library(
    name = "partition_function",
    hdrs = ["partition_function.hpp"],
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
    ],
)

cc_binary(
    name = "bench",
    srcs = [
        "bench.cpp",
    ],
    deps = [
        ":polyominos",
        "@google_benchmark//:benchmark",
    ],
)

cc_binary(
    name = "avx_match_test",
    srcs = [
        "avx_match_test.cpp",
    ],
    deps = [
        "@googletest//:gtest_main",
    ],
)
