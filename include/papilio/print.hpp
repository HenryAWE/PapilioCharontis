#pragma once

#include <cstdio> // FILE*
#include "format.hpp"


namespace papilio
{
    void vprint(std::FILE* file, std::string_view fmt, const dynamic_format_arg_store& store);
    void vprintln(std::FILE* file, std::string_view fmt, const dynamic_format_arg_store& store);

    template <typename... Args>
    void print(std::FILE* file, std::string_view fmt, Args&&... args)
    {
        papilio::vprint(file, fmt, papilio::make_format_args(std::forward<Args>(args)...));
    }
    template <typename... Args>
    void print(std::string_view fmt, Args&&... args)
    {
        papilio::print(stdout, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void println(std::FILE* file, std::string_view fmt, Args&&... args)
    {
        papilio::vprintln(file, fmt, papilio::make_format_args(std::forward<Args>(args)...));
    }
    template <typename... Args>
    void println(std::string_view fmt, Args&&... args)
    {
        papilio::println(stdout, fmt, std::forward<Args...>(args)...);
    }

    void println(std::FILE* file);
    void println();
}
