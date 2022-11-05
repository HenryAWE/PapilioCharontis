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

option(papilio_build_unit_test "build unit tests" OFF)
option(papilio_build_module "build module (experimental)" OFF)

project(
    "Papilio Charontis"
    VERSION 0.1.0
    LANGUAGES C CXX
)

set(glob_expr "src/*.cpp")
if(${papilio_build_module})
    list(APPEND glob_expr "src/*.cppm")
endif()
file(GLOB_RECURSE papilio_src ${glob_expr})
add_library(papilio ${papilio_src})
target_include_directories(papilio PUBLIC include)

if(${papilio_build_unit_test})
    enable_testing()
    add_subdirectory(test)
endif()
add_subdirectory(bind)
