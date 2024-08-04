#ifndef PAPILIO_DETAIL_CONFIG_HPP
#define PAPILIO_DETAIL_CONFIG_HPP

#pragma once

#include <version>

#ifdef PAPILIO_BUILD_MODULES
#    define PAPILIO_EXPORT export
#else
#    define PAPILIO_EXPORT
#endif

#if defined(_WIN32) || defined(_WIN64)
#    define PAPILIO_PLATFORM_WINDOWS 1
#elif defined(__EMSCRIPTEN__)
#    define PAPILIO_PLATFORM_EMSCRIPTEN 1
#elif defined(__linux__) || defined(__gnu_linux__)
#    define PAPILIO_PLATFORM_LINUX 1
#endif

#ifdef __GLIBCXX__
// libstdc++
#    define PAPILIO_STDLIB_LIBSTDCPP __GLIBCXX__
#endif
#ifdef _LIBCPP_VERSION
// libc++
#    define PAPILIO_STDLIB_LIBCPP _LIBCPP_VERSION
#endif
#ifdef _CPPLIB_VER
// MSVC STL
#    define PAPILIO_STDLIB_MSVC_STL _CPPLIB_VER
#endif

#ifdef _MSC_VER
#    ifdef __clang__
#        define PAPILIO_COMPILER_CLANG    __clang_major__
#        define PAPILIO_COMPILER_CLANG_CL __clang__
#    else
#        define PAPILIO_COMPILER_MSVC _MSC_VER
#    endif
#endif
#ifdef __GNUC__
#    ifndef __clang__
#        define PAPILIO_COMPILER_GCC __GNUC__
#    else
#        define PAPILIO_COMPILER_CLANG __clang_version__
#    endif
#endif

#ifdef PAPILIO_COMPILER_MSVC
#    define PAPILIO_CPLUSPLUS _MSVC_LANG
#else
#    define PAPILIO_CPLUSPLUS __cplusplus
#endif

#if PAPILIO_CPLUSPLUS < 202002L
#    error "C++20 or later is required."
#endif

#ifdef PAPILIO_COMPILER_CLANG
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wc++98-compat-pedantic"
#endif

#define PAPILIO_HAS_VA_OPT_HELPER(_0, _1, result, ...) result
#define PAPILIO_HAS_VA_OPT_IMPL(...) \
    PAPILIO_HAS_VA_OPT_HELPER(__VA_OPT__(, ), 1, 0)

#if PAPILIO_HAS_VA_OPT_IMPL(_0) == 1
#    define PAPILIO_HAS_VA_OPT 1
#endif

#undef PAPILIO_HAS_VA_OPT_HELPER
#undef PAPILIO_HAS_VA_OPT_IMPL

#ifdef PAPILIO_COMPILER_CLANG
#    pragma clang diagnostic pop
#endif

#if defined(__cpp_multidimensional_subscript) && __cpp_multidimensional_subscript >= 202110L
#    define PAPILIO_HAS_MULTIDIMENSIONAL_SUBSCRIPT __cpp_multidimensional_subscript
#endif

#if defined(__cpp_lib_stacktrace) && __cpp_lib_stacktrace >= 202011L
#    define PAPILIO_HAS_LIB_STACKTRACE __cpp_lib_stacktrace
#endif

#if defined(__cpp_lib_expected) && __cpp_lib_expected >= 202202L
#    define PAPILIO_HAS_LIB_EXPECTED __cpp_lib_expected
#endif

#endif
