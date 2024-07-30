#ifndef PAPILIO_FORMATTER_TUPLE_HPP
#define PAPILIO_FORMATTER_TUPLE_HPP

#pragma once

#include <tuple>
#include "../core.hpp"
#include "../format.hpp"
#include "../detail/prefix.hpp"

namespace papilio
{
namespace detail
{
    template <typename CharT>
    class tuple_fmt_base
    {
    public:
        using string_view_type = std::basic_string_view<CharT>;

    protected:
        // ", "
        static string_view_type default_sep() noexcept
        {
            static constexpr CharT str[2] = {',', ' '};
            return string_view_type(str, 2);
        }

        // "("
        static string_view_type default_opening() noexcept
        {
            static constexpr CharT ch = '(';
            return string_view_type(&ch, 1);
        }

        // ")"
        static string_view_type default_closing() noexcept
        {
            static constexpr CharT ch = ')';
            return string_view_type(&ch, 1);
        }

        // ": "
        static string_view_type pair_sep() noexcept
        {
            static constexpr CharT str[2] = {':', ' '};
            return string_view_type(str, 2);
        }
    };
} // namespace detail

PAPILIO_EXPORT template <tuple_like Tuple, typename CharT>
class formatter<Tuple, CharT> : public detail::tuple_fmt_base<CharT>
{
    using my_base = detail::tuple_fmt_base<CharT>;

public:
    using string_view_type = std::basic_string_view<CharT>;

    void set_separator(string_view_type sep) noexcept
    {
        m_sep = sep;
    }

    void set_brackets(string_view_type opening, string_view_type closing) noexcept
    {
        m_opening = opening;
        m_closing = closing;
    }

    template <typename ParseContext>
    auto parse(ParseContext& ctx)
        -> typename ParseContext::iterator
    {
        auto it = ctx.begin();
        if(it == ctx.end()) [[unlikely]]
            return it;

        char32_t type = *it;
        if constexpr(std::tuple_size_v<Tuple> == 2)
        { // Pair-like only
            if(type == U'm')
            {
                set_separator(my_base::pair_sep());
                clear_brackets();
                ++it;
            }
        }
        if(type == U'n')
        {
            clear_brackets();
            ++it;
        }

        return it;
    }

    template <typename FormatContext>
    auto format(const Tuple& tp, FormatContext& ctx) const
        -> typename FormatContext::iterator
    {
        using context_t = format_context_traits<FormatContext>;

        context_t::append(ctx, m_opening);

        bool first = true;
        PAPILIO_NS tuple_for_each(
            tp,
            [&](const auto& v)
            {
                if(!first)
                    context_t::append(ctx, m_sep);
                first = false;

                context_t::format_to(
                    ctx,
                    PAPILIO_TSTRING_VIEW(CharT, "{}"),
                    v
                );
            }
        );

        context_t::append(ctx, m_closing);
        return ctx.out();
    }

private:
    string_view_type m_sep = my_base::default_sep();
    string_view_type m_opening = my_base::default_opening();
    string_view_type m_closing = my_base::default_closing();

    void clear_brackets() noexcept
    {
        set_brackets(string_view_type(), string_view_type());
    }
};
} // namespace papilio

#include "../detail/suffix.hpp"

#endif
