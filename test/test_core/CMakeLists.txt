file(GLOB test_core_src "*.cpp" "*.hpp")
add_executable(test_core ${test_core_src})
configure_test(test_core)
