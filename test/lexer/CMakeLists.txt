add_executable(lexer_test lexer_test.cpp)
target_link_libraries(lexer_test PRIVATE papilio)
target_link_libraries(lexer_test PRIVATE GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main)
add_test(
    NAME "TestLexer"
    COMMAND lexer_test
)
