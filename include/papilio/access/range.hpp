#pragma once

#include <string_view>
#include <span>
#include <ranges>
#include <vector>
#include <map>
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

    [[nodiscard]]
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

    [[nodiscard]]
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

    [[nodiscard]]
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

PAPILIO_EXPORT template <typename T, std::size_t Capacity, typename Context>
struct accessor<fixed_vector<T, Capacity>, Context> :
    public contiguous_range_accessor<fixed_vector<T, Capacity>, Context>
{};

PAPILIO_EXPORT template <typename T, std::size_t Capacity, typename Allocator, typename Context>
struct accessor<small_vector<T, Capacity, Allocator>, Context> : 
    public contiguous_range_accessor<small_vector_base<T, Allocator>, Context>
{};

// Specialization for std::vector<bool>
PAPILIO_EXPORT template <typename Allocator, typename Context>
struct accessor<std::vector<bool, Allocator>, Context>
{
    using char_type = typename Context::char_type;
    using attribute_name_type = basic_attribute_name<char_type>;
    using format_arg_type = basic_format_arg<Context>;

    using vector_type = std::vector<bool, Allocator>;

    static format_arg_type index(const vector_type& vec, ssize_t i)
    {
        if(i < 0)
        {
            i = vec.size() + i;
            if(i < 0)
                return format_arg_type();
        }

        if(i >= vec.size())
            return format_arg_type();

        return format_arg_type(std::in_place_type<bool>, vec[i]);
    }

    static format_arg_type attribute(const vector_type& vec, const attribute_name_type& attr)
    {
        if(attr == PAPILIO_TSTRING_VIEW(char_type, "size"))
            return vec.size();

        throw_invalid_attribute(attr);
    }
};

namespace detail
{
    template <typename Compare>
    struct cp_is_less : public std::false_type
    {};

    template <typename T>
    struct cp_is_less<std::less<T>> : public std::true_type
    {};

    template <typename Compare>
    struct cp_is_greater : public std::false_type
    {};

    template <typename T>
    struct cp_is_greater<std::greater<T>> : public std::true_type
    {};
} // namespace detail

template <typename MapType, typename Context>
requires(map_like<MapType>)
struct map_accessor
{
    using char_type = typename Context::char_type;
    using format_arg_type = basic_format_arg<Context>;
    using attribute_name_type = basic_attribute_name<char_type>;
    using string_view_type = std::basic_string_view<char_type>;

    using key_type = typename MapType::key_type;
    using mapped_type = typename MapType::mapped_type;

    [[nodiscard]]
    static format_arg_type index(const MapType& m, ssize_t i)
        requires(std::integral<key_type>)
    {
        if constexpr(std::is_unsigned_v<key_type>)
        {
            if(i < 0)
                return format_arg_type();
        }

        auto it = m.find(static_cast<key_type>(i));
        return it != m.end() ?
                   static_cast<mapped_type>(it->second) :
                   format_arg_type();
    }

    [[nodiscard]]
    static format_arg_type index(const MapType& m, string_view_type k)
        requires(basic_string_like<key_type, char_type>)
    {
        if constexpr(is_transparent_v<typename MapType::key_compare>)
        {
            auto it = m.find(k);
            return it != m.end() ?
                       static_cast<mapped_type>(it->second) :
                       format_arg_type();
        }
        else
        {
            auto it = m.find(static_cast<key_type>(k));
            return it != m.end() ?
                       static_cast<mapped_type>(it->second) :
                       format_arg_type();
        }
    }

    [[nodiscard]]
    static format_arg_type attribute(const MapType& m, const attribute_name_type& attr)
    {
        if(attr == PAPILIO_TSTRING_VIEW(char_type, "size"))
        {
            return m.size();
        }
        if constexpr(detail::cp_is_less<typename MapType::key_compare>::value)
        {
            if(attr == PAPILIO_TSTRING_VIEW(char_type, "min"))
            {
                return !m.empty() ? m.begin()->second : format_arg_type();
            }
            else if(attr == PAPILIO_TSTRING_VIEW(char_type, "max"))
            {
                return !m.empty() ? std::prev(m.end())->second : format_arg_type();
            }
        }
        if constexpr(detail::cp_is_greater<typename MapType::key_compare>::value)
        {
            if(attr == PAPILIO_TSTRING_VIEW(char_type, "max"))
            {
                return !m.empty() ? m.begin()->second : format_arg_type();
            }
            else if(attr == PAPILIO_TSTRING_VIEW(char_type, "min"))
            {
                return !m.empty() ? std::prev(m.end())->second : format_arg_type();
            }
        }

        throw_invalid_attribute(attr);
    }
};

PAPILIO_EXPORT template <typename Key, typename T, typename Compare, typename Allocator, typename Context>
struct accessor<std::map<Key, T, Compare, Allocator>, Context> :
    public map_accessor<std::map<Key, T, Compare, Allocator>, Context>
{};

PAPILIO_EXPORT template <typename Key, typename T, std::size_t Capacity, typename Compare, typename Context>
struct accessor<fixed_flat_map<Key, T, Capacity, Compare>, Context> :
    public map_accessor<fixed_flat_map<Key, T, Capacity, Compare>, Context>
{};
} // namespace papilio
