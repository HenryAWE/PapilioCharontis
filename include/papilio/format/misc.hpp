#pragma once

#include <ranges>
#include "../core.hpp"

namespace papilio
{
template <typename R, typename CharT>
class formatter<joiner<R, CharT>>
{
public:
    using joiner_t = joiner<R, CharT>;

    template <typename ParseContext>
    auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const joiner_t& j, FormatContext& ctx) const
    {
        using context_t = format_context_traits<FormatContext>;

        bool first = true;

        for(auto&& i : j)
        {
            if(!first)
            {
                context_t::append(ctx, j.separator());
            }
            first = false;

            ctx.advance_to(
                PAPILIO_NS format_to(ctx.out(), "{}", i)
            );
        }

        return ctx.out();
    }
};
} // namespace papilio
