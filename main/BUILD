cc_library(
    name = "pc_controller",
    srcs = ["pc_controller.cpp"],
    hdrs = ["pc_controller.h"],
    deps = [],
)

cc_test(
  name = "pc_controller_test",
  size = "small",
  srcs = ["pc_controller_test.cpp"],
  deps = [
    "@com_google_googletest//:gtest_main",
    ":pc_controller",
  ],
)