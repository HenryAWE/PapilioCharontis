#pragma once

#include <string_view>
#include "../access.hpp"
#include "../utf/string.hpp"

namespace papilio
{
template <typename Context>
struct accessor<utf::basic_string_container<typename Context::char_type>, Context>
{
    using char_type = typename Context::char_type;
    using format_arg_type = basic_format_arg<Context>;
    using string_view_type = std::basic_string_view<char_type>;
    using string_container_type = utf::basic_string_container<char_type>;
    using attribute_name_type = basic_attribute_name<char_type>;

    [[nodiscard]]
    static utf::codepoint index(const string_container_type& str, ssize_t i)
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
    static format_arg_type attribute(const string_container_type& str, const attribute_name_type& attr)
    {
        if(attr == PAPILIO_TSTRING_VIEW(char_type, "length"))
            return str.length();
        else if(attr == PAPILIO_TSTRING_VIEW(char_type, "size"))
            return str.size();
        else
            throw_invalid_attribute(attr);
    }
};
} // namespace papilio
