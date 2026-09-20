# The target platform (--platforms in .bazelrc): the hermetic toolchain's
# defaults, except for the C++ standard library. NVIDIA's CUDA headers refuse
# libc++ on x86, so everything is built against libstdc++ instead, which the
# toolchain also builds from source, and links statically into linkstatic
# binaries.
#
# 16.1 because nothing older analyzes: below GCC 16 the toolchain's
# libstdcxx_cxx23 cc_library has an empty srcs filegroup, which bazel rejects.
platform(
    name = "linux_x86_64",
    constraint_values = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
        "@llvm//constraints/libc:gnu.2.28",
        "@llvm//constraints/cxxstdlib:libstdcxx.16.1.0",
    ],
)
