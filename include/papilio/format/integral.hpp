#pragma once

#include <concepts>
#include "../core.hpp"


namespace papilio
{
    template <std::integral T, typename CharT>
    class formatter<T, CharT>
    {
    public:
        format_parse_context::iterator parse(format_parse_context& ctx)
        {
            // TODO

            return ctx.begin();
        }

        template <typename FormatContext>
        FormatContext::iterator format(T val, FormatContext& ctx) const
        {
            CharT buf[sizeof(T) * 8];
            std::size_t buf_size = 0;

            static constexpr CharT digits[16] = {
                '0', '1', '2', '3', '4',
                '5', '6', '7', '8', '9',
                'a', 'b', 'c', 'd', 'e', 'f'
            };

            do
            {
                const T digit = val % m_base;
                buf[buf_size++] = digits[digit];
                val /= m_base;
            } while(val);

            using context_t = format_context_traits<FormatContext>;

            for(std::size_t i = buf_size; i > 0; --i)
                context_t::append(ctx, buf[i - 1]);

            return context_t::out(ctx);
        }

    private:
        int m_base = 10;
    };
}
