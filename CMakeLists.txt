cmake_minimum_required(VERSION 3.20)

# Use C++ 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Force UTF-8 on MSVC
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

option(papilio_build_unit_test "build unit tests" OFF)
option(papilio_build_example "build examples" OFF)
option(papilio_build_module "build module (experimental)" OFF)

file(STRINGS "include/papilio/macros.hpp" papilio_version_major_line REGEX "^#define PAPILIO_VERSION_MAJOR [0-9]+$")
file(STRINGS "include/papilio/macros.hpp" papilio_version_minor_line REGEX "^#define PAPILIO_VERSION_MINOR [0-9]+$")
file(STRINGS "include/papilio/macros.hpp" papilio_version_patch_line REGEX "^#define PAPILIO_VERSION_PATCH [0-9]+$")
string(REGEX REPLACE "^#define PAPILIO_VERSION_MAJOR ([0-9]+)$" "\\1" papilio_version_major "${papilio_version_major_line}")
string(REGEX REPLACE "^#define PAPILIO_VERSION_MINOR ([0-9]+)$" "\\1" papilio_version_minor "${papilio_version_minor_line}")
string(REGEX REPLACE "^#define PAPILIO_VERSION_PATCH ([0-9]+)$" "\\1" papilio_version_patch "${papilio_version_patch_line}")

message("Papilio Charontis v${papilio_version_major}.${papilio_version_minor}.${papilio_version_patch}")

project(
    "Papilio Charontis"
    VERSION ${papilio_version_major}.${papilio_version_minor}.${papilio_version_patch}
    LANGUAGES C CXX
)

set(glob_expr "src/*.cpp" "include/*.hpp")
file(GLOB_RECURSE papilio_src ${glob_expr})

if(${papilio_build_module})
    file(GLOB_RECURSE papilio_modules "modules/*.ixx")
    list(APPEND papilio_src ${papilio_modules})
else()
    set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
endif()

add_library(papilio ${papilio_src})
target_sources(papilio PUBLIC "natvis/papilio.natvis")
target_include_directories(papilio PUBLIC include)
target_compile_options(papilio PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

add_subdirectory(binding)
if(${papilio_build_example})
    add_subdirectory(example)
endif()
if(${papilio_build_unit_test})
    enable_testing()
    add_subdirectory(test)
endif()
