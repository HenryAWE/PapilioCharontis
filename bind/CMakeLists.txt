option(papilio_build_pybind BOOL OFF)

if(${papilio_build_pybind})
    add_subdirectory(pybind)
endif()
