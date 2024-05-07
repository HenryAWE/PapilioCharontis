cmake_minimum_required(VERSION 3.20)

# Use C++ 20 by default
if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Force UTF-8 on MSVC
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

option(papilio_build_unit_test "build unit tests" OFF)
mark_as_advanced(papilio_build_unit_test)

option(papilio_build_example "build examples" OFF)
option(papilio_build_module "build module (experimental)" OFF)

message("Papilio Charontis v${papilio_version_major}.${papilio_version_minor}.${papilio_version_patch}")

project(
    "Papilio Charontis"
    VERSION 0.3.0
    LANGUAGES CXX
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    option(papilio_use_libcpp "Use libc++ for Clang" OFF)
    if(${papilio_use_libcpp})
        add_compile_options("-stdlib=libc++")
        add_link_options("-stdlib=libc++")
    endif()
endif()

set(glob_expr "src/*.cpp" "include/*.hpp" "include/*.inl")
file(GLOB_RECURSE papilio_src ${glob_expr})

if(${papilio_build_module})
    set(CMAKE_CXX_SCAN_FOR_MODULES ON)
endif()

add_library(papilio ${papilio_src})
target_sources(papilio PUBLIC "natvis/papilio.natvis")
target_sources(papilio PUBLIC "cpp.hint")
target_include_directories(papilio PUBLIC include)
target_compile_options(papilio PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
    $<$<CXX_COMPILER_ID:Clang>:-Weverything>
)
if(${papilio_build_module})
    file(GLOB_RECURSE papilio_modules_src "modules/*.cppm")
    target_sources(
        papilio
        PUBLIC
            FILE_SET papilio_modules TYPE CXX_MODULES FILES
            ${papilio_modules_src}
    )
endif()

if(${papilio_build_example})
    add_subdirectory(example)
endif()
if(${papilio_build_unit_test})
    message(STATUS "Fetching GoogleTest v1.14.0 from GitHub")

    include(FetchContent)

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    set(BUILD_GMOCK OFF)
    set(INSTALL_GTEST OFF)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    add_subdirectory(test)
endif()
