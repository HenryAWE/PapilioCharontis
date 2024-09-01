file(GLOB test_access_src "*.cpp" "*.hpp")
add_executable(test_access ${test_access_src})
target_link_libraries(test_access PRIVATE papilio_test)
gtest_discover_tests(test_access)
