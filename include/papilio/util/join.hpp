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
}
