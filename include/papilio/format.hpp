#pragma once

#include "macros.hpp"
#include "core.hpp"
#include "script.hpp"

#ifdef PAPILIO_COMPILER_CLANG_CL
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wc++98-compat"
#endif

namespace papilio
{
// NOTE: This class does not provide any compile-time check.
PAPILIO_EXPORT template <typename CharT, typename... Args>
class basic_format_string
{
public:
    using char_type = CharT;
    using string_view_type = std::basic_string_view<CharT>;
    using args_type = std::tuple<Args...>;

    template <std::convertible_to<string_view_type> T>
    constexpr basic_format_string(const T& fmt) noexcept(std::is_nothrow_convertible_v<string_view_type, T>)
        : m_fmt(fmt)
    {}

    constexpr basic_format_string& operator=(const basic_format_string&) noexcept = default;

    [[nodiscard]]
    constexpr string_view_type get() const noexcept
    {
        return m_fmt;
    }

private:
    string_view_type m_fmt;
};

PAPILIO_EXPORT template <typename... Args>
using format_string = basic_format_string<char, std::type_identity_t<Args>...>;
PAPILIO_EXPORT template <typename... Args>
using wformat_string = basic_format_string<wchar_t, std::type_identity_t<Args>...>;

namespace detail
{
    template <typename CharT, typename OutputIt, typename Context>
    OutputIt vformat_to_impl(
        OutputIt out,
        locale_ref loc,
        std::basic_string_view<CharT> fmt,
        const basic_format_args_ref<Context>& args
    )
    {
        static_assert(std::is_same_v<OutputIt, typename Context::iterator>);

        basic_format_parse_context<Context> parse_ctx(fmt, args);
        Context fmt_ctx(loc, out, args);

        script::basic_interpreter<Context> intp;
        intp.format(parse_ctx, fmt_ctx);

        return fmt_ctx.out();
    }

    template <typename OutputIt, typename CharT>
    using vfmt_args = basic_format_args_ref<basic_format_context<OutputIt, CharT>>;
} // namespace detail

PAPILIO_EXPORT template <typename OutputIt>
struct format_to_n_result
{
    OutputIt out;
    std::iter_difference_t<OutputIt> size;
};

PAPILIO_EXPORT template <typename OutputIt>
OutputIt vformat_to(
    OutputIt out,
    std::string_view fmt,
    const detail::vfmt_args<std::type_identity_t<OutputIt>, char>& args
)
{
    using context_type = basic_format_context<OutputIt, char>;
    return detail::vformat_to_impl<char, OutputIt, context_type>(
        std::move(out),
        nullptr,
        fmt,
        args
    );
}

PAPILIO_EXPORT template <typename OutputIt>
OutputIt vformat_to(
    OutputIt out,
    const std::locale& loc,
    std::string_view fmt,
    const detail::vfmt_args<std::type_identity_t<OutputIt>, char>& args
)
{
    using context_type = basic_format_context<OutputIt, char>;
    return detail::vformat_to_impl<char, OutputIt, context_type>(
        std::move(out),
        loc,
        fmt,
        args
    );
}

PAPILIO_EXPORT template <typename OutputIt>
OutputIt vformat_to(
    OutputIt out,
    std::wstring_view fmt,
    const detail::vfmt_args<std::type_identity_t<OutputIt>, wchar_t>& args
)
{
    using context_type = basic_format_context<OutputIt, wchar_t>;
    return detail::vformat_to_impl<wchar_t, OutputIt, context_type>(
        std::move(out),
        nullptr,
        fmt,
        args
    );
}

PAPILIO_EXPORT template <typename OutputIt>
OutputIt vformat_to(
    OutputIt out,
    const std::locale& loc,
    std::wstring_view fmt,
    const detail::vfmt_args<std::type_identity_t<OutputIt>, wchar_t>& args
)
{
    using context_type = basic_format_context<OutputIt, wchar_t>;
    return detail::vformat_to_impl<wchar_t, OutputIt, context_type>(
        std::move(out),
        loc,
        fmt,
        args
    );
}

namespace detail
{
    template <typename OutputIt>
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

        template <typename T>
        requires(!std::is_same_v<std::remove_cvref_t<T>, format_to_n_wrapper>)
        format_to_n_wrapper& operator=(T&& val)
        {
            if(m_counter == m_max_count)
                return *this;
            *m_out = std::forward<T>(val);
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

    template <typename CharT>
    class formatted_size_counter
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
            return *this;
        }

        constexpr formatted_size_counter& operator*() noexcept
        {
            return *this;
        }

        constexpr formatted_size_counter& operator++() noexcept
        {
            ++m_counter;
            return *this;
        }

        constexpr formatted_size_counter operator++(int) noexcept
        {
            formatted_size_counter tmp(*this);
            ++(*this);
            return tmp;
        }

        [[nodiscard]]
        constexpr std::size_t get_result() const noexcept
        {
            return m_counter;
        }

    private:
        std::size_t m_counter;
    };
} // namespace detail

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
        PAPILIO_NS make_wformat_args<context_type>(std::forward<Args>(args)...)
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
        PAPILIO_NS make_wformat_args<context_type>(std::forward<Args>(args)...)
    );
}

PAPILIO_EXPORT template <typename OutputIt, typename... Args>
format_to_n_result<OutputIt> format_to_n(
    OutputIt out,
    std::iter_difference_t<OutputIt> n,
    format_string<Args...> fmt,
    Args&&... args
)
{
    using wrapper = detail::format_to_n_wrapper<OutputIt>;
    using context_type = basic_format_context<wrapper, char>;

    return PAPILIO_NS vformat_to(
               wrapper(out, n),
               fmt.get(),
               PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)
    )
        .get_result();
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
    using wrapper = detail::format_to_n_wrapper<OutputIt>;
    using context_type = basic_format_context<wrapper, char>;

    return PAPILIO_NS vformat_to(
               wrapper(out, n),
               loc,
               fmt.get(),
               PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)
    )
        .get_result();
}

PAPILIO_EXPORT template <typename OutputIt, typename... Args>
format_to_n_result<OutputIt> format_to_n(
    OutputIt out,
    std::iter_difference_t<OutputIt> n,
    wformat_string<Args...> fmt,
    Args&&... args
)
{
    using wrapper = detail::format_to_n_wrapper<OutputIt>;
    using context_type = basic_format_context<wrapper, wchar_t>;

    return PAPILIO_NS vformat_to(
               wrapper(out, n),
               fmt.get(),
               PAPILIO_NS make_wformat_args<context_type>(std::forward<Args>(args)...)
    )
        .get_result();
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
    using wrapper = detail::format_to_n_wrapper<OutputIt>;
    using context_type = basic_format_context<wrapper, wchar_t>;

    return PAPILIO_NS vformat_to(
               wrapper(out, n),
               loc,
               fmt.get(),
               PAPILIO_NS make_wformat_args<context_type>(std::forward<Args>(args)...)
    )
        .get_result();
}

PAPILIO_EXPORT template <typename... Args>
[[nodiscard]]
std::size_t formatted_size(
    format_string<Args...> fmt,
    Args&&... args
)
{
    using iter_t = detail::formatted_size_counter<char>;
    using context_type = basic_format_context<iter_t, char>;

    return PAPILIO_NS vformat_to(
               iter_t(),
               fmt.get(),
               PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)
    )
        .get_result();
}

PAPILIO_EXPORT template <typename... Args>
[[nodiscard]]
std::size_t formatted_size(
    const std::locale& loc,
    format_string<Args...> fmt,
    Args&&... args
)
{
    using iter_t = detail::formatted_size_counter<char>;
    using context_type = basic_format_context<iter_t, char>;

    return PAPILIO_NS vformat_to(
               iter_t(),
               loc,
               fmt.get(),
               PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)
    )
        .get_result();
}

PAPILIO_EXPORT template <typename... Args>
[[nodiscard]]
std::size_t formatted_size(
    wformat_string<Args...> fmt,
    Args&&... args
)
{
    using iter_t = detail::formatted_size_counter<wchar_t>;
    using context_type = basic_format_context<iter_t, wchar_t>;

    return PAPILIO_NS vformat_to(
               iter_t(),
               fmt.get(),
               PAPILIO_NS make_wformat_args<context_type>(std::forward<Args>(args)...)
    )
        .get_result();
}

PAPILIO_EXPORT template <typename... Args>
[[nodiscard]]
std::size_t formatted_size(
    const std::locale& loc,
    wformat_string<Args...> fmt,
    Args&&... args
)
{
    using iter_t = detail::formatted_size_counter<wchar_t>;
    using context_type = basic_format_context<iter_t, wchar_t>;

    return PAPILIO_NS vformat_to(
               iter_t(),
               loc,
               fmt.get(),
               PAPILIO_NS make_wformat_args<context_type>(std::forward<Args>(args)...)
    )
        .get_result();
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
} // namespace papilio

#include "format/fundamental.hpp"
#include "format/misc.hpp"

#ifdef PAPILIO_COMPILER_CLANG_CL
#    pragma clang diagnostic pop
#endif
