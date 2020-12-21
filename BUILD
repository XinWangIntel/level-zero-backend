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
    #+ glob(["//:utils/include/utils/*hpp", "//:utils/include/test_harness/*hpp",]),
    includes = ["utils/include/utils", "utils/include/test_harness", "/usr/local/include",],
    #hdrs = glob([
    #        "util/include/*h",
    #]),
    copts = [
      "-std=c++11",
    #  "-I/usr/local/include/level_zero",
    ],
    linkopts = [
      "-ldl",
      "-g",
      #"-lze_loader",
      #"-llz_wrapper",
    ],
    deps = [
            "//utils:lz_wrapper",
            #"@Level_Zero//:ze_loader",
    ],
    data = [
            "spirv_0",
    ],
    linkstatic = 1,
)
