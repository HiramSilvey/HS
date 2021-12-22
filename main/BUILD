cc_library(
    name = "controller",
    srcs = ["controller.cpp"],
    hdrs = ["controller.h"],
    deps = [],
)

cc_library(
    name = "decoder",
    srcs = ["decoder.cpp"],
    hdrs = ["decoder.h"],
    deps = [],
)

cc_library(
    name = "hall_joystick",
    srcs = ["hall_joystick.cpp"],
    hdrs = ["hall_joystick.h"],
    deps = [":tlv493d_sensor"],
)

cc_library(
    name = "hall_sensor",
    hdrs = ["hall_sensor.h"],
    deps = [],
)

cc_library(
    name = "mcu",
    hdrs = ["mcu.h"],
    deps = [],
)

cc_library(
    name = "ns_controller",
    srcs = ["ns_controller.cpp"],
    hdrs = ["ns_controller.h"],
    deps = [],
)

cc_library(
    name = "pc_controller",
    srcs = ["pc_controller.cpp"],
    hdrs = ["pc_controller.h"],
    deps = [],
)

cc_library(
    name = "pins",
    srcs = ["pins.cpp"],
    hdrs = ["pins.h"],
    deps = [],
)

cc_library(
    name = "profile",
    srcs = ["profile.pb.c"],
    hdrs = ["profile.pb.h"],
    deps = [],
)

cc_library(
    name = "teensy",
    hdrs = ["teensy.h"],
    deps = [],
)

cc_library(
    name = "tlv493d_sensor",
    hdrs = ["tlv493d_sensor.h"],
    deps = [":hall_sensor"],
)

cc_library(
    name = "util",
    srcs = ["util.cpp"],
    hdrs = ["util.h"],
    deps = [],
)

# TESTING CODE

cc_library(
    name = "mock_hall_sensor",
    hdrs = ["test/mock_hall_sensor.h"],
    deps = [":hall_sensor"],
)

cc_library(
    name = "mock_mcu",
    hdrs = ["test/mock_mcu.h"],
    deps = [":mcu"],
)

cc_test(
  name = "pc_controller_test",
  size = "small",
  srcs = ["test/pc_controller_test.cpp"],
  deps = [
    "@com_google_googletest//:gtest_main",
    ":pc_controller",
  ],
)