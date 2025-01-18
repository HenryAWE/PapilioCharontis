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
        simple_formatter_parser<ParseContext> parser{};
        auto [data, it] = parser.parse(ctx);
        m_data = data;
        if(it == ctx.end())
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
        string_formatter<CharT> fmt;
        fmt.set_data(m_data);
        return fmt.format(to_str(tp, ctx), ctx);
    }

private:
    simple_formatter_data m_data;
    string_view_type m_sep = my_base::default_sep();
    string_view_type m_opening = my_base::default_opening();
    string_view_type m_closing = my_base::default_closing();

    void clear_brackets() noexcept
    {
        set_brackets(string_view_type(), string_view_type());
    }

    template <typename FormatContext>
    std::basic_string<CharT> to_str(const Tuple& tp, FormatContext& ctx) const
    {
        std::basic_string<CharT> result;
        using iter_t = decltype(std::back_inserter(result));

        auto result_ctx = format_context_traits<FormatContext>::template rebind_context<iter_t>(
            ctx, std::back_inserter(result)
        );
        using context_t = format_context_traits<decltype(result_ctx)>;

        context_t::append(result_ctx, m_opening);

        PAPILIO_NS tuple_for_each(
            tp,
            [this, &result_ctx, first = true]<typename T>(const T& v) mutable
            {
                if(!first)
                    context_t::append(result_ctx, m_sep);
                first = false;

                context_t::append_by_formatter(result_ctx, v, true);
            }
        );

        context_t::append(result_ctx, m_closing);

        return result;
    }
};
} // namespace papilio

#include "../detail/suffix.hpp"

#endif
