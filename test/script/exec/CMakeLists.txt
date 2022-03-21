add_executable(exec_test exec_test.cpp)
target_link_libraries(exec_test PRIVATE papilio)
target_link_libraries(exec_test PRIVATE GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main)
add_test(
    NAME "TestExec"
    COMMAND exec_test
)
