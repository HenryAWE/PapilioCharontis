#include <concepts>

namespace papilio
{
PAPILIO_EXPORT template <std::integral T, typename CharT>
requires(!std::is_same_v<T, bool> && !char_like<T>)
class formatter<T, CharT>
{
public:
    template <typename ParseContext>
    auto parse(ParseContext& ctx) -> typename ParseContext::iterator
    {
        using namespace std::literals;

        using parser_t = std_formatter_parser<ParseContext, false>;

        parser_t parser;

        typename ParseContext::iterator it{};
        std::tie(m_data, it) = parser.parse(ctx, U"XxBbodc"sv);

        ctx.advance_to(it);
        return it;
    }

    template <typename FormatContext>
    auto format(T val, FormatContext& ctx) const -> typename FormatContext::iterator
    {
        if(m_data.type == U'c')
        {
            if(std::cmp_greater(val, std::numeric_limits<std::uint32_t>::max()))
                throw format_error("integer value out of range");

            codepoint_formatter fmt;
            fmt.set_data(m_data);
            return fmt.format(static_cast<char32_t>(val), ctx);
        }
        else
        {
            int_formatter<T, CharT> fmt;
            fmt.set_data(m_data);
            return fmt.format(val, ctx);
        }
    }

private:
    std_formatter_data m_data;
};

PAPILIO_EXPORT template <std::floating_point T, typename CharT>
class formatter<T, CharT>
{
public:
    template <typename ParseContext>
    auto parse(ParseContext& ctx) -> typename ParseContext::iterator
    {
        using namespace std::literals;

        using parser_t = std_formatter_parser<ParseContext, false>;

        parser_t parser;

        typename ParseContext::iterator it{};
        std::tie(m_data, it) = parser.parse(ctx, U"fFgGeEaA"sv);

        ctx.advance_to(it);
        return it;
    }

    template <typename FormatContext>
    auto format(T val, FormatContext& ctx) const -> typename FormatContext::iterator
    {
        float_formatter<T, CharT> fmt;
        fmt.set_data(m_data);
        return fmt.format(val, ctx);
    }

private:
    std_formatter_data m_data;
};

PAPILIO_EXPORT template <typename CharT>
class formatter<utf::codepoint, CharT>
{
public:
    template <typename ParseContext>
    auto parse(ParseContext& ctx) -> typename ParseContext::iterator
    {
        using namespace std::literals;

        using parser_t = std_formatter_parser<ParseContext, false>;

        parser_t parser;

        typename ParseContext::iterator it{};
        std::tie(m_data, it) = parser.parse(ctx, U"XxBbodc"sv);

        ctx.advance_to(it);
        return it;
    }

    template <typename FormatContext>
    auto format(utf::codepoint cp, FormatContext& ctx) const -> typename FormatContext::iterator
    {
        if(!m_data.contains_type(U'c'))
        {
            int_formatter<std::uint32_t, CharT> fmt;
            fmt.set_data(m_data);
            return fmt.format(static_cast<char32_t>(cp), ctx);
        }
        else
        {
            codepoint_formatter fmt;
            fmt.set_data(m_data);
            return fmt.format(cp, ctx);
        }
    }

private:
    std_formatter_data m_data;
};

PAPILIO_EXPORT template <typename CharT>
class formatter<bool, CharT>
{
public:
    template <typename ParseContext>
    auto parse(ParseContext& ctx) -> typename ParseContext::iterator
    {
        using namespace std::literals;

        using parser_t = std_formatter_parser<ParseContext, false>;

        parser_t parser;

        typename ParseContext::iterator it{};
        std::tie(m_data, it) = parser.parse(ctx, U"sXxBbod"sv);

        ctx.advance_to(it);
        return it;
    }

    template <typename FormatContext>
    auto format(bool val, FormatContext& ctx) const -> typename FormatContext::iterator
    {
        if(!m_data.contains_type(U's'))
        {
            int_formatter<std::uint8_t, CharT> fmt;
            fmt.set_data(m_data);
            return fmt.format(static_cast<std::uint8_t>(val), ctx);
        }
        else
        {
            string_formatter<CharT> fmt;
            fmt.set_data(m_data);
            const auto str = get_str(val, ctx.getloc_ref());
            return fmt.format(str, ctx);
        }
    }

private:
    std_formatter_data m_data;

    utf::basic_string_container<CharT> get_str(bool val, locale_ref loc) const
    {
        if(!m_data.use_locale) [[likely]]
        {
            static constexpr CharT true_str[] = {'t', 'r', 'u', 'e'};
            std::basic_string_view<CharT> true_sv(true_str, 4);
            static constexpr CharT false_str[] = {'f', 'a', 'l', 's', 'e'};
            std::basic_string_view<CharT> false_sv(false_str, 5);

            return val ? true_sv : false_sv;
        }
        else
        {
            const auto& facet = std::use_facet<std::numpunct<CharT>>(loc);
            return val ? facet.truename() : facet.falsename();
        }
    }
};

PAPILIO_EXPORT template <typename CharT>
class formatter<utf::basic_string_container<CharT>, CharT>
{
public:
    using string_container_type = utf::basic_string_container<CharT>;

    template <typename ParseContext>
    auto parse(ParseContext& ctx) -> typename ParseContext::iterator
    {
        using namespace std::literals;

        using parser_t = std_formatter_parser<ParseContext, true>;

        parser_t parser;

        typename ParseContext::iterator it{};
        std::tie(m_data, it) = parser.parse(ctx, U"s"sv);

        ctx.advance_to(it);
        return it;
    }

    template <typename FormatContext>
    auto format(const string_container_type& str, FormatContext& ctx) const -> typename FormatContext::iterator
    {
        string_formatter<CharT> fmt;
        fmt.set_data(m_data);
        return fmt.format(str, ctx);
    }

private:
    std_formatter_data m_data;
};

PAPILIO_EXPORT template <typename CharT>
class formatter<const void*, CharT>
{
public:
    template <typename ParseContext>
    auto parse(ParseContext& ctx) -> typename ParseContext::iterator
    {
        using namespace std::literals;

        using parser_t = std_formatter_parser<ParseContext, true>;

        parser_t parser;

        typename ParseContext::iterator it{};
        std::tie(m_data, it) = parser.parse(ctx, U"pP"sv);

        if(m_data.use_locale)
            throw format_error("invalid format");

        switch(m_data.type)
        {
        case U'\0':
        case U'p':
            m_data.type = 'x';
            break;

        case U'P':
            m_data.type = 'X';
            break;

        default:
            PAPILIO_UNREACHABLE();
        }

        m_data.alternate_form = true;

        ctx.advance_to(it);
        return it;
    }

    template <typename FormatContext>
    auto format(const void* p, FormatContext& ctx) const -> typename FormatContext::iterator
    {
        int_formatter<std::uintptr_t, CharT> fmt;
        fmt.set_data(m_data);
        return fmt.format(reinterpret_cast<std::uintptr_t>(p), ctx);
    }

private:
    std_formatter_data m_data{.type = U'x', .alternate_form = true};
};

template <typename T, typename CharT>
requires std::is_enum_v<T>
class formatter<T, CharT> : public enum_formatter<T, CharT>
{};
} // namespace papilio
