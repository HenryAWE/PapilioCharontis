file(GLOB test_container_src "*.cpp" "*.hpp")
add_executable(test_container ${test_container_src})
target_link_libraries(test_container PRIVATE papilio_test)
gtest_discover_tests(test_container)
