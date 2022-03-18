#pragma once

#include "context.hpp"
#include "lexer.hpp"


namespace papilio::script
{
    template <typename CharT>
    class basic_compiler
    {
    public:
        typedef CharT char_type;
        typedef basic_keywords<char_type> keywords;
        typedef basic_operators<char_type> operators;
        typedef basic_lexeme<char_type> lexeme;
    };

    typedef basic_compiler<char> compiler;
    typedef basic_compiler<wchar_t> wcompiler;
    typedef basic_compiler<char16_t> u16compiler;
    typedef basic_compiler<char32_t> u32compiler;
    typedef basic_compiler<char8_t> u8compiler;
}
