#pragma once

#include <stdexcept>
#include <string>
#include <iterator>
#include <vector>
#include <span>
#include "utility.hpp"
#include "block.hpp"
#include "script/lexer.hpp"


namespace papilio
{
    namespace detailed
    {
        /* Converting escape sequence into standalone character */
        template <typename Iterator>
        class format_string_reader
        {
        public:
            typedef Iterator underlying_type;
            typedef std::iterator_traits<Iterator>::value_type value_type;
            typedef std::char_traits<value_type> traits_type;

            format_string_reader() = default;
            format_string_reader(Iterator begin, Iterator end)
                : m_begin(begin), m_current(begin), m_end(end) {}

            void assign(Iterator begin, Iterator end)
            {
                m_begin = begin;
                m_current = begin;
                m_end = end;
            }

            value_type get()
            {
                if (m_current == m_end)
                    return eof();
                value_type c = *m_current;
                ++m_current;
                return c;
            }
            value_type peek()
            {
                Iterator saved = m_current;
                value_type c = get();
                m_current = saved;

                return c;
            }

            static value_type eof()
            {
                return traits_type::eof();
            }

            underlying_type current()
            {
                return m_current;
            }

        private:
            Iterator m_begin, m_current, m_end;
        };
    }

    template <typename CharT>
    class basic_format_parser
    {
    public:
        typedef CharT value_type;
        typedef std::basic_string<CharT> string_type;
        typedef std::basic_string_view<CharT> string_view_type;
        typedef basic_block<value_type> block;

        void parse(string_view_type input)
        {
            m_blocks = build_blocks(input);
        }

        std::span<block> blocks() noexcept
        {
            return m_blocks;
        }

    private:
        std::vector<block> m_blocks;

        std::vector<block> build_blocks(string_view_type input)
        {
            std::vector<block> result;

            detailed::format_string_reader reader(input.begin(), input.end());
            value_type c;
            string_type current_string;
            block_type current_type = block_type::text;
            while((c = reader.get()) != reader.eof())
            {
                bool is_key_char = false;
                switch(c)
                {
                case value_type('{'):
                case value_type('['):
                    if(current_type == block_type::text && c != reader.peek())
                    {
                        result.emplace_back(current_type, std::move(current_string));
                        if(c == value_type('{'))
                        {
                            current_type = block_type::relpacement_field;
                        }
                        else if(c == value_type('['))
                        {
                            current_type = block_type::script;
                        }
                        is_key_char = true;
                    }
                    else
                    {
                        reader.get();
                    }
                    break;

                case value_type('}'):
                case value_type(']'):
                    if(current_type != block_type::text && c != reader.peek())
                    {
                        result.emplace_back(current_type, std::move(current_string));
                        current_type = block_type::text;
                        is_key_char = true;
                    }
                    else
                    {
                        reader.get();
                    }
                    break;
                }

                if(!is_key_char)
                    current_string += c;
            }

            return result;
        }
    };

    typedef basic_format_parser<char> format_parser;
    typedef basic_format_parser<wchar_t> wformat_parser;
    typedef basic_format_parser<char16_t> u16format_parser;
    typedef basic_format_parser<char32_t> u32format_parser;
    typedef basic_format_parser<char8_t> u8format_parser;
}
