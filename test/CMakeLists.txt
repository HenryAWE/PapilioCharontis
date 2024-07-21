enable_testing()

include(GoogleTest)
add_subdirectory(papilio_test)

macro(define_papilio_test test_name)
    add_executable(${test_name} ${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE papilio_test)
    gtest_discover_tests(${test_name} DISCOVERY_TIMEOUT 300)
endmacro()

define_papilio_test(test_access)

define_papilio_test(test_memory)

define_papilio_test(test_locale)

define_papilio_test(test_print)

define_papilio_test(test_utility)

define_papilio_test(test_utf_stralgo)

define_papilio_test(test_utf_codepoint)

define_papilio_test(test_utf_string)

if(${papilio_build_module})
    define_papilio_test(test_modules)
endif()

add_subdirectory(test_core)

add_subdirectory(test_format)

add_subdirectory(test_container)
