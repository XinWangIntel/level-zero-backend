package(default_visibility = ["//visibility:public"])

load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "test",
    srcs = [
        "src/main.cpp",
        #"utils/include/utils/logging.hpp",
        #"utils/include/utils/utils.hpp",
        #"utils/include/test_harness/test_harness.hpp"
    ],
    #hdrs = glob([
    #        "util/include/*h",
    #]),
    copts = [
        "-std=c++11",
        #  "-I/usr/local/include/level_zero",
    ],
    data = [
        "spirv_0",
    ],
    #+ glob(["//:utils/include/utils/*hpp", "//:utils/include/test_harness/*hpp",]),
    includes = [
        "/usr/local/include",
        "utils/include/test_harness",
        "utils/include/utils",
    ],
    linkopts = [
        "-ldl",
        "-g",
        #"-lze_loader",
        #"-llz_wrapper",
    ],
    linkstatic = 1,
    deps = [
        "//utils:lz_wrapper",
        #"@Level_Zero//:ze_loader",
    ],
)
