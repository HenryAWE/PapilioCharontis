add_subdirectory(fundamental)

add_custom_target(all_format_tests)
add_dependencies(
    all_format_tests
    fundamental_test
)
