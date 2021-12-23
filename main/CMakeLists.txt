cmake_minimum_required(VERSION 3.16)
project(HS)

set(CMAKE_CXX_STANDARD 11)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/97a467571a0f615a4d96e79e4399c43221ca1232.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_library(
  mcu INTERFACE
)
target_include_directories(
  mcu INTERFACE
  .
)

add_library(
  pb INTERFACE
)
target_include_directories(
  pb INTERFACE
  /opt/arduino-1.8.13/libraries/Nanopb/
)

add_library(
  profile
  profile.pb.h
  profile.pb.c
)
target_link_libraries(
  profile
  pb
)

add_library(
  controller
  controller.h
  controller.cpp
)
target_link_libraries(
  controller
  mcu
  profile
)

add_library(
  hall_sensor INTERFACE
)
target_include_directories(
  hall_sensor INTERFACE
  .
)

add_library(
  hall_joystick
  hall_joystick.h
  hall_joystick.cpp
)
target_link_libraries(
  hall_joystick
  hall_sensor
  mcu
  util
)

add_library(
  pc_controller
  pc_controller.h
  pc_controller.cpp
)
target_link_libraries(
  pc_controller
  controller
  hall_joystick
  mcu
)

add_library(
  mock_mcu OBJECT
  test/mock_mcu.h
)
target_link_libraries(
  mock_mcu
  mcu
)

add_executable(
  pc_controller_test
  test/pc_controller_test.cpp
)
target_link_libraries(
  pc_controller_test
  gtest_main
  mock_mcu
)

include(GoogleTest)
gtest_discover_tests(pc_controller_test)
