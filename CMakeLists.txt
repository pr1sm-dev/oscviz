cmake_minimum_required(VERSION 3.28.3)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(oscviz VERSION 0.1.0 DESCRIPTION "Spherical audio visualizer")

option(OSCVIZ_BUILD_TESTS "Build tests" ON)

find_package(glfw3 3.5 REQUIRED)

include(FetchContent)
FetchContent_Declare(
  portaudio
  GIT_REPOSITORY https://github.com/PortAudio/portaudio.git
  GIT_TAG 1ca0eeedb678c3cca80f3306c474d69a7a4b5278
  FIND_PACKAGE_ARGS NAMES PortAudio
)
FetchContent_MakeAvailable(portaudio)

add_subdirectory(libs)
add_subdirectory(src)

if(OSCVIZ_BUILD_TESTS)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/cd8e73f067129f8489e35c7b51c7a68552b289d2.zip
  )

  FetchContent_MakeAvailable(googletest)
  enable_testing()
  add_subdirectory(tests)
endif()

