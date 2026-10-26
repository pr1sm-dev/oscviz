cmake_minimum_required(VERSION 3.28.3)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

project(oscviz VERSION 0.1.0 DESCRIPTION "Spherical audio visualizer")

find_package(glfw3 3.5 REQUIRED)

add_executable(oscviz)
target_sources(oscviz
PRIVATE
  src/main.cpp
  src/glad.c

  src/graphics/shader.cpp
  src/window/window.cpp

#  imgui/imgui.cpp
#  imgui/backends/imgui_impl_opengl3.cpp
)

file(GLOB IMGUI_SOURCES imgui/*.cpp)
add_library(imgui STATIC ${IMGUI_SOURCES})

target_include_directories(oscviz PRIVATE imgui)
target_include_directories(oscviz PRIVATE include)

target_link_libraries(oscviz glfw imgui)

