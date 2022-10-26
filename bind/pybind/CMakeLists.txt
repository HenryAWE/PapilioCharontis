find_package(pybind11 REQUIRED)

pybind11_add_module(pypapilio MODULE pypapilio.cpp)
set_target_properties(pypapilio PROPERTIES SUFFIX ".pyd")
target_link_libraries(pypapilio PRIVATE papilio)
