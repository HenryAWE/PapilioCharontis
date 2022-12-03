add_executable(basic_format basic_format.cpp)
target_link_libraries(basic_format PRIVATE papilio)

add_executable(playground playground.cpp)
target_link_libraries(playground PRIVATE papilio)

add_executable(unicode_output unicode_output.cpp)
target_link_libraries(unicode_output PRIVATE papilio)
