package(default_visibility = ["//visibility:public"])

load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "test",
    srcs = [
        "src/main.cpp",
    ],
    copts = [
        "-std=c++11",
    ],
    data = [
        "spirv_0",
    ],
    includes = [
        "utils/include",
    ],
    linkopts = [
        "-ldl",
        "-g",
    ],
    linkstatic = 1,
    deps = [
        "//utils:lz_wrapper",
        #"@Level_Zero//:ze_loader",
    ],
)
