#pragma once

#include <cstdio> // FILE*
#include <iosfwd>
#include "macros.hpp"
#include "format.hpp"

namespace papilio
{
void vprint(std::FILE* file, std::string_view fmt, const dynamic_format_args& args);
void vprintln(std::FILE* file, std::string_view fmt, const dynamic_format_args& args);
void vprint_conv(std::FILE* file, std::string_view fmt, const dynamic_format_args& args);
void vprintln_conv(std::FILE* file, std::string_view fmt, const dynamic_format_args& args);
void vprint_conv(std::string_view fmt, const dynamic_format_args& args);
void vprintln_conv(std::string_view fmt, const dynamic_format_args& args);

template <typename... Args>
void print(std::FILE* file, std::string_view fmt, Args&&... args)
{
    PAPILIO_NS vprint(file, fmt, PAPILIO_NS make_format_args(std::forward<Args>(args)...));
}

template <typename... Args>
void print(std::string_view fmt, Args&&... args)
{
    PAPILIO_NS vprint_conv(fmt, PAPILIO_NS make_format_args(std::forward<Args>(args)...));
}

template <typename... Args>
void println(std::FILE* file, std::string_view fmt, Args&&... args)
{
    PAPILIO_NS vprintln(file, fmt, PAPILIO_NS make_format_args(std::forward<Args>(args)...));
}

template <typename... Args>
void println(std::string_view fmt, Args&&... args)
{
    PAPILIO_NS vprintln_conv(fmt, PAPILIO_NS make_format_args(std::forward<Args>(args)...));
}

void println(std::FILE* file);
void println();

void vprint(std::ostream& os, std::string_view fmt, const dynamic_format_args& args);

template <typename... Args>
void print(std::ostream& os, std::string_view fmt, Args&&... args)
{
    PAPILIO_NS vprint(os, fmt, PAPILIO_NS make_format_args(std::forward<Args>(args)...));
}

void println(std::ostream& os);
} // namespace papilio
