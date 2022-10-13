#pragma once

#include "core.hpp"
#include <concepts>
#include <system_error>
#include <charconv>


namespace papilio
{
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
        void format(value_type val, format_context& ctx) const\
        {\
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
            ctx.append(buf, result.ptr);\
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
