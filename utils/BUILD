load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "lz_wrapper",
    srcs = glob([
            "src/*cpp",
    ]),
    includes = ["include",],
    hdrs = glob([
            "include/*hpp",
    ]),
    copts = [
      "-std=c++11",
      "-g",
    #  "-I/usr/local/include",
    ],
    #linkopts = [
      #"-lze_loader",
    #  "-L/usr/local/lib",
    #],
    deps = [
      #"@Level_Zero//:ze_loader",
      "@level_zero//:ze_loader",
    ],
    visibility = ["//visibility:public"],
    linkstatic = 1,
)
