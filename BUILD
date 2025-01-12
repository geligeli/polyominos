load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")
load("@rules_proto//proto:defs.bzl", "proto_library")
load("@rules_cuda//cuda:defs.bzl", "cuda_library")

cc_library(
    name = "combinatorics",
    srcs = ["combinatorics.cpp"],
    hdrs = ["combinatorics.hpp"],
    deps = [],
)

cc_test(
    name = "combinatorics_test",
    srcs = ["combinatorics_test.cpp"],
    deps = [
        ":combinatorics",
        "@com_google_googletest//:gtest_main",
    ],
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
        "@com_google_googletest//:gtest_main",
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
        ":avx_match",
        ":dl_matrix",
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
        "@com_google_googletest//:gtest_main",
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
        ":avx_match",
        ":combinatorics",
        ":loggers",
        ":partition_function",
        ":polyominos",
        ":puzzle_solver",
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
    copts = ["-masm=intel"],
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
        "@com_google_googletest//:gtest_main",
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

sh_binary(
    name = "deploy",
    srcs = ["deploy.sh"],
    data = [
        ":puzzle_maker",
    ],
)

# load("@rules_proto_grpc_cpp//:defs.bzl", "cpp_grpc_compile")
# load("@rules_proto//proto:defs.bzl", "proto_library")
# load("@protobuf//bazel:proto_library.bzl", "proto_library")

# load("@com_github_grpc_grpc//bazel:grpc_build_system.bzl", "grpc_proto_library")
# load("@grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")

proto_library(
    name = "helloworld_proto",
    srcs = ["helloworld.proto"],
)

cc_proto_library(
    name = "helloworld_cc_proto",
    deps = [":helloworld_proto"],
)

cc_grpc_library(
    name = "helloworld_cc_grpc",
    srcs = [":helloworld_proto"],
    grpc_only = True,
    deps = [":helloworld_cc_proto"],
)

cc_binary(
    name = "greeter_server",
    srcs = ["greeter_server.cpp"],
    defines = ["BAZEL_BUILD"],
    deps = [
        ":helloworld_cc_grpc",
        # http_archive made this label available for binding
        "@com_github_grpc_grpc//:grpc++",
    ],
)

cuda_library(
    name = "kernel",
    rdc = True,
    srcs = ["kernel.cu"],
    hdrs = ["kernel.h"],
)

cc_binary(
    name = "cu_main",
    srcs = ["cu_main.cpp"],
    deps = [
        ":kernel",
        ":puzzle_solver",
        ":polyominos",
        ":avx_match",
    ],
)