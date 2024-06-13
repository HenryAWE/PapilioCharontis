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
#include <type_traits> // std::is_same_v
#include <string_view>
#include "detail/config.hpp"
#include "detail/compat.hpp"

#define PAPILIO_VERSION_MAJOR 0
#define PAPILIO_VERSION_MINOR 3
#define PAPILIO_VERSION_PATCH 0

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

/**
 * @brief Stringize given text.
 */
#define PAPILIO_STRINGIZE(text) #text
/**
 * @brief Stringize given text without expanding any macros in it.
 */
#define PAPILIO_STRINGIZE_EX(text) PAPILIO_STRINGIZE(text)

#define PAPILIO_TSTRING_EX(char_t, str, suffix, decl)         \
    []() noexcept -> decltype(auto)                           \
    {                                                         \
        decl;                                                 \
        if constexpr(::std::is_same_v<char_t, char>)          \
            return str##suffix;                               \
        else if constexpr(::std::is_same_v<char_t, wchar_t>)  \
            return L##str##suffix;                            \
        else if constexpr(::std::is_same_v<char_t, char8_t>)  \
            return u8##str##suffix;                           \
        else if constexpr(::std::is_same_v<char_t, char32_t>) \
            return U##str##suffix;                            \
        else if constexpr(::std::is_same_v<char_t, char16_t>) \
            return u##str##suffix;                            \
    }()

#define PAPILIO_TSTRING(char_t, str)      PAPILIO_TSTRING_EX(char_t, str, , )

#define PAPILIO_TSTRING_VIEW(char_t, str) PAPILIO_TSTRING_EX( \
    char_t, str, sv, using namespace ::std                    \
)

#define PAPILIO_UNREACHABLE() (PAPILIO_NS unreachable())

#endif
