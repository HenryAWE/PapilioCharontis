#pragma once

#include "core.hpp"
#include "script.hpp"
#include <cmath>


namespace papilio
{
    template <typename Context>
    class format_executor
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;

        format_executor(string_view_type fmt, Context& ctx) noexcept
            : m_fmt(fmt), m_ctx(ctx) {}
        format_executor(const format_executor&) = delete;

        void execute()
        {
            format_context_traits traits(m_ctx);
            format_parse_context parse_ctx(m_fmt, traits.get_store());
            script::lexer lex;
            script::interpreter intp;

            auto it = parse_ctx.begin();
            for(; it != parse_ctx.end(); it = parse_ctx.begin())
            {
                char_type ch = *it;
                if(ch == '{')
                {
                    auto next_it = std::next(it);
                    if(next_it == parse_ctx.end())
                    {
                        throw std::runtime_error("missing replacement field");
                    }

                    if(*next_it == '{')
                    {
                        traits.append('{');
                        parse_ctx.advance_to(std::next(next_it));
                        continue;
                    }
                    else
                    {
                        std::optional<std::size_t> default_arg_id = std::nullopt;
                        if(!parse_ctx.manual_indexing())
                            default_arg_id = parse_ctx.current_arg_id();
                        auto field_begin = std::next(it);
                        lex.clear();
                        auto result = lex.parse(
                            string_view_type(field_begin, parse_ctx.end()),
                            script::lexer_mode::replacement_field,
                            default_arg_id
                        );
                        if(result.default_arg_idx_used)
                            parse_ctx.next_arg_id();
                        else
                            parse_ctx.enable_manual_indexing();
                        auto arg_end = std::next(field_begin, result.parsed_char);
                        if(arg_end == parse_ctx.end())
                        {
                            throw std::runtime_error("missing right brace ('}')");
                        }

                        auto pred = [counter = std::size_t(0)](char_type ch) mutable
                        {
                            if(ch == '{')
                                ++counter;
                            if(ch == '}')
                            {
                                if(counter == 0)
                                    return true;
                                --counter;
                            }

                            return false;
                        };
                        auto fmt_begin = arg_end;
                        if(*arg_end == ':')
                            ++fmt_begin;
                        auto field_end = std::find_if(fmt_begin, parse_ctx.end(), pred);
                        if(field_end == parse_ctx.end())
                        {
                            throw std::runtime_error("missing right brace ('}')");
                        }

                        auto [idx, acc] = intp.access(lex.lexemes());
                        // TODO: Separate parsing and formatting

                        format_spec_parse_context spec_ctx(
                            parse_ctx,
                            string_view_type(fmt_begin, field_end)
                        );
                        acc.access(traits.get_store().get(idx)).format(
                            spec_ctx,
                            m_ctx
                        );

                        parse_ctx.advance_to(std::next(field_end));
                    }
                }
                else if(ch == '}')
                {
                    auto next_it = std::next(it);
                    if(next_it != parse_ctx.end() && *next_it == '}')
                    {
                        traits.append('}');
                        parse_ctx.advance_to(std::next(next_it));
                    }
                    else
                    {
                        throw std::runtime_error("invalid right brace ('}')");
                    }
                }
                else if(ch == '[')
                {
                    auto next_it = std::next(it);
                    if(next_it == parse_ctx.end())
                    {
                        throw std::runtime_error("missing script block");
                    }

                    if(*next_it == '[')
                    {
                        traits.append('[');
                        parse_ctx.advance_to(std::next(next_it));
                        continue;
                    }
                    else
                    {
                        string_view_type src(next_it, parse_ctx.end());
                        lex.clear();
                        auto result = lex.parse(
                            src,
                            script::lexer_mode::script_block
                        );
                        auto script_end = std::next(next_it, result.parsed_char);
                        if(script_end == parse_ctx.end() || *script_end != ']')
                        {
                            throw std::runtime_error("missing right bracket (']')");
                        }

                        script::executor::context ex_ctx(traits.get_store());
                        intp.compile(lex.lexemes())(ex_ctx);

                        traits.append(ex_ctx.get_result());
                        parse_ctx.advance_to(std::next(script_end));
                    }
                }
                else if(ch == ']')
                {
                    auto next_it = std::next(it);
                    if(next_it != parse_ctx.end() && *next_it == ']')
                    {
                        traits.append(']');
                        parse_ctx.advance_to(std::next(next_it));
                    }
                    else
                    {
                        throw std::runtime_error("invalid right brace ('}')");
                    }
                }
                else
                {
                    traits.append(ch);
                    parse_ctx.advance_to(std::next(it));
                }
            }
        }

    private:
        string_view_type m_fmt;
        Context& m_ctx;
    };

    template <typename... Args>
    dynamic_format_arg_store make_format_args(Args&&... args)
    {
        return dynamic_format_arg_store(std::forward<Args>(args)...);
    }

    std::string vformat(std::string_view fmt, const dynamic_format_arg_store& store);
    template <typename... Args>
    std::string format(std::string_view fmt, Args&&... args)
    {
        // use namespace prefix to avoid collision with std::format caused by ADL
        return vformat(fmt, papilio::make_format_args(std::forward<Args>(args)...));
    }

    namespace detail
    {
        template <typename T>
        concept supported_integral =
            std::is_same_v<T, signed char> ||
            std::is_same_v<T, unsigned char> ||
            std::is_same_v<T, short> ||
            std::is_same_v<T, unsigned short> ||
            std::is_same_v<T, int> ||
            std::is_same_v<T, unsigned int> ||
            std::is_same_v<T, long> ||
            std::is_same_v<T, unsigned long> ||
            std::is_same_v<T, long long> ||
            std::is_same_v<T, signed char> ||
            std::is_same_v<T, unsigned long long>;

        class integral_formatter_base
        {
        public:
            static char map_char(unsigned char digit, bool uppercase) noexcept
            {
                if(digit < 10)
                    return '0' + digit;
                else
                {
                    if(uppercase)
                        return 'A' + (digit - 10);
                    else
                        return 'a' + (digit - 10);
                }
            }
        };

        // front and back
        std::pair<std::size_t, std::size_t> calc_fill_width(format_align align, std::size_t width, std::size_t current);
    }

    template <detail::supported_integral Integral>
    class formatter<Integral> : public detail::integral_formatter_base
    {
    public:
        using char_type = char;
        using value_type = Integral;
        using unsigned_value_type = std::make_unsigned_t<value_type>;

        void parse(format_spec_parse_context& ctx)
        {
            m_spec.parse(ctx);
            std::tie(m_base, m_uppercase) = get_base(m_spec.type_char_or(U'd'));

            if(m_spec.sign() == format_sign::default_sign)
                m_spec.sign(format_sign::negative);
            if(m_spec.width() != 0)
            {
                if(m_spec.align() == format_align::default_align)
                    m_spec.align(format_align::right);
                if(!m_spec.has_fill())
                {
                    if(!(m_spec.align() == format_align::right && m_spec.fill_zero()))
                        m_spec.fill(U' ');
                }
            }
        }
        template <typename Context>
        void format(value_type val, Context& ctx) const
        {
            format_impl(
                val, ctx,
                m_spec, m_base, m_uppercase
            );
        }

        // return base of the integer
        // the second Boolean value indicates whether use the uppercase letters for hexadecimal output
        static std::pair<unsigned int, bool> get_base(char32_t type)
        {
            if(type == U'd')
                return std::make_pair(10, false);
            else if(type == U'x')
                return std::make_pair(16, false);
            else if(type == U'X')
                return std::make_pair(16, true);
            else if(type == U'b')
                return std::make_pair(2, false);
            else if(type == U'o')
                return std::make_pair(8, false);
            else
            {
                throw invalid_format("invalid type '" + std::string(utf8::codepoint(type)) + "' for integer");
            }
        }

        // Internal API
        // This function can be used by other formatter (e.g. formatter for single character)
        // when they need to redirect their outputs to integral formatter.
        // For example, when character formatter needs to output the code point of character
        // it can redirect its parsed specification to here
        template <typename Context>
        static void format_impl(
            value_type val, Context& ctx,
            const common_format_spec& spec, unsigned int base, bool uppercase)
        {
            std::size_t len = 0;
            if(spec.sign() == format_sign::negative)
            {
                if constexpr(std::is_signed_v<value_type>)
                {
                    if(val < 0)
                        ++len;
                }
            }
            else
                ++len;
            if(spec.alternate_form() && base != 10)
                len += 2;
            std::size_t raw_num_len = raw_formatted_size(val, base);
            len += raw_num_len;

            format_context_traits traits(ctx);

            std::size_t fill_front = 0;
            std::size_t fill_back = 0;
            if(spec.has_fill())
            {
                std::tie(fill_front, fill_back) = detail::calc_fill_width(
                    spec.align(),
                    spec.width(),
                    len
                );
            }

            if(fill_front != 0)
                traits.append(spec.fill(), fill_front);

            switch(spec.sign())
            {
            case format_sign::negative:
                if(val < 0)
                    traits.append('-');
                break;
            case format_sign::positive:
                if(val < 0)
                    traits.append('-');
                else
                    traits.append('+');
                break;
            case format_sign::space:
                if(val < 0)
                    traits.append('-');
                else
                    traits.append(' ');
                break;
            }
            if(spec.alternate_form())
            {
                switch(base)
                {
                case 2:
                    traits.append("0b");
                    break;
                [[unlikely]] case 8:
                    traits.append("0o");
                    break;
                case 16:
                    if(uppercase)
                        traits.append("0X");
                    else
                        traits.append("0x");
                    break;
                }
            }

            if(!spec.has_fill() && spec.fill_zero())
            {
                using enum format_align;
                if(spec.align() == right || spec.align() == default_align)
                {
                    std::size_t to_fill = spec.width();
                    to_fill = raw_num_len > to_fill ? 0 : to_fill - raw_num_len;
                    traits.append('0', to_fill);
                }
            }

            unsigned_value_type uval = val < 0 ? -val : val;
            char buf[sizeof(value_type) * 8];
            std::size_t buf_idx = raw_num_len;
            do
            {
                unsigned_value_type rem = uval % base;
                uval /= base;
                buf[--buf_idx] = map_char(rem, uppercase);
            } while(uval != 0);

            traits.append(buf, buf + raw_num_len);

            if(fill_back != 0)
                traits.append(spec.fill(), fill_back);
        }

    private:
        unsigned int m_base = 10;
        bool m_uppercase = false;
        common_format_spec m_spec;

        static std::size_t raw_formatted_size(value_type val, unsigned int base) noexcept
        {
            if constexpr(std::is_signed_v<value_type>)
            {
                if(val < 0)
                    val = -val;
            }

            unsigned_value_type uval = static_cast<unsigned_value_type>(val);
            std::size_t counter = 1;
            while(uval /= base)
                ++counter;
            return counter;
        }
    };

    template <>
    class formatter<utf8::codepoint>
    {
    public:
        void parse(format_spec_parse_context& ctx)
        {
            m_spec.parse(ctx);
            if(m_spec.align() == format_align::default_align)
                m_spec.align(format_align::left);
        }
        template <typename Context>
        void format(utf8::codepoint val, Context& ctx)
        {
            format_context_traits traits(ctx);

            if(auto type = m_spec.type_char_or(U'c'); type != U'c')
            {
                using int_formatter = formatter<std::uint32_t>;
                auto [base, uppercase] = int_formatter::get_base(type);

                int_formatter::format_impl(
                    static_cast<std::uint32_t>(val.to_int().first), ctx,
                    m_spec, base, uppercase
                );
                return;
            }

            std::size_t fill_front = 0;
            std::size_t fill_back = 0;
            if(std::size_t w = m_spec.width(); w != 0)
            {
                std::tie(fill_front, fill_back) = detail::calc_fill_width(
                    m_spec.align(),
                    w,
                    val.estimate_width()
                );
            }

            if(fill_front != 0)
                traits.append(m_spec.fill_or(U' '), fill_front);

            traits.append(val);

            if(fill_back != 0)
                traits.append(m_spec.fill_or(U' '), fill_back);
        }

    private:
        common_format_spec m_spec;
    };
    template <>
    class formatter<string_container>
    {
    public:
        void parse(format_spec_parse_context& ctx)
        {
            m_spec.parse(ctx);
            if(m_spec.type_char_or(U's') != U's')
                throw invalid_format("invalid string format");
            if(m_spec.width() != 0 && !m_spec.has_fill())
            {
                if(m_spec.fill_zero())
                    m_spec.fill(U'0');
                else
                    m_spec.fill(U' ');
            }
            if(m_spec.align() == format_align::default_align)
                m_spec.align(format_align::left);
        }
        template <typename Context>
        void format(const string_container& val, Context& ctx)
        {
            format_impl(val, ctx, m_spec);
        }

        // Internal API
        template <typename Context>
        static void format_impl(
            const string_container& val, Context& ctx,
            const common_format_spec& spec
        ) {
            format_context_traits traits(ctx);

            auto [str_end, est_width] = find_str_end(val, spec.precision());

            std::size_t fill_front = 0;
            std::size_t fill_back = 0;
            if(std::size_t w = spec.width(); w != 0)
            {
                std::tie(fill_front, fill_back) = detail::calc_fill_width(
                    spec.align(),
                    w,
                    est_width
                );
            }

            if(fill_front != 0)
                traits.append(spec.fill(), fill_front);

            for(auto it = val.begin(); it != str_end; ++it)
                traits.append(*it);

            if(fill_back != 0)
                traits.append(spec.fill(), fill_back);
        }

    private:
        common_format_spec m_spec;

        static std::pair<string_container::iterator, std::size_t> find_str_end(
            const string_container& str,
            std::size_t precision
        ) {
            std::size_t current_width = 0;
            auto it = str.begin();
            const auto end = str.end();
            for(; it != end; ++it)
            {
                std::size_t est_cp_width = (*it).estimate_width();
                if(current_width + est_cp_width > precision)
                    break;
                current_width += est_cp_width;
            }

            return std::make_pair(it, current_width);
        }
    };

    template <>
    class formatter<bool>
    {
    public:
        void parse(format_spec_parse_context& ctx)
        {
            m_spec.parse(ctx);
        }
        template <typename Context>
        void format(bool val, Context& ctx)
        {
            if(char32_t type = m_spec.type_char_or(U's'); type == U's')
            {
                using str_formatter = formatter<string_container>;
                str_formatter::format_impl(
                    get_name(val), ctx,
                    m_spec
                );
            }
            else
            {
                using int_formatter = formatter<unsigned int>;
                auto [base, uppercase] = int_formatter::get_base(type);
                int_formatter::format_impl(
                    static_cast<unsigned int>(val), ctx,
                    m_spec, base, uppercase
                );
            }
        }

        [[nodiscard]]
        string_container get_name(bool val) noexcept
        {
            if(val)
            {
                return "true";
            }
            else
            {
                return "false";
            }
        }

    private:
        common_format_spec m_spec;
    };

    template <>
    class formatter<const void*>
    {
    public:
        void parse(format_spec_parse_context& ctx)
        {
            m_spec.parse(ctx);
            if(m_spec.type_char_or(U'p') != U'p')
            {
                throw invalid_format("invalid pointer format");
            }

            m_spec.type_char(U'x');
            m_spec.alternate_form(true);
        }
        template <typename Context>
        void format(const void* val, Context& ctx)
        {
            using int_formatter = formatter<std::uintptr_t>;
            int_formatter::format_impl(
                reinterpret_cast<std::uintptr_t>(val), ctx,
                m_spec, 16, false
            );
        }

    private:
        common_format_spec m_spec;
    };

    namespace detail
    {
        class floating_pointer_formatter_base
        {
        public:
            static void nan_to_chars(char* buf, bool uppercase) noexcept
            {
                if(uppercase)
                {
                    buf[0] = 'N';
                    buf[1] = 'A';
                    buf[2] = 'N';
                }
                else
                {
                    buf[0] = 'n';
                    buf[1] = 'a';
                    buf[2] = 'n';
                }
            }
            static void inf_to_chars(char* buf, bool uppercase) noexcept
            {
                if(uppercase)
                {
                    buf[0] = 'I';
                    buf[1] = 'N';
                    buf[2] = 'F';
                }
                else
                {
                    buf[0] = 'i';
                    buf[1] = 'n';
                    buf[2] = 'f';
                }
            }
        };
    }

    template <std::floating_point Float>
    class formatter<Float> : public detail::floating_pointer_formatter_base
    {
    public:
        using value_type = Float;

        void parse(format_spec_parse_context& ctx)
        {
            m_spec.parse(ctx);
            if(!m_spec.has_type_char())
                return;
            if(char32_t type = m_spec.type_char(); type == U'a' || type == U'A')
            {
                m_chars_fmt = std::chars_format::hex;
                if(type == 'A')
                    m_uppercase = true;
            }
            else if(type == 'e' || type == 'E')
            {
                m_chars_fmt = std::chars_format::scientific;
                if(m_spec.precision() == -1)
                    m_spec.precision(6);
                if(type == 'E')
                    m_uppercase = true;
            }
            else if(type == 'f' || type == 'F')
            {
                m_chars_fmt = std::chars_format::fixed;
                if(m_spec.precision() == -1)
                    m_spec.precision(6);
            }
            else if(type == 'g' || type == 'G')
            {
                m_chars_fmt = std::chars_format::general;
                if(m_spec.precision() == -1)
                    m_spec.precision(6);
                if(type == 'G')
                    m_uppercase = true;
            }

            if(m_spec.width() != 0)
            {
                if(m_spec.align() == format_align::default_align)
                    m_spec.align(format_align::right);
                if(!m_spec.has_fill())
                {
                    if(!(m_spec.align() == format_align::right && m_spec.fill_zero()))
                        m_spec.fill(U' ');
                }
            }
        }
        template <typename Context>
        void format(Float val, Context& ctx)
        {
            format_impl(
                val, ctx,
                m_spec, m_chars_fmt, m_uppercase
            );
        }

        template <typename Context>
        static void format_impl(
            value_type val, Context& ctx,
            const common_format_spec& spec, std::chars_format chars_fmt, bool uppercase
        ) {
            char buf[128];
            char* p_end = buf;
            std::size_t raw_num_len = 0;
            bool is_special_val = false;

            if(std::isnan(val))
            {
                nan_to_chars(buf, uppercase);
                p_end += 3;
                is_special_val = true;
            }
            else if(std::isinf(val))
            {
                inf_to_chars(buf, uppercase);
                p_end += 3;
                is_special_val = true;
            }
            else
            {
                if(spec.precision() == -1)
                {
                    std::to_chars_result result = std::to_chars(
                        buf, buf + 128,
                        val,
                        chars_fmt
                    );
                    if(result.ec != std::errc())
                    {
                        throw std::system_error(std::make_error_code(result.ec));
                    }
                    p_end = result.ptr;
                }
                else
                {
                    std::to_chars_result result = std::to_chars(
                        buf, buf + 128,
                        val,
                        chars_fmt,
                        static_cast<int>(spec.precision())
                    );
                    if(result.ec != std::errc())
                    {
                        throw std::system_error(std::make_error_code(result.ec));
                    }
                    p_end = result.ptr;
                }

                raw_num_len = p_end - buf;

                if(uppercase)
                {
                    for(std::size_t i = 0; i < p_end - buf; ++i)
                    {
                        if('a' <= buf[i] && buf[i] <= 'z')
                        {
                            buf[i] -= 'a' - 'A';
                        }
                    }
                }
            }

            std::size_t num_len = 0;
            char sign_ch = get_sign(spec.sign(), std::signbit(val));
            if(sign_ch != '\0')
                ++num_len;
            num_len += p_end - buf;

            std::size_t fill_front = 0;
            std::size_t fill_back = 0;
            if(std::size_t w = spec.width(); w != 0)
            {
                std::tie(fill_front, fill_back) = detail::calc_fill_width(
                    spec.align(),
                    w,
                    num_len
                );
            }

            format_context_traits traits(ctx);

            if(fill_front != 0)
                traits.append(spec.fill(), fill_front);

            if(!spec.has_fill() && spec.fill_zero() && !is_special_val)
            {
                using enum format_align;
                if(spec.align() == right || spec.align() == default_align)
                {
                    std::size_t to_fill = spec.width();
                    to_fill = raw_num_len > to_fill ? 0 : to_fill - raw_num_len;
                    traits.append('0', to_fill);
                }
            }

            if(sign_ch != '\0')
                traits.append(sign_ch);
            traits.append(buf, p_end);

            if(fill_back != 0)
                traits.append(spec.fill(), fill_back);
        }

    private:
        common_format_spec m_spec;
        std::chars_format m_chars_fmt = std::chars_format::general;
        bool m_uppercase = false;

        // returns null character when sign character is not needed
        static char get_sign(format_sign sign, bool sign_bit)
        {
            switch(sign)
            {
                using enum format_sign;
            case positive:
                if(sign_bit)
                    return '-';
                else
                    return '+';

            default:
            case default_sign:
            [[likely]] case negative:
                if(sign_bit)
                    return '-';
                else
                    return '\0';

            case space:
                if(sign_bit)
                    return '-';
                else
                    return ' ';
            }
        }
    };
}
