#pragma once

#include <string_view>


namespace papilio::script
{
    template <typename CharT>
    class basic_keywords
    {
    public:
        typedef CharT value_type;
        typedef std::basic_string_view<value_type> string_view_type;

        static string_view_type if_() noexcept
        {
            static constexpr const value_type str[] =
            {
                value_type('i'),
                value_type('f')
            };

            return string_view_type(str, std::size(str));
        }
        static string_view_type else_() noexcept
        {
            static constexpr const value_type str[] =
            {
                value_type('e'),
                value_type('l'),
                value_type('s'),
                value_type('e')
            };

            return string_view_type(str, std::size(str));
        }
        static string_view_type elif() noexcept
        {
            static constexpr const value_type str[] =
            {
                value_type('e'),
                value_type('l'),
                value_type('i'),
                value_type('f')
            };

            return string_view_type(str, std::size(str));
        }
        static string_view_type and_() noexcept
        {
            static constexpr const value_type str[] =
            {
                value_type('a'),
                value_type('n'),
                value_type('d')
            };

            return string_view_type(str, std::size(str));
        }
        static string_view_type or_() noexcept
        {
            static constexpr const value_type str[] =
            {
                value_type('o'),
                value_type('r')
            };

            return string_view_type(str, std::size(str));
        }
    };

    typedef basic_keywords<char> keywords;
    typedef basic_keywords<wchar_t> wkeywords;
    typedef basic_keywords<char16_t> u16keywords;
    typedef basic_keywords<char32_t> u32keywords;
    typedef basic_keywords<char8_t> u8keywords;

    template <typename CharT>
    bool is_keyword(std::basic_string_view<CharT> word)
    {
        using K = basic_keywords<CharT>;
        return
            word == K::if_() ||
            word == K::else_() ||
            word == K::elif() ||
            word == K::and_() ||
            word == K::or_();
    }
    template <typename CharT>
    bool is_keyword(const CharT* word)
    {
        return is_keyword(std::basic_string_view<CharT>(word));
    }
}
