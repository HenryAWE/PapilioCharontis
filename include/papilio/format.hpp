/**
 * @file format.hpp
 * @author HenryAWE
 * @brief Format APIs
 */

#ifndef PAPILIO_FORMAT_HPP
#define PAPILIO_FORMAT_HPP

#pragma once

#include "macros.hpp"
#include "fmtfwd.hpp"
#include "core.hpp"
#include "detail/prefix.hpp"

namespace papilio
{
PAPILIO_EXPORT
[[nodiscard]]
std::string vformat(std::string_view fmt, const format_args_ref& args);

PAPILIO_EXPORT
[[nodiscard]]
std::string vformat(const std::locale& loc, std::string_view fmt, const format_args_ref& args);

PAPILIO_EXPORT
[[nodiscard]]
std::wstring vformat(std::wstring_view fmt, const wformat_args_ref& args);

PAPILIO_EXPORT
[[nodiscard]]
std::wstring vformat(const std::locale& loc, std::wstring_view fmt, const wformat_args_ref& args);

PAPILIO_EXPORT template <typename OutputIt, typename... Args>
OutputIt format_to(OutputIt out, format_string<Args...> fmt, Args&&... args)
{
    using context_type = basic_format_context<OutputIt, char>;
    return PAPILIO_NS vformat_to(
        out,
        fmt.get(),
        PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)
    );
}

PAPILIO_EXPORT template <typename OutputIt, typename... Args>
OutputIt format_to(
    OutputIt out,
    std::locale& loc,
    format_string<Args...> fmt,
    Args&&... args
)
{
    using context_type = basic_format_context<OutputIt, char>;
    return PAPILIO_NS vformat_to(
        out,
        loc,
        fmt.get(),
        PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)
    );
}

PAPILIO_EXPORT template <typename OutputIt, typename... Args>
OutputIt format_to(
    OutputIt out,
    wformat_string<Args...> fmt,
    Args&&... args
)
{
    using context_type = basic_format_context<OutputIt, wchar_t>;
    return PAPILIO_NS vformat_to(
        out,
        fmt.get(),
        PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)
    );
}

PAPILIO_EXPORT template <typename OutputIt, typename... Args>
OutputIt format_to(
    OutputIt out,
    std::locale& loc,
    wformat_string<Args...> fmt,
    Args&&... args
)
{
    using context_type = basic_format_context<OutputIt, wchar_t>;
    return PAPILIO_NS vformat_to(
        out,
        loc,
        fmt.get(),
        PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)
    );
}

PAPILIO_EXPORT template <typename OutputIt>
struct format_to_n_result
{
    OutputIt out;
    std::iter_difference_t<OutputIt> size;
};

namespace detail
{
    template <typename OutputIt, typename CharT>
    class format_to_n_wrapper
    {
    public:
        using iterator_category = std::output_iterator_tag;
        using value_type = std::iter_value_t<OutputIt>;
        using difference_type = std::iter_difference_t<OutputIt>;
        using pointer = void;
        using reference = void;

        format_to_n_wrapper(const format_to_n_wrapper&) noexcept(std::is_nothrow_copy_constructible_v<OutputIt>) = default;
        format_to_n_wrapper(format_to_n_wrapper&&) noexcept(std::is_nothrow_move_constructible_v<OutputIt>) = default;

        format_to_n_wrapper(OutputIt it, difference_type n) noexcept(std::is_nothrow_move_constructible_v<OutputIt>)
            : m_out(std::move(it)), m_max_count(n), m_counter(0) {}

        format_to_n_wrapper& operator*() noexcept
        {
            return *this;
        }

        format_to_n_wrapper& operator=(const CharT& ch)
        {
            PAPILIO_ASSERT(m_counter <= m_max_count);

            if(m_counter == m_max_count)
                return *this;
            *m_out = ch;
            ++m_out;
            ++m_counter;
            return *this;
        }

        format_to_n_wrapper& operator=(const format_to_n_wrapper&) noexcept(std::is_nothrow_copy_assignable_v<OutputIt>) = default;
        format_to_n_wrapper& operator=(format_to_n_wrapper&&) noexcept(std::is_nothrow_move_assignable_v<OutputIt>) = default;

        format_to_n_wrapper& operator++() noexcept
        {
            return *this;
        }

        format_to_n_wrapper operator++(int) noexcept
        {
            return *this;
        }

        [[nodiscard]]
        format_to_n_result<OutputIt> get_result() const noexcept(std::is_nothrow_copy_constructible_v<OutputIt>)
        {
            return format_to_n_result<OutputIt>{.out = m_out, .size = m_counter};
        }

    private:
        OutputIt m_out;
        difference_type m_max_count;
        difference_type m_counter;
    };

    template <typename CharT, typename OutputIt, typename... Args>
    format_to_n_result<OutputIt> format_to_n_impl(
        OutputIt out,
        std::iter_difference_t<OutputIt> n,
        locale_ref loc,
        std::basic_string_view<CharT> fmt,
        Args&&... args
    )
    {
        using iter_t = detail::format_to_n_wrapper<OutputIt, CharT>;
        using context_type = basic_format_context<iter_t, CharT>;

        return PAPILIO_NS detail::vformat_to_impl<CharT, iter_t, context_type>(
                   iter_t(out, n),
                   loc,
                   fmt,
                   PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)
        )
            .get_result();
    }
} // namespace detail

PAPILIO_EXPORT template <typename OutputIt, typename... Args>
format_to_n_result<OutputIt> format_to_n(
    OutputIt out,
    std::iter_difference_t<OutputIt> n,
    format_string<Args...> fmt,
    Args&&... args
)
{
    return detail::format_to_n_impl<char, OutputIt>(
        std::move(out),
        n,
        nullptr,
        fmt.get(),
        std::forward<Args>(args)...
    );
}

PAPILIO_EXPORT template <typename OutputIt, typename... Args>
format_to_n_result<OutputIt> format_to_n(
    OutputIt out,
    std::iter_difference_t<OutputIt> n,
    const std::locale& loc,
    format_string<Args...> fmt,
    Args&&... args
)
{
    return detail::format_to_n_impl<char, OutputIt>(
        std::move(out),
        n,
        loc,
        fmt.get(),
        std::forward<Args>(args)...
    );
}

PAPILIO_EXPORT template <typename OutputIt, typename... Args>
format_to_n_result<OutputIt> format_to_n(
    OutputIt out,
    std::iter_difference_t<OutputIt> n,
    wformat_string<Args...> fmt,
    Args&&... args
)
{
    return detail::format_to_n_impl<wchar_t, OutputIt>(
        std::move(out),
        n,
        nullptr,
        fmt.get(),
        std::forward<Args>(args)...
    );
}

PAPILIO_EXPORT template <typename OutputIt, typename... Args>
format_to_n_result<OutputIt> format_to_n(
    OutputIt out,
    std::iter_difference_t<OutputIt> n,
    const std::locale& loc,
    wformat_string<Args...> fmt,
    Args&&... args
)
{
    return detail::format_to_n_impl<wchar_t, OutputIt>(
        std::move(out),
        n,
        loc,
        fmt.get(),
        std::forward<Args>(args)...
    );
}

namespace detail
{
    class formatted_size_counter_base
    {
    public:
        [[nodiscard]]
        constexpr std::size_t get_result() const noexcept
        {
            return m_counter;
        }

    protected:
        constexpr void count() noexcept
        {
            ++m_counter;
        }

    private:
        std::size_t m_counter = 0;
    };

    template <typename CharT>
    class formatted_size_counter : public formatted_size_counter_base
    {
    public:
        using iterator_category = std::output_iterator_tag;
        using value_type = CharT;
        using difference_type = std::ptrdiff_t;
        using pointer = void;
        using reference = void;

        constexpr formatted_size_counter() noexcept = default;
        constexpr formatted_size_counter(const formatted_size_counter&) noexcept = default;

        constexpr formatted_size_counter& operator=(const formatted_size_counter&) noexcept = default;

        constexpr formatted_size_counter& operator=(const value_type&) noexcept
        {
            count();
            return *this;
        }

        constexpr formatted_size_counter& operator*() noexcept
        {
            return *this;
        }

        constexpr formatted_size_counter& operator++() noexcept
        {
            return *this;
        }

        constexpr formatted_size_counter operator++(int) noexcept
        {
            formatted_size_counter tmp(*this);
            ++(*this);
            return tmp;
        }
    };

    template <typename CharT>
    using fmt_size_ctx_type = basic_format_context<formatted_size_counter<CharT>, CharT>;

    std::size_t formatted_size_impl(
        locale_ref loc,
        std::string_view fmt,
        const basic_format_args_ref<fmt_size_ctx_type<char>>& args
    );
    std::size_t formatted_size_impl(
        locale_ref loc,
        std::wstring_view fmt,
        const basic_format_args_ref<fmt_size_ctx_type<wchar_t>>& args
    );

    template <typename CharT, typename... Args>
    std::size_t formatted_size_helper(
        locale_ref loc,
        std::basic_string_view<CharT> fmt,
        Args&&... args
    )
    {
        using iter_t = detail::formatted_size_counter<CharT>;
        using context_type = basic_format_context<iter_t, CharT>;

        return formatted_size_impl(
            loc,
            fmt,
            PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)
        );
    }
} // namespace detail

PAPILIO_EXPORT template <typename... Args>
[[nodiscard]]
std::size_t formatted_size(
    format_string<Args...> fmt,
    Args&&... args
)
{
    return detail::formatted_size_helper<char>(
        nullptr,
        fmt.get(),
        std::forward<Args>(args)...
    );
}

PAPILIO_EXPORT template <typename... Args>
[[nodiscard]]
std::size_t formatted_size(
    const std::locale& loc,
    format_string<Args...> fmt,
    Args&&... args
)
{
    return detail::formatted_size_helper<char>(
        loc,
        fmt.get(),
        std::forward<Args>(args)...
    );
}

PAPILIO_EXPORT template <typename... Args>
[[nodiscard]]
std::size_t formatted_size(
    wformat_string<Args...> fmt,
    Args&&... args
)
{
    return detail::formatted_size_helper<wchar_t>(
        nullptr,
        fmt.get(),
        std::forward<Args>(args)...
    );
}

PAPILIO_EXPORT template <typename... Args>
[[nodiscard]]
std::size_t formatted_size(
    const std::locale& loc,
    wformat_string<Args...> fmt,
    Args&&... args
)
{
    return detail::formatted_size_helper<wchar_t>(
        loc,
        fmt.get(),
        std::forward<Args>(args)...
    );
}

PAPILIO_EXPORT template <typename... Args>
[[nodiscard]]
std::string format(format_string<Args...> fmt, Args&&... args)
{
    return PAPILIO_NS vformat(
        fmt.get(), PAPILIO_NS make_format_args(std::forward<Args>(args)...)
    );
}

PAPILIO_EXPORT template <typename... Args>
[[nodiscard]]
std::string format(const std::locale& loc, format_string<Args...> fmt, Args&&... args)
{
    return PAPILIO_NS vformat(
        loc, fmt.get(), PAPILIO_NS make_format_args(std::forward<Args>(args)...)
    );
}

PAPILIO_EXPORT template <typename... Args>
[[nodiscard]]
std::wstring format(wformat_string<Args...> fmt, Args&&... args)
{
    return PAPILIO_NS vformat(
        fmt.get(), PAPILIO_NS make_wformat_args(std::forward<Args>(args)...)
    );
}

PAPILIO_EXPORT template <typename... Args>
[[nodiscard]]
std::wstring format(const std::locale& loc, wformat_string<Args...> fmt, Args&&... args)
{
    return PAPILIO_NS vformat(
        loc, fmt.get(), PAPILIO_NS make_wformat_args(std::forward<Args>(args)...)
    );
}

// ^^^ format APIs ^^^ / vvv formatters vvv

PAPILIO_EXPORT template <typename R, typename CharT>
class formatter<joiner<R, CharT>, CharT>
{
public:
    formatter() = default;
    formatter(const formatter&) = default;

    formatter& operator=(const formatter&) = default;

    using joiner_t = joiner<R, CharT>;
    using range_type = typename joiner_t::range_type;
    using value_type = std::ranges::range_value_t<range_type>;

    template <typename ParseContext, typename FormatContext>
    requires formattable_with<value_type, FormatContext>
    auto format(const joiner_t& j, ParseContext& parse_ctx, FormatContext& fmt_ctx) const
    {
        using formatter_t = typename FormatContext::template formatter_type<value_type>;

        if constexpr(formatter_traits<formatter_t>::template parsable<FormatContext>())
        {
            formatter_t fmt;
            parse_ctx.advance_to(fmt.parse(parse_ctx));

            bool first = true;
            for(auto&& i : j)
            {
                if(!first)
                    append_sep(fmt_ctx, j);
                first = false;

                fmt_ctx.advance_to(
                    fmt.format(i, fmt_ctx)
                );
            }
        }
        else
        {
            bool first = true;
            for(auto&& i : j)
            {
                if(!first)
                    append_sep(fmt_ctx, j);
                first = false;

                fmt_ctx.advance_to(
                    PAPILIO_NS format_to(fmt_ctx.out(), "{}", i)
                );
            }
        }

        return fmt_ctx.out();
    }

private:
    template <typename FormatContext>
    static void append_sep(FormatContext& fmt_ctx, const joiner_t& j)
    {
        using context_t = format_context_traits<FormatContext>;
        context_t::append(fmt_ctx, j.separator());
    }
};
} // namespace papilio

#include "detail/suffix.hpp"

#include "formatter/tuple.hpp"
#include "formatter/vocabulary.hpp"

#endif
