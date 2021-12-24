# Template source: https://crascit.com/2015/07/25/cmake-gtest/
cmake_minimum_required(VERSION 3.16)
project(HS VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)

# Download and unpack googletest at configure time.
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download"
)
execute_process(COMMAND "${CMAKE_COMMAND}" --build .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download"
)

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This adds the following targets:
# gtest, gtest_main, gmock and gmock_main.
add_subdirectory("${CMAKE_BINARY_DIR}/googletest-src"
                 "${CMAKE_BINARY_DIR}/googletest-build"
)

# The gtest/gmock targets carry header search path dependencies
# automatically when using CMake 2.8.11 or later. Otherwise we
# have to add them here ourselves.
if(CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include"
                        "${gmock_SOURCE_DIR}/include"
    )
endif()

# Now simply link your own targets against gtest, gmock,
# etc. as appropriate
enable_testing()

add_executable(
  pc_controller_test
  test/pc_controller_test.cpp
  )
target_link_libraries(
  pc_controller_test
  gtest_main
  gmock_main
  )
target_include_directories(
  pc_controller_test PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  /opt/arduino-1.8.13/libraries/Nanopb
  )

include(GoogleTest)
gtest_discover_tests(pc_controller_test)
