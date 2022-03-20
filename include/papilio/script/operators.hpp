#pragma once

#include <string_view>


namespace papilio::script
{
    template <typename CharT>
    class basic_operators
    {
    public:
        typedef CharT value_type;
        typedef std::basic_string_view<value_type> string_view_type;

        static string_view_type op_equal() noexcept
        {
            static constexpr const value_type str[] =
            {
                value_type('='),
                value_type('=')
            };

            return string_view_type(str, std::size(str));
        }
        static string_view_type op_not_equal() noexcept
        {
            static constexpr const value_type str[] =
            {
                value_type('!'),
                value_type('=')
            };

            return string_view_type(str, std::size(str));
        }
        static string_view_type op_greater_than() noexcept
        {
            static constexpr const value_type str[] =
            {
                value_type('>')
            };

            return string_view_type(str, std::size(str));
        }
        static string_view_type op_greater_equal() noexcept
        {
            static constexpr const value_type str[] =
            {
                value_type('>'),
                value_type('=')
            };

            return string_view_type(str, std::size(str));
        }
        static string_view_type op_less_than() noexcept
        {
            static constexpr const value_type str[] =
            {
                value_type('<')
            };

            return string_view_type(str, std::size(str));
        }
        static string_view_type op_less_equal() noexcept
        {
            static constexpr const value_type str[] =
            {
                value_type('<'),
                value_type('=')
            };

            return string_view_type(str, std::size(str));
        }
        static string_view_type op_condition_end() noexcept
        {
            static constexpr const value_type str[] =
            {
                value_type(':')
            };

            return string_view_type(str, std::size(str));
        }
    };

    typedef basic_operators<char> operators;
    typedef basic_operators<wchar_t> woperators;
    typedef basic_operators<char16_t> u16operators;
    typedef basic_operators<char32_t> u32operators;
    typedef basic_operators<char8_t> u8operators;

    template <typename CharT>
    bool is_comparator(std::basic_string_view<CharT> word) noexcept
    {
        using Op = basic_operators<CharT>;
        return
            word == Op::op_equal() ||
            word == Op::op_not_equal() ||
            word == Op::op_greater_than() ||
            word == Op::op_greater_equal() ||
            word == Op::op_less_than() ||
            word == Op::op_less_equal();
    }
    template <typename CharT>
    bool is_comparator(const std::basic_string<CharT>& word) noexcept
    {
        return is_comparator(std::basic_string_view<CharT>(word));
    }
    template <typename CharT>
    bool is_comparator(const CharT* word) noexcept
    {
        return is_comparator(std::basic_string_view<CharT>(word));
    }
    template <typename CharT>
    bool is_operator(std::basic_string_view<CharT> word) noexcept
    {
        using Op = basic_operators<CharT>;
        return
            is_comparator(word) ||
            word == Op::op_condition_end();
    }
    template <typename CharT>
    bool is_operator(const CharT* word) noexcept
    {
        return is_operator(std::basic_string_view<CharT>(word));
    }
}
