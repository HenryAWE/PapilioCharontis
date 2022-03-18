find_package(GTest CONFIG REQUIRED)

add_subdirectory(block)
add_subdirectory(compiler)
add_subdirectory(context)
add_subdirectory(lexer)
add_subdirectory(misc)
add_subdirectory(utility)

add_custom_target(all_tests)
add_dependencies(
    all_tests
    block_test
    compiler_test
    context_test
    lexer_test
    misc_test
    utility_test
)