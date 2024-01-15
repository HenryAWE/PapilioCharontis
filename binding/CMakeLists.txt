option(papilio_c_interface "build C interface" ON)

if(${papilio_c_interface})
    add_subdirectory("c_papilio")
endif()
