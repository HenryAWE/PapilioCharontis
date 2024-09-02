file(GLOB test_core_src "*.cpp" "*.hpp")
add_executable(test_core ${test_core_src})
target_link_libraries(test_core PRIVATE papilio_test)
gtest_discover_tests(test_core DISCOVERY_TIMEOUT 300)
