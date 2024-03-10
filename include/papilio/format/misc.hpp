#ifndef PAPILIO_FORMAT_MISC_HPP
#define PAPILIO_FORMAT_MISC_HPP

#pragma once

#include <ranges>
#include "../core.hpp"

namespace papilio
{
PAPILIO_EXPORT template <typename R, typename CharT>
class formatter<joiner<R, CharT>, CharT>
{
public:
    formatter() = default;
    formatter(const formatter&) = default;

    formatter& operator=(const formatter&) = default;

    using joiner_t = joiner<R, CharT>;

    template <typename ParseContext, typename FormatContext>
    auto format(const joiner_t& j, ParseContext& parse_ctx, FormatContext& fmt_ctx) const
    {
        using value_t = std::ranges::range_value_t<typename joiner_t::range_type>;
        using formatter_t = typename FormatContext::template formatter_type<value_t>;

        if constexpr(formatter_traits<formatter_t>::template parsable<FormatContext>())
        {
            formatter_t fmt;
            parse_ctx.advance_to(fmt.parse(parse_ctx));

            bool first = true;
            for(auto&& i : j)
            {
                if(!first)
                    append_sep(fmt_ctx, j);
                first = false;

                fmt_ctx.advance_to(
                    fmt.format(i, fmt_ctx)
                );
            }
        }
        else
        {
            bool first = true;
            for(auto&& i : j)
            {
                if(!first)
                    append_sep(fmt_ctx, j);
                first = false;

                fmt_ctx.advance_to(
                    PAPILIO_NS format_to(fmt_ctx.out(), "{}", i)
                );
            }
        }

        return fmt_ctx.out();
    }

private:
    template <typename FormatContext>
    static void append_sep(FormatContext& fmt_ctx, const joiner_t& j)
    {
        using context_t = format_context_traits<FormatContext>;
        context_t::append(fmt_ctx, j.separator());
    }
};
} // namespace papilio

#endif
