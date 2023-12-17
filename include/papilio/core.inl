#include "core.hpp"

namespace papilio
{
template <typename Context>
template <typename T>
inline bool basic_format_arg<Context>::handle_impl<T>::is_formattable() const noexcept
{
    return formattable<value_type, char_type>;
}

template <typename Context>
template <typename T>
void basic_format_arg<Context>::handle_impl_ptr<T>::format(parse_context& parse_ctx, Context& out_ctx) const
{
    if constexpr(formattable<value_type, char_type>)
    {
        using formatter_t = PAPILIO_NS formatter<value_type, char_type>;
        formatter_t fmt;
        parse_ctx.advance_to(fmt.parse(parse_ctx));

        using context_t = format_context_traits<Context>;
        context_t::advance_to(out_ctx, fmt.format(*m_ptr, out_ctx));
    }
    else
    {
        throw_unformattable();
    }
}

template <typename Context>
template <typename T>
void basic_format_arg<Context>::handle_impl_soo<T>::format(parse_context& parse_ctx, Context& out_ctx) const
{
    if constexpr(formattable<value_type, char_type>)
    {
        using formatter_t = PAPILIO_NS formatter<value_type, char_type>;
        formatter_t fmt;
        parse_ctx.advance_to(fmt.parse(parse_ctx));

        using context_t = format_context_traits<Context>;
        context_t::advance_to(out_ctx, fmt.format(m_val, out_ctx));
    }
    else
    {
        throw_unformattable();
    }
}

template <typename Context>
inline bool basic_format_arg<Context>::is_formattable() const noexcept
{
    return visit(
        []<typename T>(const T& v) -> bool
        {
            if constexpr(std::is_same_v<T, handle>)
            {
                return v.is_formattable();
            }
            else
            {
                return formattable<T, char_type>;
            }
        }
    );
}

template <typename Context>
void basic_format_arg<Context>::format(parse_context& parse_ctx, Context& out_ctx) const
{
    visit(
        [&]<typename T>(const T& v)
        {
            if constexpr(std::is_same_v<T, handle>)
            {
                v.format(parse_ctx, out_ctx);
            }
            else if constexpr(formattable<T>)
            {
                using formatter_t = PAPILIO_NS formatter<T, char_type>;
                formatter_t fmt;
                parse_ctx.advance_to(fmt.parse(parse_ctx));

                using context_t = format_context_traits<Context>;
                context_t::advance_to(out_ctx, fmt.format(v, out_ctx));
            }
            else
            {
                throw_unformattable();
            }
        }
    );
}
} // namespace papilio
