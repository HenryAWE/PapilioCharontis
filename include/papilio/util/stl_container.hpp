#pragma once

#include <ranges>
#include <tuple>
#include <map>
#include <unordered_map>
#include <array>
#include <vector>
#include <span>
#include "../core.hpp"


namespace papilio
{
    namespace detail
    {
        template <std::ranges::sized_range Range>
        class sized_range_accessor
        {
        public:
            static format_arg get_attr(const Range& val, const attribute_name& attr)
            {
                using namespace std::literals;
                if(attr == "size"sv)
                    return std::size(val);
                else
                    throw invalid_attribute(attr);
            }
        };

        template <typename Tuple, bool EnablePairAttr = false>
        class tuple_accessor
        {
        public:
            using has_index = void;

            template <std::size_t Index>
            static format_arg get_element(const Tuple& val) noexcept
            {
                return format_arg(std::get<Index>(val));
            }

            using getter = format_arg(*)(const Tuple& val);

            template <std::size_t... Indices>
            static constexpr const std::array<getter, std::tuple_size_v<Tuple>>& get_map(std::index_sequence<Indices...>) noexcept
            {
                static constexpr std::array<getter, std::tuple_size_v<Tuple>> getter_map{
                    (&get_element<Indices>)...
                };
                return getter_map;
            }

            static format_arg get(const Tuple& val, indexing_value::index_type i)
            {
                const auto& map = get_map(std::make_index_sequence<std::tuple_size_v<Tuple>>());

                if(i < 0)
                {
                    i = -i;
                    if(i > std::tuple_size_v<Tuple>)
                        return format_arg();
                    return map[std::tuple_size_v<Tuple> - i](val);
                }
                else
                {
                    if(i >= std::tuple_size_v<Tuple>)
                        return format_arg();
                    return map[i](val);
                }
            }

            static format_arg get_attr(const Tuple& val, const attribute_name& attr)
            {
                using namespace std::literals;

                if constexpr(EnablePairAttr)
                {
                    static_assert(std::tuple_size_v<Tuple> == 2, "EnablePairAttr requires tuple_size == 2");
                    if(attr == "first"sv)
                        return format_arg(std::get<0>(val));
                    else if(attr == "second"sv)
                        return format_arg(std::get<1>(val));
                }

                if(attr == "size"sv)
                    return std::tuple_size_v<Tuple>;
                else
                    throw invalid_attribute(attr);
            }
        };
    }

    template <typename... Ts>
    struct accessor<std::tuple<Ts...>> :
        public detail::tuple_accessor<std::tuple<Ts...>> {};
    template <typename T1, typename T2>
    struct accessor<std::pair<T1, T2>> :
        public detail::tuple_accessor<std::pair<T1, T2>, true> {};

    namespace detail
    {
        template <typename Map>
        class string_map_accessor : public sized_range_accessor<Map>
        {
        public:
            using has_key = void;

            using string_type = typename Map::key_type;
            using key_compare = typename Map::key_compare;
            static constexpr bool transparent_compare =
                requires() { typename key_compare::is_transparent; };

            static format_arg get(const Map& val, const string_type& k)
            {
                auto it = val.find(k);
                if(it == val.end())
                    return format_arg();
                return format_arg(it->second);
            }
            template <typename Arg> requires transparent_compare
            static auto get(const Map& val, Arg&& k)
            {
                auto it = val.find(k);
                if(it == val.end())
                    return format_arg();
                return format_arg(it->second);
            }
        };
        template <typename Map>
        class int_map_accessor : public sized_range_accessor<Map>
        {
        public:
            using has_index = void;

            using key_type = typename Map::key_type;

            static format_arg get(const Map& val, indexing_value::index_type i)
            {
                auto it = val.find(static_cast<key_type>(i));
                if(it == val.end())
                    return format_arg();
                return format_arg(it->second);
            }
        };
    }

    template <typename T, typename Compare, typename Allocator>
    struct accessor<std::map<std::string, T, Compare, Allocator>> :
        public detail::string_map_accessor<std::map<std::string, T, Compare, Allocator>> {};
    template <typename T, typename Compare, typename Allocator>
    struct accessor<std::map<std::string_view, T, Compare, Allocator>> :
        public detail::string_map_accessor<std::map<std::string_view, T, Compare, Allocator>> {};
    template <std::integral Key, typename T, typename Compare, typename Allocator>
    struct accessor<std::map<Key, T, Compare, Allocator>> :
        public detail::int_map_accessor<std::map<Key, T, Compare, Allocator>> {};

    namespace detail
    {
        template <std::ranges::contiguous_range Range>
        class contiguous_range_accessor : public sized_range_accessor<Range>
        {
        public:
            using has_index = void;

            static format_arg get(const Range& val, indexing_value::index_type i)
            {
                if(i < 0)
                {
                    i = -i;
                    if(i > std::size(val))
                        return format_arg();
                    return val[std::size(val) - i];
                }
                else
                {
                    if(i >= std::size(val))
                        return format_arg();
                    return val[i];
                }
            }
        };
    }

    template <typename T, std::size_t Size>
    struct accessor<std::array<T, Size>> :
        public detail::contiguous_range_accessor<std::array<T, Size>> {};
    template <typename T, typename Allocator>
    struct accessor<std::vector<T, Allocator>> :
        public detail::contiguous_range_accessor<std::vector<T, Allocator>> {};
    template <typename T, std::size_t Extent>
    struct accessor<std::span<T, Extent>> :
        public detail::contiguous_range_accessor<std::span<T, Extent>> {};
}
