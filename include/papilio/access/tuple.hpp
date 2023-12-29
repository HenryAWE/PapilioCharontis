#pragma once

#include <string_view>
#include "../access.hpp"
#include "../utility.hpp"

namespace papilio
{
PAPILIO_EXPORT template <typename Tuple, typename Context>
requires(tuple_like<Tuple>)
struct accessor<Tuple, Context>
{
    using char_type = typename Context::char_type;
    using attribute_name_type = basic_attribute_name<char_type>;
    using format_arg_type = basic_format_arg<Context>;

    [[nodiscard]]
    static format_arg_type index(const Tuple& tp, ssize_t i)
    {
        if constexpr(std::tuple_size_v<Tuple> == 0)
            return format_arg_type();
        else
        {
            if(i < 0)
            {
                i = std::tuple_size_v<Tuple> + i;
                if(i < 0) [[unlikely]]
                    return format_arg_type();
            }

            if(i >= std::tuple_size_v<Tuple>) [[unlikely]]
                return format_arg_type();

            return [&]<std::size_t... Is>(std::index_sequence<Is...>)
            {
                using func_t = format_arg_type (*)(const Tuple&);

                static const func_t table[] = {&index_helper<Is>...};

                return table[i](tp);
            }(std::make_index_sequence<std::tuple_size_v<Tuple>>());
        }
    }

    [[nodiscard]]
    static format_arg_type attribute(const Tuple& tp, const attribute_name_type& attr)
    {
        if constexpr(std::tuple_size_v<Tuple> == 2) // pair-like
        {
            if(attr == PAPILIO_TSTRING_VIEW(char_type, "first"))
                return std::get<0>(tp);
            else if(attr == PAPILIO_TSTRING_VIEW(char_type, "second"))
                return std::get<1>(tp);
        }

        if(attr == PAPILIO_TSTRING_VIEW(char_type, "size"))
            return std::tuple_size_v<Tuple>;

        throw_invalid_attribute(attr);
    }

private:
    template <std::size_t Idx>
    static format_arg_type index_helper(const Tuple& tp)
    {
        return format_arg_type(std::get<Idx>(tp));
    }
};
} // namespace papilio
