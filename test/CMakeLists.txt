enable_testing()

include(GoogleTest)
add_subdirectory(papilio_test)

function(configure_test test_target)
    target_link_libraries(${test_target} PRIVATE papilio_test)
    set_target_properties(${test_target} PROPERTIES
        CMAKE_CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
    )
    # Use a large DISCOVERY_TIMEOUT value reduce accidental CI failure
    gtest_discover_tests(${test_target} DISCOVERY_TIMEOUT 1000)
    if(WIN32)
        target_compile_definitions(${test_target} PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()
endfunction()

macro(papilio_simple_test test_name)
    add_executable(${test_name} ${test_name}.cpp)
    configure_test(${test_name})
endmacro()

papilio_simple_test(test_memory)

papilio_simple_test(test_locale)

papilio_simple_test(test_print)

papilio_simple_test(test_utility)

papilio_simple_test(test_utf_stralgo)

papilio_simple_test(test_utf_codepoint)

papilio_simple_test(test_utf_string)

if(${papilio_build_module})
    papilio_simple_test(test_modules)
    target_compile_options(test_modules PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/wd5050>)
endif()

add_subdirectory(test_core)

add_subdirectory(test_access)

add_subdirectory(test_format)

add_subdirectory(test_container)
