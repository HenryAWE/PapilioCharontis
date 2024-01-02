#pragma once

#include <string_view>
#include <span>
#include <ranges>
#include <vector>
#include "../access.hpp"
#include "../utility.hpp"

namespace papilio
{
PAPILIO_EXPORT template <std::ranges::contiguous_range Range, typename Context>
struct contiguous_range_accessor
{
    using char_type = typename Context::char_type;
    using format_arg_type = basic_format_arg<Context>;
    using attribute_name_type = basic_attribute_name<char_type>;

    static format_arg_type index(const Range& rng, ssize_t i)
    {
        namespace stdr = std::ranges;

        if(i < 0)
        {
            i = stdr::ssize(rng) + i;
            if(i < 0) [[unlikely]]
                return format_arg_type();
        }

        if(i >= stdr::ssize(rng))
            return format_arg_type();

        return stdr::cdata(rng)[i];
    }

    static format_arg_type index(const Range& rng, slice s)
    {
        namespace stdr = std::ranges;
        auto ptr = stdr::cdata(rng);

        using span_t = std::span<std::add_const_t<stdr::range_value_t<Range>>>;

        std::size_t sz = stdr::size(rng);
        s = s.normalize(sz);

        if(s.first >= sz)
            return format_arg_type();
        else if(s.second > sz)
            s.second = sz;

        if(s.first >= s.second)
            return format_arg_type();

        return span_t(ptr + s.first, s.length());
    }

    static format_arg_type attribute(const Range& rng, const attribute_name_type& attr)
    {
        if(attr == PAPILIO_TSTRING_VIEW(char_type, "size"))
            return static_cast<std::size_t>(std::ranges::size(rng));

        throw_invalid_attribute(attr);
    }
};

PAPILIO_EXPORT template <typename T, typename Context>
struct accessor<std::span<T>, Context> :
    public contiguous_range_accessor<std::span<T>, Context>
{};

PAPILIO_EXPORT template <typename T, typename Allocator, typename Context>
requires(!std::is_same_v<T, bool>) // Avoid std::vector<bool>
struct accessor<std::vector<T, Allocator>, Context> :
    public contiguous_range_accessor<std::vector<T, Allocator>, Context>
{};
} // namespace papilio
