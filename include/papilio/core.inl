#include "core.hpp"

namespace papilio
{
template <typename Context>
template <typename T>
inline bool basic_format_arg<Context>::handle_impl<T>::is_formattable() const noexcept
{
    return formattable_with<value_type, Context>;
}

template <typename Context>
template <typename T>
void basic_format_arg<Context>::handle_impl_ptr<T>::format(parse_context& parse_ctx, Context& out_ctx) const
{
    if constexpr(formattable_with<value_type, Context>)
    {
        using formatter_t = typename Context::template formatter_type<value_type>;
        formatter_traits<formatter_t>::format(
            *m_ptr, parse_ctx, out_ctx
        );
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
    if constexpr(formattable_with<value_type, Context>)
    {
        using formatter_t = typename Context::template formatter_type<value_type>;
        formatter_traits<formatter_t>::format(
            m_val, parse_ctx, out_ctx
        );
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
            else if constexpr(formattable_with<T, Context>)
            {
                using formatter_t = typename Context::template formatter_type<T>;
                formatter_traits<formatter_t>::format(
                    v, parse_ctx, out_ctx
                );
            }
            else
            {
                throw_unformattable();
            }
        }
    );
}
} // namespace papilio
