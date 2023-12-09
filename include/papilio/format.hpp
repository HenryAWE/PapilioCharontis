#pragma once

#include "macros.hpp"
#include "core.hpp"
#include "script/interpreter.hpp"
#include "format/fundamental.hpp"

namespace papilio
{
namespace detail
{
    template <typename OutputIt>
    OutputIt vformat_to_impl(OutputIt out, locale_ref loc, std::string_view fmt, const dynamic_format_args& args)
    {
        format_parse_context parse_ctx(fmt, args);
        basic_format_context<OutputIt> fmt_ctx(out, args);

        using context_t = format_context_traits<basic_format_context<OutputIt>>;

        auto parse_it = parse_ctx.begin();

        while(parse_it != parse_ctx.end())
        {
            utf::codepoint ch = *parse_it;

            if(ch == U'}')
            {
                ++parse_it;
                if(parse_it == parse_ctx.end())
                    throw invalid_format("invalid format");

                ch = *parse_it;
                if(ch != U'}')
                    throw invalid_format("invalid format");

                context_t::append(fmt_ctx, U'}');
                ++parse_it;
            }
            else if(ch == U'{')
            {
                ++parse_it;
                if(parse_it == parse_ctx.end())
                    throw invalid_format("invalid format");

                ch = *parse_it;
                if(ch == U'{')
                {
                    context_t::append(fmt_ctx, '{');

                    ++parse_it;
                }
                else if(ch == U'$')
                {
                    ++parse_it;

                    script::interpreter intp;
                    parse_ctx.advance_to(parse_it);
                    auto [result, next_it] = intp.run(parse_ctx);

                    auto sc = script::variable(result.to_variant()).as<utf::string_container>();

                    for(utf::codepoint cp : sc)
                        context_t::append(fmt_ctx, cp);

                    parse_it = next_it;
                    if(parse_it == parse_ctx.end() || *parse_it != U'}')
                        throw invalid_format("invalid format");
                    ++parse_it;
                }
                else
                {
                    parse_ctx.advance_to(parse_it);
                    script::interpreter intp;
                    auto [arg, next_it] = intp.access(parse_ctx);

                    if(next_it == parse_ctx.end())
                        throw invalid_format("invalid format");
                    if(*next_it == U':')
                        ++next_it;
                    parse_ctx.advance_to(next_it);

                    arg.format(parse_ctx, fmt_ctx);

                    parse_it = parse_ctx.begin();
                    if(parse_it == parse_ctx.end() || *parse_it != U'}')
                        throw invalid_format("invalid format");
                    ++parse_it;
                }
            }
            else
            {
                // normal character
                context_t::append(fmt_ctx, ch);
                ++parse_it;
            }
        }

        return context_t::out(fmt_ctx);
    }
} // namespace detail

template <typename OutputIt>
struct format_to_n_result
{
    OutputIt out;
    std::iter_difference_t<OutputIt> size;
};

template <typename OutputIt>
OutputIt vformat_to(OutputIt out, std::string_view fmt, const dynamic_format_args& args)
{
    return detail::vformat_to_impl<OutputIt>(
        std::move(out),
        nullptr,
        fmt,
        args
    );
}

template <typename OutputIt>
OutputIt vformat_to(OutputIt out, const std::locale& loc, std::string_view fmt, const dynamic_format_args& args)
{
    return detail::vformat_to_impl<OutputIt>(
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
    };
} // namespace detail

template <typename OutputIt>
format_to_n_result<OutputIt> vformat_to_n(OutputIt out, std::iter_difference_t<OutputIt> n, std::string_view fmt, const dynamic_format_args& store)
{
    auto result = PAPILIO_NS vformat_to(
        detail::format_to_n_wrapper<OutputIt>(out, n),
        fmt,
        store
    );
    return {result.out, result.counter};
}

template <typename OutputIt>
format_to_n_result<OutputIt> vformat_to_n(OutputIt out, std::iter_difference_t<OutputIt> n, const std::locale& loc, std::string_view fmt, const dynamic_format_args& store)
{
    auto result = PAPILIO_NS vformat_to(
        detail::format_to_n_wrapper<OutputIt>(out, n),
        loc,
        fmt,
        store
    );
    return {result.out, result.counter};
}

std::size_t vformatted_size(std::string_view fmt, const dynamic_format_args& store);
std::size_t vformatted_size(const std::locale& loc, std::string_view fmt, const dynamic_format_args& store);
std::string vformat(std::string_view fmt, const dynamic_format_args& store);
std::string vformat(const std::locale& loc, std::string_view fmt, const dynamic_format_args& store);

template <typename OutputIt, typename... Args>
OutputIt format_to(OutputIt out, std::string_view fmt, Args&&... args)
{
    // use namespace prefix to avoid collision with std::format caused by ADL
    return vformat_to(out, fmt, PAPILIO_NS make_format_args(std::forward<Args>(args)...));
}

template <typename OutputIt, typename... Args>
OutputIt format_to(OutputIt out, std::locale& loc, std::string_view fmt, Args&&... args)
{
    // use namespace prefix to avoid collision with std::format caused by ADL
    return vformat_to(out, loc, fmt, PAPILIO_NS make_format_args(std::forward<Args>(args)...));
}

template <typename OutputIt, typename... Args>
format_to_n_result<OutputIt> format_to_n(OutputIt out, std::iter_difference_t<OutputIt> n, std::string_view fmt, Args&&... args)
{
    return vformat_to_n(out, n, fmt, PAPILIO_NS make_format_args(std::forward<Args>(args)...));
}

template <typename OutputIt, typename... Args>
format_to_n_result<OutputIt> format_to_n(OutputIt out, std::iter_difference_t<OutputIt> n, const std::locale& loc, std::string_view fmt, Args&&... args)
{
    return vformat_to_n(out, n, loc, fmt, PAPILIO_NS make_format_args(std::forward<Args>(args)...));
}

template <typename... Args>
std::size_t formatted_size(std::string_view fmt, Args&&... args)
{
    return vformatted_size(fmt, PAPILIO_NS make_format_args(std::forward<Args>(args)...));
}

template <typename... Args>
std::size_t formatted_size(const std::locale& loc, std::string_view fmt, Args&&... args)
{
    return vformatted_size(loc, fmt, PAPILIO_NS make_format_args(std::forward<Args>(args)...));
}

template <typename... Args>
std::string format(std::string_view fmt, Args&&... args)
{
    return vformat(fmt, PAPILIO_NS make_format_args(std::forward<Args>(args)...));
}

template <typename... Args>
std::string format(const std::locale& loc, std::string_view fmt, Args&&... args)
{
    // use namespace prefix to avoid collision with std::format caused by ADL
    return vformat(loc, fmt, PAPILIO_NS make_format_args(std::forward<Args>(args)...));
}
} // namespace papilio
