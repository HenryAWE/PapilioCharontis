/**
 * @file macros.hpp
 * @author HenryAWE
 * @brief Header of macro definitions.
 */

#ifndef PAPILIO_MACROS_HPP
#define PAPILIO_MACROS_HPP

#pragma once

#include <cassert>
#include <version>
#include <type_traits> // std::same_as
#include <string_view>
#include "detail/config.hpp"
#include "detail/compat.hpp"

// clang-format off: Used by CMakeLists.txt for parsing version

#define PAPILIO_VERSION_MAJOR 1
#define PAPILIO_VERSION_MINOR 1
#define PAPILIO_VERSION_PATCH 0

// clang-format on

/**
 * Currently, this macro is just an alias of the `assert()` macro.
 * It might be changed to a custom implementation for providing more information.
 */
#define PAPILIO_ASSERT(expr) assert(expr)

/**
 * Use this macro to avoid errors caused by ADL,
 * especially when mixing this library and the standard <format>.
*/
#define PAPILIO_NS ::papilio::

#if defined PAPILIO_COMPILER_MSVC
// [[no_unique_address]] is ignored by MSVC even in C++20 mode.
// https://devblogs.microsoft.com/cppblog/msvc-cpp20-and-the-std-cpp20-switch/#msvc-extensions-and-abi
#    define PAPILIO_NO_UNIQUE_ADDRESS          [[msvc::no_unique_address]]
#    define PAPILIO_HAS_ATTR_NO_UNIQUE_ADDRESS "[[msvc::no_unique_address]]"
// Clang does not support [[no_unique_address]] on Windows.
// https://github.com/llvm/llvm-project/issues/49358
#elif __has_cpp_attribute(no_unique_address)
#    define PAPILIO_NO_UNIQUE_ADDRESS          [[no_unique_address]]
#    define PAPILIO_HAS_ATTR_NO_UNIQUE_ADDRESS "[[no_unique_address]]"
#else
// Placeholder
#    define PAPILIO_NO_UNIQUE_ADDRESS
#endif

#define PAPILIO_UNREACHABLE() (PAPILIO_NS unreachable())

#endif
