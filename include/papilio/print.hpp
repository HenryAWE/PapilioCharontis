#pragma once

#include <cstdio> // FILE*
#include <iosfwd>
#include "format.hpp"


namespace papilio
{
    void vprint(std::FILE* file, std::string_view fmt, const dynamic_format_arg_store& store);
    void vprintln(std::FILE* file, std::string_view fmt, const dynamic_format_arg_store& store);
    void vprint_conv(std::FILE* file, std::string_view fmt, const dynamic_format_arg_store& store);
    void vprintln_conv(std::FILE* file, std::string_view fmt, const dynamic_format_arg_store& store);
    void vprint_conv(std::string_view fmt, const dynamic_format_arg_store& store);
    void vprintln_conv(std::string_view fmt, const dynamic_format_arg_store& store);

    template <typename... Args>
    void print(std::FILE* file, std::string_view fmt, Args&&... args)
    {
        papilio::vprint(file, fmt, papilio::make_format_args(std::forward<Args>(args)...));
    }
    template <typename... Args>
    void print(std::string_view fmt, Args&&... args)
    {
        papilio::vprint_conv(fmt, papilio::make_format_args(std::forward<Args>(args)...));
    }
    template <typename... Args>
    void println(std::FILE* file, std::string_view fmt, Args&&... args)
    {
        papilio::vprintln(file, fmt, papilio::make_format_args(std::forward<Args>(args)...));
    }
    template <typename... Args>
    void println(std::string_view fmt, Args&&... args)
    {
        papilio::vprintln_conv(fmt, papilio::make_format_args(std::forward<Args>(args)...));
    }

    void println(std::FILE* file);
    void println();

    void vprint(std::ostream& os, std::string_view fmt, const dynamic_format_arg_store& store);

    template <typename... Args>
    void print(std::ostream& os, std::string_view fmt, Args&&... args)
    {
        vprint(os, fmt, papilio::make_format_args(std::forward<Args>(args)...));
    }
}
