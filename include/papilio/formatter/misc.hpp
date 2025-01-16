#ifndef PAPILIO_FORMATTER_MISC_HPP
#define PAPILIO_FORMATTER_MISC_HPP

#pragma once

#include "../detail/config.hpp"
#include <thread>
#include <sstream>
#ifdef PAPILIO_HAS_LIB_STACKTRACE
#    include <stacktrace>
#endif
#include "../core.hpp"
#include "../format.hpp"
#include "../detail/prefix.hpp"

namespace papilio
{
PAPILIO_EXPORT template <typename CharT>
class formatter<std::thread::id, CharT>
{
public:
    template <typename ParseContext>
    auto parse(ParseContext& ctx)
        -> typename ParseContext::iterator
    {
        simple_formatter_parser<ParseContext> parser;
        auto [data, it] = parser.parse(ctx);
        m_data = data;

        return it;
    }

    template <typename FormatContext>
    auto format(const std::thread::id& id, FormatContext& ctx) const
    {
        std::basic_stringstream<CharT> ss;
        ss << id;

        string_formatter<CharT> fmt;
        fmt.set_data(m_data.to_std_data());
        return fmt.format(std::move(ss).str(), ctx);
    }

private:
    simple_formatter_data m_data;
};

#ifdef PAPILIO_HAS_LIB_STACKTRACE

PAPILIO_EXPORT template <typename Alloc, typename CharT>
class formatter<std::basic_stacktrace<Alloc>, CharT>
{
public:
    using value_type = std::basic_stacktrace<Alloc>;

    template <typename ParseContext>
    auto parse(ParseContext& ctx)
        -> typename ParseContext::iterator
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const value_type& val, FormatContext& ctx) const
        -> typename FormatContext::iterator
    {
        using context_t = format_context_traits<FormatContext>;

        std::string info = std::to_string(val);

        if constexpr(char8_like<CharT>)
        {
            std::basic_string_view<CharT> sv{
                std::bit_cast<const CharT*>(info.data()),
                info.size()
            };
            context_t::append(ctx, sv);
        }
        else
        {
            context_t::append(
                ctx,
                utf::string_ref(info).to_string<CharT>()
            );
        }

        return ctx.out();
    }
};

PAPILIO_EXPORT template <typename CharT>
class formatter<std::stacktrace_entry, CharT>
{
public:
    template <typename ParseContext>
    auto parse(ParseContext& ctx)
        -> typename ParseContext::iterator
    {
        simple_formatter_parser<ParseContext> parser;
        auto [data, it] = parser.parse(ctx);
        m_data = data;

        return it;
    }

    template <typename FormatContext>
    auto format(const std::stacktrace_entry val, FormatContext& ctx) const
        -> typename FormatContext::iterator
    {
        auto helper = [&val]()
        {
            if constexpr(std::same_as<CharT, char>)
                return std::to_string(val);
            else
                return utf::string_ref(std::to_string(val)).to_string<CharT>();
        };

        string_formatter<CharT> fmt;
        fmt.set_data(m_data.to_std_data());
        return fmt.format(helper(), ctx);
    }

private:
    simple_formatter_data m_data;
};

#endif
} // namespace papilio

#include "../detail/suffix.hpp"

#endif
