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
#include "detail/config.hpp" // IWYU pragma: export
#include "detail/compat.hpp" // IWYU pragma: export

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
#define PAPILIO_NS            ::papilio::

#define PAPILIO_UNREACHABLE() (PAPILIO_NS unreachable())

#endif
