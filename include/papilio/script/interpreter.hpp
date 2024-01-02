#pragma once

#include <optional>
#include <charconv>
#include "../core.hpp"
#include "../access.hpp"

namespace papilio::script
{
PAPILIO_EXPORT class bad_variable_access : public std::bad_variant_access
{};

PAPILIO_EXPORT class invalid_conversion : public std::invalid_argument
{
public:
    using invalid_argument::invalid_argument;
};

namespace detail
{
    class variable_base
    {
    protected:
        [[noreturn]]
        static void throw_invalid_conversion(const char* msg = "invalid conversion")
        {
            throw invalid_conversion(msg);
        }

        [[noreturn]]
        static void throw_bad_access()
        {
            throw bad_variable_access();
        }
    };

    template <typename CharT>
    using variable_data_type = std::variant<
        bool,
        std::int64_t,
        long double,
        utf::basic_string_container<CharT>>;
} // namespace detail

PAPILIO_EXPORT template <typename CharT>
class basic_variable : public detail::variable_base
{
public:
    using variant_type = detail::variable_data_type<CharT>;

    using char_type = CharT;
    using string_type = std::basic_string<CharT>;
    using string_view_type = std::basic_string_view<CharT>;
    using string_container_type = utf::basic_string_container<CharT>;
    using int_type = std::int64_t;
    using float_type = long double;

private:
    template <typename T>
    static variant_type convert_arg(T&& arg)
    {
        using namespace std;
        using arg_type = remove_cvref_t<T>;

        if constexpr(is_same_v<arg_type, bool>)
        {
            return variant_type(in_place_type<bool>, arg);
        }
        else if constexpr(is_same_v<arg_type, utf::codepoint>)
        {
            return variant_type(in_place_type<string_container_type>, 1, arg);
        }
        else if constexpr(integral<arg_type>)
        {
            return variant_type(in_place_type<int_type>, arg);
        }
        else if constexpr(floating_point<arg_type>)
        {
            return variant_type(in_place_type<float_type>, arg);
        }
        else if constexpr(basic_string_like<arg_type, CharT>)
        {
            return variant_type(in_place_type<string_container_type>, forward<T>(arg));
        }
        else
        {
            throw_invalid_conversion();
        }
    }

    template <typename Variant>
    static variant_type convert_variant(Variant&& var)
    {
        return std::visit(
            []<typename T>(T&& v)
            {
                return convert_arg(std::forward<T>(v));
            },
            var
        );
    }

public:
    basic_variable() = delete;
    basic_variable(const basic_variable&) = default;
    basic_variable(basic_variable&&) noexcept = default;

    basic_variable(bool v) noexcept
        : m_var(v) {}

    template <std::integral T>
    basic_variable(T i)
        : m_var(std::in_place_type<int_type>, i)
    {}

    template <std::floating_point T>
    basic_variable(T f)
        : m_var(std::in_place_type<float_type>, f)
    {}

    basic_variable(string_container_type str)
        : m_var(std::move(str)) {}

    template <basic_string_like<CharT> String>
    basic_variable(String&& str)
        : m_var(std::in_place_type<string_container_type>, std::forward<String>(str))
    {}

    template <basic_string_like<CharT> String>
    basic_variable(independent_t, String&& str)
        : m_var(std::in_place_type<string_container_type>, independent, std::forward<String>(str))
    {}

    template <typename... Ts>
    basic_variable(std::variant<Ts...>&& var)
        : m_var(convert_variant(std::move(var)))
    {}

    template <typename... Ts>
    basic_variable(const std::variant<Ts...>& var)
        : m_var(convert_variant(var))
    {}

    basic_variable& operator=(const basic_variable&) = default;
    basic_variable& operator=(basic_variable&&) noexcept = default;

    template <typename T>
    [[nodiscard]]
    bool holds() const noexcept
    {
        return std::holds_alternative<T>(m_var);
    }

    [[nodiscard]]
    bool holds_bool() const noexcept
    {
        return holds<bool>();
    }

    [[nodiscard]]
    bool holds_int() const noexcept
    {
        return holds<int_type>();
    }

    [[nodiscard]]
    bool holds_float() const noexcept
    {
        return holds<float_type>();
    }

    [[nodiscard]]
    bool holds_string() const noexcept
    {
        return holds<string_container_type>();
    }

    [[nodiscard]]
    bool has_ownership() const noexcept
    {
        const auto* ptr = get_if<string_container_type>();
        return ptr ? ptr->has_ownership() : true;
    }

    template <typename T>
    [[nodiscard]]
    const T& get() const
    {
        const T* ptr = get_if<T>();
        if(!ptr)
            throw_bad_access();
        return *ptr;
    }

    template <typename T>
    [[nodiscard]]
    const T* get_if() const noexcept
    {
        return std::get_if<T>(&m_var);
    }

    template <typename Target>
    [[nodiscard]]
    Target as() const
    {
        return std::visit(
            [](const auto& v)
            { return as_impl<Target>(v); },
            to_variant()
        );
    }

    [[nodiscard]]
    variant_type& to_variant() & noexcept
    {
        return m_var;
    }

    [[nodiscard]]
    variant_type&& to_variant() && noexcept
    {
        return m_var;
    }

    [[nodiscard]]
    const variant_type& to_variant() const& noexcept
    {
        return m_var;
    }

    [[nodiscard]]
    std::partial_ordering compare(const basic_variable& var) const noexcept
    {
        return std::visit(
            [](const auto& lhs, const auto& rhs) noexcept
            {
                return compare_impl(lhs, rhs);
            },
            m_var,
            var.m_var
        );
    }

    // clang-format off

    [[nodiscard]]
    std::partial_ordering operator<=>(const basic_variable& rhs) const
    {
        return compare(rhs);
    }

    // clang-format on

    [[nodiscard]]
    bool equal(
        const basic_variable& var,
        float_type epsilon = std::numeric_limits<float_type>::epsilon()
    ) const noexcept
    {
        return std::visit(
            [epsilon](const auto& lhs, const auto& rhs) noexcept
            {
                return equal_impl(epsilon, lhs, rhs);
            },
            m_var,
            var.m_var
        );
    }

    [[nodiscard]]
    bool operator==(const basic_variable& rhs) const noexcept
    {
        return equal(rhs);
    }

private:
    variant_type m_var;

    template <typename Target, typename T>
    static Target as_impl(const T& v)
    {
        using std::is_same_v;

        if constexpr(is_same_v<Target, bool>)
        {
            if constexpr(is_same_v<T, string_container_type>)
                return !v.empty();
            else
                return static_cast<bool>(v);
        }
        else if constexpr(std::is_arithmetic_v<Target>)
        {
            if constexpr(is_same_v<T, string_container_type>)
                throw_invalid_conversion();
            else // bool, int_type, and float_type
                return static_cast<Target>(v);
        }
        else if constexpr(basic_string_like<Target, CharT>)
        {
            if constexpr(is_same_v<T, string_container_type>)
            {
                if constexpr(is_same_v<T, Target>)
                    return v;
                else if constexpr(std::is_constructible_v<Target, string_view_type>)
                    return Target(string_view_type(v));
                else
                    static_assert(!sizeof(Target), "invalid target type");
            }
            else
            {
                throw_invalid_conversion();
            }
        }
        else
        {
            static_assert(!sizeof(Target), "invalid target type");
        }
    }

    template <typename T>
    static std::partial_ordering compare_impl(const T& lhs, const T& rhs) noexcept
    {
        return lhs <=> rhs;
    }

    template <typename T, typename U>
    requires(std::is_arithmetic_v<T> && std::is_arithmetic_v<U>)
    static std::partial_ordering compare_impl(T lhs, U rhs) noexcept
    {
        using common_t = std::common_type_t<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;
        return static_cast<common_t>(lhs) <=> static_cast<common_t>(rhs);
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    static std::partial_ordering compare_impl(const string_container_type&, T) noexcept
    {
        return std::partial_ordering::unordered;
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    static std::partial_ordering compare_impl(T, const string_container_type&) noexcept
    {
        return std::partial_ordering::unordered;
    }

    template <std::integral T, std::integral U>
    static bool equal_impl(float_type, T lhs, U rhs) noexcept
    {
        // int_type and bool
        using commont_t = std::common_type_t<T, U>;
        return static_cast<commont_t>(lhs) == static_cast<commont_t>(rhs);
    }

    static bool equal_impl(float_type epsilon, float_type lhs, float_type rhs) noexcept
    {
        return std::abs(lhs - rhs) < epsilon;
    }

    template <std::integral T>
    static bool equal_impl(float_type epsilon, T lhs, float_type rhs) noexcept
    {
        return equal_impl(epsilon, static_cast<float_type>(lhs), rhs);
    }

    template <std::integral T>
    static bool equal_impl(float_type epsilon, float_type lhs, T rhs) noexcept
    {
        return equal_impl(epsilon, lhs, static_cast<float_type>(rhs));
    }

    static bool equal_impl(float_type, const string_container_type& lhs, const string_container_type& rhs) noexcept
    {
        return lhs == rhs;
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    static bool equal_impl(float_type, T, const string_container_type&) noexcept
    {
        return false;
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    static bool equal_impl(float_type, const string_container_type&, T) noexcept
    {
        return false;
    }
};

PAPILIO_EXPORT using variable = basic_variable<char>;
PAPILIO_EXPORT using wvariable = basic_variable<wchar_t>;

PAPILIO_EXPORT template <typename T, typename CharT>
concept basic_variable_storable =
    std::is_same_v<T, bool> ||
    std::is_same_v<T, variable::int_type> ||
    std::is_same_v<T, variable::float_type> ||
    std::is_same_v<T, utf::basic_string_container<CharT>>;

PAPILIO_EXPORT template <typename T, typename CharT>
struct is_basic_variable_storable :
    public std::bool_constant<basic_variable_storable<T, CharT>>
{};

PAPILIO_EXPORT template <typename T, typename CharT>
inline constexpr bool is_basic_variable_storable_v =
    is_basic_variable_storable<T, CharT>::value;

PAPILIO_EXPORT template <typename T>
using is_variable_storable = is_basic_variable_storable<T, char>;
PAPILIO_EXPORT template <typename T>
inline constexpr bool is_variable_storable_v = is_variable_storable<T>::value;

// ^^^ variable ^^^ / vvv interpreter vvv

PAPILIO_EXPORT class interpreter_base
{
public:
    static constexpr char32_t script_start = U'$';
};

PAPILIO_EXPORT template <typename FormatContext>
class interpreter : public interpreter_base
{
public:
    using char_type = typename FormatContext::char_type;
    using variable_type = basic_variable<char_type>;
    using string_view_type = std::basic_string_view<char_type>;
    using string_ref_type = utf::basic_string_ref<char_type>;
    using string_container_type = utf::basic_string_container<char_type>;
    using indexing_value_type = basic_indexing_value<char_type>;
    using format_arg_type = basic_format_arg<FormatContext>;

    using parse_context = basic_format_parse_context<FormatContext>;
    using iterator = typename parse_context::iterator;

    interpreter() = default;

    std::pair<format_arg_type, iterator> access(parse_context& ctx) const
    {
        return access_impl(ctx, ctx.begin(), ctx.end());
    }

    std::pair<format_arg_type, iterator> run(parse_context& ctx) const
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
        ++start;

        string_container_type result_1;
        if(cond_result)
            std::tie(result_1, start) = parse_string(start, sentinel);
        else
            start = skip_string(start, sentinel);
        start = skip_ws(start, sentinel);

        if(start != sentinel && *start == U':')
        {
            ++start;
            start = skip_ws(start, sentinel);

            if(*start != U'\'')
                throw_error("invalid script");
            ++start;

            if(!cond_result)
            {
                string_container_type result_2;
                std::tie(result_2, start) = parse_string(start, sentinel);
                return std::make_pair(std::move(result_2), skip_ws(start, sentinel));
            }
            else
            {
                start = skip_string(start, sentinel);
            }
        }

        if(cond_result)
            return std::make_pair(std::move(result_1), skip_ws(start, sentinel));
        else
            return std::make_pair(string_container_type(), skip_ws(start, sentinel));
    }

    void format(
        parse_context& parse_ctx,
        FormatContext& fmt_ctx
    )
    {
        using context_t = format_context_traits<FormatContext>;

        auto parse_it = parse_ctx.begin();

        while(parse_it != parse_ctx.end())
        {
            char32_t ch = *parse_it;

            if(ch == U'}')
            {
                ++parse_it;
                if(parse_it == parse_ctx.end())
                    throw_error("invalid format");

                ch = *parse_it;
                if(ch != U'}')
                    throw_error("invalid format");

                context_t::append(fmt_ctx, U'}');
                ++parse_it;
            }
            else if(ch == U'{')
            {
                ++parse_it;
                if(parse_it == parse_ctx.end())
                    throw_error("invalid format");

                ch = *parse_it;
                if(ch == U'{')
                {
                    context_t::append(fmt_ctx, '{');

                    ++parse_it;
                }
                else if(ch == script_start)
                {
                    ++parse_it;

                    parse_ctx.advance_to(parse_it);
                    auto [result, next_it] = run(parse_ctx);

                    auto sc = variable_type(result.to_variant()).template as<string_container_type>();

                    for(utf::codepoint cp : sc)
                        context_t::append(fmt_ctx, cp);

                    parse_it = next_it;
                    if(parse_it == parse_ctx.end() || *parse_it != U'}')
                        throw_error("invalid format");
                    ++parse_it;
                }
                else
                {
                    parse_ctx.advance_to(parse_it);
                    auto [arg, next_it] = access(parse_ctx);

                    if(next_it == parse_ctx.end())
                        throw_error("invalid format");
                    if(*next_it == U':')
                        ++next_it;
                    parse_ctx.advance_to(next_it);

                    arg.format(parse_ctx, fmt_ctx);

                    parse_it = parse_ctx.begin();
                    if(parse_it == parse_ctx.end() || *parse_it != U'}')
                        throw_error("invalid format");
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
    }

private:
    static std::pair<format_arg_type, iterator> access_impl(parse_context& ctx, iterator start, iterator stop)
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

        return digit ||
               (U'A' <= ch && ch <= U'A') ||
               (U'a' <= ch && ch <= U'z') ||
               ch == U'_' ||
               ch >= 128;
    }

    static iterator find_id_end(iterator start, iterator stop, bool first = true) noexcept
    {
        while(start != stop)
        {
            if(!is_id_char(*start, first))
                break;
            first = false;
            ++start;
        }

        return start;
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
        return ch == U'=' ||
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

    static bool execute_op(op_id op, const variable_type& lhs, const variable_type& rhs)
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
        return std::find_if_not(start, stop, utf::is_whitespace);
    }

    static std::pair<variable_type, iterator> parse_variable(parse_context& ctx, iterator start, iterator stop)
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
            return std::make_pair(
                variable_type(std::move(arg).to_variant()),
                next_it
            );
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
                negative ? start + 1 : start, stop, utf::is_digit
            );
            if(int_end != stop && *int_end == U'.')
            {
                ++int_end;
                iterator float_end = std::find_if_not(int_end, stop, utf::is_digit);

                long double val = 0.0;
                if constexpr(char8_like<char_type>)
                {
                    auto result = std::from_chars(
                        std::bit_cast<const char_type*>(start.to_address()),
                        std::bit_cast<const char_type*>(float_end.to_address()),
                        val
                    );
                }
                else
                {
                    std::string tmp = string_ref_type(start, float_end).to_string();
                    auto result = std::from_chars(
                        std::to_address(tmp.begin()),
                        std::to_address(tmp.end()),
                        val
                    );
                }

                return std::make_pair(val, float_end);
            }
            else
            {
                using int_t = variable_type::int_type;
                int_t val = parse_integer<int_t>(start, int_end).first;

                return std::make_pair(val, int_end);
            }
        }

        throw_error("invalid variable");
    }

    static std::pair<bool, iterator> parse_condition(parse_context& ctx, iterator start, iterator stop)
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
            return std::make_pair(!var.template as<bool>(), next_it);
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
                return std::make_pair(var.template as<bool>(), next_it);
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

    static std::pair<format_arg_type, iterator> parse_field_id(
        parse_context& ctx, iterator start, iterator stop
    )
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
            iterator str_end = find_id_end(start, stop, false);

            string_ref_type name(str_start, str_end);

            return std::make_pair(
                ctx.get_args().get(string_view_type(name)),
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

    static std::pair<format_arg_type, iterator> parse_chained_access(
        format_arg_type& base_arg, iterator start, iterator stop
    )
    {
        format_arg_type current = base_arg;

        while(start != stop)
        {
            char32_t first_ch = *start;
            if(first_ch == U'.')
            {
                ++start;
                iterator str_start = start;

                // clang-format off

                iterator str_end = find_id_end(start, stop);

                // clang-format on

                string_ref_type attr_name(str_start, str_end);
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

        return std::make_pair(std::move(current), start);
    }

    static std::pair<indexing_value_type, iterator> parse_indexing_value(iterator start, iterator stop)
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

    static iterator skip_string(iterator start, iterator stop) noexcept
    {
        bool esc = false;
        while(start != stop)
        {
            char32_t ch = *start;
            ++start;

            if(esc)
            {
                esc = false;
                continue;
            }
            if(ch == U'\'')
                break;
            if(ch == '\\')
                esc = true;
        }

        return start;
    }

    static std::pair<string_container_type, iterator> parse_string(iterator start, iterator stop)
    {
        iterator it = start;
        for(; it != stop; ++it)
        {
            char32_t ch = *it;

            // Turn to another mode for parsing escape sequence
            if(ch == U'\\')
            {
                string_container_type result(start, it);

                ++it;
                if(it == stop)
                    throw_error("invalid string");

                result.push_back(utf::codepoint(get_esc_ch(*it++)));

                for(; it != stop; ++it)
                {
                    ch = *it;
                    if(ch == U'\\')
                    {
                        ++it;
                        if(it == stop)
                            throw_error("invalid string");

                        result.push_back(utf::codepoint(get_esc_ch(*it)));
                    }
                    else if(ch == U'\'')
                    {
                        ++it; // skip '\''
                        break;
                    }
                    else
                    {
                        result.push_back(utf::codepoint(ch));
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
            string_container_type(start, it),
            it + 1 // +1 to skip '\''
        );
    }

    [[noreturn]]
    static void throw_error(const char* msg = "script error")
    {
        throw invalid_format(msg); // TODO: Better exception
    }
};
} // namespace papilio::script
