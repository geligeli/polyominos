load("@grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")
load("@hedron_compile_commands//:refresh_compile_commands.bzl", "refresh_compile_commands")
load("@protobuf//bazel:cc_proto_library.bzl", "cc_proto_library")
load("@protobuf//bazel:proto_library.bzl", "proto_library")
load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cc//cc:cc_test.bzl", "cc_test")
load("@rules_shell//shell:sh_binary.bzl", "sh_binary")

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
        "@googletest//:gtest_main",
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
    visibility = ["//visibility:public"],
    deps = [
        ":loggers",
        ":partition_function",
        # The backend of libstdc++'s std::execution::par: <execution> uses it
        # if <tbb/tbb.h> can be included, and runs serially otherwise.
        "@onetbb//:tbb",
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
    visibility = ["//visibility:public"],
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
    visibility = ["//visibility:public"],
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

sh_binary(
    name = "deploy",
    srcs = ["deploy.sh"],
    data = [
        ":puzzle_maker",
    ],
)

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
        "@grpc//:grpc++",
    ],
)

# bazel run //:refresh_compile_commands, for clangd. Not hedron's own
# refresh_all: that also walks the toolchain, whose runtimes (libstdc++, glibc)
# are built from source here and whose header-parsing actions the extractor
# cannot digest. Only this repository's own sources are of interest anyway.
refresh_compile_commands(
    name = "refresh_compile_commands",
    exclude_external_sources = True,
    targets = "//...",
)
