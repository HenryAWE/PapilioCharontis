#pragma once

#include "core.hpp"
#include <concepts>
#include <system_error>
#include <charconv>


namespace papilio
{
    namespace detail
    {
        template <typename T>
        format_arg handle_impl<T>::index(const indexing_value& idx) const
        {
            return accessor_traits<T>::get_arg(*m_ptr, idx);
        }
        template <typename T>
        format_arg handle_impl<T>::attribute(const attribute_name& attr) const
        {
            return accessor_traits<T>::get_attr(*m_ptr, attr);
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

        static format_arg get_attr(const string_container& str, const attribute_name& attr)
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
}
