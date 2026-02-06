/**
 * @file fmtfwd.hpp
 * @author HenryAWE
 * @brief Forward declarations
 */

#ifndef PAPILIO_FMTFWD_HPP
#define PAPILIO_FMTFWD_HPP

#pragma once

#include <string>
#include <iterator>
#include "macros.hpp" // IWYU pragma: export
#include "detail/prefix.hpp"

namespace papilio
{
namespace utf
{
    template <typename CharT>
    class decoder;
    class codepoint;

    template <typename CharT>
    class basic_string_ref;
    template <typename CharT>
    class basic_string_container;
} // namespace utf

template <typename CharT>
class basic_indexing_value;

using indexing_value = basic_indexing_value<char>;
using windexing_value = basic_indexing_value<wchar_t>;

template <typename CharT>
class basic_attribute_name;

using attribute_name = basic_attribute_name<char>;
using wattribute_name = basic_attribute_name<wchar_t>;

class invalid_attribute_base;
template <typename CharT>
class basic_invalid_attribute;

using invalid_attribute = basic_invalid_attribute<char>;

template <typename Context>
class basic_format_arg;

template <typename FormatContext>
class basic_format_parse_context;

template <typename OutputIt, typename CharT = char>
class basic_format_context;

template <typename T, typename CharT = char>
class formatter;

template <typename CharT>
using format_iterator_for = std::back_insert_iterator<std::basic_string<CharT>>;

using format_context = basic_format_context<format_iterator_for<char>, char>;
using wformat_context = basic_format_context<format_iterator_for<wchar_t>, wchar_t>;
using format_arg = basic_format_arg<format_context>;
using wformat_arg = basic_format_arg<wformat_context>;

template <typename CharT, typename... Args>
class basic_format_string;

template <typename... Args>
using format_string = basic_format_string<char, std::type_identity_t<Args>...>;
template <typename... Args>
using wformat_string = basic_format_string<wchar_t, std::type_identity_t<Args>...>;

template <typename FormatContext>
class basic_format_parse_context;

using format_parse_context = basic_format_parse_context<format_context>;
using wformat_parse_context = basic_format_parse_context<wformat_context>;

template <typename Context, typename CharT = typename Context::char_type>
class basic_dynamic_format_args;

using dynamic_format_args = basic_dynamic_format_args<format_context, char>;
using wdynamic_format_args = basic_dynamic_format_args<wformat_context, wchar_t>;

template <typename Context, typename CharT = typename Context::char_type>
class basic_format_args_ref;

using format_args_ref = basic_format_args_ref<format_context, char>;
using wformat_args_ref = basic_format_args_ref<wformat_context, wchar_t>;

template <typename CharT>
class basic_variable;

using variable = basic_variable<char>;
using wvariable = basic_variable<wchar_t>;

template <typename CharT, bool Debug = false>
class basic_interpreter_base;

template <typename FormatContext, bool Debug = false>
class basic_interpreter;

using interpreter = basic_interpreter<format_context>;

class locale_ref;

template <typename OutputIt, typename CharT = char>
using format_args_ref_for = basic_format_args_ref<
    basic_format_context<std::type_identity_t<OutputIt>, CharT>>;

template <typename OutputIt>
struct format_to_n_result;
} // namespace papilio

#include "detail/suffix.hpp"

#endif
