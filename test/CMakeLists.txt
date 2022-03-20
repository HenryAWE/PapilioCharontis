find_package(GTest CONFIG REQUIRED)

add_subdirectory(script)
add_subdirectory(utility)

add_custom_target(all_tests)
add_dependencies(
    all_tests
    all_script_tests
    utility_test
)