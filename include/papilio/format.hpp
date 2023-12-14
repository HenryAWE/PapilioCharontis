#pragma once

#include "macros.hpp"
#include "core.hpp"
#include "script/interpreter.hpp"
#include "format/fundamental.hpp"

namespace papilio
{
template <typename CharT, typename... Args>
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

template <typename... Args>
using format_string = basic_format_string<char, std::type_identity_t<Args>...>;
template <typename... Args>
using wformat_string = basic_format_string<wchar_t, std::type_identity_t<Args>...>;

namespace detail
{
    template <typename CharT, typename OutputIt, typename Context>
    OutputIt vformat_to_impl(
        OutputIt out,
        locale_ref loc,
        std::basic_string_view<CharT> fmt,
        const dynamic_format_args<Context>& args
    )
    {
        static_assert(std::is_same_v<OutputIt, typename Context::iterator>);

        format_parse_context<Context> parse_ctx(fmt, args);
        Context fmt_ctx(out, args);

        script::interpreter<Context> intp;
        intp.format(parse_ctx, fmt_ctx, loc);

        return fmt_ctx.out();
    }
} // namespace detail

template <typename OutputIt>
struct format_to_n_result
{
    OutputIt out;
    std::iter_difference_t<OutputIt> size;
};

template <typename OutputIt>
OutputIt vformat_to(
    OutputIt out,
    std::string_view fmt,
    const dynamic_format_args<basic_format_context<std::type_identity_t<OutputIt>, char>>& args
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

template <typename OutputIt>
OutputIt vformat_to(
    OutputIt out,
    const std::locale& loc,
    std::string_view fmt,
    const dynamic_format_args<basic_format_context<std::type_identity_t<OutputIt>, char>>& args
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

template <typename OutputIt>
OutputIt vformat_to(
    OutputIt out,
    std::wstring_view fmt,
    const dynamic_format_args<basic_format_context<std::type_identity_t<OutputIt>, wchar_t>>& args
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

namespace detail
{
    template <typename OutputIt>
    struct format_to_n_wrapper
    {
        using iterator_category = std::output_iterator_tag;
        using value_type = std::iter_value_t<OutputIt>;
        using difference_type = std::iter_difference_t<OutputIt>;
        using pointer = void;
        using reference = void;

        OutputIt out;
        difference_type max_count;
        difference_type counter = 0;

        format_to_n_wrapper(const format_to_n_wrapper&) = default;
        format_to_n_wrapper(format_to_n_wrapper&&) = default;

        format_to_n_wrapper(OutputIt it, difference_type n) noexcept(std::is_nothrow_move_constructible_v<OutputIt>)
            : out(std::move(it)), max_count(n) {}

        format_to_n_wrapper& operator*() noexcept
        {
            return *this;
        }

        template <typename T>
        requires(!std::is_same_v<std::remove_cvref_t<T>, format_to_n_wrapper>)
        format_to_n_wrapper& operator=(T&& val)
        {
            if(counter == max_count)
                return *this;
            *out = std::forward<T>(val);
            ++out;
            ++counter;
            return *this;
        }

        format_to_n_wrapper& operator=(const format_to_n_wrapper&) = default;
        format_to_n_wrapper& operator=(format_to_n_wrapper&&) = default;

        format_to_n_wrapper& operator++() noexcept
        {
            return *this;
        }

        format_to_n_wrapper operator++(int) noexcept
        {
            return *this;
        }

        format_to_n_result<OutputIt> get_result() const noexcept(std::is_nothrow_copy_constructible_v<OutputIt>)
        {
            return format_to_n_result<OutputIt>{
                .out = out,
                .size = counter
            };
        }
    };

    template <typename CharT>
    struct formatted_size_counter
    {
        using iterator_category = std::output_iterator_tag;
        using value_type = CharT;
        using difference_type = std::ptrdiff_t;
        using pointer = void;
        using reference = void;

        std::size_t value = 0;

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
            ++value;
            return *this;
        }

        constexpr formatted_size_counter operator++(int) noexcept
        {
            formatted_size_counter tmp(*this);
            ++(*this);
            return tmp;
        }
    };
} // namespace detail

std::string vformat(std::string_view fmt, const dynamic_format_args<format_context>& args);

std::string vformat(const std::locale& loc, std::string_view fmt, const dynamic_format_args<format_context>& args);

std::wstring vformat(std::wstring_view fmt, const dynamic_format_args<wformat_context>& args);

template <typename OutputIt, typename... Args>
OutputIt format_to(OutputIt out, format_string<Args...> fmt, Args&&... args)
{
    using context_type = basic_format_context<OutputIt, char>;
    return vformat_to(
        out,
        fmt.get(),
        PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)
    );
}

template <typename OutputIt, typename... Args>
OutputIt format_to(
    OutputIt out,
    std::locale& loc,
    format_string<Args...> fmt,
    Args&&... args
)
{
    using context_type = basic_format_context<OutputIt, char>;
    return vformat_to(
        out,
        loc,
        fmt.get(),
        PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)
    );
}

template <typename OutputIt, typename... Args>
OutputIt format_to(
    OutputIt out,
    wformat_string<Args...> fmt,
    Args&&... args
)
{
    using context_type = basic_format_context<OutputIt, wchar_t>;
    return vformat_to(
        out,
        fmt.get(),
        PAPILIO_NS make_wformat_args<context_type>(std::forward<Args>(args)...)
    );
}

template <typename OutputIt, typename... Args>
format_to_n_result<OutputIt> format_to_n(
    OutputIt out,
    std::iter_difference_t<OutputIt> n,
    format_string<Args...> fmt,
    Args&&... args
)
{
    using wrapper = detail::format_to_n_wrapper<OutputIt>;
    using context_type = basic_format_context<wrapper, char>;

    return vformat_to(
               wrapper(out, n),
               fmt.get(),
               PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)
    )
        .get_result();
}

template <typename OutputIt, typename... Args>
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

    return vformat_to(
               wrapper(out, n),
               loc,
               fmt.get(),
               PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)
    )
        .get_result();
}

template <typename OutputIt, typename... Args>
format_to_n_result<OutputIt> format_to_n(
    OutputIt out,
    std::iter_difference_t<OutputIt> n,
    wformat_string<Args...> fmt,
    Args&&... args
)
{
    using wrapper = detail::format_to_n_wrapper<OutputIt>;
    using context_type = basic_format_context<wrapper, wchar_t>;

    return vformat_to(
               wrapper(out, n),
               fmt.get(),
               PAPILIO_NS make_wformat_args<context_type>(std::forward<Args>(args)...)
    )
        .get_result();
}

template <typename... Args>
std::size_t formatted_size(
    format_string<Args...> fmt,
    Args&&... args
)
{
    using iter_t = detail::formatted_size_counter<char>;
    using context_type = basic_format_context<iter_t, char>;

    return vformat_to(
               iter_t(),
               fmt.get(),
               PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)
    )
        .value;
}

template <typename... Args>
std::size_t formatted_size(
    const std::locale& loc,
    format_string<Args...> fmt,
    Args&&... args
)
{
    using iter_t = detail::formatted_size_counter<char>;
    using context_type = basic_format_context<iter_t, char>;

    return vformat_to(
               iter_t(),
               loc,
               fmt.get(),
               PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)
    )
        .value;
}

template <typename... Args>
std::size_t formatted_size(
    wformat_string<Args...> fmt,
    Args&&... args
)
{
    using iter_t = detail::formatted_size_counter<wchar_t>;
    using context_type = basic_format_context<iter_t, wchar_t>;

    return vformat_to(
               iter_t(),
               fmt.get(),
               PAPILIO_NS make_wformat_args<context_type>(std::forward<Args>(args)...)
    )
        .value;
}

template <typename... Args>
std::string format(format_string<Args...> fmt, Args&&... args)
{
    return PAPILIO_NS vformat(
        fmt.get(), PAPILIO_NS make_format_args(std::forward<Args>(args)...)
    );
}

template <typename... Args>
std::string format(const std::locale& loc, format_string<Args...> fmt, Args&&... args)
{
    return PAPILIO_NS vformat(
        loc, fmt.get(), PAPILIO_NS make_format_args(std::forward<Args>(args)...)
    );
}

template <typename... Args>
std::wstring format(wformat_string<Args...> fmt, Args&&... args)
{
    return PAPILIO_NS vformat(
        fmt.get(), PAPILIO_NS make_wformat_args(std::forward<Args>(args)...)
    );
}
} // namespace papilio
