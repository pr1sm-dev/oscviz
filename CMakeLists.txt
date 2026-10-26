cmake_minimum_required(VERSION 3.28.3)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(oscviz VERSION 0.1.0 DESCRIPTION "Spherical audio visualizer")

option(OSCVIZ_BUILD_TESTS "Build tests" ON)

find_package(glfw3 3.5 REQUIRED)

add_subdirectory(libs)
add_subdirectory(src)

if(OSCVIZ_BUILD_TESTS)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/cd8e73f067129f8489e35c7b51c7a68552b289d2.zip
  )

  FetchContent_MakeAvailable(googletest)
  enable_testing()
  add_subdirectory(tests)
endif()
