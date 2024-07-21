cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED CMAKE_CXX_STANDARD)
    # Use C++ 20 by default
    set(CMAKE_CXX_STANDARD 20)
elseif(CMAKE_CXX_STANDARD LESS 20)
    message(FATAL_ERROR "C++ 20 is required")
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(papilio_build_unit_test "build unit tests" OFF)
mark_as_advanced(papilio_build_unit_test)

# If you only want to build the documentation, you can disable building the library
option(papilio_build_lib "build the library" ON)
mark_as_advanced(papilio_build_lib)

option(papilio_build_example "build examples" OFF)
option(papilio_build_module "build module (experimental)" OFF)
option(papilio_build_doc "build documents" OFF)

file(STRINGS "include/papilio/macros.hpp" papilio_version_major_line REGEX "^#define PAPILIO_VERSION_MAJOR [0-9]+$")
file(STRINGS "include/papilio/macros.hpp" papilio_version_minor_line REGEX "^#define PAPILIO_VERSION_MINOR [0-9]+$")
file(STRINGS "include/papilio/macros.hpp" papilio_version_patch_line REGEX "^#define PAPILIO_VERSION_PATCH [0-9]+$")
string(REGEX REPLACE "^#define PAPILIO_VERSION_MAJOR ([0-9]+)$" "\\1" papilio_version_major "${papilio_version_major_line}")
string(REGEX REPLACE "^#define PAPILIO_VERSION_MINOR ([0-9]+)$" "\\1" papilio_version_minor "${papilio_version_minor_line}")
string(REGEX REPLACE "^#define PAPILIO_VERSION_PATCH ([0-9]+)$" "\\1" papilio_version_patch "${papilio_version_patch_line}")
unset(papilio_version_major_line)
unset(papilio_version_minor_line)
unset(papilio_version_patch_line)

message("Papilio Charontis v${papilio_version_major}.${papilio_version_minor}.${papilio_version_patch}")
project(
    "Papilio Charontis"
    VERSION ${papilio_version_major}.${papilio_version_minor}.${papilio_version_patch}
)
if(${papilio_build_lib})
    enable_language(CXX)

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

    include(GNUInstallDirs)

    add_library(papilio ${papilio_src})
    target_sources(papilio PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/natvis/papilio.natvis>")
    target_include_directories(papilio PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    # Add an ALIAS target for using the library by add_subdirectory()
    add_library(papilio::papilio ALIAS papilio)

    # Enable warnings for development
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

    install(
        TARGETS papilio
        EXPORT PapilioTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    install(
        DIRECTORY ${PROJECT_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hpp" PATTERN "*.inl"
    )

    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
        "PapilioConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )
    configure_package_config_file(
        "${CMAKE_CURRENT_LIST_DIR}/cmake/PapilioConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/PapilioConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake/papilio
    )
    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/PapilioConfig.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake/papilio
    )
    install(
        EXPORT PapilioTargets
        FILE PapilioTargets.cmake
        NAMESPACE papilio::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake/papilio
    )
endif()

if(${papilio_build_doc})
    find_package(Doxygen REQUIRED)
    if(${DOXYGEN_VERSION} VERSION_LESS 1.9.2)
        # C++ concepts need Doxygen 1.9.2 or letter
        message(FATAL_ERROR "Doxygen 1.9.2 or later is required")
    endif()

    configure_file(script/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

    add_custom_target(
        papilio_doc
        COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
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
