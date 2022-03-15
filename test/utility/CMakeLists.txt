add_executable(utility_test utility_test.cpp)
target_link_libraries(utility_test PRIVATE papilio)
target_link_libraries(utility_test PRIVATE GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main)
add_test(
    NAME "TestUtility"
    COMMAND utility_test
)
