add_subdirectory(context)
add_subdirectory(type)

add_custom_target(all_common_tests)
add_dependencies(
    all_common_tests
    context_test
    type_test
)
