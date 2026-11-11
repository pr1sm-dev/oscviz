add_library(oscviz_core STATIC glad.c)

target_include_directories(oscviz_core PUBLIC
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/libs
)

target_sources(oscviz_core PRIVATE
  graphics/shader.cpp
  graphics/sphere.cpp
  window/window.cpp
  audio/algorithms.cpp
  audio/engine.cpp
)

target_link_libraries(oscviz_core PUBLIC glfw imgui portaudio nfd miniaudio)

add_executable(oscviz main.cpp)
target_link_libraries(oscviz oscviz_core)
