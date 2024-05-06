enable_testing()

include(GoogleTest)

macro(define_papilio_test test_name)
    add_executable(${test_name} ${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE papilio)
    target_link_libraries(${test_name} PRIVATE GTest::gtest GTest::gtest_main)
    target_sources(${test_name} PRIVATE "cpp.hint")
    gtest_discover_tests(${test_name})
    target_compile_options(${test_name} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
        # Workaround for GCC and GoogleTest, see https://github.com/google/googletest/issues/4232
        $<$<CXX_COMPILER_ID:GNU>:-Wno-error=restrict>
    )
endmacro()

define_papilio_test(test_access)

define_papilio_test(test_memory)

define_papilio_test(test_locale)

define_papilio_test(test_print)

define_papilio_test(test_script_interpreter)

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
