#pragma once

#ifdef _MSC_VER
#   define PAPILIO_COMPILER_MSVC 1
#endif
#ifdef __GUNC__
#   define PAPILIO_COMPILER_GCC 1
#endif

#ifdef PAPILIO_COMPILER_MSVC
// [[no_unique_address]] is ignored by MSVC even in C++20 mode.
#   define PAPILIO_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
#   define PAPILIO_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

#if __has_cpp_attribute(assume)
#   define PAPILIO_ASSUME(expr) [[assume(expr)]]
#elif defined PAPILIO_COMPILER_MSVC
#   define PAPILIO_ASSUME(expr) __assume(expr)
#elif defined PAPILIO_COMPILER_GCC
#   define PAPILIO_ASSUME(expr) __attribute__((assume(expr)))
#else
#   define PAPILIO_ASSUME(expr)
#endif
