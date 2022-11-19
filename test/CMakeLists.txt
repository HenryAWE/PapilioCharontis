enable_testing()

find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

add_executable(test_chrono test_chrono.cpp)
target_link_libraries(test_chrono PRIVATE papilio)
target_link_libraries(test_chrono PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
gtest_discover_tests(test_chrono)

add_executable(test_core test_core.cpp)
target_link_libraries(test_core PRIVATE papilio)
target_link_libraries(test_core PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
gtest_discover_tests(test_core)

add_executable(test_format test_format.cpp)
target_link_libraries(test_format PRIVATE papilio)
target_link_libraries(test_format PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
gtest_discover_tests(test_format)

add_executable(test_locale test_locale.cpp)
target_link_libraries(test_locale PRIVATE papilio)
target_link_libraries(test_locale PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
gtest_discover_tests(test_locale)

if(${papilio_build_module})
    add_executable(test_module test_module.cpp)
    target_link_libraries(test_module PRIVATE papilio)
    target_link_libraries(test_module PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
    gtest_discover_tests(test_module)
endif()

add_executable(test_print test_print.cpp)
target_link_libraries(test_print PRIVATE papilio)
target_link_libraries(test_print PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
gtest_discover_tests(test_print)

add_executable(test_script test_script.cpp)
target_link_libraries(test_script PRIVATE papilio)
target_link_libraries(test_script PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
gtest_discover_tests(test_script)

add_executable(test_stl_container test_stl_container.cpp)
target_link_libraries(test_stl_container PRIVATE papilio)
target_link_libraries(test_stl_container PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
gtest_discover_tests(test_stl_container)

add_executable(test_utf8 test_utf8.cpp)
target_link_libraries(test_utf8 PRIVATE papilio)
target_link_libraries(test_utf8 PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
gtest_discover_tests(test_utf8)
