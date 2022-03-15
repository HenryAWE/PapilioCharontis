#pragma once

#include <cassert>
#include <algorithm>
#include <string>
#include <vector>
#include "error.hpp"
#include "literal.hpp"
#include "keywords.hpp"
#include "operators.hpp"
#include "../utility.hpp"


namespace papilio::script
{
    enum class lexeme_type
    {
        keyword,
        operator_,
        identifier,
        literal
    };

    template <typename CharT>
    class basic_lexeme
    {
    public:
        typedef CharT value_type;
        typedef std::basic_string<value_type> string_type;
        typedef std::basic_string_view<value_type> string_view_type;

        basic_lexeme(lexeme_type type_, string_type str_) noexcept
            : m_type(type_), m_str(std::move(str_)) {}

        constexpr lexeme_type type() const noexcept { return m_type; }
        constexpr const string_type& str() const noexcept { return m_str; }

    private:
        lexeme_type m_type;
        string_type m_str;
    };

    template <typename CharT>
    class basic_lexer
    {
    public:
        typedef CharT value_type;
        typedef std::basic_string<value_type> string_type;
        typedef std::basic_string_view<value_type> string_view_type;
        typedef basic_lexeme<value_type> lexeme;

        void parse(string_view_type src)
        {
            auto it = src.begin();
            while(it != src.end())
            {
                value_type c = *it;
                if(is_whitespace(c))
                {
                    ++it;
                    continue;
                }
                else if(c == value_type('@'))
                {
                    ++it;
                    if(it == src.end())
                        break;
                    if(is_digit(*it))
                    {
                        auto identifier_end = std::find_if_not(
                            it, src.end(),
                            [this](value_type v) { return is_digit(v); }
                        );
                        m_lexemes.emplace_back(
                            lexeme_type::identifier,
                            string_type(it, identifier_end)
                        );
                        it = identifier_end;
                        continue;
                    }    
                }
                else if(is_keyword_char(c))
                {
                    auto keyword_end = std::find_if_not(
                        it, src.end(),
                        [this](value_type v) { return is_keyword_char(v); }
                    );
                    // Check
                    string_view_type kw_sv(it, keyword_end);
                    it = keyword_end;
                    if(is_keyword(kw_sv))
                    {
                        m_lexemes.emplace_back(
                            lexeme_type::keyword,
                            string_type(kw_sv)
                        );
                    }
                    else
                    {
                        throw syntax_error();
                    }
                }
                else if(is_operator_char(c))
                {
                    auto operator_end = std::find_if_not(
                        it, src.end(),
                        [this](value_type v) { return is_operator_char(v); }
                    );
                    // Check
                    string_view_type op_sv(it, operator_end);
                    it = operator_end;
                    if(is_operator(op_sv))
                    {
                        m_lexemes.emplace_back(
                            lexeme_type::operator_,
                            string_type(op_sv)
                        );
                    }
                    else
                    {
                        throw syntax_error();
                    }
                }
                else if(c == value_type('"'))
                {
                    std::string literal;
                    it = read_string_literal(
                        it, src.end(),
                        std::back_inserter(literal)
                    );
                    ++it; // Skip quote at the end

                    m_lexemes.emplace_back(
                        lexeme_type::literal,
                        std::move(literal)
                    );
                }
                else
                {
                    ++it;
                }
            }
        }

        std::span<lexeme> lexemes() noexcept
        {
            return m_lexemes;
        }

    private:
        std::vector<lexeme> m_lexemes;

        static bool is_whitespace(value_type c)
        {
            return detailed::is_whitespace(c);
        }
        static constexpr bool is_digit(value_type c) noexcept
        {
            return c <= value_type('9') && c >= value_type('0');
        }
        static bool is_keyword_char(value_type c) noexcept
        {
            return c <= value_type('z') && c >= value_type('A');
        }
        static bool is_operator_char(value_type c) noexcept
        {
            return
                c == value_type(':') ||
                c == value_type('=') ||
                c == value_type('<') ||
                c == value_type('>') ||
                c == value_type('!');
        }

        template <typename InputIt, typename OutputIt>
        InputIt read_string_literal(InputIt begin, InputIt end, OutputIt out)
        {
            assert(*begin == value_type('"'));

            ++begin; // Skip the quote at the beginning

            bool escaping = false;
            for(auto it = begin; it != end; ++it)
            {
                if(*it == value_type('\\') && !escaping)
                {
                    escaping = true;
                    continue;
                }
                else if(*it == value_type('"') && !escaping)
                {
                    return it;
                }

                *out = *it;
                escaping = false;
            }

            // Missing closing quote at the right
            throw syntax_error();
        }
    };

    typedef basic_lexer<char> lexer;
    typedef basic_lexer<wchar_t> wlexer;
    typedef basic_lexer<char16_t> u16lexer;
    typedef basic_lexer<char32_t> u32lexer;
    typedef basic_lexer<char8_t> u8lexer;
}
