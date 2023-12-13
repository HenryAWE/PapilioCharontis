#pragma once

#include "macros.hpp"
#include "core.hpp"
#include "script/interpreter.hpp"
#include "format/fundamental.hpp"

namespace papilio
{
namespace detail
{
    template <typename OutputIt, typename Context>
    OutputIt vformat_to_impl(OutputIt out, locale_ref loc, std::string_view fmt, const dynamic_format_args<Context>& args)
    {
        static_assert(std::is_same_v<OutputIt, typename Context::iterator>);

        format_parse_context parse_ctx(fmt, args);
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
OutputIt vformat_to(OutputIt out,
    std::string_view fmt,
    const dynamic_format_args<basic_format_context<OutputIt>>& args)
{
    return detail::vformat_to_impl<OutputIt, basic_format_context<OutputIt>>(
        std::move(out),
        nullptr,
        fmt,
        args
    );
}

template <typename OutputIt>
OutputIt vformat_to(
    OutputIt out,
    const std::locale& loc, std::string_view fmt,
    const dynamic_format_args<basic_format_context<OutputIt>>& args)
{
    return detail::vformat_to_impl<OutputIt, basic_format_context<OutputIt>>(
        std::move(out),
        loc,
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
        using value_type = char;
        using difference_type = std::ptrdiff_t;

        OutputIt out;
        std::iter_difference_t<OutputIt> n;
        std::iter_difference_t<OutputIt> counter = 0;

        format_to_n_wrapper(const format_to_n_wrapper&) = default;
        format_to_n_wrapper(format_to_n_wrapper&&) = default;

        format_to_n_wrapper(OutputIt out_, std::iter_difference_t<OutputIt> n_)
            : out(std::move(out_)), n(n_) {}

        format_to_n_wrapper& operator*() noexcept
        {
            return *this;
        }

        format_to_n_wrapper& operator=(char ch)
        {
            if(counter == n)
                return *this;
            *out = ch;
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

        format_to_n_wrapper operator++(int)
        {
            return *this;
        }

        format_to_n_result<OutputIt> get_result() const
        {
            return format_to_n_result<OutputIt>{
                .out = out,
                .size = counter
            };
        }
    };

    struct formatted_size_counter
    {
        using iterator_category = std::output_iterator_tag;
        using value_type = char;
        using difference_type = std::ptrdiff_t;

        std::size_t value = 0;

        formatted_size_counter() noexcept = default;
        formatted_size_counter(const formatted_size_counter&) noexcept = default;

        formatted_size_counter& operator=(const formatted_size_counter&) noexcept = default;

        formatted_size_counter& operator=(char) noexcept
        {
            return *this;
        }

        formatted_size_counter& operator*() noexcept
        {
            return *this;
        }

        formatted_size_counter& operator++() noexcept
        {
            ++value;
            return *this;
        }

        formatted_size_counter operator++(int) noexcept
        {
            formatted_size_counter tmp(*this);
            ++(*this);
            return tmp;
        }
    };
} // namespace detail

std::string vformat(std::string_view fmt, const dynamic_format_args<format_context>& args);

std::string vformat(const std::locale& loc, std::string_view fmt, const dynamic_format_args<format_context>& args);

template <typename OutputIt, typename... Args>
OutputIt format_to(OutputIt out, std::string_view fmt, Args&&... args)
{
    using context_type = basic_format_context<OutputIt>;
    return vformat_to(out, fmt, PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...));
}

template <typename OutputIt, typename... Args>
OutputIt format_to(OutputIt out, std::locale& loc, std::string_view fmt, Args&&... args)
{
    using context_type = basic_format_context<OutputIt>;
    return vformat_to(out, loc, fmt, PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...));
}

template <typename OutputIt, typename... Args>
format_to_n_result<OutputIt> format_to_n(OutputIt out, std::iter_difference_t<OutputIt> n, std::string_view fmt, Args&&... args)
{
    using wrapper = detail::format_to_n_wrapper<OutputIt>;
    using context_type = basic_format_context<wrapper>;

    return vformat_to<wrapper>(wrapper(out, n), fmt, PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)).get_result();
}

template <typename OutputIt, typename... Args>
format_to_n_result<OutputIt> format_to_n(OutputIt out, std::iter_difference_t<OutputIt> n, const std::locale& loc, std::string_view fmt, Args&&... args)
{
    using wrapper = detail::format_to_n_wrapper<OutputIt>;
    using context_type = basic_format_context<wrapper>;

    return vformat_to<wrapper>(wrapper(out, n), loc, fmt, PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)).get_result();
}

template <typename... Args>
std::size_t formatted_size(std::string_view fmt, Args&&... args)
{
    using iter_t = detail::formatted_size_counter;
    using context_type = basic_format_context<iter_t>;

    return vformat_to<iter_t>(iter_t(), fmt, PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)).value;
}

template <typename... Args>
std::size_t formatted_size(const std::locale& loc, std::string_view fmt, Args&&... args)
{
    using iter_t = detail::formatted_size_counter;
    using context_type = basic_format_context<iter_t>;

    return vformat_to<iter_t>(iter_t(), loc, fmt, PAPILIO_NS make_format_args<context_type>(std::forward<Args>(args)...)).value;
}

template <typename... Args>
std::string format(std::string_view fmt, Args&&... args)
{
    return PAPILIO_NS vformat(fmt, PAPILIO_NS make_format_args(std::forward<Args>(args)...));
}

template <typename... Args>
std::string format(const std::locale& loc, std::string_view fmt, Args&&... args)
{
    // use namespace prefix to avoid collision with std::format caused by ADL
    return PAPILIO_NS vformat(loc, fmt, PAPILIO_NS make_format_args(std::forward<Args>(args)...));
}
} // namespace papilio
