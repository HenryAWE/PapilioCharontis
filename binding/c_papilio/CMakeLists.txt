set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

file(GLOB c_pailio_src "src/*.cpp")
add_library(c_papilio STATIC ${c_pailio_src})
target_include_directories(c_papilio PUBLIC include)
target_link_libraries(c_papilio PUBLIC papilio)

if(${papilio_c_interface})
    add_executable(c_interface "example/c_interface.c")
    target_link_libraries(c_interface PRIVATE c_papilio)
    set_target_properties(c_interface
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/example"
    )
endif()
