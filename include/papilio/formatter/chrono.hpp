#ifndef PAPILIO_FORMATTER_CHRONO_HPP
#define PAPILIO_FORMATTER_CHRONO_HPP

#pragma once

#include <ctime>
#include <span>
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
        if(m_fmt.empty())
        {
            CharT buf[24];
            default_tm_fmt_impl(val, buf);

            string_formatter<CharT> fmt;
            fmt.set_data(m_data);
            return fmt.format(std::basic_string_view<CharT>(buf, 24), ctx);
        }
        else
        {
            std::basic_stringstream<CharT> ss;
            ss.imbue(ctx.getloc());
            ss << std::put_time(&val, m_fmt.c_str());

            string_formatter<CharT> fmt;
            fmt.set_data(m_data);
            return fmt.format(std::move(ss).str(), ctx);
        }
    }

private:
    simple_formatter_data m_data;
    utf::basic_string_container<CharT> m_fmt;

    // Simulating `asctime()` but without the trailing newline
    static void default_tm_fmt_impl(const std::tm& val, std::span<CharT, 24> buf)
    {
        constexpr CharT weekdays[7][3] = {
            {'S', 'u', 'n'},
            {'M', 'o', 'n'},
            {'T', 'u', 'e'},
            {'W', 'e', 'd'},
            {'T', 'h', 'u'},
            {'F', 'r', 'i'},
            {'S', 'a', 't'}
        };
        constexpr CharT months[12][3] = {
            {'J', 'a', 'n'},
            {'F', 'e', 'b'},
            {'M', 'a', 'r'},
            {'A', 'p', 'r'},
            {'M', 'a', 'y'},
            {'J', 'u', 'n'},
            {'J', 'u', 'l'},
            {'A', 'u', 'g'},
            {'S', 'e', 'p'},
            {'O', 'c', 't'},
            {'N', 'o', 'v'},
            {'D', 'e', 'c'}
        };

        PAPILIO_NS format_to_n(
            buf.data(),
            buf.size(),
            PAPILIO_TSTRING_VIEW(CharT, "{} {} {:2d} {:02d}:{:02d}:{:02d} {:4d}"),
            std::basic_string_view<CharT>(weekdays[val.tm_wday], 3),
            std::basic_string_view<CharT>(months[val.tm_mon], 3),
            val.tm_mday,
            val.tm_hour,
            val.tm_min,
            val.tm_sec,
            val.tm_year + 1900
        );
    }
};
} // namespace papilio

#include "../detail/suffix.hpp"

#endif
