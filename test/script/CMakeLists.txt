add_subdirectory(block)
add_subdirectory(compiler)
add_subdirectory(exec)
add_subdirectory(lexer)
add_subdirectory(misc)

add_custom_target(all_script_tests)
add_dependencies(
    all_script_tests
    block_test
    compiler_test
    exec_test
    lexer_test
    misc_test
)