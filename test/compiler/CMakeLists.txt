add_executable(compiler_test compiler_test.cpp)
target_link_libraries(compiler_test PRIVATE papilio)
target_link_libraries(compiler_test PRIVATE GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main)
add_test(
    NAME "TestCompiler"
    COMMAND compiler_test
)
