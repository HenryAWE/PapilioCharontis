cmake_minimum_required(VERSION 3.20)

# Use C++ 20
if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.12.25835)
    set(CMAKE_CXX20_STANDARD_COMPILE_OPTION "-std:c++latest")
    set(CMAKE_CXX20_EXTENSION_COMPILE_OPTION "-std:c++latest")
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Force UTF-8 on MSVC
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")


project(
    "Papilio Charontis"
    VERSION 0.0.1
    LANGUAGES C CXX
)

aux_source_directory(src papilio_src)
add_library(papilio ${papilio_src})
target_include_directories(papilio PUBLIC include)

enable_testing()
add_subdirectory(test)
