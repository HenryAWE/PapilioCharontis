#ifndef PAPILIO_ACCESSOR_VOCABULARY_HPP
#define PAPILIO_ACCESSOR_VOCABULARY_HPP

#pragma once

#include <optional>
#include <variant>
#include "../detail/config.hpp"
#ifdef PAPILIO_HAS_LIB_EXPECTED
#    include <expected>
#endif
#include "../access.hpp"
#include "../detail/prefix.hpp"

namespace papilio
{
PAPILIO_EXPORT template <typename T, typename Context>
struct accessor<std::optional<T>, Context>
{
    using char_type = typename Context::char_type;
    using attribute_name_type = basic_attribute_name<char_type>;
    using format_arg_type = basic_format_arg<Context>;

    static format_arg_type attribute(const std::optional<T>& opt, const attribute_name_type& attr)
    {
        if(attr == PAPILIO_TSTRING_VIEW(char_type, "value"))
        {
            if(opt.has_value())
                return *opt;
            else
                return format_arg_type();
        }
        else if(attr == PAPILIO_TSTRING_VIEW(char_type, "has_value"))
            return opt.has_value();

        throw_invalid_attribute(attr);
    }
};

PAPILIO_EXPORT template <typename... Ts, typename Context>
struct accessor<std::variant<Ts...>, Context>
{
    using char_type = typename Context::char_type;
    using attribute_name_type = basic_attribute_name<char_type>;
    using format_arg_type = basic_format_arg<Context>;

    using variant_type = std::variant<Ts...>;

    static format_arg_type index(const variant_type& var, ssize_t i)
    {
        constexpr std::size_t var_size = std::variant_size_v<variant_type>;

        if(i < 0)
            i = static_cast<ssize_t>(var_size) + i;
        if(i < 0) [[unlikely]]
            return format_arg_type();

        std::size_t idx = static_cast<std::size_t>(i);
        if(idx >= var_size) [[unlikely]]
            return format_arg_type();

        return [idx, &var]<std::size_t... Is>(std::index_sequence<Is...>)
        {
            using func_t = format_arg_type (*)(const variant_type&);

            static const func_t table[] = {&index_helper<Is>...};

            return table[idx](var);
        }(std::make_index_sequence<var_size>());
    }

    static format_arg_type attribute(const std::variant<Ts...>& var, const attribute_name_type& attr)
    {
        if(attr == PAPILIO_TSTRING_VIEW(char_type, "index"))
            return var.index();
        else if(attr == PAPILIO_TSTRING_VIEW(char_type, "value"))
        {
            return std::visit(
                [](auto&& v) -> format_arg_type
                {
                    return v;
                },
                var
            );
        }

        throw_invalid_attribute(attr);
    }

private:
    template <std::size_t Idx>
    static format_arg_type index_helper(const variant_type& var)
    {
        auto* ptr = std::get_if<Idx>(&var);
        if(!ptr)
            return format_arg_type();

        return format_arg_type(*ptr);
    }
};

#ifdef PAPILIO_HAS_LIB_EXPECTED

PAPILIO_EXPORT template <typename T, typename E, typename Context>
struct accessor<std::expected<T, E>, Context>
{
    using char_type = typename Context::char_type;
    using attribute_name_type = basic_attribute_name<char_type>;
    using format_arg_type = basic_format_arg<Context>;

    using expected_type = std::expected<T, E>;

    static format_arg_type attribute(const expected_type& ex, const attribute_name_type& attr)
    {
        if(attr == PAPILIO_TSTRING_VIEW(char_type, "value"))
        {
            if(ex.has_value())
                return *ex;
            else
                return format_arg_type();
        }
        else if(attr == PAPILIO_TSTRING_VIEW(char_type, "error"))
        {
            if(!ex.has_value())
                return ex.error();
            else
                return format_arg_type();
        }
        else if(attr == PAPILIO_TSTRING_VIEW(char_type, "has_value"))
            return ex.has_value();

        throw_invalid_attribute(attr);
    }
};

#endif
} // namespace papilio

#include "../detail/suffix.hpp"

#endif
