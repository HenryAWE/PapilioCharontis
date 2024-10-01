file(GLOB test_container_src "*.cpp" "*.hpp")
add_executable(test_container ${test_container_src})
configure_test(test_container)
