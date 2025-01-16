/**
 * @file core.inl
 * @author HenryAWE
 * @brief Implementation of the core module.
 */

#ifndef PAPILIO_CORE_INL
#define PAPILIO_CORE_INL

#include "core.hpp"
#include <sstream>

#pragma once

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
void basic_format_arg<Context>::handle_impl<T>::skip_spec(parse_context& parse_ctx) const
{
    if constexpr(formattable_with<value_type, Context>)
    {
        using formatter_t = typename Context::template formatter_type<value_type>;
        formatter_traits<formatter_t>::skip_spec(parse_ctx);
    }
    else
    {
        throw_unformattable();
    }
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
            if constexpr(std::same_as<T, handle>)
            {
                return v.is_formattable();
            }
            else
            {
                return formattable_with<T, Context>;
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
            if constexpr(std::same_as<T, handle>)
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

template <typename Context>
void basic_format_arg<Context>::skip_spec(parse_context& parse_ctx)
{
    visit(
        [&]<typename T>(const T& v)
        {
            if constexpr(std::same_as<T, handle>)
            {
                v.skip_spec(parse_ctx);
            }
            else if constexpr(formattable_with<T, Context>)
            {
                using formatter_t = typename Context::template formatter_type<T>;
                formatter_traits<formatter_t>::skip_spec(parse_ctx);
            }
            else
            {
                throw_unformattable();
            }
        }
    );
}

template <typename T, typename CharT>
requires streamable<T, CharT>
template <typename ParseContext>
auto streamable_formatter<T, CharT>::parse(ParseContext& ctx)
{
    simple_formatter_parser<ParseContext, true> parser;
    auto [result, it] = parser.parse(ctx);

    m_data = result;

    return it;
}

template <typename T, typename CharT>
requires streamable<T, CharT>
template <typename Context>
auto streamable_formatter<T, CharT>::format(const T& val, Context& ctx) const
{
    using os_t = basic_oiterstream<
        CharT,
        typename Context::iterator>;

    const bool use_iter_stream =
        m_data.align == format_align::default_align &&
        m_data.width == 0 &&
        m_data.has_fill();

    if(use_iter_stream)
    {
        os_t os(ctx.out());

        setup_locale(os, ctx);

        os << val;

        return os.base();
    }
    else
    {
        std::basic_stringstream<CharT> ss;

        setup_locale(ss, ctx);

        ss << val;

        string_formatter<CharT> fmt;
        fmt.set_data(m_data);

        return fmt.format(std::move(ss).str(), ctx);
    }
}

template <typename Context>
void format_context_traits<Context>::vformat_to(
    context_type& ctx,
    string_view_type fmt,
    const format_args_ref_type& args
)
{
    advance_to(
        ctx,
        PAPILIO_NS detail::vformat_to_impl<char_type, iterator, context_type>(
            ctx.out(),
            ctx.getloc_ref(),
            fmt,
            args
        )
    );
}
} // namespace papilio

#endif
