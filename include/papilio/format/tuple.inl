#include <tuple>
#include "../utility.hpp"

namespace papilio
{
template <tuple_like Tuple, typename CharT>
class formatter<Tuple, CharT>
{
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
                set_separator(PAPILIO_TSTRING_VIEW(CharT, ": "));
                set_brackets(string_view_type(), string_view_type());
                ++it;
            }
        }
        if(type == U'n')
        {
            set_brackets(string_view_type(), string_view_type());
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
    string_view_type m_sep = PAPILIO_TSTRING_VIEW(CharT, ", ");
    string_view_type m_opening = PAPILIO_TSTRING_VIEW(CharT, "(");
    string_view_type m_closing = PAPILIO_TSTRING_VIEW(CharT, ")");
};
} // namespace papilio
