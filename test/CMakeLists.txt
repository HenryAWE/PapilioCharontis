enable_testing()

find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

macro(define_papilio_test test_name)
    add_executable(${test_name} ${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE papilio)
    target_link_libraries(${test_name} PRIVATE GTest::gtest GTest::gtest_main)
    gtest_discover_tests(${test_name})
endmacro()

define_papilio_test(test_core)

define_papilio_test(test_container)

define_papilio_test(test_iterstream)

define_papilio_test(test_memory)

define_papilio_test(test_format)

define_papilio_test(test_locale)

define_papilio_test(test_print)

define_papilio_test(test_script)

define_papilio_test(test_stl_container)

define_papilio_test(test_type)

define_papilio_test(test_utf_common)

define_papilio_test(test_utf_codepoint)

define_papilio_test(test_utf_string)
