#pragma once

#include <tuple>
#include <ranges>
#include "../core.hpp"


namespace papilio
{
    template <std::ranges::range Range>
    class joiner
    {
    public:
        using char_type = char;
        using string_view_type = std::basic_string_view<char_type>;

        joiner() = delete;
        constexpr joiner(const joiner&) noexcept = default;
        constexpr joiner(const Range& rng, string_view_type sep) noexcept
            : m_rng(rng), m_sep(sep) {}

        [[nodiscard]]
        const Range& get_range() const noexcept
        {
            return m_rng;
        }
        [[nodiscard]]
        constexpr string_view_type get_separator() const noexcept
        {
            return m_sep;
        }

    private:
        const Range& m_rng;
        string_view_type m_sep;
    };
    template <typename Tuple>
    class tuple_joiner
    {
    public:
        using char_type = char;
        using string_view_type = std::basic_string_view<char_type>;

        tuple_joiner() = delete;
        constexpr tuple_joiner(const tuple_joiner&) noexcept = default;
        constexpr tuple_joiner(const Tuple& tp, string_view_type sep) noexcept
            : m_tp(tp), m_sep(sep) {}

        [[nodiscard]]
        constexpr const Tuple& get_tuple() const noexcept
        {
            return m_tp;
        }
        [[nodiscard]]
        constexpr string_view_type get_separator() const noexcept
        {
            return m_sep;
        }

    private:
        const Tuple& m_tp;
        string_view_type m_sep;
    };

    template <std::ranges::range Range>
    constexpr joiner<Range> join(const Range& rng, std::string_view sep) noexcept
    {
        return joiner<Range>(rng, sep);
    }
    template <typename... Ts>
    constexpr auto join(const std::tuple<Ts...>& tp, std::string_view sep)
    {
        return tuple_joiner<std::tuple<Ts...>>(tp, sep);
    }
    template <typename T1, typename T2>
    constexpr auto join(const std::pair<T1, T2>& p, std::string_view sep)
    {
        return tuple_joiner<std::pair<T1, T2>>(p, sep);
    }

    template <std::ranges::range Range>
    class formatter<joiner<Range>>
    {
    public:
        using joiner_type = joiner<Range>;
        using value_type = std::ranges::range_value_t<Range>;
        using underlying_formatter = formatter_traits<value_type>::formatter_type;

        void parse(format_spec_parse_context& spec_ctx)
        {
            m_fmt.parse(spec_ctx);
        }
        template <typename Context>
        void format(const joiner<Range>& rng, Context& ctx)
        {
            using context_traits = format_context_traits<Context>;

            bool first = true;
            for(auto&& i : rng.get_range())
            {
                if(!first)
                {
                    context_traits::append(ctx, rng.get_separator());
                }
                m_fmt.format(i, ctx);
                first = false;
            }
        }

    private:
        underlying_formatter m_fmt;
    };
    template <typename Tuple>
    class formatter<tuple_joiner<Tuple>>
    {
    public:
        using joiner_type = tuple_joiner<Tuple>;
        using tuple_type = Tuple;
        using string_view_type = std::basic_string_view<char>;

        void parse(format_spec_parse_context& spec_ctx)
        {
            if(!string_view_type(spec_ctx).empty())
            {
                throw invalid_format(
                    "invalid format specification for tuple joiner: " +
                    std::string(spec_ctx)
                );
            }
        }
        template <typename Context>
        void format(const joiner_type& tp_joiner, Context& ctx)
        {
            auto helper = [&]<std::size_t... Is> (std::index_sequence<Is...>)
            {
                (format_helper<Is>(tp_joiner, ctx), ...);
            };
            helper(std::make_index_sequence<std::tuple_size_v<Tuple>>());
        }

    private:
        template <std::size_t Index, typename Context>
        void format_helper(const joiner_type& tp_joiner, Context& ctx)
        {
            using context_traits = format_context_traits<Context>;
            const auto& val = std::get<Index>(tp_joiner.get_tuple());

            // TODO: Optimization
            if constexpr(Index != 0)
            {
                context_traits::append(ctx, tp_joiner.get_separator());
            }
            context_traits::advance_to(
                ctx,
                papilio::format_to(context_traits::out(ctx), "{}", val)
            );
        }
    };
}
