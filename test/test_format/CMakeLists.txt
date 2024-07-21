file(GLOB test_format_src "*.cpp" "*.hpp")
add_executable(test_format ${test_format_src})
target_link_libraries(test_format PRIVATE papilio_test)
gtest_discover_tests(test_format)
