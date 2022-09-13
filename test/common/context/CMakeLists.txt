add_executable(context_test context_test.cpp)
target_link_libraries(context_test PRIVATE papilio)
target_link_libraries(context_test PRIVATE GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main)
add_test(
    NAME "TestContext"
    COMMAND context_test
)
