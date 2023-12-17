#pragma once

#include <string_view>
#include "../access.hpp"
#include "../utf/string.hpp"

namespace papilio
{
template <typename CharT>
struct accessor<utf::basic_string_container<CharT>>
{
    using char_type = CharT;
    using string_view_type = std::basic_string_view<CharT>;
    using string_container_type = utf::basic_string_container<CharT>;
    using attribute_name_type = basic_attribute_name<CharT>;

    [[nodiscard]]
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

    [[nodiscard]]
    static string_container_type index(const string_container_type& str, slice s)
    {
        return str.template substr<utf::substr_behavior::empty_string>(s);
    }

    [[nodiscard]]
    static std::size_t attribute(const string_container_type& str, const attribute_name_type& attr)
    {
        using namespace std::literals;

        constexpr CharT attr_length[] = {'l', 'e', 'n', 'g', 't', 'h'};
        constexpr CharT attr_size[] = {'s', 'i', 'z', 'e'};

        if(attr == string_view_type(attr_length, std::size(attr_length)))
            return str.length();
        else if(attr == string_view_type(attr_size, std::size(attr_size)))
            return str.size();
        else
            throw_invalid_attribute(attr);
    }
};
} // namespace papilio
