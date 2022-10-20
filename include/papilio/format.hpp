#pragma once

#include "core.hpp"
#include "script.hpp"


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

#   define PAPILIO_IMPL_INTEGER_FORMATTER(int_type) \
    template <>\
    class formatter<int_type>\
    {\
    public:\
        using char_type = char;\
        using value_type = int_type;\
        void parse(format_spec_parse_context& ctx)\
        {\
            m_spec = detail::parse_std_format_spec(ctx);\
            if(m_spec.align == format_align::default_align)\
                m_spec.align = format_align::right;\
            if(m_spec.type_char == '\0' || m_spec.type_char == 'd')\
                m_base = 10;\
            else if(m_spec.type_char == 'x')\
                m_base = 16;\
            else if(m_spec.type_char == 'b')\
                m_base = 2;\
            else if(m_spec.type_char == 'o')\
                m_base = 8;\
            else\
                throw invalid_format("invalid type specifier");\
        }\
        template <typename Context>\
        void format(value_type val, Context& ctx) const\
        {\
            format_context_traits traits(ctx);\
            constexpr std::size_t bufsize = sizeof(value_type) * 8 + 8;\
            char_type buf[bufsize];\
            auto result = std::to_chars(\
                buf, buf + bufsize,\
                val,\
                m_base\
            );\
            if(result.ec != std::errc())\
            {\
                throw std::system_error(std::make_error_code(result.ec));\
            }\
            traits.append(buf, result.ptr);\
        }\
    private:\
        int m_base = 10;\
        detail::std_format_spec m_spec;\
    };

    PAPILIO_IMPL_INTEGER_FORMATTER(signed char);
    PAPILIO_IMPL_INTEGER_FORMATTER(unsigned char);
    PAPILIO_IMPL_INTEGER_FORMATTER(short);
    PAPILIO_IMPL_INTEGER_FORMATTER(unsigned short);
    PAPILIO_IMPL_INTEGER_FORMATTER(int);
    PAPILIO_IMPL_INTEGER_FORMATTER(unsigned int);
    PAPILIO_IMPL_INTEGER_FORMATTER(long);
    PAPILIO_IMPL_INTEGER_FORMATTER(unsigned long);
    PAPILIO_IMPL_INTEGER_FORMATTER(long long);
    PAPILIO_IMPL_INTEGER_FORMATTER(unsigned long long);
}
