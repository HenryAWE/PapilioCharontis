/**
 * @file xchar.hpp
 * @author HenryAWE
 * @brief Format support for external character types.
 *
 * @note This header will not be included by the main header `<papilio/papilio.hpp>`.
 */

#ifndef PAPILIO_XCHAR_HPP
#define PAPILIO_XCHAR_HPP

#pragma once

#include "format.hpp"
#include "detail/prefix.hpp"

#ifdef PAPILIO_ENABLE_XCHAR

namespace papilio
{
/// @defgroup XChar Format support for external character types
/// @brief Support for `char8_t`, `char16_t`, and `char32_t`.
/// @{

template <typename... Args>
using u8format_string = basic_format_string<char8_t, std::type_identity_t<Args>...>;
using u8format_context = basic_format_context<format_iterator_for<char8_t>, char8_t>;
using u8format_args_ref = basic_format_args_ref<u8format_context, char8_t>;

template <typename... Args>
auto make_u8format_args(Args&&... args)
{
    using context_t = format_context_traits<u8format_context>;
    return context_t::make_format_args(std::forward<Args>(args)...);
}

template <typename OutputIt>
OutputIt vformat_to(
    OutputIt out,
    std::u8string_view fmt,
    const u8format_args_ref& args
)
{
    std::u8string buf;
    detail::vformat_to_impl<char8_t, format_iterator_for<char8_t>, u8format_context>(
        std::back_inserter(buf),
        nullptr,
        fmt,
        args
    );
    return std::copy(buf.begin(), buf.end(), std::move(out));
}

template <typename OutputIt, typename... Args>
OutputIt format_to(
    OutputIt out,
    u8format_string<Args...> fmt,
    Args&&... args
)
{
    return PAPILIO_NS vformat_to(
        out,
        fmt,
        PAPILIO_NS make_u8format_args(std::forward<Args>(args)...)
    );
}

[[nodiscard]]
std::u8string vformat(std::u8string_view fmt, const u8format_args_ref& args);

template <typename... Args>
[[nodiscard]]
std::u8string format(u8format_string<Args...> fmt, Args&&... args)
{
    return vformat(
        fmt.get(), PAPILIO_NS make_u8format_args(std::forward<Args>(args)...)
    );
}

template <typename OutputIt, typename... Args>
format_to_n_result<OutputIt> format_to_n(
    OutputIt out,
    std::iter_difference_t<OutputIt> n,
    u8format_string<Args...> fmt,
    Args&&... args
)
{
    return PAPILIO_NS detail::format_to_n_impl<char8_t, OutputIt>(
        std::move(out),
        n,
        nullptr,
        fmt.get(),
        std::forward<Args>(args)...
    );
}

template <typename... Args>
[[nodiscard]]
std::size_t formatted_size(
    u8format_string<Args...> fmt,
    Args&&... args
)
{
    return detail::formatted_size_helper<char8_t>(
        nullptr,
        fmt.get(),
        std::forward<Args>(args)...
    );
}

template <typename... Args>
using u16format_string = basic_format_string<char16_t, std::type_identity_t<Args>...>;
using u16format_context = basic_format_context<format_iterator_for<char16_t>, char16_t>;
using u16format_args_ref = basic_format_args_ref<u16format_context, char16_t>;

template <typename... Args>
auto make_u16format_args(Args&&... args)
{
    using context_t = format_context_traits<u16format_context>;
    return context_t::make_format_args(std::forward<Args>(args)...);
}

template <typename OutputIt>
OutputIt vformat_to(
    OutputIt out,
    std::u16string_view fmt,
    const u16format_args_ref& args
)
{
    std::u16string buf;
    detail::vformat_to_impl<char16_t, format_iterator_for<char16_t>, u16format_context>(
        std::back_inserter(buf),
        nullptr,
        fmt,
        args
    );
    return std::copy(buf.begin(), buf.end(), std::move(out));
}

template <typename OutputIt, typename... Args>
OutputIt format_to(
    OutputIt out,
    u16format_string<Args...> fmt,
    Args&&... args
)
{
    return PAPILIO_NS vformat_to(
        out,
        fmt,
        PAPILIO_NS make_u16format_args(std::forward<Args>(args)...)
    );
}

[[nodiscard]]
std::u16string vformat(std::u16string_view fmt, const u16format_args_ref& args);

template <typename... Args>
[[nodiscard]]
std::u16string format(u16format_string<Args...> fmt, Args&&... args)
{
    return vformat(
        fmt.get(), PAPILIO_NS make_u16format_args(std::forward<Args>(args)...)
    );
}

template <typename OutputIt, typename... Args>
format_to_n_result<OutputIt> format_to_n(
    OutputIt out,
    std::iter_difference_t<OutputIt> n,
    u16format_string<Args...> fmt,
    Args&&... args
)
{
    return PAPILIO_NS detail::format_to_n_impl<char16_t, OutputIt>(
        std::move(out),
        n,
        nullptr,
        fmt.get(),
        std::forward<Args>(args)...
    );
}

template <typename... Args>
[[nodiscard]]
std::size_t formatted_size(
    u16format_string<Args...> fmt,
    Args&&... args
)
{
    return detail::formatted_size_helper<char16_t>(
        nullptr,
        fmt.get(),
        std::forward<Args>(args)...
    );
}

template <typename... Args>
using u32format_string = basic_format_string<char32_t, std::type_identity_t<Args>...>;

using u32format_context = basic_format_context<format_iterator_for<char32_t>, char32_t>;
using u32format_args_ref = basic_format_args_ref<u32format_context, char32_t>;

template <typename... Args>
auto make_u32format_args(Args&&... args)
{
    using context_t = format_context_traits<u32format_context>;
    return context_t::make_format_args(std::forward<Args>(args)...);
}

template <typename OutputIt>
OutputIt vformat_to(
    OutputIt out,
    std::u32string_view fmt,
    const u32format_args_ref& args
)
{
    std::u32string buf;
    detail::vformat_to_impl<char32_t, format_iterator_for<char32_t>, u32format_context>(
        std::back_inserter(buf),
        nullptr,
        fmt,
        args
    );
    return std::copy(buf.begin(), buf.end(), std::move(out));
}

template <typename OutputIt, typename... Args>
OutputIt format_to(
    OutputIt out,
    u32format_string<Args...> fmt,
    Args&&... args
)
{
    return PAPILIO_NS vformat_to(
        out,
        fmt,
        PAPILIO_NS make_u32format_args(std::forward<Args>(args)...)
    );
}

[[nodiscard]]
std::u32string vformat(std::u32string_view fmt, const u32format_args_ref& args);

template <typename... Args>
[[nodiscard]]
std::u32string format(u32format_string<Args...> fmt, Args&&... args)
{
    return vformat(
        fmt.get(), PAPILIO_NS make_u32format_args(std::forward<Args>(args)...)
    );
}

template <typename OutputIt, typename... Args>
format_to_n_result<OutputIt> format_to_n(
    OutputIt out,
    std::iter_difference_t<OutputIt> n,
    u32format_string<Args...> fmt,
    Args&&... args
)
{
    return PAPILIO_NS detail::format_to_n_impl<char32_t, OutputIt>(
        std::move(out),
        n,
        nullptr,
        fmt.get(),
        std::forward<Args>(args)...
    );
}

template <typename... Args>
[[nodiscard]]
std::size_t formatted_size(
    u32format_string<Args...> fmt,
    Args&&... args
)
{
    return detail::formatted_size_helper<char32_t>(
        nullptr,
        fmt.get(),
        std::forward<Args>(args)...
    );
}

// Explicitly instantiate the most common xchar paths in the library.
extern template format_iterator_for<char8_t> vformat_to(
    format_iterator_for<char8_t>,
    std::u8string_view,
    const u8format_args_ref&
);

extern template format_iterator_for<char16_t> vformat_to(
    format_iterator_for<char16_t>,
    std::u16string_view,
    const u16format_args_ref&
);

extern template format_iterator_for<char32_t> vformat_to(
    format_iterator_for<char32_t>,
    std::u32string_view,
    const u32format_args_ref&
);

extern template class basic_format_arg<u8format_context>;
extern template class basic_format_arg<u16format_context>;
extern template class basic_format_arg<u32format_context>;

extern template class basic_interpreter<u8format_context, false>;
extern template class basic_interpreter<u16format_context, false>;
extern template class basic_interpreter<u32format_context, false>;

extern template class basic_interpreter_base<char8_t, false>;
extern template class basic_interpreter_base<char16_t, false>;
extern template class basic_interpreter_base<char32_t, false>;

namespace detail
{
    extern template format_iterator_for<char8_t> vformat_to_impl<
        char8_t,
        format_iterator_for<char8_t>,
        u8format_context>(
        format_iterator_for<char8_t>,
        locale_ref,
        std::u8string_view,
        const basic_format_args_ref<u8format_context>&
    );

    extern template format_iterator_for<char16_t> vformat_to_impl<
        char16_t,
        format_iterator_for<char16_t>,
        u16format_context>(
        format_iterator_for<char16_t>,
        locale_ref,
        std::u16string_view,
        const basic_format_args_ref<u16format_context>&
    );

    extern template format_iterator_for<char32_t> vformat_to_impl<
        char32_t,
        format_iterator_for<char32_t>,
        u32format_context>(
        format_iterator_for<char32_t>,
        locale_ref,
        std::u32string_view,
        const basic_format_args_ref<u32format_context>&
    );
} // namespace detail

// @}
} // namespace papilio

#endif

#include "detail/suffix.hpp"

#endif
