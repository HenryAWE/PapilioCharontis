enable_testing()

find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

macro(define_papilio_test test_name)
    add_executable(${test_name} ${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE papilio)
    target_link_libraries(${test_name} PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
    gtest_discover_tests(${test_name})
endmacro()

define_papilio_test(test_chrono)

define_papilio_test(test_core)

define_papilio_test(test_detail_container)

define_papilio_test(test_format)

define_papilio_test(test_locale)

if(${papilio_build_module})
    define_papilio_test(test_module)
endif()

define_papilio_test(test_print)

define_papilio_test(test_script)

define_papilio_test(test_stl_container)

define_papilio_test(test_utf8)
