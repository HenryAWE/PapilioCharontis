add_executable(block_test block_test.cpp)
target_link_libraries(block_test PRIVATE papilio)
target_link_libraries(block_test PRIVATE GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main)
add_test(
    NAME "TestBlock"
    COMMAND block_test
)
