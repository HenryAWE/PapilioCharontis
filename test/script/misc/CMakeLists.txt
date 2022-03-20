add_executable(misc_test misc_test.cpp)
target_link_libraries(misc_test PRIVATE papilio)
target_link_libraries(misc_test PRIVATE GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main)
add_test(
    NAME "TestMisc"
    COMMAND misc_test
)
