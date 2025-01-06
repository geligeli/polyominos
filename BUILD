cc_library(
    name = "combinatorics",
    hdrs = ["combinatorics.hpp"],
    srcs = ["combinatorics.cpp"],
    deps = []
)

cc_test(
    name = "combinatorics_test",
    srcs = ["combinatorics_test.cpp"],
    deps = [
        ":combinatorics",
        "@googletest//:gtest_main",
    ]
)

cc_library(
    name = "loggers",
    srcs = ["loggers.cpp"],
    hdrs = ["loggers.hpp"],
)

cc_library(
    name = "partition_function",
    srcs = ["partition_function.cpp"],
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

cc_library(
    name = "dl_matrix",
    srcs = [
        "dl_matrix.cpp",
    ],
    hdrs = [
        "dl_matrix.hpp",
    ],
)

cc_test(
    name = "dl_matrix_test",
    srcs = [
        "dl_matrix_test.cpp",
    ],
    deps = [
        ":dl_matrix",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "puzzle_solver",
    srcs = [
        "puzzle_solver.cpp",
    ],
    hdrs = [
        "puzzle_solver.hpp",
    ],
    deps = [
        ":dl_matrix",
        ":avx_match",
        ":polyominos",
    ],
)

cc_test(
    name = "puzzle_solver_test",
    srcs = [
        "puzzle_solver_test.cpp",
    ],
    deps = [
        ":puzzle_solver",
        "@googletest//:gtest_main",
    ],
)

cc_binary(
    name = "puzzle_solver_bench",
    srcs = [
        "puzzle_solver_bench.cpp",
    ],
    deps = [
        ":puzzle_solver",
        "@google_benchmark//:benchmark",
    ],
)

cc_binary(
    name = "puzzle_maker",
    srcs = [
        "puzzle_maker.cpp",
    ],
    deps = [
        ":combinatorics",
        ":avx_match",
        ":puzzle_solver",
        ":loggers",
        ":partition_function",
        ":polyominos",
    ],
)

cc_library(
    name = "avx_match",
    srcs = [
        "avx_match.cpp",
    ],
    hdrs = [
        "avx_match.hpp",
    ],
    deps = [
        ":polyominos",
    ],
)

cc_test(
    name = "avx_match_test",
    srcs = [
        "avx_match_test.cpp",
    ],
    deps = [
        ":avx_match",
        ":polyominos",
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
