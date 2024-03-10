#ifndef PAPILIO_FORMAT_FUNDAMENTAL_HPP
#define PAPILIO_FORMAT_FUNDAMENTAL_HPP

#pragma once

#include <concepts>
#include <utility>
#include <limits>
#include <charconv>
#include <cmath>
#include "../core.hpp"

#ifdef PAPILIO_COMPILER_CLANG_CL
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wpre-c++20-compat-pedantic"
#    pragma clang diagnostic ignored "-Wcovered-switch-default"
#endif

namespace papilio
{
PAPILIO_EXPORT struct std_formatter_data
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
    bool use_locale = false;

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
            throw format_error("invalid format type");
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

PAPILIO_EXPORT template <typename ParseContext, bool EnablePrecision = false>
class std_formatter_parser
{
public:
    using char_type = typename ParseContext::char_type;
    using iterator = typename ParseContext::iterator;

    using result_type = std_formatter_data;
    using interpreter_type = script::basic_interpreter<typename ParseContext::format_context_type>;

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
                throw format_error("invalid format");

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
                throw format_error("invalid precision");

            ctx.advance_to(start);
            std::tie(result.precision, start) = parse_value<true>(ctx);
        }

        if(check_stop(start, stop))
            goto parse_end;
        if(*start == U'L')
        {
            result.use_locale = true;
            ++start;
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
            throw format_error("invalid format");
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
                throw format_error("invalid format");
            }
        }

        if(first_ch == U'{')
        {
            ++start;

            interpreter_type intp{};
            ctx.advance_to(start);
            auto [arg, next_it] = intp.access(ctx);

            if(next_it == stop || *next_it != U'}')
            {
                throw format_error("invalid format");
            }
            ++next_it;

            auto var = script::variable(arg.to_variant());
            if(!var.holds_int())
                throw format_error("invalid type");
            ssize_t val = var.as<ssize_t>();
            if constexpr(IsPrecision)
            {
                if(val < 0)
                    throw format_error("invalid format");
            }
            else
            {
                if(val <= 0)
                    throw format_error("invalid format");
            }

            return std::make_pair(val, next_it);
        }
        else if(utf::is_digit(first_ch))
        {
            ++start;
            std::size_t val = static_cast<std::size_t>(first_ch - U'0');

            while(start != stop)
            {
                char32_t ch = *start;
                if(!utf::is_digit(ch))
                    break;
                val *= 10;
                val += static_cast<std::size_t>(ch - U'0');

                ++start;
            }

            PAPILIO_ASSERT(IsPrecision || val != 0);

            return std::make_pair(val, start);
        }

        throw format_error("invalid format");
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

#ifdef PAPILIO_COMPILER_CLANG_CL
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#endif

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
                const T t_base = static_cast<T>(base);
                const T digit = val % t_base;
                buf[buf_size++] = digits[digit];
                val /= static_cast<T>(t_base);
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

    template <std::floating_point T, typename CharT>
    class float_formatter : public std_formatter_base
    {
    public:
        void set_data(const std_formatter_data& data)
        {
            PAPILIO_ASSERT(data.contains_type(U"fFgGeEaA"));

            m_data = data;
            m_data.fill = data.fill_or(U' ');
            if(m_data.align == format_align::default_align)
                m_data.align = format_align::right;
        }

        template <typename FormatContext>
        auto format(T val, FormatContext& ctx) const
        {
            using context_t = format_context_traits<FormatContext>;

            CharT buf[buf_size];

            std::size_t fp_size = 0;

            bool neg = std::signbit(val);
            val = std::abs(val);

            if(std::isinf(val)) [[unlikely]]
            {
                buf[0] = static_cast<CharT>('i');
                buf[1] = static_cast<CharT>('n');
                buf[2] = static_cast<CharT>('f');
                fp_size = 3;
            }
            else if(std::isnan(val)) [[unlikely]]
            {
                buf[0] = static_cast<CharT>('n');
                buf[1] = static_cast<CharT>('a');
                buf[2] = static_cast<CharT>('n');
                fp_size = 3;
            }
            else [[likely]]
            {
                fp_size = conv<CharT>(buf, val);
            }

            std::size_t used = fp_size;

            switch(m_data.sign)
            {
            case format_sign::default_sign:
            case format_sign::negative:
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

            auto [left, right] = calc_fill(used);

            fill(ctx, left);

            switch(m_data.sign)
            {
            case format_sign::default_sign:
            case format_sign::negative:
                if(neg)
                    context_t::append(ctx, static_cast<CharT>('-'));
                break;

            case format_sign::positive:
                context_t::append(ctx, static_cast<CharT>(neg ? '-' : '+'));
                break;

            case format_sign::space:
                context_t::append(ctx, static_cast<CharT>(neg ? '-' : ' '));
                break;
            }

            context_t::append(ctx, buf, buf + fp_size);

            fill(ctx, right);

            return context_t::out(ctx);
        }

    private:
        static constexpr std::size_t buf_size = 32;

        std::pair<std::chars_format, bool> get_chars_fmt() const
        {
            std::chars_format ch_fmt{};
            bool uppercase = false;

            switch(m_data.type)
            {
            case U'G':
                uppercase = true;
                [[fallthrough]];
            case U'\0':
            case U'g':
                ch_fmt = std::chars_format::general;
                break;

            case 'F':
            case 'f':
                ch_fmt = std::chars_format::fixed;
                break;

            case 'A':
                uppercase = true;
                [[fallthrough]];
            case 'a':
                ch_fmt = std::chars_format::hex;
                break;

            case 'E':
                uppercase = true;
                [[fallthrough]];
            case 'e':
                ch_fmt = std::chars_format::scientific;
                break;
            }

            return std::make_pair(ch_fmt, uppercase);
        }

        template <char8_like U>
        std::size_t conv(std::span<U, buf_size> buf, T val) const
        {
            using namespace std::literals;

            std::to_chars_result result;
            auto [ch_fmt, uppercase] = get_chars_fmt();

            int precision = static_cast<int>(m_data.precision);
            if(precision == 0 &&
               U"fFeEgG"sv.find(m_data.type) != std::u32string::npos)
            {
                precision = 6;
            }

            char* start = std::bit_cast<char*>(std::to_address(buf.begin()));
            char* stop = std::bit_cast<char*>(std::to_address(buf.end()));

            if(precision == 0)
                result = std::to_chars(start, stop, val, ch_fmt);
            else
                result = std::to_chars(start, stop, val, ch_fmt, precision);

            if(result.ec == std::errc::value_too_large) [[unlikely]]
            {
                throw format_error("value too large");
            }
            else if(result.ec != std::errc())
            {
                PAPILIO_UNREACHABLE();
            }

            std::size_t size = static_cast<std::size_t>(result.ptr - buf.data());

            if(uppercase)
            {
                for(std::size_t i = 0; i < size; ++i)
                {
                    if('a' <= buf[i] && buf[i] <= 'z')
                        buf[i] -= 'a' - 'A'; // to upper
                }
            }

            return size;
        }

        template <typename U>
        requires(!char8_like<U>)
        std::size_t conv(std::span<U, buf_size> buf, T val) const
        {
            char narrow_buf[buf_size];
            std::size_t size = conv<char>(narrow_buf, val);
            PAPILIO_ASSERT(size <= buf.size());

            std::copy_n(narrow_buf, size, buf.begin());
            return size;
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

#ifdef PAPILIO_COMPILER_CLANG_CL
#    pragma clang diagnostic pop
#endif
} // namespace detail

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
        detail::float_formatter<T, CharT> fmt;
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
        if(!m_data.contains_type(U"s"))
        {
            detail::int_formatter<std::uint8_t, CharT> fmt;
            fmt.set_data(m_data);
            return fmt.format(static_cast<std::uint8_t>(val), ctx);
        }
        else
        {
            detail::string_formatter<CharT> fmt;
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
        detail::string_formatter<CharT> fmt;
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
        detail::int_formatter<std::uintptr_t, CharT> fmt;
        fmt.set_data(m_data);
        return fmt.format(reinterpret_cast<std::uintptr_t>(p), ctx);
    }

private:
    std_formatter_data m_data{.type = U'x', .alternate_form = true};
};

namespace detail
{

    template <typename Enum, typename CharT = char>
    requires std::is_enum_v<Enum>
    class enum_formatter
    {
    public:
        template <typename ParseContext>
        auto parse(ParseContext& ctx) -> typename ParseContext::iterator
        {
            std_formatter_parser<ParseContext, true> parser;

            auto [result, it] = parser.parse(ctx, U"sBbXxod");

            m_data = result;

            return it;
        }

        template <typename FormatContext>
        auto format(Enum e, FormatContext& ctx) const -> typename FormatContext::iterator
        {
            if(m_data.type_or(U's') == U's')
            {
                detail::string_formatter<CharT> fmt;
                fmt.set_data(m_data);
                if constexpr(char8_like<CharT>)
                {
                    std::string_view name = enum_name<Enum>(e, true);
                    auto name_conv = std::basic_string_view<CharT>(
                        std::bit_cast<const CharT*>(name.data()),
                        name.size()
                    );
                    return fmt.format(
                        name_conv,
                        ctx
                    );
                }
                else
                {
                    utf::string_ref name = enum_name<Enum>(e, true);
                    auto name_conv = name.to_string_as<CharT>();
                    return fmt.format(
                        name_conv,
                        ctx
                    );
                }
            }
            else
            {
                detail::int_formatter<std::underlying_type_t<Enum>, CharT> fmt;
                fmt.set_data(m_data);
                return fmt.format(PAPILIO_NS to_underlying(e), ctx);
            }
        }

    private:
        std_formatter_data m_data{.type = U's'};
    };
} // namespace detail

template <typename T, typename CharT>
requires std::is_enum_v<T>
class formatter<T, CharT> : public detail::enum_formatter<T, CharT>
{};
} // namespace papilio

#ifdef PAPILIO_COMPILER_CLANG_CL
#    pragma clang diagnostic pop
#endif

#endif
