add_executable(info "info.cpp")
target_link_libraries(info PRIVATE papilio)

if(${papilio_c_interface})
    add_executable(c_interface "c_interface.c")
    target_link_libraries(c_interface PRIVATE papilio)
endif()
