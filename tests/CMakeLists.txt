include(GoogleTest)


add_executable(oscviz_test)
target_sources(oscviz_test PRIVATE
  audio/algorithms_test.cpp
  audio/ringbuffer_test.cpp
  graphics/sphere_test.cpp
)

target_link_libraries(oscviz_test PRIVATE oscviz_core GTest::gtest_main portaudio)

target_compile_options(oscviz_test PRIVATE)
gtest_discover_tests(oscviz_test)
