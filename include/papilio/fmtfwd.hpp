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
    PAPILIO_EXPORT template <typename CharT>
    class decoder;
    PAPILIO_EXPORT class codepoint;

    PAPILIO_EXPORT template <typename CharT>
    class basic_string_ref;
    PAPILIO_EXPORT template <typename CharT>
    class basic_string_container;
} // namespace utf

PAPILIO_EXPORT template <typename CharT>
class basic_indexing_value;

PAPILIO_EXPORT using indexing_value = basic_indexing_value<char>;
PAPILIO_EXPORT using windexing_value = basic_indexing_value<wchar_t>;

PAPILIO_EXPORT template <typename CharT>
class basic_attribute_name;

PAPILIO_EXPORT using attribute_name = basic_attribute_name<char>;
PAPILIO_EXPORT using wattribute_name = basic_attribute_name<wchar_t>;

PAPILIO_EXPORT class invalid_attribute_base;
PAPILIO_EXPORT template <typename CharT>
class basic_invalid_attribute;

PAPILIO_EXPORT using invalid_attribute = basic_invalid_attribute<char>;

PAPILIO_EXPORT template <typename Context>
class basic_format_arg;

PAPILIO_EXPORT template <typename FormatContext>
class basic_format_parse_context;

PAPILIO_EXPORT template <typename OutputIt, typename CharT = char>
class basic_format_context;

PAPILIO_EXPORT template <typename T, typename CharT = char>
class formatter;

PAPILIO_EXPORT template <typename CharT>
using format_iterator_for = std::back_insert_iterator<std::basic_string<CharT>>;

PAPILIO_EXPORT using format_context = basic_format_context<format_iterator_for<char>, char>;
PAPILIO_EXPORT using wformat_context = basic_format_context<format_iterator_for<wchar_t>, wchar_t>;
PAPILIO_EXPORT using format_arg = basic_format_arg<format_context>;
PAPILIO_EXPORT using wformat_arg = basic_format_arg<wformat_context>;

PAPILIO_EXPORT template <typename CharT, typename... Args>
class basic_format_string;

PAPILIO_EXPORT template <typename... Args>
using format_string = basic_format_string<char, std::type_identity_t<Args>...>;
PAPILIO_EXPORT template <typename... Args>
using wformat_string = basic_format_string<wchar_t, std::type_identity_t<Args>...>;

PAPILIO_EXPORT template <typename FormatContext>
class basic_format_parse_context;

PAPILIO_EXPORT using format_parse_context = basic_format_parse_context<format_context>;
PAPILIO_EXPORT using wformat_parse_context = basic_format_parse_context<wformat_context>;

PAPILIO_EXPORT template <typename Context, typename CharT = typename Context::char_type>
class basic_dynamic_format_args;

PAPILIO_EXPORT using dynamic_format_args = basic_dynamic_format_args<format_context, char>;
PAPILIO_EXPORT using wdynamic_format_args = basic_dynamic_format_args<wformat_context, wchar_t>;

PAPILIO_EXPORT template <typename Context, typename CharT = typename Context::char_type>
class basic_format_args_ref;

PAPILIO_EXPORT using format_args_ref = basic_format_args_ref<format_context, char>;
PAPILIO_EXPORT using wformat_args_ref = basic_format_args_ref<wformat_context, wchar_t>;

PAPILIO_EXPORT template <typename CharT>
class basic_variable;

PAPILIO_EXPORT using variable = basic_variable<char>;
PAPILIO_EXPORT using wvariable = basic_variable<wchar_t>;

PAPILIO_EXPORT template <typename CharT, bool Debug = false>
class basic_interpreter_base;

PAPILIO_EXPORT template <typename FormatContext, bool Debug = false>
class basic_interpreter;

PAPILIO_EXPORT using interpreter = basic_interpreter<format_context>;

PAPILIO_EXPORT class locale_ref;

PAPILIO_EXPORT template <typename OutputIt, typename CharT = char>
using format_args_ref_for = basic_format_args_ref<
    basic_format_context<std::type_identity_t<OutputIt>, CharT>>;

PAPILIO_EXPORT template <typename OutputIt>
struct format_to_n_result;
} // namespace papilio

#include "detail/suffix.hpp"

#endif
