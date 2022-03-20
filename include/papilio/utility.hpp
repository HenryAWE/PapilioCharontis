#pragma once


namespace papilio
{
    namespace detailed
    {
        template <typename CharT>
        bool is_whitespace(CharT c) noexcept
        {
            switch(c)
            {
            default:
                return false;
            case CharT(' '):
            case CharT('\n'):
            case CharT('\t'):
                return true;
            }
        }
    }
}
