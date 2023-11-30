#pragma once

#include <string_view>
#include "../access.hpp"
#include "../utf/string.hpp"


namespace papilio
{
    template <typename CharT>
    struct accessor<utf::basic_string_container<CharT>>
    {
        using string_container_type = utf::basic_string_container<CharT>;
        using attribute_name_type = basic_attribute_name<CharT>;

        static utf::codepoint index(const string_container_type& str, indexing_value::index_type i)
        {
            if(i >= 0)
                return str.index_or(i, utf::codepoint());
            else
            {
                i = -(i + 1);
                return str.index_or(reverse_index, i, utf::codepoint());
            }
        }

        static utf::string_container index(const string_container_type& str, slice s)
        {
            return str.substr<utf::substr_behavior::empty_string>(s);
        }

        static std::size_t attribute(const string_container_type& str, const attribute_name_type& attr)
        {
            using namespace std::literals;

            if(attr == "length"sv)
                return str.length();
            else if(attr == "size"sv)
                return str.size();
            else
                throw_invalid_attribute(attr);
        }
    };
}
