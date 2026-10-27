include(GoogleTest)

file(GLOB TEST_SOURCES graphics/*.cpp)

add_executable(oscviz_test)
target_sources(oscviz_test PRIVATE
  ${TEST_SOURCES}
)

target_link_libraries(oscviz_test PRIVATE oscviz_core GTest::gtest_main)
gtest_discover_tests(oscviz_test)
