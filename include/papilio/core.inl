#pragma once

#include "core.hpp"
#include <concepts>
#include <system_error>
#include <charconv>


namespace papilio
{
    namespace detail
    {
        template <string_like String>
        auto string_accessor::get(const String& str, indexing_value::index_type i)
        {
            if(i < 0)
                return utf8::rindex(str, -(i + 1));
            else
                return utf8::index(str, i);
        }

        template <string_like String>
        auto string_accessor::get(const String& str, slice s)
        {
            return utf8::substr(str, s);
        }

        template <string_like String>
        bool string_accessor::has_attr(const String&, const attribute_name& attr)
        {
            using namespace std::literals;
            return
                attr == "length"sv ||
                attr == "size"sv;
        }
        template <string_like String>
        format_arg string_accessor::get_attr(const String& str, const attribute_name& attr)
        {
            using namespace std::literals;
            if(attr == "length"sv)
                return format_arg(utf8::strlen(str));
            else if(attr == "size"sv)
                return format_arg(str.size());
            else
                throw invalid_attribute(attr);
        }
    }

    template <>
    struct accessor<string_container>
    {
        using has_index = void;
        using has_slice = void;

        static utf8::codepoint get(const string_container& str, indexing_value::index_type i)
        {
            return str[i];
        }

        static string_container get(const string_container& str, slice s)
        {
            return str.substr(s);
        }

        static format_arg get_attr(string_container str, const attribute_name& attr)
        {
            using namespace std::literals;

            if(attr == "length"sv)
                return str.length();
            else if(attr == "size")
                return str.size();
            else
                throw invalid_attribute(attr);
        }
    };

    template <typename T>
    template <typename U>
    format_arg accessor_traits<T>::get_arg(U&& object, const indexing_value& idx)
    {
        auto visitor = [&object]<typename Arg>(Arg&& v)->format_arg
        {
            using result_type = decltype(
                index_handler(std::forward<U>(object), std::forward<Arg>(v))
            );
            if constexpr(std::is_void_v<result_type>)
            {
                // trigger exception
                get(std::forward<U>(object), std::forward<Arg>(v));
                // placeholder
                return format_arg();
            }
            else
            {
                return format_arg(get(std::forward<U>(object), std::forward<Arg>(v)));
            }
        };
        return std::visit(visitor, idx.to_underlying());
    }
    template <typename T>
    template <typename U>
    format_arg  accessor_traits<T>::get_attr(U&& object, const attribute_name& attr)
    {
        if constexpr(requires() { accessor_type::get_attr(std::forward<U>(object), attr); })
        {
            return accessor_type::get_attr(std::forward<U>(object), attr);
        }
        else
            throw invalid_attribute(attr);
    }

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
