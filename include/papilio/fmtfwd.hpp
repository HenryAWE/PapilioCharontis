// Forward declarations

#ifndef PAPILIO_FMTFWD_HPP
#define PAPILIO_FMTFWD_HPP

#pragma once

#include <string>
#include <iterator>
#include "macros.hpp"
#include "detail/prefix.hpp"

namespace papilio
{
namespace detail
{
    template <typename CharT>
    using fmt_iter_for = std::back_insert_iterator<std::basic_string<CharT>>;
}

PAPILIO_EXPORT template <typename Context>
class basic_format_arg;

PAPILIO_EXPORT template <typename FormatContext>
class basic_format_parse_context;

PAPILIO_EXPORT template <typename OutputIt, typename CharT = char>
class basic_format_context;

PAPILIO_EXPORT template <typename T, typename CharT = char>
class formatter;

PAPILIO_EXPORT using format_context = basic_format_context<detail::fmt_iter_for<char>, char>;
PAPILIO_EXPORT using wformat_context = basic_format_context<detail::fmt_iter_for<wchar_t>, wchar_t>;
PAPILIO_EXPORT using format_arg = basic_format_arg<format_context>;
PAPILIO_EXPORT using wformat_arg = basic_format_arg<wformat_context>;
} // namespace papilio

#include "detail/suffix.hpp"

#endif
