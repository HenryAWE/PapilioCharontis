/**
 * @file print.hpp
 * @author HenryAWE
 * @brief Printing support.
 */

#ifndef PAPILIO_PRINT_HPP
#define PAPILIO_PRINT_HPP

#pragma once

#include <iostream>
#include <iterator>
#include "os/os.hpp"
#include "format.hpp"
#include "color.hpp"
#include "detail/prefix.hpp"

namespace papilio
{
/// @defgroup Print Printing support
/// @brief Print formatted string to file, terminal, or stream.
/// @sa Format
/// @{

namespace detail
{
    void vprint_impl(
        std::FILE* file,
        std::string_view fmt,
        format_args_ref args,
        bool conv_unicode,
        bool newline,
        text_style st = text_style()
    );
} // namespace detail

/// @defgroup PrintFile Print to file
/// @{

PAPILIO_EXPORT void println(std::FILE* file);

PAPILIO_EXPORT template <typename... Args>
void print(std::FILE* file, format_string<Args...> fmt, Args&&... args)
{
    detail::vprint_impl(
        file,
        fmt.get(),
        PAPILIO_NS make_format_args(std::forward<Args>(args)...),
        os::is_terminal(file),
        false
    );
}

PAPILIO_EXPORT template <typename... Args>
void println(std::FILE* file, format_string<Args...> fmt, Args&&... args)
{
    detail::vprint_impl(
        file,
        fmt.get(),
        PAPILIO_NS make_format_args(std::forward<Args>(args)...),
        os::is_terminal(file),
        true
    );
}

/// @}

/// @defgroup PrintTerminal Print to terminal
/// @brief Print to `stdout` with necessary encoding conversion.
/// @sa os::is_terminal
/// @sa os::output_conv
/// @{

PAPILIO_EXPORT void println();

PAPILIO_EXPORT template <typename... Args>
void print(format_string<Args...> fmt, Args&&... args)
{
    PAPILIO_NS print(stdout, fmt, std::forward<Args>(args)...);
}

PAPILIO_EXPORT template <typename... Args>
void print(text_style st, format_string<Args...> fmt, Args&&... args)
{
    detail::vprint_impl(
        stdout,
        fmt.get(),
        PAPILIO_NS make_format_args(std::forward<Args>(args)...),
        os::is_terminal(stdout),
        false,
        st
    );
}

PAPILIO_EXPORT template <typename... Args>
void println(format_string<Args...> fmt, Args&&... args)
{
    PAPILIO_NS println(stdout, fmt, std::forward<Args>(args)...);
}

PAPILIO_EXPORT template <typename... Args>
void println(text_style st, format_string<Args...> fmt, Args&&... args)
{
    detail::vprint_impl(
        stdout,
        fmt.get(),
        PAPILIO_NS make_format_args(std::forward<Args>(args)...),
        os::is_terminal(stdout),
        true,
        st
    );
}

/// @}

/// @defgroup PrintStream Print to output stream
/// @{

PAPILIO_EXPORT template <typename... Args>
void print(std::ostream& os, format_string<Args...> fmt, Args&&... args)
{
    using iter_t = std::ostream_iterator<char>;
    using context_type = basic_format_context<iter_t>;
    PAPILIO_NS vformat_to(
        iter_t(os),
        os.getloc(),
        fmt.get(),
        PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)
    );
}

PAPILIO_EXPORT void println(std::ostream& os);

PAPILIO_EXPORT template <typename... Args>
void println(std::ostream& os, format_string<Args...> fmt, Args&&... args)
{
    PAPILIO_NS print(os, fmt.get(), std::forward<Args>(args)...);
    PAPILIO_NS println(os);
}

/// @}

/// @}
} // namespace papilio

#include "detail/suffix.hpp"

#endif
