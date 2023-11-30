#pragma once

#include "core.hpp"
#include <concepts>
#include <system_error>
#include <charconv>
#include <sstream>
#include "iterstream.hpp"


namespace papilio
{
    namespace detail
    {
        template <typename T>
        format_arg handle_impl<T>::index(const indexing_value& idx) const
        {
            return idx.visit(
                [this](auto&& i) { return accessor_traits<T>::template index<format_arg>(*m_ptr, i); }
            );
        }
        template <typename T>
        format_arg handle_impl<T>::attribute(const attribute_name& attr) const
        {
            return accessor_traits<T>::template attribute<format_arg>(*m_ptr, attr);
        }
    }
}
