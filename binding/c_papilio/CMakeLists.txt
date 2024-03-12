if(${papilio_c_interface})
    set(CMAKE_C_STANDARD 11)
    set(CMAKE_C_STANDARD_REQUIRED ON)

    file(GLOB c_pailio_src "src/*.cpp")
    add_library(c_papilio STATIC ${c_pailio_src})
    target_include_directories(c_papilio PUBLIC include)
    target_sources(c_papilio PUBLIC "cpp.hint")
    target_link_libraries(c_papilio PUBLIC papilio)

    if(${papilio_build_example})
        add_executable(c_interface "example/c_interface.c")
        target_link_libraries(c_interface PRIVATE c_papilio)
        set_target_properties(c_interface
            PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/example"
        )
    endif()

    if(${papilio_build_unit_test})
        include(GoogleTest)

        enable_testing()

        file(GLOB test_c_papilio_src "test/*.cpp" "test/*.h" "test/*.c")

        add_executable(test_c_papilio ${test_c_papilio_src})
        target_sources(test_c_papilio PRIVATE "test/cpp.hint")
        target_link_libraries(test_c_papilio PRIVATE c_papilio)
        target_link_libraries(test_c_papilio PRIVATE GTest::gtest GTest::gtest_main)
        gtest_discover_tests(test_c_papilio)
    endif()
endif()