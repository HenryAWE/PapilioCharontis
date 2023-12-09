#pragma once

#include <utility>
#include "../macros.hpp"

namespace papilio
{
namespace detail
{
    // Implement unreachable() of C++ 23 for compatibility.
    [[noreturn]]
    inline void unreachable()
    {
#ifdef __cpp_lib_unreachable
        std::unreachable();
#elif defined PAPILIO_COMPILER_MSVC
        __assume(false);
#elif defined PAPILIO_COMPILER_GCC
        __builtin_unreachable();
#else
        // An empty function body and the [[noreturn]] attribute is enough to raise undefined behavior.
#endif
    }
} // namespace detail

// forwark_like<T, U> of C++ 23
// Use implementation from cppreference.com
template <class T, class U>
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
} // namespace papilio

#define PAPILIO_UNREACHABLE() (::papilio::detail::unreachable())
