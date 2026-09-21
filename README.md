# polyominos
Polyominos Puzzle Generator


# Generate Compile Commands
bazel run //:refresh_compile_commands

# Building

The compiler (clang with libstdc++, the `llvm` module) and the CUDA toolkit (NVIDIA
redistributables via `rules_cuda`, compiled by that same clang) are both
downloaded by bazel; nothing but bazelisk is needed on the host or on an RBE
worker. libstdc++ is linked statically, so a binary such as `puzzle_maker`
depends on nothing but glibc (2.28 or newer) and can be copied to another
machine as a single file. Running the CUDA binaries needs an NVIDIA driver, building them does
not.

    bazel test //...                      # AVX2 baseline, runs anywhere
    bazel build --config=znver4 //...     # AVX-512 paths; workstation and c3d
