#pragma once

#include "macros.hpp"
#include "core.hpp"
#include "script.hpp"
#include <cmath>


namespace papilio
{
    template <typename OutputIt>
    struct format_to_n_result
    {
        OutputIt out;
        std::iter_difference_t<OutputIt> size;
    };

    template <typename OutputIt>
    OutputIt vformat_to(OutputIt out, std::string_view fmt, const dynamic_format_args& store)
    {
        // TODO
        return out; // placeholder
    }
    template <typename OutputIt>
    OutputIt vformat_to(OutputIt out, const std::locale& loc, std::string_view fmt, const dynamic_format_args& store)
    {
        // TODO
        return out; // placeholder
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

            format_to_n_wrapper& operator*() noexcept { return *this; }
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
            format_to_n_wrapper& operator++() noexcept { return *this; }
            format_to_n_wrapper operator++(int) { return *this; }
        };
    }

    template <typename OutputIt>
    format_to_n_result<OutputIt> vformat_to_n(OutputIt out, std::iter_difference_t<OutputIt> n, std::string_view fmt, const dynamic_format_args& store)
    {
        auto result = vformat_to(
            detail::format_to_n_wrapper<OutputIt>(out, n),
            fmt,
            store
        );
        return { result.out, result.counter };
    }
    template <typename OutputIt>
    format_to_n_result<OutputIt> vformat_to_n(OutputIt out, std::iter_difference_t<OutputIt> n, const std::locale& loc, std::string_view fmt, const dynamic_format_args& store)
    {
        auto result = vformat_to(
            detail::format_to_n_wrapper<OutputIt>(out, n),
            loc,
            fmt,
            store
        );
        return { result.out, result.counter };
    }
    std::size_t vformatted_size(std::string_view fmt, const dynamic_format_args& store);
    std::size_t vformatted_size(const std::locale& loc, std::string_view fmt, const dynamic_format_args& store);
    std::string vformat(std::string_view fmt, const dynamic_format_args& store);
    std::string vformat(const std::locale& loc, std::string_view fmt, const dynamic_format_args& store);

    template <typename OutputIt, typename... Args>
    OutputIt format_to(OutputIt out, std::string_view fmt, Args&&... args)
    {
        // use namespace prefix to avoid collision with std::format caused by ADL
        return vformat_to(out, fmt, papilio::make_format_args(std::forward<Args>(args)...));
    }
    template <typename OutputIt, typename... Args>
    OutputIt format_to(OutputIt out, std::locale& loc, std::string_view fmt, Args&&... args)
    {
        // use namespace prefix to avoid collision with std::format caused by ADL
        return vformat_to(out, loc, fmt, papilio::make_format_args(std::forward<Args>(args)...));
    }
    template <typename OutputIt, typename... Args>
    format_to_n_result<OutputIt> format_to_n(OutputIt out, std::iter_difference_t<OutputIt> n, std::string_view fmt, Args&&... args)
    {
        return vformat_to_n(out, n, fmt, papilio::make_format_args(std::forward<Args>(args)...));
    }
    template <typename OutputIt, typename... Args>
    format_to_n_result<OutputIt> format_to_n(OutputIt out, std::iter_difference_t<OutputIt> n, const std::locale& loc, std::string_view fmt, Args&&... args)
    {
        return vformat_to_n(out, n, loc, fmt, papilio::make_format_args(std::forward<Args>(args)...));
    }
    template <typename... Args>
    std::size_t formatted_size(std::string_view fmt, Args&&... args)
    {
        // use namespace prefix to avoid collision with std::format caused by ADL
        return vformatted_size(fmt, papilio::make_format_args(std::forward<Args>(args)...));
    }
    template <typename... Args>
    std::size_t formatted_size(const std::locale& loc, std::string_view fmt, Args&&... args)
    {
        // use namespace prefix to avoid collision with std::format caused by ADL
        return vformatted_size(loc, fmt, papilio::make_format_args(std::forward<Args>(args)...));
    }
    template <typename... Args>
    std::string format(std::string_view fmt, Args&&... args)
    {
        // use namespace prefix to avoid collision with std::format caused by ADL
        return vformat(fmt, papilio::make_format_args(std::forward<Args>(args)...));
    }
    template <typename... Args>
    std::string format(const std::locale& loc, std::string_view fmt, Args&&... args)
    {
        // use namespace prefix to avoid collision with std::format caused by ADL
        return PAPILIO_NS vformat(loc, fmt, papilio::make_format_args(std::forward<Args>(args)...));
    }
}
