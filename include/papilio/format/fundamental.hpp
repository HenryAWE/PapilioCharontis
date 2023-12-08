#pragma once

#include <concepts>
#include "../core.hpp"


namespace papilio
{
    namespace detail
    {
        inline bool is_align_ch(char32_t ch) noexcept
        {
            return ch == '<' || ch == '>' || ch == '^';
        }

        inline format_align get_align(char32_t ch) noexcept
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

        inline bool is_sign_ch(char32_t ch) noexcept
        {
            return ch == U'+' || ch == ' ' || ch == '-';
        }

        inline format_sign get_sign(char32_t ch) noexcept
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

        template <typename CharT, bool EnablePrecision = false>
        class std_fmt_parser
        {
        public:
            using iterator = format_parse_context::iterator;

            iterator parse(format_parse_context& ctx, std::u32string_view types)
            {
                iterator start = ctx.begin();
                const iterator stop = ctx.end();
                if(start == stop)
                    return start;
                if(*start == U'}')
                    return start;

                if(iterator next = start + 1; next != stop)
                {
                    if(char32_t ch = *next; is_align_ch(ch))
                    {
                        m_fill_ch = *start;
                        m_align = get_align(ch);
                        ++next;
                        start = next;
                    }
                }

                if(check_stop(start, stop))
                    goto parse_end;
                if(char32_t ch = *start; is_align_ch(ch))
                {
                    m_align = get_align(ch);
                    ++start;
                }

                if(check_stop(start, stop))
                    goto parse_end;
                if(char32_t ch = *start; is_sign_ch(ch))
                {
                    m_sign = get_sign(ch);
                    ++start;
                }

                if(check_stop(start, stop))
                    goto parse_end;
                if(*start == U'#')
                {
                    m_alt_form = true;
                    ++start;
                }

                if(check_stop(start, stop))
                    goto parse_end;
                if(*start == U'0')
                {
                    m_fill_zero = true;
                    ++start;
                }

                if(check_stop(start, stop))
                    goto parse_end;
                if(char32_t ch = *start; utf::is_digit(ch))
                {
                    if(ch == U'0')
                        throw invalid_format("invalid format");

                    ctx.advance_to(start);
                    std::tie(m_width, start) = parse_value<false>(ctx);
                }

                if(check_stop(start, stop))
                    goto parse_end;
                if(*start == U'.')
                {
                    ++start;
                    if(start == stop)
                        throw invalid_format("invalid precision");

                    ctx.advance_to(start);
                    std::tie(m_precision, start) = parse_value<true>(ctx);
                }

                if(check_stop(start, stop))
                    goto parse_end;
                if(char32_t ch = *start; types.find(ch) != types.npos)
                {
                    m_type = ch;
                }
                else
                {
                    throw invalid_format("invalid format");
                }

            parse_end:
                ctx.advance_to(start);
                return start;
            }

            [[nodiscard]]
            std::size_t width() const noexcept { return m_width; }
            [[nodiscard]]
            std::size_t precision() const noexcept { return m_precision; }

            [[nodiscard]]
            format_align align() const noexcept { return m_align; }
            [[nodiscard]]
            format_sign sign() const noexcept { return m_sign; }

            [[nodiscard]]
            bool alternate_form() const noexcept { return m_alt_form; }
            [[nodiscard]]
            bool fill_zero() const noexcept { return m_fill_zero; }

            [[nodiscard]]
            char32_t type_or(char32_t val) const noexcept
            {
                return m_type == U'\0' ? val : m_type;
            }

            [[nodiscard]]
            utf::codepoint fill_or(utf::codepoint val) const noexcept
            {
                return m_fill_ch ? m_fill_ch : val;
            }

        private:
            std::size_t m_width = 0;
            std::size_t m_precision = 6;
            utf::codepoint m_fill_ch = utf::codepoint();
            format_align m_align = format_align::default_align;
            format_sign m_sign = format_sign::default_sign;
            bool m_alt_form = false;
            bool m_fill_zero = false;
            char32_t m_type = U'\0';

            static bool is_spec_ch(char32_t ch, std::u32string_view types) noexcept
            {
                return
                    is_sign_ch(ch) ||
                    is_align_ch(ch) ||
                    utf::is_digit(ch) ||
                    ch == U'{' ||
                    ch == U'.' ||
                    ch == U'#' ||
                    ch == U'L';
            }

            static bool check_stop(iterator start, iterator stop) noexcept
            {
                return start == stop || *start == U'}';
            }

            // parse width or precision
            template <bool IsPrecision>
            static std::pair<std::size_t, iterator> parse_value(format_parse_context& ctx)
            {
                iterator start = ctx.begin();
                const iterator stop = ctx.end();
                PAPILIO_ASSERT(start != stop);

                char32_t first_ch = *start;

                if(!IsPrecision && first_ch == U'0')
                {
                    throw invalid_format("invalid format");
                }

                if(first_ch == U'{')
                {
                    ++start;

                    script::interpreter intp;
                    ctx.advance_to(start);
                    auto [arg, next_it] = intp.access(ctx);

                    if(next_it == stop || *next_it != U'}')
                    {
                        throw invalid_format("invalid format");
                    }
                    ++next_it;

                    auto var = arg.as_variable();
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
    }

    template <std::integral T, typename CharT> requires(!std::is_same_v<T, bool>)
    class formatter<T, CharT>
    {
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

        std::pair<std::size_t, std::size_t> calc_fill(std::size_t used) const noexcept
        {
            std::size_t n = m_width > used ? m_width - used : 0;

            switch(m_align)
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

        static std::size_t alt_prefix_size(int base) noexcept
        {
            switch(base)
            {
            default:
                return 0;

            case 2:
            case 16:
                return 2;

            case 8:
                return 1;
            }
        }


    public:
        format_parse_context::iterator parse(format_parse_context& ctx)
        {
            using namespace std::literals;

            detail::std_fmt_parser<CharT, false> parser;
            ctx.advance_to(parser.parse(ctx, U"XxBbodc"sv));

            m_fill_ch = parser.fill_or(U' ');
            m_align = parser.align();
            m_sign = parser.sign();
            m_fill_zero = parser.fill_zero();
            m_alt_form = parser.alternate_form();
            m_type = parser.type_or('d');
            m_width = parser.width();

            return ctx.begin();
        }

        template <typename FormatContext>
        FormatContext::iterator format(T val, FormatContext& ctx) const
        {
            CharT buf[sizeof(T) * 8];
            std::size_t buf_size = 0;

            auto [base, uppercase] = apply_type_ch(m_type);

            const CharT digits[16] = {
                '0', '1', '2', '3', '4',
                '5', '6', '7', '8', '9',
                uppercase ? 'A' : 'a',
                uppercase ? 'B' : 'b',
                uppercase ? 'C' : 'c',
                uppercase ? 'D' : 'd',
                uppercase ? 'E' : 'e',
                uppercase ? 'F' : 'f'
            };

            do
            {
                const T digit = val % base;
                buf[buf_size++] = digits[digit];
                val /= base;
            } while(val);

            using context_t = format_context_traits<FormatContext>;

            std::size_t used = buf_size;
            if(m_alt_form)
                used += alt_prefix_size(base);

            auto [left, right] = calc_fill(used);

            context_t::append(ctx, utf::codepoint(m_fill_ch), left);

            if(m_alt_form && base != 10)
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

            for(std::size_t i = buf_size; i > 0; --i)
                context_t::append(ctx, buf[i - 1]);

            context_t::append(ctx, utf::codepoint(m_fill_ch), right);

            return context_t::out(ctx);
        }

    private:
        std::size_t m_width = 0;
        format_align m_align = format_align::default_align;
        format_sign m_sign = format_sign::default_sign;
        bool m_alt_form = false;
        bool m_fill_zero = false;
        bool m_use_loc = false;
        char m_type = 'd';
        char32_t m_fill_ch = U' ';
    };
}
