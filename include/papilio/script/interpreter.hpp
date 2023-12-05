#pragma once

#include <optional>
#include <charconv>
#include "../core.hpp"
#include "../access.hpp"


namespace papilio::script
{
    class interpreter
    {
    public:
        using char_type = char;

        using iterator = format_parse_context::iterator;

        interpreter() = default;

        std::pair<format_arg, iterator> access(format_parse_context& ctx) const
        {
            return access_impl(ctx, ctx.begin(), ctx.end());
        }

        std::pair<format_arg, iterator> run(format_parse_context& ctx) const
        {
            iterator start = ctx.begin();
            const iterator sentinel = ctx.end();

            bool cond_result;
            std::tie(cond_result, start) = parse_condition(ctx, start, sentinel);

            start = skip_ws(start, sentinel);
            if(start == sentinel)
                throw_error("invalid script");
            if(*start != U'\'')
                throw_error("invalid script");

            utf::string_container result_1;
            ++start;
            std::tie(result_1, start) = parse_string(start, sentinel);
            start = skip_ws(start, sentinel);

            if(start != sentinel && *start == U':')
            {
                ++start;
                start = skip_ws(start, sentinel);

                if(*start != U'\'')
                    throw_error("invalid script");
                ++start;

                utf::string_container result_2;

                std::tie(result_2, start) = parse_string(start, sentinel);

                if(!cond_result)
                    return std::make_pair(std::move(result_2), skip_ws(start, sentinel));
            }

            if(cond_result)
                return std::make_pair(std::move(result_1), skip_ws(start, sentinel));
            else
                return std::make_pair(utf::string_container(), skip_ws(start, sentinel));
        }

    private:
        static std::pair<format_arg, iterator> access_impl(format_parse_context& ctx, iterator start, iterator stop)
        {
            if(start == stop)
                throw_error("invalid access");

            auto [arg, next_it] = parse_field_id(ctx, start, stop);

            std::tie(arg, next_it) = parse_chained_access(arg, next_it, stop);

            return std::make_pair(std::move(arg), next_it);
        }

        static bool is_id_char(char32_t ch, bool first = false) noexcept
        {
            bool digit = utf::is_digit(ch);
            if(digit && first)
                return false;

            return
                digit ||
                (U'A' <= ch && ch <= U'A') ||
                (U'a' <= ch && ch <= U'z') ||
                ch == U'_' ||
                ch >= 128;
        }

        enum class op_id
        {
            equal, // !=
            not_equal, // = and ==
            greater_equal, // >=
            less_equal, // <=
            greater, // >
            less // <
        };

        static bool is_op_ch(char32_t ch) noexcept
        {
            return
                ch == U'=' ||
                ch == U'!' ||
                ch == U'>' ||
                ch == U'<';
        }

        static std::pair<op_id, iterator> parse_op(iterator start, iterator stop)
        {
            if(start == stop)
                throw_error("invalid operator");

            char32_t first_ch = *start;
            if(first_ch == U'=')
            {
                ++start;
                if(start != stop)
                {
                    if(*start != U'=')
                        throw_error("invalid operator");
                    ++start;
                }

                return std::make_pair(op_id::equal, start);
            }
            else if(first_ch == U'!')
            {
                ++start;
                if(start == stop || *start != U'=')
                    throw_error("invalid operator");
                ++start;
                return std::make_pair(op_id::not_equal, start);
            }
            else if(first_ch == U'>' || first_ch == '<')
            {
                ++start;
                if(start != stop && *start == U'=')
                {
                    ++start;
                    if(first_ch == U'>')
                        return std::make_pair(op_id::greater_equal, start);
                    if(first_ch == U'<')
                        return std::make_pair(op_id::less_equal, start);
                    PAPILIO_UNREACHABLE();
                }
                else
                {
                    if(first_ch == U'>')
                        return std::make_pair(op_id::greater, start);
                    if(first_ch == U'<')
                        return std::make_pair(op_id::less, start);
                    PAPILIO_UNREACHABLE();
                }
            }

            throw_error("invalid operator");
        }

        static bool execute_op(op_id op, const variable& lhs, const variable& rhs)
        {
            switch(op)
            {
            case op_id::equal:
                return lhs == rhs;
            case op_id::not_equal:
                return lhs != rhs;
            case op_id::greater_equal:
                return lhs >= rhs;
            case op_id::less_equal:
                return lhs <= rhs;
            case op_id::greater:
                return lhs > rhs;
            case op_id::less:
                return lhs < rhs;

            default:
                PAPILIO_UNREACHABLE();
            }
        }

        // Skips white spaces
        static iterator skip_ws(iterator start, iterator stop) noexcept
        {
            while(start != stop)
            {
                char32_t ch = *start;
                if(!utf::is_whitespace(ch))
                    break;
                ++start;
            }

            return start;
        }

        static std::pair<variable, iterator> parse_variable(format_parse_context& ctx, iterator start, iterator stop)
        {
            if(start == stop)
                throw_error("invalid value");

            char32_t first_ch = *start;
            if(first_ch == U'{')
            {
                ++start;
                auto [arg, next_it] = access_impl(ctx, start, stop);
                if(next_it == stop || *next_it != U'}')
                    throw_error("invalid script");

                ++next_it;
                return std::make_pair(arg.as_variable(), next_it);
            }
            else if(first_ch == U'\'')
            {
                ++start;
                auto [str, next_it] = parse_string(start, stop);

                return std::make_pair(std::move(str), next_it);
            }
            else if(first_ch == U'-' || utf::is_digit(first_ch) || first_ch == U'.')
            {
                bool negative = first_ch == U'-';

                iterator int_end = std::find_if_not(
                    negative ? start + 1 : start, stop,
                    utf::is_digit
                );
                if(int_end != stop && *int_end == U'.')
                {
                    ++int_end;
                    iterator float_end = std::find_if_not(int_end, stop, utf::is_digit);

                    long double val = 0.0;
                    auto result = std::from_chars(
                        start.to_address(), float_end.to_address(),
                        val
                    );

                    return std::make_pair(val, float_end);
                }
                else
                {
                    variable::int_type val = parse_integer<variable::int_type>(start, int_end).first;

                    return std::make_pair(val, int_end);
                }
            }

            throw_error("invalid variable");
        }

        static std::pair<bool, iterator> parse_condition(format_parse_context& ctx, iterator start, iterator stop)
        {
            start = skip_ws(start, stop);
            if(start == stop)
                throw_error("invalid condition");

            char32_t first_ch = *start;
            if(first_ch == U'!')
            {
                ++start;
                start = skip_ws(start, stop);

                auto [var, next_it] = parse_variable(ctx, start, stop);
                next_it = skip_ws(next_it, stop);
                if(next_it == stop)
                    throw_error("invalid condition");
                if(*next_it != U':')
                    throw_error("invalid condition");

                ++next_it;
                return std::make_pair(!var.as<bool>(), next_it);
            }
            else if(first_ch == U'{')
            {
                auto [var, next_it] = parse_variable(ctx, start, stop);

                next_it = skip_ws(next_it, stop);
                if(next_it == stop)
                    throw_error("invalid condition");

                char32_t ch = *next_it;
                if(ch == U':')
                {
                    ++next_it;
                    return std::make_pair(var.as<bool>(), next_it);
                }
                else if(is_op_ch(ch))
                {
                    op_id op;
                    std::tie(op, next_it) = parse_op(next_it, stop);

                    next_it = skip_ws(next_it, stop);

                    auto [var_2, next_it_2] = parse_variable(ctx, next_it, stop);
                    next_it = skip_ws(next_it_2, stop);

                    if(next_it == stop || *next_it != U':')
                        throw_error("invalid condition");

                    ++next_it;
                    return std::make_pair(
                        execute_op(op, var, var_2),
                        next_it
                    );
                }
            }

            throw_error("invalid condition");
        }

        // Parses integer value
        template <std::integral T>
        static std::pair<T, iterator> parse_integer(iterator start, iterator stop)
        {
            if(start == stop)
                throw_error("invalid integer");

            T value = 0;
            bool negative = false;
            if(*start == U'-')
            {
                negative = true;
                ++start;
            }

            while(start != stop)
            {
                char32_t ch = *start;
                if(!utf::is_digit(ch))
                    break;

                value *= 10;
                value += ch - U'0';
                ++start;
            }

            if(negative)
            {
                if constexpr(std::is_signed_v<T>)
                    value = -value;
                else
                    throw_error("invalid integer");
            }

            return std::make_pair(value, start);
        }

        static std::pair<format_arg, iterator> parse_field_id(format_parse_context& ctx, iterator start, iterator stop)
        {
            using namespace std::literals;

            if(start == stop)
            {
                throw_error("script error");
            }

            char32_t first_ch = *start;

            if(utf::is_digit(first_ch))
            {
                std::size_t idx = first_ch - U'0';
                ++start;

                while(start != stop)
                {
                    char32_t ch = *start;
                    if(!utf::is_digit(ch))
                        break;

                    idx *= 10;
                    idx += ch - U'0';
                    ++start;
                }

                ctx.check_arg_id(idx);
                return std::make_pair(ctx.get_args()[idx], start);
            }
            else if(is_id_char(first_ch, true))
            {
                iterator str_start = start;
                ++start;
                iterator str_end = std::find_if_not(
                    start, stop,
                    [](utf::codepoint cp) { return is_id_char(cp, false); }
                );

                utf::string_ref name(str_start, str_end);

                return std::make_pair(
                    ctx.get_args().get(std::string_view(name)),
                    str_end
                );
            }
            else if(U"}:.["sv.find(first_ch) != std::u32string_view::npos) // default value
            {
                std::size_t idx = ctx.current_arg_id();
                ctx.next_arg_id();
                return std::make_pair(ctx.get_args()[idx], start);
            }

            throw_error("invalid field id");
        }

        static std::pair<format_arg, iterator> parse_chained_access(format_arg& base_arg, iterator start, iterator stop)
        {
            format_arg current = base_arg;

            while(start != stop)
            {
                char32_t first_ch = *start;
                if(first_ch == U'.')
                {
                    ++start;
                    iterator str_start = start;
                    iterator str_end = std::find_if_not(
                        start, stop,
                        [first = true](utf::codepoint cp) mutable
                        {
                            bool result = is_id_char(cp, first);
                            first = false;
                            return result;
                        }
                    );

                    utf::string_ref attr_name(str_start, str_end);
                    if(attr_name.empty())
                        throw_error("missing attribute name after '.'");

                    current = current.attribute(attr_name);

                    start = str_end;
                }
                else if(first_ch == U'[')
                {
                    ++start;
                    auto [idx, next_it] = parse_indexing_value(start, stop);
                    if(next_it == stop || *next_it != U']')
                        throw_error("invalid index");
                    ++next_it;

                    current = current.index(idx);

                    start = next_it;
                }
                else
                {
                    break;
                }
            }

            return std::make_pair(current, start);
        }

        static std::pair<indexing_value, iterator> parse_indexing_value(iterator start, iterator stop)
        {
            if(start == stop)
                throw_error("invalid index");

            char32_t first_ch = *start;
            if(first_ch == U'\'')
            {
                ++start;
                auto [str, next_it] = parse_string(start, stop);

                return std::make_pair(std::move(str), next_it);
            }
            else if(first_ch == U'-' || utf::is_digit(first_ch))
            {
                auto [idx, next_it] = parse_integer<ssize_t>(start, stop);

                if(*next_it == U':')
                {
                    ++next_it;
                    if(next_it == stop)
                        throw_error("invalid index");

                    char32_t next_ch = *next_it;
                    ssize_t next_idx = slice::npos;
                    if(next_ch == '-' || utf::is_digit(next_ch))
                    {
                        std::tie(next_idx, next_it) = parse_integer<ssize_t>(next_it, stop);
                    }

                    return std::make_pair(slice(idx, next_idx), next_it);
                }

                return std::make_pair(idx, next_it);
            }
            else if(first_ch == U':')
            {
                ++start;
                if(start == stop)
                    throw_error("invalid index");

                char32_t next_ch = *start;
                if(next_ch == '-' || utf::is_digit(next_ch))
                {
                    auto [idx, next_it] = parse_integer<ssize_t>(start, stop);
                    return std::make_pair(slice(0, idx), next_it);
                }
                else
                {
                    return std::make_pair(slice(), start);
                }
            }

            throw_error("invalid index");
        }

        static char32_t get_esc_ch(char32_t ch) noexcept
        {
            switch(ch)
            {
            case U'n':
                return U'\n';
            case U't':
                return U'\t';

            case U'\'':
            default:
                return ch;
            }
        }

        static std::pair<utf::string_container, iterator> parse_string(iterator start, iterator stop)
        {
            iterator it = start;
            for(; it != stop; ++it)
            {
                char32_t ch = *it;

                // Turn to another mode for parsing escape sequence
                if(ch == U'\\')
                {
                    utf::string_container result(utf::string_ref(start, it));

                    ++it;
                    if(it == stop)
                        throw_error("invalid string");

                    result.push_back(utf::codepoint(get_esc_ch(*it++)));

                    for(; it != stop; ++it)
                    {
                        utf::codepoint cp = *it;
                        if(cp == U'\\')
                        {
                            ++it;
                            if(it == stop)
                                throw_error("invalid string");

                            result.push_back(utf::codepoint(get_esc_ch(cp)));
                        }
                        else if(cp == U'\'')
                        {
                            ++it; // skip '\''
                            break;
                        }
                        else
                        {
                            result.push_back(cp);
                        }
                    }

                    return std::make_pair(std::move(result), it);
                }
                else if(ch == U'\'')
                {
                    break;
                }
            }

            return std::make_pair(
                utf::string_container(utf::string_ref(start, it)),
                it + 1 // +1 to skip '\''
            );
        }

        [[noreturn]]
        static void throw_error(const char* msg = "script error")
        {
            throw std::runtime_error(msg); // TODO: Better exception
        }
    };
}
