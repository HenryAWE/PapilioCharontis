file(GLOB playground_src "*.cpp" "*.hpp")
add_executable(playground ${playground_src})
target_link_libraries(playground PRIVATE papilio)
