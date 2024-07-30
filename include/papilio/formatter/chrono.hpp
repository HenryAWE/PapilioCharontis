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
PAPILIO_EXPORT template <typename CharT>
struct chrono_formatter_data
{
    using string_container_type = utf::basic_string_container<CharT>;

    simple_formatter_data basic;
    string_container_type chrono_spec;
};

template <typename ParseContext>
PAPILIO_EXPORT class chrono_formatter_parser
{
public:
    using char_type = typename ParseContext::char_type;
    using iterator = typename ParseContext::iterator;

    using result_type = chrono_formatter_data<char_type>;
    using interpreter_type = basic_interpreter<typename ParseContext::format_context_type>;

    static std::pair<result_type, iterator> parse(ParseContext& ctx)
    {
        result_type result;

        simple_formatter_parser<ParseContext, true> basic_parser;
        auto [basic_result, it] = basic_parser.parse(ctx);
        result.basic = basic_result;

        auto spec_end = std::find_if(
            it,
            ctx.end(),
            [](char32_t v)
            { return v == U'}'; }
        );
        result.chrono_spec.assign(it, spec_end);

        return std::make_pair(std::move(result), spec_end);
    }
};

template <typename CharT>
class formatter<std::tm, CharT>
{
public:
    template <typename ParseContext>
    auto parse(ParseContext& ctx)
        -> typename ParseContext::iterator
    {
        chrono_formatter_parser<ParseContext> parser;
        auto [result, it] = parser.parse(ctx);

        m_data = result;

        return it;
    }

    template <typename Context>
    auto format(const std::tm& val, Context& ctx) const
        -> typename Context::iterator
    {
        if(m_data.chrono_spec.empty())
        {
            CharT buf[24];
            default_tm_fmt_impl(val, buf);

            string_formatter<CharT> fmt;
            fmt.set_data(m_data.basic);
            return fmt.format(std::basic_string_view<CharT>(buf, 24), ctx);
        }
        else
        {
            std::basic_stringstream<CharT> ss;
            if(m_data.basic.use_locale)
                ss.imbue(ctx.getloc());
            else
                ss.imbue(std::locale::classic());
            ss << std::put_time(&val, m_data.chrono_spec.c_str());

            string_formatter<CharT> fmt;
            fmt.set_data(m_data.basic);
            return fmt.format(std::move(ss).str(), ctx);
        }
    }

private:
    chrono_formatter_data<CharT> m_data;

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
