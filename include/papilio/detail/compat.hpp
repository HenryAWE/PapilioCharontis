#ifndef PAPILIO_DETAIL_COMPAT_HPP
#define PAPILIO_DETAIL_COMPAT_HPP

#pragma once

#include <utility>
#include "prefix.hpp"

namespace papilio
{
// Implement unreachable() of C++ 23 for compatibility.
PAPILIO_EXPORT [[noreturn]]
inline void unreachable()
{
#ifdef __cpp_lib_unreachable
#    define PAPILIO_HAS_UNREACHABLE "std::unreachable()"
    std::unreachable();
#elif defined PAPILIO_COMPILER_MSVC || defined PAPILIO_COMPILER_CLANG_CL
#    define PAPILIO_HAS_UNREACHABLE "__assume(false)"
    __assume(false);
#elif defined PAPILIO_COMPILER_GCC || defined PAPILIO_COMPILER_CLANG
#    define PAPILIO_HAS_UNREACHABLE "__builtin_unreachable()"
    __builtin_unreachable();
#else
#    define PAPILIO_HAS_UNREACHABLE "[[noreturn]]"
    // An empty function body and the [[noreturn]] attribute is enough to raise undefined behavior.
#endif
}

// forward_like<T, U> of C++ 23
// Use implementation from cppreference.com
PAPILIO_EXPORT template <class T, class U>
[[nodiscard]]
constexpr auto&& forward_like(U&& x) noexcept
{
    constexpr bool is_adding_const = std::is_const_v<std::remove_reference_t<T>>;
    if constexpr(std::is_lvalue_reference_v<T&&>)
    {
        if constexpr(is_adding_const)
            return std::as_const(x);
        else
            return static_cast<U&>(x);
    }
    else
    {
        if constexpr(is_adding_const)
            return std::move(std::as_const(x));
        else
            return std::move(x);
    }
}

// to_underlying<Enum> of C++ 23
PAPILIO_EXPORT template <typename Enum>
[[nodiscard]]
constexpr std::underlying_type_t<Enum> to_underlying(Enum e) noexcept
{
    return static_cast<std::underlying_type_t<Enum>>(e);
}
} // namespace papilio

#include "suffix.hpp"

#endif
