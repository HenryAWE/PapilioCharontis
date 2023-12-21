#pragma once

// Forward declarations

#include <iterator>

namespace papilio
{
namespace detail
{
    template <typename CharT>
    using fmt_iter_for = std::back_insert_iterator<std::basic_string<CharT>>;
}

template <typename Context>
class basic_format_arg;

template <typename FormatContext>
class basic_format_parse_context;

template <typename OutputIt, typename CharT = char>
class basic_format_context;

template <typename T, typename CharT = char>
class formatter;

using format_context = basic_format_context<detail::fmt_iter_for<char>, char>;
using wformat_context = basic_format_context<detail::fmt_iter_for<wchar_t>, wchar_t>;
using format_arg = basic_format_arg<format_context>;
using wformat_arg = basic_format_arg<wformat_context>;
} // namespace papilio
