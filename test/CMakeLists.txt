find_package(GTest CONFIG REQUIRED)

add_subdirectory(common)
add_subdirectory(format)
add_subdirectory(script)
add_subdirectory(utility)

add_custom_target(all_tests)
add_dependencies(
    all_tests
    all_common_tests
    all_format_tests
    all_script_tests
    utility_test
)