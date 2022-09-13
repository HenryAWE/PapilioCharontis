add_executable(type_test type_test.cpp)
target_link_libraries(type_test PRIVATE papilio)
target_link_libraries(type_test PRIVATE GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main)
add_test(
    NAME "TestType"
    COMMAND type_test
)
