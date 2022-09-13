add_executable(fundamental_test fundamental_test.cpp)
target_link_libraries(fundamental_test PRIVATE papilio)
target_link_libraries(fundamental_test PRIVATE GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main)
add_test(
    NAME "FundamentalType"
    COMMAND fundamental_test
)
