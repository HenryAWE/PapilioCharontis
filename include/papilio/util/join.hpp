#pragma once

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

    template <std::ranges::range Range>
    constexpr joiner<Range> join(const Range& rng, std::string_view sep) noexcept
    {
        return joiner<Range>(rng, sep);
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
            bool first = true;
            for(auto&& i : rng.get_range())
            {
                if(!first)
                {
                    format_context_traits traits(ctx);
                    traits.append(rng.get_separator());
                }
                m_fmt.format(i, ctx);
                first = false;
            }
        }

    private:
        underlying_formatter m_fmt;
    };
}
