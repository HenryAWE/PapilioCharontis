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

        private:
            script::executor m_ex;
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
}
