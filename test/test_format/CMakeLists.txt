file(GLOB test_format_src "*.cpp" "*.hpp")
add_executable(test_format ${test_format_src})
configure_test(test_format)

if(CMAKE_CXX_STANDARD GREATER_EQUAL 23)
    # For #include <stacktrace>
    if(CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 14.0)
        target_link_libraries(test_format PRIVATE stdc++exp)
    endif()
endif()
