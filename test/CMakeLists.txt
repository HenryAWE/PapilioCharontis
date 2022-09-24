enable_testing()

find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

add_executable(test_core test_core.cpp)
target_link_libraries(test_core PRIVATE papilio)
target_link_libraries(test_core PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
gtest_discover_tests(test_core)

add_executable(test_script test_script.cpp)
target_link_libraries(test_script PRIVATE papilio)
target_link_libraries(test_script PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
gtest_discover_tests(test_script)

add_executable(test_utf8 test_utf8.cpp)
target_link_libraries(test_utf8 PRIVATE papilio)
target_link_libraries(test_utf8 PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
gtest_discover_tests(test_utf8)
