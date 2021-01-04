load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

local_repository(
    name = "Level_Zero",
    path = "../level-zero",
)

# Todo,  add level_zero.BUILD
http_archive(
    name = "level_zero",
    build_file = "@//vendor/level_zero:level_zero.BUILD",
    sha256 = "ad9a757c5e07cd44d8e63019e63ba8a8ea19981ea4b7567d3099d3ef80567908",
    strip_prefix = "level-zero-1.0.22",
    url = "https://github.com/oneapi-src/level-zero/archive/v1.0.22.zip",
)
