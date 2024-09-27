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
#include "locale.hpp" // For workaround
#include "detail/prefix.hpp"

namespace papilio
{
/// @defgroup XChar Format support for external character types
/// @brief Support for `char8_t`, `char16_t`, and `char32_t`.
/// @{

PAPILIO_EXPORT template <typename... Args>
using u8format_string = basic_format_string<char8_t, std::type_identity_t<Args>...>;
PAPILIO_EXPORT using u8format_context = basic_format_context<format_iterator_for<char8_t>, char8_t>;
PAPILIO_EXPORT using u8format_args_ref = basic_format_args_ref<u8format_context, char8_t>;

PAPILIO_EXPORT template <typename... Args>
auto make_u8format_args(Args&&... args)
{
    using context_t = format_context_traits<u8format_context>;
    return context_t::make_format_args(std::forward<Args>(args)...);
}

PAPILIO_EXPORT template <typename OutputIt>
OutputIt vformat_to(
    OutputIt out,
    std::u8string_view fmt,
    const format_args_ref_for<OutputIt, char8_t>& args
)
{
    using context_type = basic_format_context<OutputIt, char8_t>;
    return detail::vformat_to_impl<char8_t, OutputIt, context_type>(
        std::move(out),
        nullptr,
        fmt,
        args
    );
}

PAPILIO_EXPORT template <typename OutputIt, typename... Args>
OutputIt format_to(
    OutputIt out,
    u8format_string<Args...> fmt,
    Args&&... args
)
{
    using context_type = basic_format_context<OutputIt, char8_t>;
    return PAPILIO_NS detail::vformat_to_impl<char8_t, OutputIt, context_type>(
        std::move(out),
        nullptr,
        fmt,
        PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)
    );
}

PAPILIO_EXPORT
[[nodiscard]]
std::u8string vformat(std::u8string_view fmt, const u8format_args_ref& args);

PAPILIO_EXPORT template <typename... Args>
[[nodiscard]]
std::u8string format(u8format_string<Args...> fmt, Args&&... args)
{
    return vformat(
        fmt.get(), PAPILIO_NS make_u8format_args(std::forward<Args>(args)...)
    );
}

PAPILIO_EXPORT template <typename OutputIt, typename... Args>
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

namespace detail
{
    std::size_t formatted_size_impl(
        locale_ref loc,
        std::u8string_view fmt,
        const basic_format_args_ref<fmt_size_ctx_type<char8_t>>& args
    );
}

PAPILIO_EXPORT template <typename... Args>
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

PAPILIO_EXPORT template <typename... Args>
using u16format_string = basic_format_string<char16_t, std::type_identity_t<Args>...>;
PAPILIO_EXPORT using u16format_context = basic_format_context<format_iterator_for<char16_t>, char16_t>;
PAPILIO_EXPORT using u16format_args_ref = basic_format_args_ref<u16format_context, char16_t>;

PAPILIO_EXPORT template <typename... Args>
auto make_u16format_args(Args&&... args)
{
    using context_t = format_context_traits<u16format_context>;
    return context_t::make_format_args(std::forward<Args>(args)...);
}

PAPILIO_EXPORT template <typename OutputIt>
OutputIt vformat_to(
    OutputIt out,
    std::u16string_view fmt,
    const format_args_ref_for<OutputIt, char16_t>& args
)
{
    using context_type = basic_format_context<OutputIt, char16_t>;
    return detail::vformat_to_impl<char16_t, OutputIt, context_type>(
        std::move(out),
        nullptr,
        fmt,
        args
    );
}

PAPILIO_EXPORT template <typename OutputIt, typename... Args>
OutputIt format_to(
    OutputIt out,
    u16format_string<Args...> fmt,
    Args&&... args
)
{
    using context_type = basic_format_context<OutputIt, char16_t>;
    return PAPILIO_NS detail::vformat_to_impl<char16_t, OutputIt, context_type>(
        std::move(out),
        nullptr,
        fmt,
        PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)
    );
}

PAPILIO_EXPORT
[[nodiscard]]
std::u16string vformat(std::u16string_view fmt, const u16format_args_ref& args);

PAPILIO_EXPORT template <typename... Args>
[[nodiscard]]
std::u16string format(u16format_string<Args...> fmt, Args&&... args)
{
    return vformat(
        fmt.get(), PAPILIO_NS make_u16format_args(std::forward<Args>(args)...)
    );
}

PAPILIO_EXPORT template <typename OutputIt, typename... Args>
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

namespace detail
{
    std::size_t formatted_size_impl(
        locale_ref loc,
        std::u16string_view fmt,
        const basic_format_args_ref<fmt_size_ctx_type<char16_t>>& args
    );
}

PAPILIO_EXPORT template <typename... Args>
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

PAPILIO_EXPORT template <typename... Args>
using u32format_string = basic_format_string<char32_t, std::type_identity_t<Args>...>;

PAPILIO_EXPORT using u32format_context = basic_format_context<format_iterator_for<char32_t>, char32_t>;
PAPILIO_EXPORT using u32format_args_ref = basic_format_args_ref<u32format_context, char32_t>;

PAPILIO_EXPORT template <typename... Args>
auto make_u32format_args(Args&&... args)
{
    using context_t = format_context_traits<u32format_context>;
    return context_t::make_format_args(std::forward<Args>(args)...);
}

PAPILIO_EXPORT template <typename OutputIt>
OutputIt vformat_to(
    OutputIt out,
    std::u32string_view fmt,
    const format_args_ref_for<OutputIt, char32_t>& args
)
{
    using context_type = basic_format_context<OutputIt, char32_t>;
    return detail::vformat_to_impl<char32_t, OutputIt, context_type>(
        std::move(out),
        nullptr,
        fmt,
        args
    );
}

PAPILIO_EXPORT template <typename OutputIt, typename... Args>
OutputIt format_to(
    OutputIt out,
    u32format_string<Args...> fmt,
    Args&&... args
)
{
    using context_type = basic_format_context<OutputIt, char32_t>;
    return PAPILIO_NS detail::vformat_to_impl<char32_t, OutputIt, context_type>(
        std::move(out),
        nullptr,
        fmt,
        PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)
    );
}

PAPILIO_EXPORT
[[nodiscard]]
std::u32string vformat(std::u32string_view fmt, const u32format_args_ref& args);

PAPILIO_EXPORT template <typename... Args>
[[nodiscard]]
std::u32string format(u32format_string<Args...> fmt, Args&&... args)
{
    return vformat(
        fmt.get(), PAPILIO_NS make_u32format_args(std::forward<Args>(args)...)
    );
}

PAPILIO_EXPORT template <typename OutputIt, typename... Args>
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

namespace detail
{
    std::size_t formatted_size_impl(
        locale_ref loc,
        std::u32string_view fmt,
        const basic_format_args_ref<fmt_size_ctx_type<char32_t>>& args
    );
}

PAPILIO_EXPORT template <typename... Args>
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

// @}
} // namespace papilio

#include "detail/suffix.hpp"

#endif
