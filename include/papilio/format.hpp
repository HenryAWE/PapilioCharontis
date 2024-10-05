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
/// @defgroup Format Format APIs
/// @{

template <typename CharT>
class formatted_range
{
public:
    using char_type = CharT;
    using string_view_type = std::basic_string_view<CharT>;
    using format_context_type = basic_format_context<
        format_iterator_for<CharT>,
        CharT>;
    using parse_context = basic_format_parse_context<
        format_context_type>;
    using format_args_type = format_args_ref_for<
        format_iterator_for<CharT>,
        CharT>;

private:
    using intp_t = basic_interpreter<
        basic_format_context<
            format_iterator_for<CharT>,
            CharT>>;
    using intp_ctx_t = typename intp_t::interpreter_context;

public:
    formatted_range(const formatted_range&) = delete;

    formatted_range(string_view_type fmt, const format_args_type& args)
        : m_parse_ctx(fmt, args) {}

    struct sentinel_t
    {};

    sentinel_t cend() const noexcept
    {
        return sentinel_t{};
    }

    class const_iterator
    {
    public:
        using value_type = CharT;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;

        const_iterator(const_iterator&& other) noexcept
            : m_fmt_ctx(std::back_inserter(m_buf), other.m_intp_ctx.input_context().get_args()),
              m_intp(),
              m_intp_ctx(m_intp.create_context(other.m_intp_ctx.input_context(), m_fmt_ctx)),
              m_buf(std::move(other.m_buf)),
              m_offset(std::exchange(other.m_offset, 0)) {}

        const_iterator(parse_context& ictx)
            : m_fmt_ctx(std::back_inserter(m_buf), ictx.get_args()),
              m_intp(),
              m_intp_ctx(m_intp.create_context(ictx, m_fmt_ctx)) {}

        const_iterator& operator=(const_iterator&& rhs) noexcept
        {
            m_fmt_ctx = format_context_type(std::back_inserter(m_buf), rhs.m_intp_ctx.input_context().get_args());
            m_intp_ctx = m_intp.create_context(rhs.m_intp_ctx.input_context(), m_fmt_ctx);
            m_buf = std::move(rhs.m_buf);
            m_offset = std::exchange(rhs.m_offset, 0);
        }

        bool operator==(sentinel_t) const
        {
            if(m_offset < m_buf.size())
                return false;
            return m_intp_ctx.input_at_end();
        }

        /**
         * @brief Placeholder for satisfying the `input_iterator` concept.
         */
        void operator++(int)
        {
            ++*this;
        }

        const_iterator& operator++()
        {
            ++m_offset;
            return *this;
        }

        CharT operator*() const
        {
            if(m_offset >= m_buf.size())
            {
                m_intp.run_n(m_intp_ctx, m_buf.size() - m_offset + 1);
            }
            return m_buf[m_offset];
        }

    private:
        mutable format_context_type m_fmt_ctx;
        mutable intp_t m_intp;
        mutable intp_ctx_t m_intp_ctx;
        mutable std::basic_string<CharT> m_buf;
        std::size_t m_offset = 0;
    };

    const_iterator cbegin() const
    {
        return const_iterator(m_parse_ctx);
    }

    using iterator = const_iterator;

    iterator begin() const
    {
        return cbegin();
    }

    sentinel_t end() const
    {
        return cend();
    }

private:
    mutable parse_context m_parse_ctx;
};

template <typename CharT, typename Args>
formatted_range(const CharT* str, Args&& args) -> formatted_range<CharT>;
template <typename CharT, typename Args>
formatted_range(std::basic_string_view<CharT> str, Args&& args) -> formatted_range<CharT>;
template <typename CharT, typename Args>
formatted_range(const std::basic_string<CharT>& str, Args&& args) -> formatted_range<CharT>;

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

        difference_type get_max_count() const noexcept
        {
            return m_max_count;
        }

        difference_type get_count() const noexcept
        {
            return m_counter;
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

        return [&](const auto& fmt_args)
        {
            basic_format_parse_context<context_type> parse_ctx(fmt, fmt_args);
            context_type fmt_ctx(loc, iter_t(out, n), fmt_args);

            basic_interpreter<context_type> intp;
            auto intp_ctx = intp.create_context(parse_ctx, fmt_ctx);

            intp.run_if(
                intp_ctx,
                [&intp_ctx]() -> bool
                {
                    const iter_t& it = intp_ctx.output_context().out_ref();
                    return it.get_count() < it.get_max_count();
                }
            );

            return intp_ctx.output_context().out_ref().get_result();
        }(PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...));
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

/// @}

/// @addtogroup Formatter
/// @{

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

/// @}
} // namespace papilio

#include "detail/suffix.hpp"

#include "formatter/tuple.hpp"
#include "formatter/vocabulary.hpp"

#endif
