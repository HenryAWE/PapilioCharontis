option(papilio_c_interface "build C interface" OFF)

if(${papilio_c_interface})
    add_subdirectory("c_papilio")
endif()

if(EMSCRIPTEN)
    option(papilio_embind "build Emscripten binding" ON)
    if(${papilio_embind})
        add_subdirectory(em_papilio)
    endif()
endif()
