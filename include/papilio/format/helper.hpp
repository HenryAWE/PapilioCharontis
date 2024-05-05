#ifndef PAPILIO_FORMAT_HELPER_HPP
#define PAPILIO_FORMAT_HELPER_HPP

#pragma once

#include "../core.hpp"
#include "../utf.hpp"
#include "../script/interpreter.hpp"
#include "../detail/prefix.hpp"

namespace papilio
{
PAPILIO_EXPORT struct std_formatter_data
{
    using size_type = std::size_t;

    size_type width = 0;
    size_type precision = 0;
    utf::codepoint fill = utf::codepoint();
    char32_t type = U'\0';
    format_align align = format_align::default_align;
    format_sign sign = format_sign::default_sign;
    bool fill_zero = false;
    bool alternate_form = false;
    bool use_locale = false;

    [[nodiscard]]
    constexpr bool contains_type(char32_t type_ch) const noexcept
    {
        if(type == U'\0')
            return true;
        return type == type_ch;
    }

    [[nodiscard]]
    constexpr bool contains_type(std::u32string_view types) const noexcept
    {
        if(type == U'\0')
            return true;
        return types.find(type) != types.npos;
    }

    constexpr void check_type(std::u32string_view types) const
    {
        if(!contains_type(types))
        {
            throw format_error("invalid format type");
        }
    }

    [[nodiscard]]
    constexpr char32_t type_or(char32_t val) const noexcept
    {
        return type == U'\0' ? val : type;
    }

    [[nodiscard]]
    constexpr utf::codepoint fill_or(utf::codepoint val) const noexcept
    {
        return fill ? fill : val;
    }
};

namespace detail
{
    class std_fmt_parser_base
    {
    protected:
        static bool is_align_ch(char32_t ch) noexcept
        {
            return ch == U'<' || ch == U'>' || ch == U'^';
        }

        static format_align get_align(char32_t ch) noexcept
        {
            PAPILIO_ASSERT(is_align_ch(ch));

            switch(ch)
            {
            case U'<': return format_align::left;
            case U'>': return format_align::right;
            case U'^': return format_align::middle;

            default: PAPILIO_UNREACHABLE();
            }
        }

        static bool is_sign_ch(char32_t ch) noexcept
        {
            return ch == U'+' || ch == U' ' || ch == U'-';
        }

        static format_sign get_sign(char32_t ch) noexcept
        {
            PAPILIO_ASSERT(is_sign_ch(ch));

            switch(ch)
            {
            case U'+': return format_sign::positive;
            case U' ': return format_sign::space;
            case U'-': return format_sign::negative;

            default: PAPILIO_UNREACHABLE();
            }
        }

        static bool is_spec_ch(char32_t ch, std::u32string_view types) noexcept
        {
            return is_sign_ch(ch) ||
                   is_align_ch(ch) ||
                   utf::is_digit(ch) ||
                   ch == U'{' ||
                   ch == U'.' ||
                   ch == U'#' ||
                   ch == U'L' ||
                   types.find(ch) != types.npos;
        }
    };
} // namespace detail

/**
 * @brief Parser for the standard format specification.
 *
 * @tparam ParseContext The parsing context.
 * @tparam EnablePrecision Enable parsing of precision.
*/
PAPILIO_EXPORT template <typename ParseContext, bool EnablePrecision = false>
class std_formatter_parser : detail::std_fmt_parser_base
{
    using my_base = detail::std_fmt_parser_base;

public:
    using char_type = typename ParseContext::char_type;
    using iterator = typename ParseContext::iterator;

    using result_type = std_formatter_data;
    using interpreter_type = script::basic_interpreter<typename ParseContext::format_context_type>;

    std::pair<result_type, iterator> parse(ParseContext& ctx, std::u32string_view types)
    {
        std_formatter_data result;

        iterator start = ctx.begin();
        const iterator stop = ctx.end();

        if(start == stop)
            goto parse_end;
        if(*start == U'}')
            goto parse_end;

        if(iterator next = start + 1; next != stop)
        {
            if(char32_t ch = *next; is_align_ch(ch))
            {
                result.fill = *start;
                result.align = get_align(ch);
                ++next;
                start = next;
            }
        }

        if(check_stop(start, stop))
            goto parse_end;
        if(char32_t ch = *start; is_align_ch(ch))
        {
            result.align = get_align(ch);
            ++start;
        }

        if(check_stop(start, stop))
            goto parse_end;
        if(char32_t ch = *start; is_sign_ch(ch))
        {
            result.sign = get_sign(ch);
            ++start;
        }

        if(check_stop(start, stop))
            goto parse_end;
        if(*start == U'#')
        {
            result.alternate_form = true;
            ++start;
        }

        if(check_stop(start, stop))
            goto parse_end;
        if(*start == U'0')
        {
            result.fill_zero = true;
            ++start;
        }

        if(check_stop(start, stop))
            goto parse_end;
        if(char32_t ch = *start; utf::is_digit(ch))
        {
            if(ch == U'0')
                throw format_error("invalid format");

            ctx.advance_to(start);
            std::tie(result.width, start) = parse_value<false>(ctx);
        }
        else if(ch == U'{')
        {
            ctx.advance_to(start);
            std::tie(result.width, start) = parse_value<false>(ctx);
        }

        if(check_stop(start, stop))
            goto parse_end;
        if(*start == U'.')
        {
            ++start;
            if(start == stop)
                throw format_error("invalid precision");

            ctx.advance_to(start);
            std::tie(result.precision, start) = parse_value<true>(ctx);
        }

        if(check_stop(start, stop))
            goto parse_end;
        if(*start == U'L')
        {
            result.use_locale = true;
            ++start;
        }

        if(check_stop(start, stop))
            goto parse_end;
        if(char32_t ch = *start; types.find(ch) != types.npos)
        {
            result.type = ch;
            ++start;
        }
        else
        {
            throw format_error("invalid format");
        }

parse_end:
        ctx.advance_to(start);
        return std::make_pair(std::move(result), std::move(start));
    }

private:
    static bool check_stop(iterator start, iterator stop) noexcept
    {
        return start == stop || *start == U'}';
    }

    // parse width or precision
    template <bool IsPrecision>
    static std::pair<std::size_t, iterator> parse_value(ParseContext& ctx)
    {
        iterator start = ctx.begin();
        const iterator stop = ctx.end();
        PAPILIO_ASSERT(start != stop);

        char32_t first_ch = *start;

        if constexpr(!IsPrecision)
        {
            if(first_ch == U'0')
            {
                throw format_error("invalid format");
            }
        }

        if(first_ch == U'{')
        {
            ++start;

            interpreter_type intp{};
            ctx.advance_to(start);
            auto [arg, next_it] = intp.access(ctx);

            if(next_it == stop || *next_it != U'}')
            {
                throw format_error("invalid format");
            }
            ++next_it;

            auto var = script::variable(arg.to_variant());
            if(!var.holds_int())
                throw format_error("invalid type");
            ssize_t val = var.as<ssize_t>();
            if constexpr(IsPrecision)
            {
                if(val < 0)
                    throw format_error("invalid format");
            }
            else
            {
                if(val <= 0)
                    throw format_error("invalid format");
            }

            return std::make_pair(val, next_it);
        }
        else if(utf::is_digit(first_ch))
        {
            ++start;
            std::size_t val = static_cast<std::size_t>(first_ch - U'0');

            while(start != stop)
            {
                char32_t ch = *start;
                if(!utf::is_digit(ch))
                    break;
                val *= 10;
                val += static_cast<std::size_t>(ch - U'0');

                ++start;
            }

            PAPILIO_ASSERT(IsPrecision || val != 0);

            return std::make_pair(val, start);
        }

        throw format_error("invalid format");
    }
};
} // namespace papilio

#include "../detail/suffix.hpp"

#endif
