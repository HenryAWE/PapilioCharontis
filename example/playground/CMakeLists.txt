file(GLOB playground_src "*.cpp" "*.hpp")
add_executable(playground ${playground_src})
target_link_libraries(playground PRIVATE papilio)
set_target_properties(playground
    PROPERTIES OUTPUT_NAME "ipapilio"
)
set_target_properties(playground
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/example"
)
