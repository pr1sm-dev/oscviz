add_library(oscviz_core STATIC glad.c)

target_include_directories(oscviz_core PUBLIC
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/libs
)

target_sources(oscviz_core PRIVATE
  graphics/shader.cpp
  graphics/sphere.cpp
  window/window.cpp
)

target_link_libraries(oscviz_core PUBLIC glfw imgui)

add_executable(oscviz main.cpp)
target_link_libraries(oscviz oscviz_core)
