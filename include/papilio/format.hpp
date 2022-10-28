#pragma once

#include "core.hpp"
#include "script.hpp"
#include <cmath>


namespace papilio
{
    class format_parser
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;
        using iterator = string_view_type::const_iterator;

        class segment {};

        class plain_text : public segment
        {
        public:
            plain_text() = delete;
            plain_text(string_type text)
                : m_text(std::move(text)) {}

            [[nodiscard]]
            const string_type& get() const& noexcept
            {
                return m_text;
            }
            [[nodiscard]]
            string_type get() && noexcept
            {
                return std::move(m_text);
            }

            template <typename Context>
            void format(Context& ctx) const
            {
                format_context_traits traits(ctx);
                traits.append(m_text);
            }

        private:
            string_type m_text;
        };

        class replacement_field : public segment
        {
        public:
            replacement_field() = delete;
            replacement_field(indexing_value arg, format_arg_access access, string_type fmt = string_type())
                : m_arg(std::move(arg)), m_access(std::move(access)), m_fmt(std::move(fmt)) {}

            [[nodiscard]]
            const indexing_value& get_arg() const noexcept
            {
                return m_arg;
            }
            const format_arg_access& get_access() const noexcept
            {
                return m_access;
            }
            [[nodiscard]]
            const string_type& get_fmt() const noexcept
            {
                return m_fmt;
            }

            template <typename Context>
            void format(Context& ctx) const
            {
                format_context_traits traits(ctx);
                format_spec_parse_context spec(m_fmt, traits.get_store());

                auto& store = traits.get_store();

                m_access.access(store.get(m_arg)).format(spec, ctx);
            }

        private:
            indexing_value m_arg;
            format_arg_access m_access;
            string_type m_fmt;
        };

        class script_block : public segment
        {
        public:
            script_block() = delete;
            script_block(script::executor ex)
                : m_ex(std::move(ex)) {}

            string_type operator()(script::executor::context& ctx) const
            {
                m_ex(ctx);
                return ctx.get_result();
            }

            template <typename Context>
            void format(Context& ctx) const
            {
                format_context_traits traits(ctx);
                script::executor::context ex_ctx(traits.get_store());
                m_ex(ex_ctx);

                traits.append(ex_ctx.get_result());
            }

        private:
            mutable script::executor m_ex;
        };

        using segment_store = std::variant<
            plain_text,
            replacement_field,
            script_block
        >;

        void parse(string_view_type str, const dynamic_format_arg_store& store);

        std::span<const segment_store> segments() const noexcept
        {
            return m_segments;
        }

    private:
        std::vector<segment_store> m_segments;

        template <typename T, typename... Args>
        void push_segment(Args&&... args)
        {
            m_segments.emplace_back(
                std::in_place_type<T>,
                std::forward<Args>(args)...
            );
        }

        std::pair<indexing_value, format_arg_access> build_arg_access(
            std::span<const script::lexeme> lexemes
        );
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
    }

    namespace detail
    {
        // front and back
        std::pair<std::size_t, std::size_t> calc_fill_width(format_align align, std::size_t width, std::size_t current)
        {
            if(width <= current)
                return std::make_pair(0, 0);

            std::size_t fill_front = 0;
            std::size_t fill_back = 0;
            std::size_t to_fill = width - current;

            using enum format_align;
            switch(align)
            {
            case left:
                fill_back = to_fill;
                break;

            [[likely]] case right:
                fill_front = to_fill;
                break;

            case middle:
                fill_front = to_fill / 2; // floor(to_fill / 2)
                fill_back = to_fill / 2 + to_fill % 2; // ceil(to_fill / 2)
                break;
            }

            return std::make_pair(fill_front, fill_back);
        }
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
        static std::pair<unsigned int, bool> get_base(utf8::codepoint type)
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
                throw invalid_format("invalid type '" + std::string(type) + "' for integer");
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
        void parse(format_spec_parse_context& spec)
        {
            m_spec = detail::parse_std_format_spec(spec);
            if(m_spec.type_char != 's' && m_spec.type_char != '\0')
                throw invalid_format("invalid string format");
        }
        template <typename Context>
        void format(const string_container& val, Context& ctx)
        {
            format_context_traits traits(ctx);

            traits.append(val);
        }

    private:
        detail::std_format_spec m_spec;
    };

    template <>
    class formatter<bool>
    {
    public:
        void parse(format_spec_parse_context& spec)
        {
            m_spec = detail::parse_std_format_spec(spec);
        }
        template <typename Context>
        void format(bool val, Context& ctx)
        {
            format_context_traits traits(ctx);

            val ? traits.append("true") : traits.append("false");
        }

    private:
        detail::std_format_spec m_spec;
    };

    template <>
    class formatter<const void*>
    {
    public:
        void parse(format_spec_parse_context& spec)
        {
            std::string_view view = spec;
            if(!view.empty() && view != "p")
            {
                throw invalid_format("invalid pointer format");
            }
        }
        template <typename Context>
        void format(const void* val, Context& ctx)
        {
            auto int_val = reinterpret_cast<std::uintptr_t>(val);
            char buf[16];
            auto result = std::to_chars(buf, buf + 16, int_val, 16);

            format_context_traits traits(ctx);
            traits.append("0x");
            traits.append(buf, result.ptr);
        }

    private:
        detail::std_format_spec m_spec;
    };

    template <std::floating_point Float>
    class formatter<Float>
    {
    public:
        void parse(format_spec_parse_context& spec)
        {
            m_spec = detail::parse_std_format_spec(spec);
            if(m_spec.type_char == 'f')
            {
                m_chars_fmt = std::chars_format::fixed;
            }
            else if(m_spec.type_char == '\0')
            {
                m_chars_fmt = std::chars_format::general;
            }
        }
        template <typename Context>
        void format(Float val, Context& ctx)
        {
            format_context_traits traits(ctx);

            char buf[128];
            auto result = std::to_chars(buf, buf + 128, val, m_chars_fmt);
            if(result.ec != std::errc())
            {
                throw std::system_error(std::make_error_code(result.ec));
            }
            traits.append(buf, result.ptr);
        }

    private:
        detail::std_format_spec m_spec;
        std::chars_format m_chars_fmt = std::chars_format::general;
    };
}
