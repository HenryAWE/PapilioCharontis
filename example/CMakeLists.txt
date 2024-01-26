add_executable(info "info.cpp")
target_link_libraries(info PRIVATE papilio)

if(${papilio_build_module})
    add_executable(import "import.cpp")
    target_link_libraries(import PRIVATE papilio)
endif()

add_subdirectory(playground)
