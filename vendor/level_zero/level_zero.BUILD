package(default_visibility = ["//visibility:public"])

load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "ze_loader",
    srcs = glob([
        "source/lib/*cpp",
        #"source/loader/*cpp",
    ]) + select({
        "@bazel_tools//src/conditions:windows": glob(["source/loader/windows/*cpp"]),
        "//conditions:default": glob(["source/loader/linux/*cpp"]),
    }),
    hdrs = [
        "source/lib/ze_lib.h",
    ] + glob([
        "include/layers/*h",
        "include/*h",
        "source/loader/*h",
        "source/inc/*h",
    ]),
    copts = [
        "-std=c++14",
        "-fpermissive",
        "-fPIC",
        "-g",
    ],
    defines = [
        'L0_LOADER_VERSION=\\"1\\"',
        'L0_VALIDATION_LAYER_SUPPORTED_VERSION=\\"1\\"',
    ],
    includes = [
        "include",
        "source/inc",
        "source/lib",
        "source/loader",
    ],
    linkopts = [
        "-ldl",
        "-pthread",
    ],
)
