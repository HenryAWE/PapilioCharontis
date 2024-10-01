file(GLOB test_access_src "*.cpp" "*.hpp")
add_executable(test_access ${test_access_src})
configure_test(test_access)
