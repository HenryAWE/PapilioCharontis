#pragma once

#include <concepts>
#include <utility>
#include <limits>
#include "../core.hpp"

namespace papilio
{
struct std_formatter_data
{
    using size_type = std::size_t;

    size_type width = 0;
    size_type precision = 0;
    utf::codepoint fill = utf::codepoint();
    char32_t type = U'\0';
    format_align align = format_align::default_align;
    format_sign sign = format_sign::default_sign;
    bool fill_zero = false;
    bool alternate_form = false;

    [[nodiscard]]
    constexpr bool contains_type(std::u32string_view types) const noexcept
    {
        if(type == U'\0')
            return true;
        return types.find(type) != types.npos;
    }

    constexpr void check_type(std::u32string_view types) const
    {
        if(!contains_type(types))
        {
            throw invalid_format("invalid format type");
        }
    }

    [[nodiscard]]
    constexpr char32_t type_or(char32_t val) const noexcept
    {
        return type == U'\0' ? val : type;
    }

    [[nodiscard]]
    constexpr utf::codepoint fill_or(utf::codepoint val) const noexcept
    {
        return fill ? fill : val;
    }
};

template <typename ParseContext, bool EnablePrecision = false>
class std_formatter_parser
{
public:
    using char_type = typename ParseContext::char_type;
    using iterator = typename ParseContext::iterator;

    using result_type = std_formatter_data;
    using interpreter_type = script::interpreter<typename ParseContext::format_context_type>;

    std::pair<result_type, iterator> parse(ParseContext& ctx, std::u32string_view types)
    {
        std_formatter_data result;

        iterator start = ctx.begin();
        const iterator stop = ctx.end();

        if(start == stop)
            goto parse_end;
        if(*start == U'}')
            goto parse_end;

        if(iterator next = start + 1; next != stop)
        {
            if(char32_t ch = *next; is_align_ch(ch))
            {
                result.fill = *start;
                result.align = get_align(ch);
                ++next;
                start = next;
            }
        }

        if(check_stop(start, stop))
            goto parse_end;
        if(char32_t ch = *start; is_align_ch(ch))
        {
            result.align = get_align(ch);
            ++start;
        }

        if(check_stop(start, stop))
            goto parse_end;
        if(char32_t ch = *start; is_sign_ch(ch))
        {
            result.sign = get_sign(ch);
            ++start;
        }

        if(check_stop(start, stop))
            goto parse_end;
        if(*start == U'#')
        {
            result.alternate_form = true;
            ++start;
        }

        if(check_stop(start, stop))
            goto parse_end;
        if(*start == U'0')
        {
            result.fill_zero = true;
            ++start;
        }

        if(check_stop(start, stop))
            goto parse_end;
        if(char32_t ch = *start; utf::is_digit(ch))
        {
            if(ch == U'0')
                throw invalid_format("invalid format");

            ctx.advance_to(start);
            std::tie(result.width, start) = parse_value<false>(ctx);
        }
        else if(ch == U'{')
        {
            ctx.advance_to(start);
            std::tie(result.width, start) = parse_value<false>(ctx);
        }

        if(check_stop(start, stop))
            goto parse_end;
        if(*start == U'.')
        {
            ++start;
            if(start == stop)
                throw invalid_format("invalid precision");

            ctx.advance_to(start);
            std::tie(result.precision, start) = parse_value<true>(ctx);
        }

        if(check_stop(start, stop))
            goto parse_end;
        if(char32_t ch = *start; types.find(ch) != types.npos)
        {
            result.type = ch;
            ++start;
        }
        else
        {
            throw invalid_format("invalid format");
        }

parse_end:
        ctx.advance_to(start);
        return std::make_pair(std::move(result), std::move(start));
    }

private:
    static bool is_align_ch(char32_t ch) noexcept
    {
        return ch == U'<' || ch == U'>' || ch == U'^';
    }

    static format_align get_align(char32_t ch) noexcept
    {
        PAPILIO_ASSERT(is_align_ch(ch));

        switch(ch)
        {
        case U'<': return format_align::left;
        case U'>': return format_align::right;
        case U'^': return format_align::middle;

        default: PAPILIO_UNREACHABLE();
        }
    }

    static bool is_sign_ch(char32_t ch) noexcept
    {
        return ch == U'+' || ch == U' ' || ch == U'-';
    }

    static format_sign get_sign(char32_t ch) noexcept
    {
        PAPILIO_ASSERT(is_sign_ch(ch));

        switch(ch)
        {
        case U'+': return format_sign::positive;
        case U' ': return format_sign::space;
        case U'-': return format_sign::negative;

        default: PAPILIO_UNREACHABLE();
        }
    }

    static bool is_spec_ch(char32_t ch, std::u32string_view types) noexcept
    {
        return is_sign_ch(ch) ||
               is_align_ch(ch) ||
               utf::is_digit(ch) ||
               ch == U'{' ||
               ch == U'.' ||
               ch == U'#' ||
               ch == U'L' ||
               types.find(ch) != types.npos;
    }

    static bool check_stop(iterator start, iterator stop) noexcept
    {
        return start == stop || *start == U'}';
    }

    // parse width or precision
    template <bool IsPrecision>
    static std::pair<std::size_t, iterator> parse_value(ParseContext& ctx)
    {
        iterator start = ctx.begin();
        const iterator stop = ctx.end();
        PAPILIO_ASSERT(start != stop);

        char32_t first_ch = *start;

        if(!IsPrecision)
        {
            if(first_ch == U'0')
            {
                throw invalid_format("invalid format");
            }
        }

        if(first_ch == U'{')
        {
            ++start;

            interpreter_type intp;
            ctx.advance_to(start);
            auto [arg, next_it] = intp.access(ctx);

            if(next_it == stop || *next_it != U'}')
            {
                throw invalid_format("invalid format");
            }
            ++next_it;

            auto var = script::variable(arg.to_variant());
            if(!var.holds_int())
                throw invalid_format("invalid type");
            ssize_t val = var.as<ssize_t>();
            if constexpr(IsPrecision)
            {
                if(val < 0)
                    throw invalid_format("invalid format");
            }
            else
            {
                if(val <= 0)
                    throw invalid_format("invalid format");
            }

            return std::make_pair(val, next_it);
        }
        else if(utf::is_digit(first_ch))
        {
            ++start;
            std::size_t val = first_ch - U'0';

            while(start != stop)
            {
                char32_t ch = *start;
                if(!utf::is_digit(ch))
                    break;
                val *= 10;
                val += ch - U'0';

                ++start;
            }

            PAPILIO_ASSERT(IsPrecision || val != 0);

            return std::make_pair(val, start);
        }

        throw invalid_format("invalid format");
    }
};

namespace detail
{
    class std_formatter_base
    {
    protected:
        std_formatter_data m_data;

        std::pair<std::size_t, std::size_t> calc_fill(std::size_t used) const noexcept
        {
            std::size_t n = m_data.width > used ? m_data.width - used : 0;

            switch(m_data.align)
            {
            default:
            case format_align::default_align:
            case format_align::right:
                return std::make_pair(n, 0);

            case format_align::left:
                return std::make_pair(0, n);

            case format_align::middle:
                return std::make_pair(n / 2, n / 2 + n % 2);
            }
        }

        template <typename FormatContext>
        void fill(FormatContext& ctx, std::size_t count) const
        {
            using context_t = format_context_traits<FormatContext>;

            PAPILIO_ASSERT(m_data.fill != U'\0');
            context_t::append(ctx, utf::codepoint(m_data.fill), count);
        }
    };

    template <std::integral T, typename CharT>
    class int_formatter : public std_formatter_base
    {
    public:
        constexpr void set_data(const std_formatter_data& data) noexcept
        {
            PAPILIO_ASSERT(data.contains_type(U"BbXxod"));

            m_data = data;
            m_data.fill = data.fill_or(U' ');
            m_data.type = data.type_or(U'd');
            if(m_data.align != format_align::default_align)
                m_data.fill_zero = false;
        }

        template <typename FormatContext>
        auto format(T val, FormatContext& ctx) const -> typename FormatContext::iterator
        {
            CharT buf[sizeof(T) * 8];
            std::size_t buf_size = 0;

            auto [base, uppercase] = apply_type_ch(m_data.type);

            // clang-format off

            const CharT digits[16] = {
                '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                static_cast<CharT>(uppercase ? 'A' : 'a'),
                static_cast<CharT>(uppercase ? 'B' : 'b'),
                static_cast<CharT>(uppercase ? 'C' : 'c'),
                static_cast<CharT>(uppercase ? 'D' : 'd'),
                static_cast<CharT>(uppercase ? 'E' : 'e'),
                static_cast<CharT>(uppercase ? 'F' : 'f')
            };

            // clang-format on

            const bool neg = val < 0;
            if constexpr(std::is_signed_v<T>)
            {
                if(neg)
                    val = -val;
            }

            do
            {
                const T digit = val % base;
                buf[buf_size++] = digits[digit];
                val /= base;
            } while(val);

            using context_t = format_context_traits<FormatContext>;

            std::size_t used = buf_size;
            if(m_data.alternate_form)
                used += alt_prefix_size(base);
            switch(m_data.sign)
            {
            case format_sign::negative:
            case format_sign::default_sign:
                if(neg)
                    ++used;
                break;

            case format_sign::positive:
            case format_sign::space:
                ++used;
                break;

            default:
                PAPILIO_UNREACHABLE();
            }

            auto [left, right] = m_data.fill_zero ?
                                     std::make_pair<std::size_t, std::size_t>(0, 0) :
                                     calc_fill(used);

            fill(ctx, left);

            switch(m_data.sign)
            {
            case format_sign::negative:
            case format_sign::default_sign:
                if(neg)
                    context_t::append(ctx, static_cast<CharT>('-'));
                break;

            case format_sign::positive:
                context_t::append(ctx, static_cast<CharT>(neg ? '-' : '+'));
                break;

            case format_sign::space:
                context_t::append(ctx, static_cast<CharT>(neg ? '-' : ' '));
                break;

            default:
                PAPILIO_UNREACHABLE();
            }

            if(m_data.alternate_form && base != 10)
            {
                context_t::append(ctx, '0');
                switch(base)
                {
                case 16:
                    context_t::append(ctx, uppercase ? 'X' : 'x');
                    break;

                case 2:
                    context_t::append(ctx, uppercase ? 'B' : 'b');
                }
            }

            if(m_data.fill_zero)
            {
                if(used < m_data.width)
                {
                    context_t::append(ctx, static_cast<CharT>('0'), m_data.width - used);
                }
            }

            for(std::size_t i = buf_size; i > 0; --i)
                context_t::append(ctx, buf[i - 1]);

            fill(ctx, right);

            return context_t::out(ctx);
        }

    private:
        static std::pair<int, bool> apply_type_ch(char32_t ch) noexcept
        {
            int base = 10;
            bool uppercase = false;

            switch(ch)
            {
            case U'X':
                uppercase = true;
                [[fallthrough]];
            case U'x':
                base = 16;
                break;

            case U'B':
                uppercase = true;
                [[fallthrough]];
            case U'b':
                base = 2;
                break;

            case U'O':
                uppercase = true;
                [[fallthrough]];
            case U'o':
                base = 8;
                break;

            case U'D':
                uppercase = true;
                [[fallthrough]];
            case U'd':
                // base is already 10
                break;

            default:
                PAPILIO_UNREACHABLE();
            }

            return std::make_pair(base, uppercase);
        }

        static std::size_t alt_prefix_size(int base) noexcept
        {
            switch(base)
            {
            default:
                PAPILIO_ASSERT(base != 10);
                return 0;

            case 2:
            case 16:
                return 2;

            case 8:
                return 1;
            }
        }
    };

    class codepoint_formatter : public std_formatter_base
    {
    public:
        void set_data(const std_formatter_data& data)
        {
            PAPILIO_ASSERT(data.contains_type(U"c"));

            m_data = data;
            m_data.type = data.type_or(U'c');
            m_data.fill = data.fill_or(U' ');
        }

        template <typename FormatContext>
        auto format(utf::codepoint cp, FormatContext& ctx)
        {
            using context_t = format_context_traits<FormatContext>;

            auto [left, right] = calc_fill(cp.estimate_width());

            fill(ctx, left);
            context_t::append(ctx, cp);
            fill(ctx, right);

            return context_t::out(ctx);
        }
    };

    template <typename CharT>
    class string_formatter : public std_formatter_base
    {
    public:
        using string_container_type = utf::basic_string_container<CharT>;

        void set_data(const std_formatter_data& data)
        {
            PAPILIO_ASSERT(data.contains_type(U"s"));

            m_data = data;
            m_data.type = data.type_or(U's');
            m_data.fill = data.fill_or(U' ');
            if(m_data.align == format_align::default_align)
                m_data.align = format_align::left;
        }

        template <typename FormatContext>
        auto format(string_container_type str, FormatContext& ctx)
        {
            PAPILIO_ASSERT(!str.has_ownership());

            using context_t = format_context_traits<FormatContext>;

            std::size_t used = 0;


            if(m_data.precision != 0)
            {
                for(auto it = str.begin(); it != str.end(); ++it)
                {
                    std::size_t w = (*it).estimate_width();
                    if(used + w > m_data.precision)
                    {
                        str.assign(str.begin(), it);
                        break;
                    }
                    used += w;
                }
            }
            else
            {
                for(utf::codepoint cp : str)
                    used += cp.estimate_width();
            }

            auto [left, right] = calc_fill(used);

            fill(ctx, left);
            context_t::append(ctx, str);
            fill(ctx, right);

            return context_t::out(ctx);
        }
    };
} // namespace detail

template <std::integral T, typename CharT>
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

        typename ParseContext::iterator it;
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
                throw invalid_format("integer value out of range");

            detail::codepoint_formatter fmt;
            fmt.set_data(m_data);
            return fmt.format(static_cast<char32_t>(val), ctx);
        }
        else
        {
            detail::int_formatter<T, CharT> fmt;
            fmt.set_data(m_data);
            return fmt.format(val, ctx);
        }
    }

private:
    std_formatter_data m_data;
};

template <typename CharT>
class formatter<utf::codepoint, CharT>
{
public:
    template <typename ParseContext>
    auto parse(ParseContext& ctx) -> typename ParseContext::iterator
    {
        using namespace std::literals;

        using parser_t = std_formatter_parser<ParseContext, false>;

        parser_t parser;

        typename ParseContext::iterator it;
        std::tie(m_data, it) = parser.parse(ctx, U"XxBbodc"sv);

        ctx.advance_to(it);
        return it;
    }

    template <typename FormatContext>
    auto format(utf::codepoint cp, FormatContext& ctx) const -> typename FormatContext::iterator
    {
        if(!m_data.contains_type(U"c"))
        {
            detail::int_formatter<std::uint32_t, CharT> fmt;
            fmt.set_data(m_data);
            return fmt.format(static_cast<char32_t>(cp), ctx);
        }
        else
        {
            detail::codepoint_formatter fmt;
            fmt.set_data(m_data);
            return fmt.format(cp, ctx);
        }
    }

private:
    std_formatter_data m_data;
};

template <typename CharT>
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

        typename ParseContext::iterator it;
        std::tie(m_data, it) = parser.parse(ctx, U"s"sv);

        ctx.advance_to(it);
        return it;
    }

    template <typename FormatContext>
    auto format(const string_container_type& str, FormatContext& ctx) const -> typename FormatContext::iterator
    {
        detail::string_formatter<CharT> fmt;
        fmt.set_data(m_data);
        return fmt.format(str, ctx);
    }

private:
    std_formatter_data m_data;
};
} // namespace papilio
