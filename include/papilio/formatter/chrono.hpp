#ifndef PAPILIO_FORMATTER_CHRONO_HPP
#define PAPILIO_FORMATTER_CHRONO_HPP

#pragma once

#include <ctime>
#include <chrono>
#include <iomanip>
#include "../format.hpp"
#include "../detail/prefix.hpp"

namespace papilio
{
template <typename CharT>
class formatter<std::tm, CharT>
{
public:
    template <typename ParseContext>
    auto parse(ParseContext& ctx)
        -> typename ParseContext::iterator
    {
        simple_formatter_parser<ParseContext> parser;
        auto [result, it] = parser.parse(ctx);

        m_data = result;
        auto end = std::find_if(
            it,
            ctx.end(),
            [](char32_t v)
            { return v == U'}'; }
        );

        m_fmt.assign(it, end);

        return end;
    }

    template <typename Context>
    auto format(const std::tm& val, Context& ctx) const
        -> typename Context::iterator
    {
        std::basic_stringstream<CharT> ss;
        ss.imbue(ctx.getloc());
        // Use "%c\n" to simulate asctime() if the time format string is empty
        const CharT* tm_fmt = m_fmt.empty() ?
                                  PAPILIO_TSTRING(CharT, "%c\n") :
                                  m_fmt.c_str();
        ss << std::put_time(&val, tm_fmt);

        string_formatter<CharT> fmt;
        fmt.set_data(m_data);
        return fmt.format(std::move(ss).str(), ctx);
    }

private:
    simple_formatter_data m_data;
    utf::basic_string_container<CharT> m_fmt;
};
} // namespace papilio

#include "../detail/suffix.hpp"

#endif
