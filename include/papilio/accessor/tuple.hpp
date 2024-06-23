#ifndef PAPILIO_ACCESSOR_TUPLE_HPP
#define PAPILIO_ACCESSOR_TUPLE_HPP

#pragma once

#include <tuple>
#include "../utility.hpp"
#include "../detail/prefix.hpp"

namespace papilio
{
PAPILIO_EXPORT template <
    tuple_like Tuple,
    typename Context,
    bool PairLike = pair_like<Tuple>>
struct tuple_accessor
{
    // pair-like -> tuple_size == 2
    static_assert(!PairLike || std::tuple_size_v<Tuple> == 2, "Invalid tuple size");

    using char_type = typename Context::char_type;
    using attribute_name_type = basic_attribute_name<char_type>;
    using format_arg_type = basic_format_arg<Context>;

    [[nodiscard]]
    static constexpr std::size_t size() noexcept
    {
        return std::tuple_size_v<Tuple>;
    }

    [[nodiscard]]
    static format_arg_type index(const Tuple& tp, ssize_t i)
    {
        if constexpr(size() == 0)
            return format_arg_type();
        else
        {
            if(i < 0)
            { // reverse index
                i = static_cast<ssize_t>(size()) + i;
                if(i < 0) [[unlikely]]
                    return format_arg_type();
            }

            if(static_cast<std::size_t>(i) >= std::tuple_size_v<Tuple>) [[unlikely]]
                return format_arg_type();

            return [&]<std::size_t... Is>(std::index_sequence<Is...>)
            {
                using func_t = format_arg_type (*)(const Tuple&);

                static const func_t table[] = {&index_helper<Is>...};

                return table[static_cast<std::size_t>(i)](tp);
            }(std::make_index_sequence<std::tuple_size_v<Tuple>>());
        }
    }

    [[nodiscard]]
    static format_arg_type attribute(const Tuple& tp, const attribute_name_type& attr)
    {
        if constexpr(PairLike)
        {
            if(attr == PAPILIO_TSTRING_VIEW(char_type, "first"))
                return get<0>(tp);
            else if(attr == PAPILIO_TSTRING_VIEW(char_type, "second"))
                return get<1>(tp);
        }

        if(attr == PAPILIO_TSTRING_VIEW(char_type, "size"))
            return std::tuple_size_v<Tuple>;

        throw_invalid_attribute(attr);
    }

private:
    template <std::size_t Idx>
    static format_arg_type index_helper(const Tuple& tp)
    {
        return format_arg_type(get<Idx>(tp));
    }
};

PAPILIO_EXPORT template <typename Context, typename... Ts>
struct accessor<std::tuple<Ts...>, Context> :
    public tuple_accessor<std::tuple<Ts...>, Context, std::tuple_size_v<std::tuple<Ts...>> == 2>
{};

PAPILIO_EXPORT template <typename Context, typename T1, typename T2>
struct accessor<std::pair<T1, T2>, Context> :
    public tuple_accessor<std::pair<T1, T2>, Context, true>
{};

PAPILIO_EXPORT template <typename Context, typename T1, typename T2>
struct accessor<compressed_pair<T1, T2>, Context> :
    public tuple_accessor<compressed_pair<T1, T2>, Context, true>
{};
} // namespace papilio

#include "../detail/suffix.hpp"

#endif
