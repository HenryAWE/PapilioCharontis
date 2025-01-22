/**
 * @file formatter/vocabulary.hpp
 * @author HenryAWE
 * @brief Formatters for vocabulary types, like std::optional, std::variant, etc.
 */

#ifndef PAPILIO_FORMATTER_VOCABULARY_HPP
#define PAPILIO_FORMATTER_VOCABULARY_HPP

#pragma once

#include <optional>
#include <variant>
#include "../detail/config.hpp"
#ifdef PAPILIO_HAS_LIB_EXPECTED
#    include <expected>
#endif
#include "../format.hpp"
#include "../detail/prefix.hpp"

namespace papilio
{
template <typename T, typename CharT>
class formatter<std::optional<T>, CharT>
{
public:
    template <typename ParseContext, typename FormatContext>
    requires formattable_with<T, FormatContext>
    auto format(const std::optional<T>& val, ParseContext& parse_ctx, FormatContext& fmt_ctx) const
        -> typename FormatContext::iterator
    {
        (void)parse_ctx;

        using context_t = format_context_traits<FormatContext>;

        if(val.has_value())
        {
            context_t::format_to(fmt_ctx, PAPILIO_TSTRING_VIEW(CharT, "{}"), *val);
        }
        else
        {
            context_t::append(fmt_ctx, PAPILIO_TSTRING_VIEW(CharT, "nullopt"));
        }

        return fmt_ctx.out();
    }
};

namespace detail
{
    template <typename T, typename Context>
    concept variant_formattable_helper =
        std::same_as<std::decay_t<T>, std::monostate> ||
        formattable_with<T, Context>;

    template <typename Context, typename... Ts>
    concept variant_formattable = (variant_formattable_helper<Ts, Context> && ...);
} // namespace detail

template <typename... Ts, typename CharT>
class formatter<std::variant<Ts...>, CharT>
{
public:
    template <typename ParseContext, typename FormatContext>
    requires detail::variant_formattable<FormatContext, Ts...>
    auto format(const std::variant<Ts...>& val, ParseContext& parse_ctx, FormatContext& fmt_ctx) const
        -> typename FormatContext::iterator
    {
        (void)parse_ctx;

        using context_t = format_context_traits<FormatContext>;

        std::visit(
            [&](auto&& v)
            {
                using type = std::decay_t<decltype(v)>;
                if constexpr(std::same_as<type, std::monostate>)
                    context_t::append(fmt_ctx, PAPILIO_TSTRING_VIEW(CharT, "monostate"));
                else
                    context_t::format_to(fmt_ctx, PAPILIO_TSTRING_VIEW(CharT, "{}"), v);
            },
            val
        );

        return fmt_ctx.out();
    }
};

#ifdef PAPILIO_HAS_LIB_EXPECTED

namespace detail
{
    template <typename T, typename E, typename Context>
    concept expected_formattable =
        formattable_with<T, Context> && formattable_with<E, Context>;
} // namespace detail

template <typename T, typename E, typename CharT>
class formatter<std::expected<T, E>, CharT>
{
public:
    template <typename ParseContext, typename FormatContext>
    requires detail::expected_formattable<T, E, FormatContext>
    auto format(const std::expected<T, E>& val, ParseContext& parse_ctx, FormatContext& fmt_ctx) const
        -> typename FormatContext::iterator
    {
        (void)parse_ctx;

        using context_t = format_context_traits<FormatContext>;

        if(val.has_value())
        {
            context_t::format_to(fmt_ctx, PAPILIO_TSTRING_VIEW(CharT, "{}"), *val);
        }
        else
        {
            context_t::format_to(fmt_ctx, PAPILIO_TSTRING_VIEW(CharT, "{}"), val.error());
        }

        return fmt_ctx.out();
    }
};

#endif
} // namespace papilio

#include "../detail/suffix.hpp"

#endif
