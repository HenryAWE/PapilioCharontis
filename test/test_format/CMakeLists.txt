file(GLOB test_format_src "*.cpp" "*.hpp")
add_executable(test_format ${test_format_src})
configure_test(test_format)

# For #include <stacktrace>
if(LINUX)
    include(CheckIncludeFileCXX)
    check_include_file_cxx("stacktrace" stdlib_has_stacktrace)
    if(${stdlib_has_stacktrace})
        target_link_libraries(test_format PRIVATE stdc++exp)
    endif()
endif()
