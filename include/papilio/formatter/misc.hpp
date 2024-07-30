#ifndef PAPILIO_FORMATTER_MISC_HPP
#define PAPILIO_FORMATTER_MISC_HPP

#pragma once

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
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const std::thread::id& id, FormatContext& ctx) const
    {
        std::basic_stringstream<CharT> ss;
        ss << id;

        using context_t = format_context_traits<FormatContext>;
        context_t::append(ctx, std::move(ss).str());

        return ctx.out();
    }
};

#ifdef PAPILIO_HAS_LIB_STACKTRACE

namespace detail
{
    template <typename CharT, typename FormatContext>
    void stack_info_append(FormatContext& ctx, std::string_view info)
    {
        using context_t = format_context_traits<FormatContext>;

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
                utf::string_ref(info).to_string_as<CharT>()
            );
        }
    }
} // namespace detail

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
        detail::stack_info_append<CharT>(
            ctx,
            std::to_string(val)
        );

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
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const std::stacktrace_entry val, FormatContext& ctx) const
        -> typename FormatContext::iterator
    {
        detail::stack_info_append<CharT>(
            ctx,
            std::to_string(val)
        );

        return ctx.out();
    }
};

#endif
} // namespace papilio

#include "../detail/suffix.hpp"

#endif
