cmake_minimum_required(VERSION 3.20)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
    message(STATUS "Use vcpkg toolchain from $ENV{VCPKG_ROOT}")
endif()

# Use C++ 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


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
