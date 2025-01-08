workspace(name = "puzzle_solver")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")


# http_archive(
#     name = "com_github_grpc_grpc",
#     urls = [
#         "https://github.com/grpc/grpc/archive/b4ef7c141d960be62e0008601261bb22cecb5d40.tar.gz",
#     ],
#     # integrity = "sha256-HRvpOBLRUbuc9Gjy8bK0pRhczAXlQZa8xi6E2kG4Z2Q=",
#     strip_prefix = "grpc-b4ef7c141d960be62e0008601261bb22cecb5d40",
# )


# load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")
# grpc_deps()
# load("@com_github_grpc_grpc//bazel:grpc_extra_deps.bzl", "grpc_extra_deps")
# grpc_extra_deps()



http_archive(
    name = "google_cloud_cpp",
    sha256 = "db69dd73ef4af8b2e816d80ded04950036d0e0dccc274f8c3d3ed1d7f5692a1b",
    strip_prefix = "google-cloud-cpp-2.32.0",
    url = "https://github.com/googleapis/google-cloud-cpp/archive/v2.32.0.tar.gz",
)


load("@google_cloud_cpp//bazel:workspace0.bzl", "gl_cpp_workspace0")
gl_cpp_workspace0()
load("@google_cloud_cpp//bazel:workspace1.bzl", "gl_cpp_workspace1")
gl_cpp_workspace1()
load("@google_cloud_cpp//bazel:workspace2.bzl", "gl_cpp_workspace2")
gl_cpp_workspace2()
load("@google_cloud_cpp//bazel:workspace3.bzl", "gl_cpp_workspace3")
gl_cpp_workspace3()
load("@google_cloud_cpp//bazel:workspace4.bzl", "gl_cpp_workspace4")
gl_cpp_workspace4()
load("@google_cloud_cpp//bazel:workspace5.bzl", "gl_cpp_workspace5")
gl_cpp_workspace5()


