/**
 * @file core.hpp
 * @author HenryAWE
 * @brief The core module of library.
 */

#ifndef PAPILIO_CORE_HPP
#define PAPILIO_CORE_HPP

#pragma once

#include <cmath>
#include <limits>
#include <variant>
#include <typeinfo>
#include <map>
#include <span>
#include <array>
#include <charconv>
#include <typeindex>
#include "macros.hpp"
#include "fmtfwd.hpp"
#include "utility.hpp"
#include "container.hpp"
#include "utf/utf.hpp"
#include "locale.hpp"
#include "access.hpp"
#include "detail/prefix.hpp"

namespace papilio
{
/**
 * @brief Base of all format error.
 *
 * @ingroup Format
 */
PAPILIO_EXPORT class format_error : public std::runtime_error
{
public:
    using runtime_error::runtime_error;

    format_error(const format_error&) = default;

    ~format_error() override;
};

/**
 * @brief Format alignment.
 * Filling character will be used for the remaining space.
 *
 * @ingroup Format
 */
PAPILIO_EXPORT enum class format_align : std::uint8_t
{
    /** Actual alignment depends on the type to be formatted. */
    default_align = 0,
    /** `<`: Align to left. */
    left = 1,
    /** `^`: Align to middle. */
    middle = 2,
    /** `>`: Align to right */
    right = 3
};

/**
 * @brief Format sign for numeric values.
 *
 * @ingroup Format
 */
PAPILIO_EXPORT enum class format_sign : std::uint8_t
{
    /** Actual meaning depends on the type to be formatted. */
    default_sign = 0,
    /** `+`: Always write sign for any values. */
    positive = 1,
    /** `-`: Only write sign for negative values. */
    negative = 2,
    /** `(space)`: Write space prefix for positive values and '-' for negative values. */
    space = 3
};

/// @defgroup RangeFormat Range format
/// @brief Specifies how a range should be formatted.
/// @ingroup Format
/// @{

/**
 * @brief Specifies how a range should be formatted.
 *
 * @ingroup Format
 */
PAPILIO_EXPORT enum class range_format
{
    /** Disallows range default formatter to format range. */
    disabled = 0,
    /**
     * Allows to format range as map representation with modified brackets `"{"`, `"}"` and separator `": "`
     * for underlying pair-like types in the following format:
     * `{ key-1 : value-1, ..., key-n : value-n }`
     */
    map = 1,
    /**
     * Allows to format range as set representation with modified brackets `"{"` and `"}"`
     * in the following format:
     * `{ key-1, ..., key-n }`
     */
    set = 2,
    /**
     * Allows to format range as sequence representation with default brackets "[", "]" and separator ", "
     * in the following format:
     * `[ element-1, ..., element-n ]`
     */
    sequence = 3,
    /** Allows to format range as string. */
    string = 4,
    /** Allows to format range as escaped string. */
    debug_string = 5
};

template <typename R>
constexpr inline range_format format_kind = []()
{
    static_assert(!sizeof(R), "Invalid type");
    return range_format::disabled;
}();

template <std::ranges::input_range R>
requires std::same_as<R, std::remove_cvref_t<R>>
constexpr inline range_format format_kind<R> = []
{
    if constexpr(std::same_as<std::remove_cvref_t<std::ranges::range_reference_t<R>>, R>)
        return range_format::disabled;
    else if constexpr(requires { typename R::key_type; })
    {
        if constexpr(requires { typename R::mapped_type; } &&
                     pair_like<std::ranges::range_reference_t<R>>)
            return range_format::map;
        else
            return range_format::set;
    }
    else
        return range_format::sequence;
}();

/// @}

/**
 * @brief Format string.
 * Provides compile-time check if possible.
 *
 * @tparam CharT Character type
 *
 * @note Currently, this class does not provide any compile-time checking.
 *       It is provided only for consistency with the STL.
 *
 * @ingroup Format
 */
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

    constexpr basic_format_string(const basic_format_string&) noexcept = default;

    constexpr basic_format_string& operator=(const basic_format_string&) noexcept = default;

    /**
     * @brief Get the format string.
     *
     * @return string_view_type The format string.
     */
    [[nodiscard]]
    constexpr string_view_type get() const noexcept
    {
        return m_fmt;
    }

private:
    string_view_type m_fmt;
};

/**
 * @brief The script error code.
 *
 * @ingroup Script
 */
PAPILIO_EXPORT enum class script_error_code : int
{
    /** No error */
    no_error = 0,
    /**
     * The interpreter reached the end of string.
     * Typically, this error is related to an incomplete format string.
     */
    end_of_string = 1,
    /** Invalid name of replacement field. */
    invalid_field_name = 2,
    /** Invalid script condition operation. */
    invalid_condition = 3,
    /** Invalid index (subscripting operator). */
    invalid_index = 4,
    /** Invalid attribute name. */
    invalid_attribute = 5,
    /** Unrecognized operator. */
    invalid_operator = 6,
    /** Invalid string constant in script. */
    invalid_string = 7,
    /**
     * Invalid format specification.
     *
     * This might be caused by a bad format parser,
     * which returns a wrong iterator position to the interpreter.
     */
    invalid_fmt_spec = 8,
    /** Unenclosed brace. */
    unenclosed_brace = 9,

    /**
     * Unknown error.
     *
     * This will only occur if there is an internal error.
     * Please report in GitHub Issue if you get this in normal code.
     */
    unknown_error = -1
};

[[nodiscard]]
std::string to_string(script_error_code ec);
[[nodiscard]]
std::wstring to_wstring(script_error_code ec);

std::ostream& operator<<(std::ostream& os, script_error_code ec);
std::wostream& operator<<(std::wostream& os, script_error_code ec);

/**
 * @brief Formatter data for standard format specification
 * @ingroup Formatter
 */
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

/**
 * @brief Formatter data for simple formatter data.
 *
 * Simple formatter data only contains width, filling character, alignment, and locale.
 *
 * @ingroup Formatter
 * @sa std_formatter_data
 */
PAPILIO_EXPORT struct simple_formatter_data
{
    using size_type = std::size_t;

    size_type width = 0;
    utf::codepoint fill = utf::codepoint();
    format_align align = format_align::default_align;
    bool use_locale = false;

    [[nodiscard]]
    constexpr bool has_fill() const noexcept
    {
        return static_cast<bool>(fill);
    }

    [[nodiscard]]
    constexpr utf::codepoint fill_or(utf::codepoint val) const noexcept
    {
        return has_fill() ? fill : val;
    }

    /**
     * @brief Convert the simple formatter data to standard formatter data.
     *
     * @return std_formatter_data The data for standard format specification.
     */
    constexpr std_formatter_data to_std_data() const noexcept
    {
        return std_formatter_data{
            .width = width,
            .fill = fill,
            .align = align,
            .use_locale = use_locale
        };
    }

    constexpr operator std_formatter_data() const noexcept
    {
        return to_std_data();
    }
};

/**
 * @brief Bad variable access
 *
 * @ingroup Variable
 */
PAPILIO_EXPORT class bad_variable_access : public std::bad_variant_access
{
public:
    using bad_variant_access::bad_variant_access;

    bad_variable_access(const bad_variable_access&) = default;

    ~bad_variable_access();
};

/**
 * @brief Invalid conversion error
 */
PAPILIO_EXPORT class invalid_conversion : public std::invalid_argument
{
public:
    using invalid_argument::invalid_argument;

    invalid_conversion(const invalid_conversion&) = default;

    ~invalid_conversion();
};

/// @defgroup Variable Script variable
/// @ingroup Script
/// @{

/**
 * @brief Base of script variable
 */
PAPILIO_EXPORT class variable_base
{
public:
    using int_type = std::int64_t;
    using float_type = float;

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

/**
 * @brief Script variable
 */
PAPILIO_EXPORT template <typename CharT>
class basic_variable : public variable_base
{
    using my_base = variable_base;

public:
    using char_type = CharT;
    using string_type = std::basic_string<CharT>;
    using string_view_type = std::basic_string_view<CharT>;
    using string_container_type = utf::basic_string_container<CharT>;
    using int_type = my_base::int_type;
    using float_type = my_base::float_type;

    using variant_type = std::variant<
        bool,
        int_type,
        float_type,
        string_container_type>;

private:
    template <typename T>
    static variant_type convert_arg(T&& arg)
    {
        using namespace std;
        using arg_type = remove_cvref_t<T>;

        if constexpr(same_as<arg_type, bool>)
        {
            return variant_type(in_place_type<bool>, arg);
        }
        else if constexpr(same_as<arg_type, utf::codepoint>)
        {
            return variant_type(in_place_type<string_container_type>, 1, arg);
        }
        else if constexpr(integral<arg_type>)
        {
            return variant_type(in_place_type<int_type>, arg);
        }
        else if constexpr(floating_point<arg_type>)
        {
            return variant_type(in_place_type<float_type>, static_cast<float_type>(arg));
        }
        else if constexpr(basic_string_like<arg_type, CharT>)
        {
            return variant_type(in_place_type<string_container_type>, std::forward<T>(arg));
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
        : m_var(std::in_place_type<float_type>, static_cast<float_type>(f))
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

    /**
     * @brief Checks if the variable holds a value of the given type.
     *
     * @tparam T Type to check against.
     */
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

    /**
     * @brief Compares two variables.
     *
     * This function compares two variables and returns a value indicating their relationship.
     *
     * @param var Another variable to compare with.
     * @return std::partial_ordering The relationship between two variables.
     */
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

    std::partial_ordering operator<=>(const basic_variable& rhs) const
    {
        return compare(rhs);
    }

    /**
     * @brief Checks if two variables are equal.
     *
     * This function checks if two variables are equal within the specified epsilon.
     *
     * @param var Another variable to compare with.
     * @param epsilon The maximum difference allowed between two variables.
     *                This parameter is only used when one variable holds a float value.
     */
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
        using std::same_as;

        if constexpr(same_as<Target, bool>)
        {
            if constexpr(same_as<T, string_container_type>)
                return !v.empty();
            else
                return static_cast<bool>(v);
        }
        else if constexpr(std::is_arithmetic_v<Target>)
        {
            if constexpr(same_as<T, string_container_type>)
                throw_invalid_conversion();
            else // bool, int_type, and float_type
                return static_cast<Target>(v);
        }
        else if constexpr(basic_string_like<Target, CharT>)
        {
            if constexpr(same_as<T, string_container_type>)
            {
                if constexpr(same_as<T, Target>)
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
        using common_t = std::common_type_t<T, U>;
        return static_cast<common_t>(lhs) == static_cast<common_t>(rhs);
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

PAPILIO_EXPORT template <typename T, typename CharT>
concept basic_variable_storable =
    std::same_as<T, bool> ||
    std::same_as<T, variable::int_type> ||
    std::same_as<T, variable::float_type> ||
    std::same_as<T, utf::basic_string_container<CharT>>;

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

/// @}

/**
 * @brief Bad handle cast
 */
PAPILIO_EXPORT class bad_handle_cast : public std::bad_cast
{
public:
    using bad_cast::bad_cast;

    bad_handle_cast(const bad_handle_cast&) = default;

    ~bad_handle_cast() override;

    const char* what() const noexcept override
    {
        return "bad handle cast";
    }
};

/**
 * @brief Default formatter. It does nothing.
 */
PAPILIO_EXPORT template <typename T, typename CharT>
class formatter
{
public:
    formatter() = delete;
    formatter(const formatter&) = delete;
    formatter(formatter&&) = delete;

    formatter& operator=(const formatter&) = delete;
    formatter& operator=(formatter&&) = delete;
};

/**
 * @brief Explicitly disabled formatter
 *
 * Derive your formatter from this class to explicitly prevent a type from being formatted
 */
PAPILIO_EXPORT class disabled_formatter
{
    disabled_formatter() = delete;
    disabled_formatter(const disabled_formatter&) = delete;
    disabled_formatter(disabled_formatter&&) = delete;

    disabled_formatter& operator=(const disabled_formatter&) = delete;
    disabled_formatter& operator=(disabled_formatter&&) = delete;
};

namespace detail
{
    template <typename T>
    concept acceptable_integral =
        !std::same_as<std::remove_cv_t<T>, bool> &&
        std::integral<std::remove_cv_t<T>> &&
        !char_like<T> &&
        sizeof(T) <= sizeof(unsigned long long int);

    template <acceptable_integral Integral>
    using convert_int_t = std::conditional_t<
        std::is_unsigned_v<Integral>,
        std::conditional_t<sizeof(Integral) <= sizeof(unsigned int), unsigned int, unsigned long long int>,
        std::conditional_t<sizeof(Integral) <= sizeof(int), int, long long int>>;

    // Acceptable floating point
    template <typename T>
    concept acceptable_fp =
        std::same_as<std::remove_cv_t<T>, float> ||
        std::same_as<std::remove_cv_t<T>, double> ||
        std::same_as<std::remove_cv_t<T>, long double>;

    // Other user-defined types
    // Use a type-erased handle class
    template <typename T, typename CharT>
    concept use_handle =
        !std::same_as<std::remove_cv_t<T>, bool> &&
        !std::same_as<std::remove_cv_t<T>, utf::codepoint> &&
        !char_like<T> &&
        !acceptable_integral<T> &&
        !acceptable_fp<T> &&
        !std::is_pointer_v<T> &&
        !std::is_bounded_array_v<T> &&
        !basic_string_like<T, CharT>;

    // Check if T satisfies the requirements for small-object optimization.
    // The implementation still needs to check the sizeof(T).
    template <typename T>
    concept use_soo_handle =
        std::is_nothrow_copy_constructible_v<std::remove_cvref_t<T>> &&
        std::is_nothrow_move_constructible_v<std::remove_cvref_t<T>>;
} // namespace detail

/**
 * @brief Format argument
 */
PAPILIO_EXPORT template <typename Context>
class basic_format_arg
{
public:
    using char_type = typename Context::char_type;
    using string_type = std::basic_string<char_type>;
    using string_view_type = std::basic_string_view<char_type>;
    using string_container_type = utf::basic_string_container<char_type>;

    using indexing_value_type = basic_indexing_value<char_type>;
    using attribute_name_type = basic_attribute_name<char_type>;

    using parse_context = basic_format_parse_context<Context>;

private:
    template <typename T>
    static constexpr bool use_handle_v = detail::use_handle<T, char_type>;

    [[noreturn]]
    static void throw_unformattable()
    {
        throw format_error("unformattable");
    }

    class handle_impl_base
    {
    public:
        handle_impl_base() = default;
        handle_impl_base(const handle_impl_base&) = delete;

        virtual ~handle_impl_base() = default;

        virtual basic_format_arg index(const indexing_value_type& idx) const = 0;
        virtual basic_format_arg attribute(const attribute_name_type& attr) const = 0;

        virtual void format(parse_context& parse_ctx, Context& out_ctx) const = 0;

        virtual void skip_spec(parse_context& parse_ctx) const = 0;

        virtual void copy(void* mem) const noexcept = 0;

        virtual void move(void* mem) noexcept = 0;

        virtual bool has_ownership() const noexcept = 0;

        virtual bool is_formattable() const noexcept = 0;

        virtual const std::type_info& type() const noexcept = 0;

        virtual const void* ptr() const noexcept = 0;

        template <typename T>
        std::add_const_t<T>* cast_to() const noexcept
        {
            if(typeid(T) == type())
                return static_cast<std::add_const_t<T>*>(ptr());
            else
                return nullptr;
        }
    };

    template <typename T>
    class handle_impl : public handle_impl_base
    {
    public:
        using value_type = std::remove_cvref_t<T>;
        using accessor_t = accessor_traits<T, Context>;

        bool is_formattable() const noexcept final;

        void skip_spec(parse_context& parse_ctx) const final;

        const std::type_info& type() const noexcept final
        {
            return typeid(value_type);
        }
    };

    template <typename T>
    class handle_impl_ptr final : public handle_impl<T>
    {
    public:
        using value_type = std::remove_cvref_t<T>;
        using accessor_t = typename handle_impl<T>::accessor_t;

        handle_impl_ptr(const T& val) noexcept
            : m_ptr(std::addressof(val), false) {}

        template <typename Arg>
        handle_impl_ptr(independent_t, Arg&& val)
            : m_ptr(make_optional_unique<const value_type>(std::forward<Arg>(val)))
        {}

        handle_impl_ptr(const handle_impl_ptr& other) noexcept
            : m_ptr(other.m_ptr) {}

        handle_impl_ptr(handle_impl_ptr&& other) noexcept
            : m_ptr(std::move(other.m_ptr)) {}

        basic_format_arg index(const indexing_value_type& idx) const override
        {
            PAPILIO_ASSERT(m_ptr);

            return accessor_t::access(*m_ptr, idx);
        }

        basic_format_arg attribute(const attribute_name_type& attr) const override
        {
            PAPILIO_ASSERT(m_ptr);

            return accessor_t::access(*m_ptr, attr);
        }

        void format(parse_context& parse_ctx, Context& out_ctx) const override;

        void copy(void* mem) const noexcept override
        {
            new(mem) handle_impl_ptr(*this);
        }

        void move(void* mem) noexcept override
        {
            new(mem) handle_impl_ptr(std::move(*this));
        }

        bool has_ownership() const noexcept override
        {
            return m_ptr.has_ownership();
        }

        const void* ptr() const noexcept override
        {
            return m_ptr.get();
        }

    private:
        optional_unique_ptr<const value_type> m_ptr;
    };

    template <typename T>
    class handle_impl_soo final : public handle_impl<T>
    {
    public:
        using value_type = std::remove_cvref_t<T>;
        using accessor_t = typename handle_impl<T>::accessor_t;

        template <typename Arg>
        requires std::is_constructible_v<T, Arg>
        handle_impl_soo(Arg&& val) noexcept
            : m_val(std::forward<Arg>(val))
        {}

        template <typename Arg>
        requires std::is_constructible_v<T, Arg>
        handle_impl_soo(independent_t, Arg&& val) noexcept
            : m_val(std::forward<Arg>(val))
        {}

        handle_impl_soo(const handle_impl_soo& other) noexcept
            : m_val(other.m_val) {}

        handle_impl_soo(handle_impl_soo&& other) noexcept
            : m_val(std::move(other.m_val)) {}

        basic_format_arg index(const indexing_value_type& idx) const override
        {
            return accessor_t::access(m_val, idx);
        }

        basic_format_arg attribute(const attribute_name_type& attr) const override
        {
            return accessor_t::access(m_val, attr);
        }

        void format(parse_context& parse_ctx, Context& out_ctx) const override;

        void copy(void* mem) const noexcept override
        {
            new(mem) handle_impl_soo(*this);
        }

        void move(void* mem) noexcept override
        {
            new(mem) handle_impl_soo(std::move(*this));
        }

        bool has_ownership() const noexcept override
        {
            return true;
        }

        const void* ptr() const noexcept override
        {
            return std::addressof(m_val);
        }

    private:
        value_type m_val;
    };

public:
    /**
     * @brief Type-erased handle to a value to be formatted.
     */
    class handle
    {
    public:
        handle() noexcept = default;

        handle(const handle& other) noexcept
            : handle()
        {
            other.copy(*this);
        }

        handle(handle&& other) noexcept
            : handle()
        {
            other.move(*this);
        }

        template <typename T>
        explicit handle(T&& val) noexcept
            : handle()
        {
            construct<T>(std::forward<T>(val));
        }

        template <typename T>
        handle(independent_t, T&& val)
            : handle()
        {
            construct<T>(independent, std::forward<T>(val));
        }

        ~handle()
        {
            destroy();
        }

        handle& operator=(const handle& rhs) noexcept
        {
            if(this == &rhs)
                return *this;
            destroy();
            rhs.copy(*this);

            return *this;
        }

        handle& operator=(handle&& rhs) noexcept
        {
            if(this == &rhs)
                return *this;
            destroy();
            rhs.move(*this);

            return *this;
        }

        [[nodiscard]]
        basic_format_arg index(const indexing_value_type& idx) const
        {
            return ptr()->index(idx);
        }

        [[nodiscard]]
        basic_format_arg attribute(const attribute_name_type& attr) const
        {
            return ptr()->attribute(attr);
        }

        void format(parse_context& parse_ctx, Context& out_ctx) const
        {
            ptr()->format(parse_ctx, out_ctx);
        }

        void skip_spec(parse_context& parse_ctx) const
        {
            ptr()->skip_spec(parse_ctx);
        }

        [[nodiscard]]
        bool has_ownership() const noexcept
        {
            return ptr()->has_ownership();
        }

        [[nodiscard]]
        bool is_formattable() const noexcept
        {
            return ptr()->is_formattable();
        }

        [[nodiscard]]
        std::type_info type()
        {
            return ptr()->type();
        }

        template <typename T>
        [[nodiscard]]
        friend std::add_const_t<T>* handle_cast(const handle* h) noexcept
        {
            static_assert(!std::is_void_v<T>, "Invalid type");

            return h->ptr()->template cast_to<std::remove_cvref_t<T>>();
        }

        template <typename T>
        [[nodiscard]]
        friend std::add_const_t<T>& handle_cast(const handle& h)
        {
            std::add_const_t<T>* ptr = handle_cast<T>(&h);
            if(!ptr)
                throw bad_handle_cast();
            return *ptr;
        }

    private:
        static constexpr std::size_t storage_size = 32;
        mutable static_storage<storage_size> m_storage;

        handle_impl_base* ptr() const noexcept
        {
            return reinterpret_cast<handle_impl_base*>(m_storage.data());
        }

        template <typename Impl, typename... Args>
        void construct_impl(Args&&... args)
        {
            static_assert(use_handle_v<typename Impl::value_type>);
            static_assert(sizeof(Impl) <= storage_size);

            new(ptr()) Impl(std::forward<Args>(args)...);
        }

        template <typename T>
        void construct(T&& val) noexcept
        {
            using type = std::remove_cvref_t<T>;
            using impl_t = std::conditional_t<
                detail::use_soo_handle<type> && sizeof(handle_impl_soo<type>) <= storage_size,
                handle_impl_soo<type>,
                handle_impl_ptr<type>>;

            construct_impl<impl_t>(std::forward<T>(val));
        }

        template <typename T>
        void construct(independent_t, T&& val)
        {
            using type = std::remove_cvref_t<T>;
            using impl_t = std::conditional_t<
                detail::use_soo_handle<type> && sizeof(handle_impl_soo<type>) <= storage_size,
                handle_impl_soo<type>,
                handle_impl_ptr<type>>;

            construct_impl<impl_t>(independent, std::forward<T>(val));
        }

        // Copy this handle to another uninitialized handle
        void copy(handle& other) const noexcept
        {
            ptr()->copy(other.ptr());
        }

        void move(handle& other) noexcept
        {
            ptr()->move(other.ptr());
        }

        void destroy() noexcept
        {
            ptr()->~handle_impl_base();
        }
    };

private:
    template <typename T>
    static consteval bool is_handle()
    {
        return std::same_as<std::remove_cvref_t<T>, handle>;
    }

public:
    using variant_type = std::variant<
        std::monostate,
        bool,
        utf::codepoint,
        int,
        unsigned int,
        long long int,
        unsigned long long int,
        float,
        double,
        long double,
        string_container_type,
        const void*,
        handle>;

    basic_format_arg() noexcept
        : m_val() {}

    basic_format_arg(const basic_format_arg&) noexcept = default;

    // Avoid accidentally mixing format arguments from different context
    template <typename AnotherContext>
    requires(!std::same_as<AnotherContext, Context>)
    basic_format_arg(const basic_format_arg<AnotherContext>&) = delete;

    basic_format_arg(basic_format_arg&& other) noexcept
        : m_val(std::move(other.m_val))
    {
        other.m_val.template emplace<std::monostate>();
    }

    basic_format_arg(bool val) noexcept
        : m_val(std::in_place_type<bool>, val) {}

    basic_format_arg(utf::codepoint cp) noexcept
        : m_val(std::in_place_type<utf::codepoint>, cp) {}

    template <char_like Char>
    basic_format_arg(Char ch) noexcept
        : m_val(std::in_place_type<utf::codepoint>, static_cast<char32_t>(ch))
    {}

    template <detail::acceptable_integral Integral>
    basic_format_arg(Integral val) noexcept
        : m_val(std::in_place_type<detail::convert_int_t<Integral>>, val)
    {}

    template <detail::acceptable_fp Float>
    basic_format_arg(Float val) noexcept
        : m_val(val)
    {}

    basic_format_arg(string_container_type str) noexcept
        : m_val(std::in_place_type<string_container_type>, std::move(str)) {}

    template <basic_string_like<char_type> String>
    basic_format_arg(String&& str)
        : m_val(std::in_place_type<string_container_type>, std::forward<String>(str))
    {}

    template <basic_string_like<char_type> String>
    basic_format_arg(independent_t, String&& str)
        : m_val(std::in_place_type<string_container_type>, independent, std::forward<String>(str))
    {}

    template <typename T, typename... Args>
    basic_format_arg(std::in_place_type_t<T>, Args&&... args)
        : m_val(std::in_place_type<T>, std::forward<Args>(args)...)
    {}

    template <typename T>
    requires(std::is_pointer_v<T> && !char_like<std::remove_pointer_t<T>>)
    basic_format_arg(T ptr) noexcept
        : m_val(std::in_place_type<const void*>, ptr)
    {}

    basic_format_arg(std::nullptr_t) noexcept
        : m_val(std::in_place_type<const void*>, nullptr) {}

    template <typename T>
    requires(use_handle_v<T>)
    basic_format_arg(const T& val) noexcept
        : m_val(std::in_place_type<handle>, val)
    {}

    template <typename T, std::size_t N>
    requires(!char_like<T>)
    basic_format_arg(T (&arr)[N]) noexcept
        : m_val(std::in_place_type<handle>, std::span<std::add_const_t<T>>(arr, N))
    {}

    template <typename T, std::size_t N>
    requires(!char_like<T>)
    basic_format_arg(const std::array<T, N>& arr) noexcept
        : m_val(std::in_place_type<handle>, std::span<const T>(arr.data(), N))
    {}

    template <typename T>
    requires(use_handle_v<T>)
    basic_format_arg(independent_t, T&& val) noexcept
        : m_val(std::in_place_type<handle>, independent, std::forward<T>(val))
    {}

    basic_format_arg(const std::type_info& info) noexcept
        : m_val(std::in_place_type<handle>, std::type_index(info)) {}

    basic_format_arg& operator=(const basic_format_arg&) = default;

    basic_format_arg& operator=(basic_format_arg&& rhs) noexcept
    {
        basic_format_arg(std::move(rhs)).swap(*this);

        return *this;
    }

    void swap(basic_format_arg& other) noexcept
    {
        using std::swap;

        swap(m_val, other.m_val);
    }

    friend void swap(basic_format_arg& lhs, basic_format_arg& rhs) noexcept
    {
        lhs.swap(rhs);
    }

    template <typename Visitor>
    decltype(auto) visit(Visitor&& vis) const // GCC needs this function to be defined in the front of the class
    {
        return std::visit(std::forward<Visitor>(vis), m_val);
    }

    [[nodiscard]]
    variant_type& to_variant() & noexcept
    {
        return m_val;
    }

    [[nodiscard]]
    variant_type&& to_variant() && noexcept
    {
        return std::move(m_val);
    }

    [[nodiscard]]
    const variant_type& to_variant() const& noexcept
    {
        return m_val;
    }

    [[nodiscard]]
    basic_format_arg index(const indexing_value_type& idx) const
    {
        return visit(
            [&idx]<typename T>(const T& v) -> basic_format_arg
            {
                if constexpr(is_handle<T>())
                {
                    return v.index(idx);
                }
                else
                {
                    using accessor_t = accessor_traits<T, Context>;
                    return accessor_t::access(v, idx);
                }
            }
        );
    }

    [[nodiscard]]
    basic_format_arg attribute(const attribute_name_type& attr) const
    {
        return visit(
            [&attr]<typename T>(const T& v) -> basic_format_arg
            {
                if constexpr(is_handle<T>())
                {
                    return v.attribute(attr);
                }
                else
                {
                    using accessor_t = accessor_traits<T, Context>;
                    return accessor_t::access(v, attr);
                }
            }
        );
    }

    template <typename T>
    [[nodiscard]]
    bool holds() const noexcept
    {
        return std::holds_alternative<T>(m_val);
    }

    [[nodiscard]]
    bool has_ownership() const noexcept
    {
        return visit(
            []<typename T>(const T& v) -> bool
            {
                if constexpr(requires() { v.has_ownership(); })
                {
                    return v.has_ownership();
                }
                else
                {
                    return true;
                }
            }
        );
    }

    [[nodiscard]]
    bool is_formattable() const noexcept;

    [[nodiscard]]
    bool empty() const noexcept
    {
        return holds<std::monostate>();
    }

    explicit operator bool() const noexcept
    {
        return !empty();
    }

    template <typename T>
    [[nodiscard]]
    friend const auto& get(const basic_format_arg& val)
    {
        if constexpr(char_like<T> || std::same_as<std::remove_cvref_t<T>, utf::codepoint>)
            return std::get<utf::codepoint>(val.m_val);
        else if constexpr(std::integral<T>)
            return std::get<detail::convert_int_t<T>>(val.m_val);
        else if constexpr(detail::acceptable_fp<T>)
            return std::get<T>(val.m_val);
        else if constexpr(basic_string_like<T, char_type>)
            return std::get<string_container_type>(val.m_val);
        else if constexpr(std::is_pointer_v<T>)
            return std::get<const void*>(val.m_val);
        else if constexpr(detail::use_handle<T, char_type>)
        {
            const handle& h = std::get<handle>(val.m_val);
            return handle_cast<T>(h);
        }
        else
        {
            static_assert(!sizeof(T), "Invalid type");
        }
    }

    void format(parse_context& parse_ctx, Context& out_ctx) const;

    void skip_spec(parse_context& parse_ctx);

private:
    variant_type m_val;
};

namespace detail
{
    template <typename T>
    consteval std::size_t count_if_named_arg() noexcept
    {
        if constexpr(is_named_arg_v<std::remove_cvref_t<T>>)
            return 1;
        else
            return 0;
    }

    template <typename... Ts>
    consteval std::size_t get_named_arg_count() noexcept
    {
        using tuple_t = std::tuple<Ts...>;
        using std::tuple_element_t;

        return []<std::size_t... Is>(std::index_sequence<Is...>) -> std::size_t
        {
            return (0 + ... + count_if_named_arg<tuple_element_t<Is, tuple_t>>());
        }(std::make_index_sequence<sizeof...(Ts)>());
    }

    template <typename... Ts>
    consteval std::size_t get_indexed_arg_count() noexcept
    {
        return sizeof...(Ts) - get_named_arg_count<Ts...>();
    }
} // namespace detail

/**
 * @brief Base of format argument storage.
 *
 * @tparam Context Format context @sa FormatContext
 */
template <typename Context, typename CharT = typename Context::char_type>
class format_args_base
{
public:
    using char_type = CharT;
    using string_type = std::basic_string<CharT>;
    using string_view_type = std::basic_string_view<CharT>;
    using string_container_type = utf::basic_string_container<CharT>;
    using format_arg_type = basic_format_arg<Context>;
    using indexing_value_type = basic_indexing_value<CharT>;
    using size_type = std::size_t;

    constexpr format_args_base() noexcept = default;
    constexpr format_args_base(const format_args_base&) noexcept = default;

    virtual constexpr ~format_args_base() = default;

    [[nodiscard]]
    virtual const format_arg_type& get(size_type i) const = 0;
    [[nodiscard]]
    virtual const format_arg_type& get(string_view_type key) const = 0;

    [[nodiscard]]
    const format_arg_type& get(const char_type* key) const
    {
        return get(string_view_type(key));
    }

    [[nodiscard]]
    const format_arg_type& get(const string_type& key) const
    {
        return get(string_view_type(key));
    }

    [[nodiscard]]
    virtual const format_arg_type& get(const indexing_value_type& idx) const
    {
        return idx.visit(
            [&]<typename T>(const T& v) -> const format_arg_type&
            {
                if constexpr(std::same_as<T, typename indexing_value_type::index_type>)
                {
                    if(v < 0)
                        throw_index_out_of_range();
                    size_type i = static_cast<size_type>(v);
                    return get(i);
                }
                else if constexpr(std::same_as<T, string_container_type>)
                {
                    return get(string_view_type(v));
                }
                else
                {
                    throw std::invalid_argument("invalid indexing value");
                }
            }
        );
    }

    [[nodiscard]]
    bool contains(size_type i) const noexcept
    {
        return i < indexed_size();
    }

    [[nodiscard]]
    virtual bool contains(string_view_type key) const noexcept = 0;

    [[nodiscard]]
    bool contains(const char_type* key) const noexcept
    {
        return contains(string_view_type(key));
    }

    [[nodiscard]]
    bool contains(const string_type& key) const noexcept
    {
        return contains(string_view_type(key));
    }

    [[nodiscard]]
    bool contains(const indexing_value_type& idx) const noexcept
    {
        return idx.visit(
            [this]<typename T>(const T& v) -> bool
            {
                if constexpr(std::same_as<T, typename indexing_value_type::index_type>)
                {
                    if(v < 0)
                        return false;
                    return contains(static_cast<size_type>(v));
                }
                else if constexpr(std::same_as<T, string_container_type>)
                {
                    return contains(string_view_type(v));
                }
                else
                {
                    return false;
                }
            }
        );
    }

    virtual size_type indexed_size() const noexcept = 0;
    virtual size_type named_size() const noexcept = 0;

    const format_arg_type& operator[](const indexing_value_type& idx) const
    {
        return get(idx);
    }

protected:
    [[noreturn]]
    static void throw_index_out_of_range()
    {
        throw std::out_of_range("index out of range");
    }

    [[noreturn]]
    static void throw_invalid_named_argument()
    {
        throw std::out_of_range("invalid named argument");
    }
};

PAPILIO_EXPORT template <typename T, typename Context = format_context>
struct is_format_args : public std::is_base_of<format_args_base<Context>, T>
{};

PAPILIO_EXPORT template <typename T, typename Context = format_context>
constexpr inline bool is_format_args_v = is_format_args<T, Context>::value;

PAPILIO_EXPORT template <
    typename Context,
    typename CharT = typename Context::char_type>
class basic_empty_format_args final : public format_args_base<Context, CharT>
{
public:
    using char_type = CharT;
    using size_type = std::size_t;
    using string_view_type = std::basic_string_view<char_type>;
    using format_arg_type = basic_format_arg<Context>;

    basic_empty_format_args() noexcept = default;

    const format_arg_type& get(size_type i) const override
    {
        (void)i;
        this->throw_index_out_of_range();
    }

    const format_arg_type& get(string_view_type key) const override
    {
        (void)key;
        this->throw_invalid_named_argument();
    }

    bool contains(string_view_type key) const noexcept override
    {
        (void)key;
        return false;
    }

    size_type indexed_size() const noexcept override
    {
        return 0;
    }

    size_type named_size() const noexcept override
    {
        return 0;
    }
};

PAPILIO_EXPORT template <typename Context>
const basic_empty_format_args<Context>& empty_format_args_for() noexcept
{
    static basic_empty_format_args<Context> args;
    return args;
}

PAPILIO_EXPORT template <
    std::size_t IndexedArgumentCount,
    std::size_t NamedArgumentCount,
    typename Context = format_context,
    typename CharT = typename Context::char_type>
class static_format_args final : public format_args_base<Context, CharT>
{
    using my_base = format_args_base<Context, CharT>;

public:
    static_assert(std::same_as<typename Context::char_type, CharT>);

    using char_type = CharT;
    using size_type = std::size_t;
    using string_view_type = std::basic_string_view<char_type>;
    using format_arg_type = basic_format_arg<Context>;

    using vector_type = fixed_vector<
        format_arg_type,
        IndexedArgumentCount>;
    using map_type = fixed_flat_map<
        string_view_type,
        format_arg_type,
        NamedArgumentCount,
        std::less<>>;

    template <typename... Args>
    static_format_args(Args&&... args)
    {
        construct(std::forward<Args>(args)...);
    }

    [[nodiscard]]
    const format_arg_type& get(size_type i) const override
    {
        if(i >= m_indexed_args.size())
            this->throw_index_out_of_range();
        return m_indexed_args[i];
    }

    [[nodiscard]]
    const format_arg_type& get(string_view_type k) const override
    {
        auto it = m_named_args.find(k);
        if(it == m_named_args.end())
            this->throw_invalid_named_argument();
        return it->second;
    }

    using my_base::get;

    [[nodiscard]]
    bool contains(string_view_type key) const noexcept override
    {
        return m_named_args.contains(key);
    }

    using my_base::contains;

    [[nodiscard]]
    size_type indexed_size() const noexcept override
    {
        PAPILIO_ASSERT(m_indexed_args.size() == IndexedArgumentCount);
        return IndexedArgumentCount;
    }

    [[nodiscard]]
    size_type named_size() const noexcept override
    {
        PAPILIO_ASSERT(m_named_args.size() == NamedArgumentCount);
        return NamedArgumentCount;
    }

private:
    template <typename... Args>
    void construct(Args&&... args) noexcept
    {
        static_assert(
            detail::get_indexed_arg_count<Args...>() == IndexedArgumentCount,
            "invalid indexed argument count"
        );
        static_assert(
            detail::get_named_arg_count<Args...>() == NamedArgumentCount,
            "invalid named argument count"
        );

        (emplace(std::forward<Args>(args)), ...);
    }

    vector_type m_indexed_args;
    map_type m_named_args;

    template <typename T>
    requires(!is_named_arg_v<T>)
    void emplace(T&& val) noexcept(std::is_nothrow_constructible_v<format_arg_type, T>)
    {
        m_indexed_args.emplace_back(std::forward<T>(val));
    }

    template <typename T>
    requires(is_named_arg_v<T> && std::same_as<char_type, typename T::char_type>)
    void emplace(T&& na) noexcept(std::is_nothrow_constructible_v<format_arg_type, typename T::value_type>)
    {
        m_named_args.insert_or_assign(
            na.name,
            PAPILIO_NS forward_like<T>(na.value)
        );
    }
};

PAPILIO_EXPORT template <typename Context, typename CharT>
class basic_dynamic_format_args : public format_args_base<Context, CharT>
{
    using my_base = format_args_base<Context, CharT>;

public:
    static_assert(std::same_as<typename Context::char_type, CharT>);

    using char_type = CharT;
    using string_type = std::basic_string<char_type>;
    using string_view_type = std::basic_string_view<char_type>;
    using size_type = std::size_t;
    using format_arg_type = basic_format_arg<Context>;

    using vector_type = small_vector<
        format_arg_type,
        6>;
    using map_type = std::map<
        string_type,
        format_arg_type,
        std::less<>>;

    basic_dynamic_format_args() = default;
    basic_dynamic_format_args(const basic_dynamic_format_args&) = delete;
    basic_dynamic_format_args(basic_dynamic_format_args&&) = default;

    template <typename... Args>
    basic_dynamic_format_args(Args&&... args)
    {
        append(std::forward<Args>(args)...);
    }

    template <typename T>
    void emplace(T&& val)
    {
        if constexpr(is_named_arg_v<std::remove_cvref_t<T>>)
        {
            static_assert(
                std::same_as<char_type, typename T::char_type>,
                "Invalid char type"
            );

            m_named_args.emplace(std::make_pair(
                PAPILIO_NS forward_like<T>(val.name),
                PAPILIO_NS forward_like<T>(val.value)
            ));
        }
        else
        {
            m_indexed_args.emplace_back(std::forward<T>(val));
        }
    }

    template <typename... Args>
    requires(sizeof...(Args) >= 1)
    void append(Args&&... args)
    {
        m_indexed_args.reserve(
            m_indexed_args.size() + detail::get_indexed_arg_count<Args...>()
        );

        (emplace(std::forward<Args>(args)), ...);
    }

    [[nodiscard]]
    const format_arg_type& get(size_type i) const override
    {
        if(i >= m_indexed_args.size())
            this->throw_index_out_of_range();
        return m_indexed_args[i];
    }

    [[nodiscard]]
    const format_arg_type& get(string_view_type key) const override
    {
        auto it = m_named_args.find(key);
        if(it == m_named_args.end())
            this->throw_invalid_named_argument();
        return it->second;
    }

    using my_base::get;

    [[nodiscard]]
    bool contains(string_view_type key) const noexcept override
    {
        return m_named_args.contains(key);
    }

    using my_base::contains;

    [[nodiscard]]
    const vector_type& indexed() const noexcept
    {
        return m_indexed_args;
    }

    [[nodiscard]]
    const map_type& named() const noexcept
    {
        return m_named_args;
    }

    [[nodiscard]]
    size_type indexed_size() const noexcept override
    {
        return m_indexed_args.size();
    }

    [[nodiscard]]
    size_type named_size() const noexcept override
    {
        return m_named_args.size();
    }

    void clear() noexcept
    {
        m_indexed_args.clear();
        m_named_args.clear();
    }

private:
    vector_type m_indexed_args;
    map_type m_named_args;
};

/**
 * @brief Type-erased reference to format arguments.
 */
PAPILIO_EXPORT template <typename Context, typename CharT>
class basic_format_args_ref final : public format_args_base<Context, CharT>
{
    using my_base = format_args_base<Context, CharT>;

public:
    using char_type = CharT;
    using string_type = std::basic_string<char_type>;
    using string_view_type = std::basic_string_view<char_type>;
    using size_type = std::size_t;
    using format_arg_type = basic_format_arg<Context>;

    basic_format_args_ref() = delete;
    constexpr basic_format_args_ref(const basic_format_args_ref&) noexcept = default;

    template <std::derived_from<my_base> T>
    constexpr basic_format_args_ref(const T& args) noexcept
        : m_ptr(&args)
    {
        PAPILIO_ASSERT(m_ptr != this); // avoid circular reference
    }

    [[nodiscard]]
    const format_arg_type& get(size_type i) const override
    {
        return m_ptr->get(i);
    }

    [[nodiscard]]
    const format_arg_type& get(string_view_type k) const override
    {
        return m_ptr->get(k);
    }

    using my_base::get;

    [[nodiscard]]
    size_type indexed_size() const noexcept override
    {
        return m_ptr->indexed_size();
    }

    [[nodiscard]]
    size_type named_size() const noexcept override
    {
        return m_ptr->named_size();
    }

    [[nodiscard]]
    bool contains(string_view_type k) const noexcept override
    {
        return m_ptr->contains(k);
    }

    using my_base::contains;

private:
    const my_base* m_ptr;
};

namespace detail
{
    template <typename T, typename FormatContext>
    concept check_member_format_simple = requires(const T& val, FormatContext& fmt_ctx) {
        {
            val.format(fmt_ctx)
        } -> std::same_as<typename FormatContext::iterator>;
    };

    template <typename T, typename FormatContext>
    concept check_member_format_complex = requires(
        const T& val, basic_format_parse_context<FormatContext>& parse_ctx, FormatContext& fmt_ctx
    ) {
        {
            val.format(parse_ctx, fmt_ctx)
        } -> std::same_as<typename FormatContext::iterator>;
    };

    template <typename T, typename FormatContext>
    concept check_member_format =
        check_member_format_simple<T, FormatContext> ||
        check_member_format_complex<T, FormatContext>;

} // namespace detail

PAPILIO_EXPORT template <
    typename T,
    typename FormatContext = format_context>
struct has_member_format :
    public std::bool_constant<detail::check_member_format<T, FormatContext>>
{};

PAPILIO_EXPORT template <
    typename T,
    typename FormatContext = format_context>
inline constexpr bool has_member_format_v =
    has_member_format<T, FormatContext>::value;

PAPILIO_EXPORT template <typename T, typename FormatContext = format_context>
class member_format_adaptor
{
public:
    using char_type = typename FormatContext::char_type;

    using parse_context = basic_format_parse_context<FormatContext>;

    auto format(const T& val, parse_context& parse_ctx, FormatContext& fmt_ctx) const
        -> typename FormatContext::iterator
    {
        if constexpr(detail::check_member_format_simple<T, FormatContext>)
        {
            return val.format(fmt_ctx);
        }
        else
        {
            return val.format(parse_ctx, fmt_ctx);
        }
    }
};

namespace detail
{
    template <typename T, typename FormatContext>
    concept check_adl_format_simple = requires(T&& val, FormatContext& fmt_ctx) {
        {
            format(val, fmt_ctx)
        } -> std::same_as<typename FormatContext::iterator>;
    };

    template <typename T, typename FormatContext>
    concept check_adl_format_complex = requires(
        T&& val, basic_format_parse_context<FormatContext>& parse_ctx, FormatContext& fmt_ctx
    ) {
        {
            format(val, parse_ctx, fmt_ctx)
        } -> std::same_as<typename FormatContext::iterator>;
    };

    template <typename T, typename FormatContext>
    concept check_adl_format =
        check_adl_format_simple<T, FormatContext> ||
        check_adl_format_complex<T, FormatContext>;

    template <typename T, typename ParseContext, typename FormatContext>
    auto invoke_adl_format(T&& val, ParseContext& parse_ctx, FormatContext& fmt_ctx)
        -> typename FormatContext::iterator
    {
        if constexpr(detail::check_adl_format_simple<T, FormatContext>)
        {
            return format(val, fmt_ctx);
        }
        else
        {
            return format(val, parse_ctx, fmt_ctx);
        }
    }
} // namespace detail

PAPILIO_EXPORT template <
    typename T,
    typename FormatContext = format_context>
struct has_adl_format :
    public std::bool_constant<detail::check_adl_format<T, FormatContext>>
{};

PAPILIO_EXPORT template <
    typename T,
    typename FormatContext = format_context>
inline constexpr bool has_adl_format_v =
    has_adl_format<T, FormatContext>::value;

PAPILIO_EXPORT template <typename T, typename FormatContext = format_context>
class adl_format_adaptor
{
public:
    using char_type = typename FormatContext::char_type;

    using parse_context = basic_format_parse_context<FormatContext>;

    auto format(const T& val, parse_context& parse_ctx, FormatContext& fmt_ctx) const
        -> typename FormatContext::iterator
    {
        // Because the adaptor already has a format() member,
        // so it need a global function as helper to use ADL.
        return PAPILIO_NS detail::invoke_adl_format(
            val, parse_ctx, fmt_ctx
        );
    }
};

template <typename T, typename CharT = char>
concept streamable =
    requires(std::basic_ostream<CharT>& os, const T& val) {
        {
            os << val
        } -> std::convertible_to<std::basic_ostream<CharT>&>;
    };

PAPILIO_EXPORT template <typename T, typename CharT>
requires streamable<T, CharT>
class streamable_formatter
{
public:
    using char_type = CharT;

    template <typename ParseContext>
    auto parse(ParseContext& ctx);

    template <typename Context>
    auto format(const T& val, Context& ctx) const;

private:
    simple_formatter_data m_data;

    template <typename FormatContext>
    void setup_locale(std::basic_ostream<CharT>& os, FormatContext& ctx) const
    {
        if(m_data.use_locale)
            os.imbue(ctx.getloc());
        else
            os.imbue(std::locale::classic());
    }
};

/// @defgroup FormatContext Format context
/// @brief APIs for implementing formatting output.
/// @ingroup Format
/// @{

namespace detail
{
    enum class formatter_tag
    {
        disabled = 0,
        ordinary = 1,
        member_func = 2,
        adl_func = 3,
        stream = 4
    };

    template <
        typename T,
        typename Context,
        typename CharT = typename Context::char_type>
    consteval formatter_tag get_formatter_tag()
    {
        using fmt_t = formatter<T, CharT>;
        constexpr bool fmt_semiregular = std::semiregular<fmt_t>;

        if constexpr(std::is_base_of_v<disabled_formatter, fmt_t>)
        {
            return formatter_tag::disabled;
        }
        else if constexpr(!fmt_semiregular)
        {
            if constexpr(has_member_format_v<T, Context>)
                return formatter_tag::member_func;
            else if constexpr(has_adl_format_v<T, Context>)
                return formatter_tag::adl_func;
            else if constexpr(streamable<T, CharT>)
                return formatter_tag::stream;
        }

        return formatter_tag::ordinary;
    }

    // For formatter_tag::ordinary and formatter_tag::disabled.
    // Because a disabled formatter class is derived from papilio::disabled_formatter,
    // so we can directly use the ordinary formatter to trigger an error.
    template <formatter_tag Tag, typename T, typename Context, typename CharT>
    struct select_formatter_impl
    {
        using type = formatter<T, CharT>;
    };

    template <typename T, typename Context, typename CharT>
    struct select_formatter_impl<formatter_tag::member_func, T, Context, CharT>
    {
        using type = member_format_adaptor<T, Context>;
    };

    template <typename T, typename Context, typename CharT>
    struct select_formatter_impl<formatter_tag::adl_func, T, Context, CharT>
    {
        using type = adl_format_adaptor<T, Context>;
    };

    template <typename T, typename Context, typename CharT>
    struct select_formatter_impl<formatter_tag::stream, T, Context, CharT>
    {
        using type = streamable_formatter<T, CharT>;
    };
} // namespace detail

/**
 * @brief Select an appropriate formatter
 *
 * @tparam T Type to be formatted
 * @tparam Context Format context
 */
template <typename T, typename Context>
struct select_formatter
{
    using type = typename detail::select_formatter_impl<
        PAPILIO_NS detail::get_formatter_tag<T, Context>(),
        T,
        Context,
        typename Context::char_type>::type;
};

template <typename T, typename Context>
using select_formatter_t = typename select_formatter<T, Context>::type;

/**
 * @brief Format context
 *
 * @sa format_context_traits
 *
 * @tparam OutputIt Output iterator
 * @tparam CharT Char type
 */
PAPILIO_EXPORT template <typename OutputIt, typename CharT>
class basic_format_context
{
public:
    using char_type = CharT;
    using iterator = OutputIt;
    using format_args_type = basic_format_args_ref<basic_format_context, char_type>;

    using indexing_value_type = basic_indexing_value<CharT>;
    using attribute_name_type = basic_attribute_name<CharT>;

    /*
     * @brief Rebind the context to a new output iterator type
     *
     * @tparam AnotherOutputIt New output iterator type
     */
    template <typename AnotherOutputIt>
    struct rebind
    {
        using type = basic_format_context<AnotherOutputIt, CharT>;
    };

    template <typename T>
    using formatter_type = select_formatter_t<T, basic_format_context>;

    basic_format_context(iterator it, format_args_type args)
        : m_out(std::move(it)), m_data(args) {}

    basic_format_context(
        const std::locale& loc, iterator it, format_args_type args
    )
        : m_out(std::move(it)), m_data(args, loc)
    {}

    basic_format_context(
        locale_ref loc, iterator it, format_args_type args
    )
        : m_out(std::move(it)), m_data(args, loc)
    {}

    [[nodiscard]]
    iterator out()
    {
        return m_out;
    }

    const iterator& out_ref() const
    {
        return m_out;
    }

    void advance_to(iterator it)
    {
        m_out = std::move(it);
    }

    [[nodiscard]]
    const format_args_type& get_args() const noexcept
    {
        return m_data.args;
    }

    [[nodiscard]]
    std::locale getloc() const requires(!xchar<CharT>)
    {
        return getloc_ref().get();
    }

    [[nodiscard]]
    locale_ref getloc_ref() const noexcept requires(!xchar<CharT>)
    {
        return m_data.loc;
    }

private:
    iterator m_out;

    struct data
    {
        format_args_type args;
        locale_ref loc;

        data(format_args_type args_, locale_ref loc_)
            : args(std::move(args_)), loc(loc_) {}

        data(format_args_type args_)
            : args(std::move(args_)), loc() {}
    };

    struct data_no_loc
    {
        format_args_type args;

        template <typename... Ignored>
        data_no_loc(format_args_type args_, Ignored&&...)
            : args(std::move(args_))
        {}
    };

    // locale is only for char and wchar_t
    using data_t = std::conditional_t<
        xchar<CharT>,
        data_no_loc,
        data>;

    data_t m_data;
};

/**
 * @brief Traits for the format context.
 *
 * It is useful to implement formatters for user-defined types.
 *
 * @sa basic_format_context
 *
 * @tparam Context Context type
 */
PAPILIO_EXPORT template <typename Context>
class format_context_traits
{
public:
    using char_type = typename Context::char_type;
    using int_type = std::uint32_t;
    using string_type = std::basic_string<char_type>;
    using string_view_type = std::basic_string_view<char_type>;
    using context_type = Context;
    using iterator = typename Context::iterator;
    using format_arg_type = format_arg;
    using format_args_type = typename Context::format_args_type;

    template <typename T>
    using formatter_type = typename Context::template formatter_type<T>;

    template <typename AnotherOutputIt>
    static constexpr bool has_rebind() noexcept
    {
        return requires {
            typename Context::template rebind<AnotherOutputIt>;
        };
    }

    /**
     * @brief Check if context supports locale
     */
    static constexpr bool use_locale() noexcept
    {
        return requires(context_type& ctx) {
            { ctx.getloc() } -> std::convertible_to<std::locale>;
        };
    }

    template <typename AnotherOutputIt>
    requires(has_rebind<AnotherOutputIt>())
    using rebind = typename Context::template rebind<AnotherOutputIt>;

    format_context_traits() = delete;

    /**
     * @brief Create a rebound context with empty format arguments
     */
    template <typename AnotherOutputIt>
    requires(has_rebind<AnotherOutputIt>())
    static auto rebind_context(context_type& ctx, AnotherOutputIt it)
    {
        using result_type = typename rebind<AnotherOutputIt>::type;
        using result_traits = format_context_traits<result_type>;

        if constexpr(result_traits::use_locale())
        {
            return result_type(
                getloc_ref(ctx),
                std::move(it),
                empty_format_args_for<result_type>()
            );
        }
        else
        {
            return result_type(
                std::move(it),
                empty_format_args_for<result_type>()
            );
        }
    }

private:
    static void append_hex_digits(context_type& ctx, int_type val, bool is_valid)
    {
        if(is_valid)
        {
            format_to(
                ctx,
                PAPILIO_TSTRING_VIEW(char_type, "\\u{{{:x}}}"),
                val
            );
        }
        else
        {
            format_to(
                ctx,
                PAPILIO_TSTRING_VIEW(char_type, "\\x{{{:x}}}"),
                val
            );
        }
    }

    template <bool DoubleQuote, bool SingleQuote>
    static void append_as_esc_seq(context_type& ctx, int_type val)
    {
        switch(val)
        {
        default:
other_ch:
            append_hex_digits(ctx, val, true);
            break;

        case '\t':
            append(ctx, PAPILIO_TSTRING_VIEW(char_type, "\\t"));
            break;

        case '\n':
            append(ctx, PAPILIO_TSTRING_VIEW(char_type, "\\n"));
            break;

        case '\r':
            append(ctx, PAPILIO_TSTRING_VIEW(char_type, "\\r"));
            break;

        case '\\':
            append(ctx, PAPILIO_TSTRING_VIEW(char_type, "\\\\"));
            break;

        case '"':
            if constexpr(DoubleQuote)
            {
                append(ctx, PAPILIO_TSTRING_VIEW(char_type, "\\\""));
                break;
            }
            else
                goto other_ch;


        case '\'':
            if constexpr(SingleQuote)
            {
                append(ctx, PAPILIO_TSTRING_VIEW(char_type, "\\'"));
                break;
            }
            else
                goto other_ch;
        }
    }

    template <bool DoubleQuote, bool SingleQuote>
    static bool has_esc_seq(int_type val) noexcept
    {
        return val < ' ' ||
               val == '\t' ||
               val == '\n' ||
               val == '\r' ||
               val == '\\' ||
               (DoubleQuote && val == '"') ||
               (SingleQuote && val == '\'') ||
               val < U' ';
    }

public:
    [[nodiscard]]
    static locale_ref getloc_ref(context_type& ctx)
    {
        if constexpr(!use_locale())
            return locale_ref();
        else
        {
            constexpr bool has_getloc_ref = requires() {
                { ctx.getloc_ref() } -> std::same_as<locale_ref>;
            };

            if constexpr(has_getloc_ref)
                return ctx.getloc_ref();
            else
                return ctx.getloc();
        }
    }

    /**
     * @brief Get the output iterator from the context.
     *
     * @param ctx Format context
     * @return iterator The output iterator
     */
    [[nodiscard]]
    static iterator out(context_type& ctx)
    {
        return ctx.out();
    }

    /**
     * @brief Advance the output iterator to a new position
     *
     * @param ctx Format context
     * @param it New iterator position
     */
    static void advance_to(context_type& ctx, iterator it)
    {
        ctx.advance_to(std::move(it));
    }

    [[nodiscard]]
    static const format_args_type& get_args(context_type& ctx) noexcept
    {
        return ctx.get_args();
    }

    /**
     * @brief Append content from an iterator range `[begin, end)`.
     *
     * @param ctx Format context
     * @param begin Begin iterator
     * @param end End iterator
     */
    template <typename InputIt>
    static void append(context_type& ctx, InputIt begin, InputIt end)
    {
        advance_to(ctx, std::copy(begin, end, out(ctx)));
    }

    /**
     * @brief Append content from a string.
     *
     * @param ctx Format context
     * @param str Content
     */
    static void append(context_type& ctx, string_view_type str)
    {
        append(ctx, str.begin(), str.end());
    }

    /**
     * @brief Append characters to the format context.
     *
     * @param ctx Format context
     * @param ch The character
     * @param count Number of times to append the character
     *
     * @note  This function will automatically convert the encoding if necessary.
     *
     * @sa utf::codepoint
     */
    template <char_like Char>
    static void append(context_type& ctx, Char ch, std::size_t count = 1)
    {
        if constexpr(sizeof(Char) <= sizeof(char_type))
        {
            advance_to(
                ctx,
                std::fill_n(out(ctx), count, static_cast<char_type>(ch))
            );
        }
        else
        {
            utf::codepoint cp(static_cast<char32_t>(ch));
            append(ctx, cp, count);
        }
    }

    /**
     * @brief Append a Unicode code point to the format context.
     *
     * @param ctx Format context
     * @param cp The code point
     * @param count Number of times to append the code point
     */
    static void append(context_type& ctx, utf::codepoint cp, std::size_t count = 1)
    {
        for(std::size_t i = 0; i < count; ++i)
        {
            advance_to(ctx, cp.append_to_as<char_type>(out(ctx)));
        }
    }

    /**
     * @brief Append a Unicode code point to the format context, using escaped sequences if possible.
     *
     * @param ctx Format context
     * @param cp The code point
     * @param count Number of times to append the code point
     *
     * @code{.cpp}
     * append_escaped(ctx, U'\''); // Appends ['\\', '\'']
     * append_escaped(ctx, U' ');  // Appends [' ']
     * append_escaped(ctx, U'"');  // Appends ['"']
     * @endcode
     */
    static void append_escaped(context_type& ctx, utf::codepoint cp, std::size_t count = 1)
    {
        for(std::size_t i = 0; i < count; ++i)
        {
            std::uint32_t ch = static_cast<char32_t>(cp);
            if(has_esc_seq<false, true>(ch))
            {
                append_as_esc_seq<false, true>(ctx, ch);
            }
            else
            {
                advance_to(ctx, cp.append_to_as<char_type>(out(ctx)));
            }
        }
    }

#ifdef PAPILIO_COMPILER_CLANG
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wsign-conversion"
#endif

private:
    static void append_escaped_impl(context_type& ctx, string_view_type str)
        requires(char8_like<char_type>)
    {
        std::size_t i = 0;
        while(i < str.size())
        {
            if(PAPILIO_NS utf::is_leading_byte(str[i]))
            {
                std::uint8_t size_bytes = PAPILIO_NS utf::byte_count(str[i]);
                if(i + size_bytes > str.size())
                {
                    for(std::size_t j = i; j < str.size(); ++j)
                    {
                        append_hex_digits(ctx, static_cast<std::uint8_t>(str[j]), false);
                    }
                    return;
                }

                if(has_esc_seq<true, false>(str[i]))
                {
                    append_as_esc_seq<true, false>(ctx, str[i]);
                }
                else if(std::all_of(
                            str.begin() + i + 1,
                            str.begin() + i + size_bytes,
                            &utf::is_trailing_byte
                        ))
                {
                    append(
                        ctx,
                        string_view_type(
                            str.begin() + i,
                            str.begin() + i + size_bytes
                        )
                    );
                }
                else
                {
                    auto stop = std::find_if_not(
                        str.begin() + i + 1,
                        str.begin() + i + size_bytes,
                        &utf::is_trailing_byte
                    );

                    for(auto it = str.begin() + i; it != stop; ++it)
                    {
                        append_hex_digits(ctx, static_cast<std::uint8_t>(*it), false);
                    }

                    i += std::distance(str.begin() + i, stop);
                    continue;
                }

                i += size_bytes;
            }
            else
            {
                append_hex_digits(ctx, str[i], false);
                ++i;
            }
        }
    }

    static void append_escaped_impl(context_type& ctx, string_view_type str)
        requires(char16_like<char_type>)
    {
        std::size_t i = 0;
        while(i < str.size())
        {
            std::uint16_t ch = static_cast<std::uint16_t>(str[i]);
            if(has_esc_seq<true, false>(ch))
            {
                append_as_esc_seq<true, false>(ctx, ch);
                ++i;
            }
            else if(PAPILIO_NS utf::is_high_surrogate(ch))
            {
                if(i + 1 >= str.size())
                {
                    append_hex_digits(ctx, ch, false);
                    return;
                }
                else if(!PAPILIO_NS utf::is_low_surrogate(str[i + 1]))
                {
                    append_hex_digits(ctx, ch, false);
                }
                else
                {
                    append(ctx, str.begin() + i, str.begin() + i + 2);
                }

                i += 2;
            }
            else
            {
                if(PAPILIO_NS utf::is_low_surrogate(ch))
                {
                    append_hex_digits(ctx, ch, false);
                }
                else
                {
                    append(ctx, ch, 1);
                }

                ++i;
            }
        }
    }

    static void append_escaped_impl(context_type& ctx, string_view_type str)
        requires(char32_like<char_type>)
    {
        for(char_type ch : str)
        {
            if(has_esc_seq<true, false>(static_cast<std::uint32_t>(ch)))
            {
                append_as_esc_seq<true, false>(ctx, static_cast<std::uint32_t>(ch));
            }
            else
            {
                append(ctx, ch, 1);
            }
        }
    }

public:
    /**
     * @brief Append content from a string, using escaped sequences if possible.
     *
     * @param ctx Format context
     * @param str Content
     *
     * @code{.cpp}
     * append_escaped(ctx, "hello\t");  // Appends "hello\\t"
     * // Invalid UTF-8
     * append_escaped(ctx, "\xc3\x28"); // Appends "\x{c3}("
     * @endcode
     */
    static void append_escaped(context_type& ctx, string_view_type str)
    {
        append_escaped_impl(ctx, str);
    }

#ifdef PAPILIO_COMPILER_CLANG
#    pragma clang diagnostic pop
#endif

    /**
     * @brief Pack format arguments into a `format_args`-like object.
     *
     * @param args Format arguments
     * @return @ref static_format_args "Packed arguments"
     *
     * @note `format_args`-like objects cannot be nested, i.e., there cannot be
     *       another `format_args`-like object in the arguments. This is to prevent accidental
     *       nesting when using format arguments.
     *
     * @sa static_format_args
     * @sa basic_dynamic_format_args
     * @sa basic_format_args_ref
     */
    template <typename... Args>
    static auto make_format_args(Args&&... args)
    {
        static_assert(
            std::conjunction_v<std::negation<is_format_args<Args, Context>>...>,
            "cannot use format_args as format argument"
        );

        using result_type = static_format_args<
            detail::get_indexed_arg_count<Args...>(),
            detail::get_named_arg_count<Args...>(),
            Context,
            char_type>;
        return result_type(std::forward<Args>(args)...);
    }

    using format_args_ref_type = basic_format_args_ref<Context, char_type>;
    template <typename... Args>
    using format_string_type = basic_format_string<char_type, std::type_identity_t<Args>...>;

    static void vformat_to(
        context_type& ctx,
        string_view_type fmt,
        const format_args_ref_type& args
    );

    template <typename... Args>
    static void format_to(
        context_type& ctx,
        format_string_type<Args...> fmt,
        Args&&... args
    )
    {
        vformat_to(ctx, fmt.get(), make_format_args(std::forward<Args>(args)...));
    }

    /**
     * @brief Append formatting result of default format specification (`{}`)
     */
    template <typename T>
    static void append_by_format(
        context_type& ctx,
        T&& val
    )
    {
        const char_type fmt[2] = {char_type('{'), char_type('}')};
        format_to(ctx, string_view_type(fmt, 2), std::forward<T>(val));
    }

    template <typename T>
    static void append_by_formatter(
        context_type& ctx,
        T&& val,
        bool try_debug_format = false
    );
};

/// @}

/// @addtogroup Format
///@{

PAPILIO_EXPORT template <typename Context = format_context, typename... Args>
auto make_format_args(Args&&... args)
{
    using context_t = format_context_traits<Context>;
    return context_t::make_format_args(std::forward<Args>(args)...);
}

PAPILIO_EXPORT template <typename... Args>
auto make_wformat_args(Args&&... args)
{
    using context_t = format_context_traits<wformat_context>;
    return context_t::make_format_args(std::forward<Args>(args)...);
}

/// @}

/// @defgroup Parse
/// @brief APIs for parsing the format string
/// @ingroup Format
/// @{

/**
 * @brief Format parse context
 */
PAPILIO_EXPORT template <typename FormatContext>
class basic_format_parse_context
{
public:
    using char_type = typename FormatContext::char_type;
    using string_type = std::basic_string<char_type>;
    using string_view_type = std::basic_string_view<char_type>;
    using string_ref_type = utf::basic_string_ref<char_type>;
    using const_iterator = typename string_ref_type::const_iterator;
    using iterator = const_iterator;
    using size_type = std::size_t;
    using format_context_type = FormatContext;
    using format_args_type = basic_format_args_ref<FormatContext, char_type>;

    basic_format_parse_context() = delete;
    basic_format_parse_context(const basic_format_parse_context&) = delete;

    /**
     * @brief Construct a format parse context
     *
     * @param str The format string @ref basic_format_string
     * @param args Format arguments @ref basic_format_arg
     */
    basic_format_parse_context(string_ref_type str, format_args_type args) noexcept
        : m_ref(str), m_args(args)
    {
        m_it = m_ref.begin();
    }

    [[nodiscard]]
    const format_args_type& get_args() const noexcept
    {
        return m_args;
    }

    void enable_manual_indexing() noexcept
    {
        m_manual_indexing = true;
    }

    void advance_to(iterator it) noexcept
    {
        m_it = it;
    }

    iterator begin() const noexcept
    {
        return m_it;
    }

    iterator end() const noexcept
    {
        return m_ref.end();
    }

    [[nodiscard]]
    size_type current_arg_id() const
    {
        if(m_manual_indexing)
            invalid_default_argument();
        return m_default_arg_idx;
    }

    size_type next_arg_id()
    {
        if(m_manual_indexing)
            invalid_default_argument();
        ++m_default_arg_idx;
        return m_default_arg_idx;
    }

    void check_arg_id(size_type i) const
    {
        enable_manual_indexing();
        if(!get_args().contains(i))
            throw format_error("invalid arg id");
    }

    void check_arg_id(string_view_type name) const
    {
        if(!get_args().contains(name))
            throw format_error("invalid arg id");
    }

    [[nodiscard]]
    bool manual_indexing() const noexcept
    {
        return m_manual_indexing;
    }

    [[noreturn]]
    static void invalid_default_argument()
    {
        throw format_error("no default argument after an explicit argument");
    }

    /**
     * @brief Default implementation of skipping unused format specification.
     *
     * @warning This function cannot skip specification that contains unbalanced braces (`{` and `}`)!
     */
    void skip_spec()
    {
        auto it = begin();

        std::size_t counter = 0;
        for(; it != end(); ++it)
        {
            char32_t ch = *it;
            if(ch == U'{')
                ++counter;
            if(ch == U'}')
            {
                if(counter == 0)
                    break;
                --counter;
            }
        }

        advance_to(it);
    }

private:
    string_ref_type m_ref;
    iterator m_it;
    format_args_type m_args;
    size_type m_default_arg_idx = 0;
    mutable bool m_manual_indexing = false;

    void enable_manual_indexing() const noexcept
    {
        m_manual_indexing = true;
    }
};

namespace detail
{
    template <typename Formatter, typename ParseContext>
    concept check_parse_method = requires(Formatter& f, ParseContext parse_ctx) {
        {
            f.parse(parse_ctx)
        } -> std::same_as<typename ParseContext::iterator>;
    };
} // namespace detail

/// @}

/// @defgroup Formatter Formatters for various types
/// @{

namespace detail
{
    template <typename T>
    struct formatter_traits_helper
    {
        using char_type = typename T::char_type;
    };

    template <typename T, typename CharT>
    struct formatter_traits_helper<formatter<T, CharT>>
    {
        using char_type = CharT;
    };
} // namespace detail

/**
 * @brief Traits for formatters
 */
PAPILIO_EXPORT template <typename Formatter>
struct formatter_traits
{
    using char_type = typename detail::formatter_traits_helper<Formatter>::char_type;
    using string_view_type = std::basic_string_view<char_type>;

    /**
     * @brief Returns true if the formatter has a @b standalone parse function
     */
    template <typename ParseContext = format_parse_context>
    static constexpr bool parsable() noexcept
    {
        return detail::check_parse_method<Formatter, ParseContext>;
    }

    /**
     * @brief Parse the format specification and format the value
     *
     * @param val The value
     * @param parse_ctx Parse context
     * @param fmt_ctx Format context
     */
    template <typename T, typename ParseContext, typename FormatContext>
    static void format(Formatter& fmt, T&& val, ParseContext& parse_ctx, FormatContext& fmt_ctx)
    {
        using context_t = format_context_traits<FormatContext>;

        if constexpr(!parsable<ParseContext>())
        {
            context_t::advance_to(
                fmt_ctx,
                fmt.format(std::forward<T>(val), parse_ctx, fmt_ctx)
            );
        }
        else
        {
            parse_ctx.advance_to(fmt.parse(parse_ctx));

            context_t::advance_to(
                fmt_ctx,
                fmt.format(std::forward<T>(val), fmt_ctx)
            );
        }
    }

    /**
     * @brief Format the value directly
     *
     * It will construct an empty parse context if it is necessary.
     *
     * @param val The value
     * @param fmt_ctx Format context
     */
    template <typename T, typename FormatContext>
    static void format(Formatter& fmt, T&& val, FormatContext& fmt_ctx)
    {
        using context_t = format_context_traits<FormatContext>;

        using parse_context = basic_format_parse_context<FormatContext>;

        if constexpr(!parsable<parse_context>())
        {
            parse_context parse_ctx(
                std::basic_string_view<char_type>(),
                empty_format_args_for<FormatContext>()
            );

            context_t::advance_to(
                fmt_ctx,
                fmt.format(std::forward<T>(val), parse_ctx, fmt_ctx)
            );
        }
        else
        {
            context_t::advance_to(
                fmt_ctx,
                fmt.format(std::forward<T>(val), fmt_ctx)
            );
        }
    }

    /**
     * @brief Skip the format specification of the current replacement field
     *
     * @param parse_ctx Parse context
     */
    template <typename ParseContext>
    static void skip_spec(ParseContext& parse_ctx) noexcept
    {
        if constexpr(parsable<ParseContext>())
        {
            Formatter tmp{};
            parse_ctx.advance_to(
                tmp.parse(parse_ctx)
            );
        }
        else
        {
            parse_ctx.skip_spec();
        }
    }

    static constexpr bool has_debug_format()
    {
        return requires(Formatter& fmt) {
            fmt.set_debug_format();
        };
    }

    static bool try_set_debug_format(Formatter& fmt)
    {
        if constexpr(has_debug_format())
        {
            fmt.set_debug_format();
            return true;
        }
        else
            return false;
    }

    static constexpr bool has_set_separator() noexcept
    {
        return requires(Formatter& fmt, string_view_type sep) {
            fmt.set_separator(sep);
        };
    }

    static bool try_set_separator(Formatter& fmt, string_view_type sep)
    {
        if constexpr(has_set_separator())
        {
            fmt.set_separator(sep);
            return true;
        }
        else
            return false;
    }

    static constexpr bool has_set_brackets() noexcept
    {
        return requires(Formatter& fmt, string_view_type opening, string_view_type closing) {
            fmt.set_brackets(opening, closing);
        };
    }

    static bool try_set_brackets(Formatter& fmt, string_view_type opening, string_view_type closing)
    {
        if constexpr(has_set_brackets())
        {
            fmt.set_brackets(opening, closing);
            return true;
        }
        else
            return false;
    }
};

/// @}

namespace detail
{
    template <
        typename T,
        typename Formatter,
        typename FormatContext>
    concept check_format_method_separated = requires(const Formatter& cf, T&& val, FormatContext fmt_ctx) {
        {
            cf.format(val, fmt_ctx)
        } -> std::same_as<typename FormatContext::iterator>;
    };

    template <
        typename T,
        typename Formatter,
        typename FormatContext,
        typename ParseContext>
    concept check_format_method_combined = requires(const Formatter& cf, T&& val, FormatContext& fmt_ctx, ParseContext& parse_ctx) {
        {
            cf.format(val, parse_ctx, fmt_ctx)
        } -> std::same_as<typename FormatContext::iterator>;
    };

    template <
        typename T,
        typename Formatter,
        typename FormatContext,
        typename ParseContext>
    concept check_format_method =
        check_format_method_combined<T, Formatter, FormatContext, ParseContext> ||
        (formatter_traits<Formatter>::template parsable<ParseContext>() &&
         check_format_method_separated<T, Formatter, FormatContext>);

    template <
        typename T,
        typename Context,
        typename Formatter = typename Context::template formatter_type<T>>
    concept formattable_with_impl =
        std::semiregular<Formatter> &&
        check_format_method<T, Formatter, Context, basic_format_parse_context<Context>>;
} // namespace detail

PAPILIO_EXPORT template <typename T, typename Context>
concept formattable_with = detail::formattable_with_impl<
    std::remove_const_t<T>,
    Context>;

PAPILIO_EXPORT template <typename T, typename CharT = char>
concept formattable = formattable_with<
    std::remove_const_t<T>,
    basic_format_context<format_iterator_for<CharT>, CharT>>;

/// @defgroup Script The embedded script
/// @ingroup Format
/// @{

/**
 * @brief Base class containing script interpreter APIs that are not related to a specific character type.
 */
PAPILIO_EXPORT class script_base
{
public:
    static constexpr char32_t script_start = U'$';
    static constexpr char32_t condition_end = U'?';

    /**
     * @brief Script error.
     *
     * @sa script_error_code
     */
    class error : public format_error
    {
    public:
        error(const error&) = default;

        /**
         * @brief Construct a script error object from an error code.
         *
         * @param ec The error code
         */
        explicit error(script_error_code ec);

        ~error();

        /**
         * @brief Get the error code.
         */
        [[nodiscard]]
        script_error_code error_code() const noexcept
        {
            return m_ec;
        }

    private:
        script_error_code m_ec;
    };

    /**
     * @brief Construct a script error object from an error code.
     *
     * @param ec The error code
     *
     * @sa script_error_code
     */
    [[nodiscard]]
    static error make_error(script_error_code ec);

#ifndef PAPILIO_DOXYGEN // Don't generate documentation for internal APIs

protected:
    [[noreturn]]
    static void throw_end_of_string();

    [[noreturn]]
    static void throw_error(script_error_code ec);

    // clang-format off

    enum class op_id
    {
        equal,         // ==
        not_equal,     // !=
        greater_equal, // >=
        less_equal,    // <=
        greater,       // >
        less           // <
    };

    // clang-format on

    static bool is_op_ch(char32_t ch) noexcept;

    static bool is_var_start_ch(char32_t ch) noexcept;

    static bool is_field_name_ch(char32_t ch, bool first = false) noexcept;

    static bool is_field_name_end_ch(char32_t ch) noexcept;

    static char32_t get_esc_ch(char32_t ch) noexcept;

#endif
};

PAPILIO_EXPORT template <typename CharT, bool Debug>
class basic_interpreter_base : public script_base
{
    using my_base = script_base;

public:
    using char_type = CharT;
    using variable_type = basic_variable<CharT>;
    using string_type = std::basic_string<CharT>;
    using string_ref_type = utf::basic_string_ref<CharT>;
    using string_container_type = utf::basic_string_container<CharT>;
    using indexing_value_type = basic_indexing_value<CharT>;

    using iterator = typename string_ref_type::const_iterator;

    /**
     * @brief Check if the interpreter is in debug mode.
     */
    [[nodiscard]]
    static constexpr bool debug() noexcept
    {
        return Debug;
    }

private:
    class error_ex : public my_base::error
    {
    public:
        using char_type = CharT;

        error_ex(script_error_code ec, iterator it)
            : error(ec), m_it(std::move(it)) {}

        [[nodiscard]]
        iterator get_iter() const noexcept
        {
            return m_it;
        }

    private:
        iterator m_it;
    };

public:
    using extended_error = std::conditional_t<
        Debug,
        error_ex,
        my_base::error>;

    [[nodiscard]]
    static extended_error make_extended_error(script_error_code ec, iterator it)
    {
        if constexpr(Debug)
            return extended_error(ec, std::move(it));
        else
            return make_error(ec);
    }

#ifndef PAPILIO_DOXYGEN // Don't generate documentation for internal APIs

protected:
    using my_base::throw_error;

    [[noreturn]]
    static void throw_error(script_error_code ec, iterator it)
    {
        if constexpr(debug())
        {
            throw make_extended_error(ec, std::move(it));
        }
        else
        {
            my_base::throw_error(ec);
        }
    }

    // Skips white spaces
    static iterator skip_ws(iterator start, iterator stop) noexcept
    {
        return std::find_if_not(start, stop, utf::is_whitespace);
    }

    static iterator find_field_name_end(iterator start, iterator stop, bool first = true) noexcept
    {
        while(start != stop)
        {
            if(!is_field_name_ch(*start, first))
                break;
            first = false;
            ++start;
        }

        return start;
    }

    static std::pair<op_id, iterator> parse_op(iterator start, iterator stop)
    {
        if(start == stop) [[unlikely]]
            throw_end_of_string();

        char32_t first_ch = *start;
        if(first_ch == U'=')
        {
            ++start;
            if(start != stop)
            {
                if(*start != U'=')
                    throw_error(script_error_code::invalid_operator, start);
                ++start;
            }

            return std::make_pair(op_id::equal, start);
        }
        else if(first_ch == U'!')
        {
            ++start;
            if(start == stop)
                throw_end_of_string();
            if(*start != U'=') [[unlikely]]
                throw_error(script_error_code::invalid_operator, start);
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

        throw_error(script_error_code::invalid_operator, start);
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

    // Parses integer value
    template <std::integral T>
    static std::pair<T, iterator> parse_integer(iterator start, iterator stop)
    {
        if(start == stop) [[unlikely]]
            throw_end_of_string();

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
                throw std::out_of_range("integer value out of range");
        }

        return std::make_pair(value, start);
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
                if(it == stop) [[unlikely]]
                    throw_error(script_error_code::invalid_string, it);

                auto push_back_impl = [&result](char32_t val)
                {
                    if constexpr(char32_like<char_type>)
                    {
                        result.push_back(static_cast<char_type>(val));
                    }
                    else
                    {
                        result.push_back(utf::codepoint(val));
                    }
                };

                push_back_impl(get_esc_ch(*it++));

                for(; it != stop; ++it)
                {
                    ch = *it;
                    if(ch == U'\\')
                    {
                        ++it;
                        if(it == stop) [[unlikely]]
                            throw_error(script_error_code::invalid_string, it);

                        push_back_impl(get_esc_ch(*it));
                    }
                    else if(ch == U'\'')
                    {
                        ++it; // skip '\''
                        break;
                    }
                    else
                    {
                        push_back_impl(ch);
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
            std::next(it) // +1 to skip '\''
        );
    }

    static std::pair<indexing_value_type, iterator> parse_indexing_value(iterator start, iterator stop)
    {
        if(start == stop) [[unlikely]]
            throw_end_of_string();

        char32_t first_ch = *start;
        if(first_ch == U'\'')
        {
            ++start;
            auto [str, next_it] = parse_string(start, stop);

            return std::make_pair(std::move(str), next_it);
        }
        else if(first_ch == U'-' || utf::is_digit(first_ch))
        {
            auto [idx, next_it] = parse_integer<ssize_t>(
                start, stop
            );

            if(*next_it == U':')
            {
                ++next_it;
                if(next_it == stop) [[unlikely]]
                    throw_end_of_string();

                char32_t next_ch = *next_it;
                ssize_t next_idx = index_range::npos;
                if(next_ch == U'-' || utf::is_digit(next_ch))
                {
                    std::tie(next_idx, next_it) = parse_integer<ssize_t>(
                        next_it, stop
                    );
                }

                return std::make_pair(index_range(idx, next_idx), next_it);
            }

            return std::make_pair(idx, next_it);
        }
        else if(first_ch == U':')
        {
            ++start;
            if(start == stop) [[unlikely]]
                throw_end_of_string();

            char32_t next_ch = *start;
            if(next_ch == U'-' || PAPILIO_NS utf::is_digit(next_ch))
            {
                auto [idx, next_it] = parse_integer<ssize_t>(
                    start, stop
                );
                return std::make_pair(index_range(0, idx), next_it);
            }
            else
            {
                return std::make_pair(index_range(), start);
            }
        }

        throw_error(script_error_code::invalid_index, start);
    }

#endif
};

/**
 * @brief The interpreter of format specification and embedded script
 *
 * @tparam FormatContext Format context @ref basic_format_context
 * @tparam Debug Enable debug options which can produce more precise error information.
 */
PAPILIO_EXPORT template <typename FormatContext, bool Debug>
class basic_interpreter :
    public basic_interpreter_base<typename FormatContext::char_type, Debug>
{
    using my_base = basic_interpreter_base<typename FormatContext::char_type, Debug>;

public:
    using char_type = typename FormatContext::char_type;
    using variable_type = basic_variable<char_type>;
    using string_view_type = std::basic_string_view<char_type>;
    using string_ref_type = utf::basic_string_ref<char_type>;
    using string_container_type = utf::basic_string_container<char_type>;
    using indexing_value_type = basic_indexing_value<char_type>;
    using format_arg_type = basic_format_arg<FormatContext>;

    using parse_context = basic_format_parse_context<FormatContext>;

    using iterator = typename my_base::iterator;
    static_assert(std::same_as<iterator, typename parse_context::iterator>);

    basic_interpreter() = default;

    /**
     * @brief Access the format argument
     *
     * @param ctx Parse context
     * @return std::pair<format_arg_type, iterator> The result and the new position of the iterator
     */
    static std::pair<format_arg_type, iterator> access(parse_context& ctx)
    {
        return access_impl(ctx, ctx.begin(), ctx.end());
    }

    class interpreter_context
    {
    public:
        using input_iterator = typename parse_context::iterator;

        interpreter_context() = delete;

        interpreter_context(const interpreter_context&) = default;

        interpreter_context(parse_context& ictx, FormatContext& octx)
            : m_parse_ctx(std::addressof(ictx)),
              m_format_ctx(std::addressof(octx)),
              m_input_it(ictx.begin()) {}

        parse_context& input_context() const noexcept
        {
            return *m_parse_ctx;
        }

        FormatContext& output_context() const noexcept
        {
            return *m_format_ctx;
        }

        auto parse_begin() const
        {
            return m_parse_ctx->begin();
        }

        auto parse_end() const
        {
            return m_parse_ctx->end();
        }

        void input_next()
        {
            ++m_input_it;
        }

        void advance_input_to(input_iterator it)
        {
            m_input_it = std::move(it);
        }

        input_iterator& input() noexcept
        {
            return m_input_it;
        }

        const input_iterator& input() const noexcept
        {
            return m_input_it;
        }

        char32_t input_value() const
        {
            return *m_input_it;
        }

        bool input_at_end() const
        {
            return m_input_it == parse_end();
        }

        void update_input_context() const
        {
            m_parse_ctx->advance_to(m_input_it);
        }

    private:
        parse_context* m_parse_ctx;
        FormatContext* m_format_ctx;
        input_iterator m_input_it;
    };

    interpreter_context create_context(parse_context& parse_ctx, FormatContext& fmt_ctx)
    {
        return interpreter_context(parse_ctx, fmt_ctx);
    }

    void run_once(interpreter_context& intp_ctx)
    {
        PAPILIO_ASSERT(!intp_ctx.input_at_end());

        using context_t = format_context_traits<FormatContext>;

        char32_t ch = intp_ctx.input_value();

        if(ch == U'}')
        {
            intp_ctx.input_next();
            if(intp_ctx.input_at_end()) [[unlikely]]
                my_base::throw_end_of_string();
            if(intp_ctx.input_value() != U'}') [[unlikely]]
                my_base::throw_error(script_error_code::unenclosed_brace, intp_ctx.input());

            context_t::append(intp_ctx.output_context(), char_type('}'));
            intp_ctx.input_next();
        }
        else if(ch == U'{')
        {
            intp_ctx.input_next();
            if(intp_ctx.input_at_end()) [[unlikely]]
                my_base::throw_end_of_string();

            ch = intp_ctx.input_value();
            if(ch == U'{')
            {
                context_t::append(intp_ctx.output_context(), char_type('{'));

                intp_ctx.input_next();
            }
            else if(ch == my_base::script_start)
            {
                intp_ctx.input_next();

                intp_ctx.update_input_context();
                exec_script(intp_ctx.input_context(), intp_ctx.output_context());

                intp_ctx.advance_input_to(intp_ctx.parse_begin());
                if(intp_ctx.input_at_end()) [[unlikely]]
                    my_base::throw_end_of_string();
                if(intp_ctx.input_value() != U'}') [[unlikely]]
                    my_base::throw_error(script_error_code::unenclosed_brace, intp_ctx.input());
                intp_ctx.input_next();
            }
            else
            {
                intp_ctx.update_input_context();
                exec_repl(intp_ctx.input_context(), intp_ctx.output_context());

                intp_ctx.advance_input_to(intp_ctx.parse_begin());
                if(intp_ctx.input_at_end()) [[unlikely]]
                    my_base::throw_end_of_string();
                if(intp_ctx.input_value() != U'}') [[unlikely]]
                    my_base::throw_error(script_error_code::invalid_fmt_spec, intp_ctx.input());
                intp_ctx.input_next();
            }
        }
        else
        {
            // Ordinary characters
            context_t::append(intp_ctx.output_context(), ch);
            intp_ctx.input_next();
        }
    }

    std::size_t run_n(interpreter_context& intp_ctx, std::size_t n)
    {
        std::size_t count = 0;
        while(!intp_ctx.input_at_end())
        {
            if(count == n)
                return n;
            ++count;
            run_once(intp_ctx);
        }

        return count;
    }

    template <typename Pred>
    std::size_t run_if(interpreter_context& intp_ctx, Pred&& pred)
    {
        std::size_t count = 0;
        while(!intp_ctx.input_at_end())
        {
            if(!pred())
                return count;
            ++count;
            run_once(intp_ctx);
        }

        return count;
    }

    void run(interpreter_context& intp_ctx)
    {
        while(!intp_ctx.input_at_end())
        {
            run_once(intp_ctx);
        }
    }

    /**
     * @brief Parse and format
     *
     * @param parse_ctx Parse context
     * @param fmt_ctx Format context
     */
    void format(
        parse_context& parse_ctx,
        FormatContext& fmt_ctx
    )
    {
        auto intp_ctx = create_context(parse_ctx, fmt_ctx);
        run(intp_ctx);
    }

private:
    static std::pair<format_arg_type, iterator> access_impl(parse_context& ctx, iterator start, iterator stop)
    {
        if(start == stop) [[unlikely]]
            my_base::throw_end_of_string();

        auto [arg, next_it] = parse_field_name(ctx, start, stop);

        std::tie(arg, next_it) = parse_chained_access(arg, next_it, stop);

        return std::make_pair(std::move(arg), next_it);
    }

    static void skip_repl(parse_context& parse_ctx)
    {
        auto [arg, next_it] = access(parse_ctx);

        if(next_it == parse_ctx.end()) [[unlikely]]
            my_base::throw_end_of_string();
        if(*next_it == U':')
            ++next_it;

        parse_ctx.advance_to(next_it);
        arg.skip_spec(parse_ctx);
    }

    /**
     * @brief Skip the current branch in script
     *
     * @note Call `skip_ws()` before calling this function.
     * @warning This function assumes `start != stop`
     */
    static iterator skip_branch(parse_context& parse_ctx)
    {
        auto start = parse_ctx.begin();
        const auto stop = parse_ctx.end();

        PAPILIO_ASSERT(start != stop);

        if(char32_t ch = *start; ch == U'\'')
        {
            ++start;
            return my_base::skip_string(start, stop);
        }
        else if(ch == U'{')
        {
            ++start;
            parse_ctx.advance_to(start);
            skip_repl(parse_ctx);

            start = parse_ctx.begin();
            if(start == parse_ctx.end()) [[unlikely]]
                my_base::throw_end_of_string();
            if(*start != U'}') [[unlikely]]
                my_base::throw_error(script_error_code::invalid_fmt_spec, start);
            ++start;

            return start;
        }
        else
        {
            my_base::throw_error(script_error_code::invalid_string, start);
        }
    }

    static iterator exec_branch(parse_context& parse_ctx, FormatContext& fmt_ctx)
    {
        auto start = parse_ctx.begin();
        const auto stop = parse_ctx.end();

        PAPILIO_ASSERT(start != stop);

        using context_t = format_context_traits<FormatContext>;

        if(char32_t ch = *start; ch == U'\'')
        {
            ++start;

            string_container_type str;
            std::tie(str, start) = my_base::parse_string(start, stop);

            context_t::append(fmt_ctx, str);

            return start;
        }
        else if(ch == U'{')
        {
            ++start;
            parse_ctx.advance_to(start);
            exec_repl(parse_ctx, fmt_ctx);

            start = parse_ctx.begin();
            if(start == parse_ctx.end()) [[unlikely]]
                my_base::throw_end_of_string();
            if(*start != U'}') [[unlikely]]
                my_base::throw_error(script_error_code::invalid_fmt_spec, start);
            ++start;

            return start;
        }
        else
        {
            my_base::throw_error(script_error_code::invalid_string, start);
        }
    }

    static iterator exec_branch_if(
        bool cond, parse_context& parse_ctx, FormatContext& fmt_ctx
    )
    {
        auto start = my_base::skip_ws(
            parse_ctx.begin(), parse_ctx.end()
        );
        const auto stop = parse_ctx.end();

        if(start == stop) [[unlikely]]
            my_base::throw_end_of_string();
        parse_ctx.advance_to(start);

        if(cond)
            start = exec_branch(parse_ctx, fmt_ctx);
        else
            start = skip_branch(parse_ctx);

        return my_base::skip_ws(start, stop);
    }

    /**
     * @brief Execute the scripted field
     */
    static void exec_script(parse_context& parse_ctx, FormatContext& fmt_ctx)
    {
        iterator start = parse_ctx.begin();
        const iterator stop = parse_ctx.end();

        bool executed = false;

        bool cond_result = false;
        std::tie(cond_result, start) = parse_condition(parse_ctx, start, stop);

        parse_ctx.advance_to(start);
        start = exec_branch_if(
            cond_result, parse_ctx, fmt_ctx
        );
        executed |= cond_result;

        while(start != stop && *start == U':')
        {
            start = my_base::skip_ws(std::next(start), stop);
            if(start != stop && *start == U'$')
            {
                start = my_base::skip_ws(std::next(start), stop);

                parse_ctx.advance_to(start);
                std::tie(cond_result, start) = parse_condition(
                    parse_ctx, start, stop
                );

                bool exec_this_branch = !executed && cond_result;

                parse_ctx.advance_to(start);
                start = exec_branch_if(
                    exec_this_branch, parse_ctx, fmt_ctx
                );
                executed |= exec_this_branch;

                continue;
            }
            else
            {
                bool exec_this_branch = !executed;

                parse_ctx.advance_to(start);
                start = exec_branch_if(
                    exec_this_branch, parse_ctx, fmt_ctx
                );
            }
        }

        parse_ctx.advance_to(start);
    }

    /**
     * @brief Execute the replacement field
     */
    static void exec_repl(parse_context& parse_ctx, FormatContext& fmt_ctx)
    {
        auto [arg, next_it] = access(parse_ctx);

        if(next_it == parse_ctx.end()) [[unlikely]]
            my_base::throw_end_of_string();
        if(*next_it == U':')
            ++next_it;

        parse_ctx.advance_to(next_it);
        arg.format(parse_ctx, fmt_ctx);
    }

    static std::pair<variable_type, iterator> parse_variable(parse_context& ctx, iterator start, iterator stop)
    {
        if(start == stop) [[unlikely]]
            my_base::throw_end_of_string();

        char32_t first_ch = *start;
        if(first_ch == U'{')
        {
            ++start;
            auto [arg, next_it] = access_impl(ctx, start, stop);
            if(next_it == stop) [[unlikely]]
                my_base::throw_end_of_string();
            if(*next_it != U'}') [[unlikely]]
                my_base::throw_error(script_error_code::unenclosed_brace, next_it);

            ++next_it;
            return std::make_pair(
                variable_type(std::move(arg).to_variant()),
                next_it
            );
        }
        else if(first_ch == U'\'')
        {
            ++start;
            auto [str, next_it] = my_base::parse_string(start, stop);

            return std::make_pair(std::move(str), next_it);
        }
        else if(first_ch == U'-' || PAPILIO_NS utf::is_digit(first_ch) || first_ch == U'.')
        {
            bool negative = first_ch == U'-';

            iterator int_end = std::find_if_not(
                negative ? start + 1 : start, stop, PAPILIO_NS utf::is_digit
            );
            using int_type = typename variable_type::int_type;
            int_type int_val = my_base::template parse_integer<int_type>(start, int_end).first;

            if(int_end != stop && *int_end == U'.')
            {
                ++int_end; // Skip the decimal point

                iterator float_end = int_end;
                int_type pow10_val = 1;
                for(; float_end != stop; ++float_end)
                {
                    if(!PAPILIO_NS utf::is_digit(*float_end))
                        break;
                    pow10_val *= 10;
                }

                int_type frac = my_base::template parse_integer<int_type>(int_end, float_end).first;

                using float_type = typename variable_type::float_type;
                float_type flt_val = static_cast<float_type>(int_val);
                flt_val += static_cast<float_type>(frac) / static_cast<float_type>(pow10_val);

                return std::make_pair(flt_val, float_end);
            }
            else
            {
                return std::make_pair(int_val, int_end);
            }
        }

        my_base::throw_error(script_error_code::invalid_condition, start);
    }

    static std::pair<bool, iterator> parse_condition(parse_context& ctx, iterator start, iterator stop)
    {
        start = my_base::skip_ws(start, stop);
        if(start == stop) [[unlikely]]
            my_base::throw_end_of_string();

        char32_t first_ch = *start;
        if(first_ch == U'!')
        {
            ++start;
            start = my_base::skip_ws(start, stop);

            auto [var, next_it] = parse_variable(ctx, start, stop);
            next_it = my_base::skip_ws(next_it, stop);
            if(next_it == stop) [[unlikely]]
                my_base::throw_end_of_string();
            if(*next_it != my_base::condition_end) [[unlikely]]
                my_base::throw_error(script_error_code::invalid_condition, next_it);

            ++next_it;
            return std::make_pair(!var.template as<bool>(), next_it);
        }
        else if(my_base::is_var_start_ch(first_ch))
        {
            auto [var, next_it] = parse_variable(ctx, start, stop);

            next_it = my_base::skip_ws(next_it, stop);
            if(next_it == stop) [[unlikely]]
                my_base::throw_end_of_string();

            char32_t ch = *next_it;
            if(ch == my_base::condition_end)
            {
                ++next_it;
                return std::make_pair(var.template as<bool>(), next_it);
            }
            else if(my_base::is_op_ch(ch))
            {
                typename my_base::op_id op{};
                std::tie(op, next_it) = my_base::parse_op(next_it, stop);

                next_it = my_base::skip_ws(next_it, stop);

                auto [var_2, next_it_2] = parse_variable(ctx, next_it, stop);
                next_it = my_base::skip_ws(next_it_2, stop);

                if(next_it == stop) [[unlikely]]
                    my_base::throw_end_of_string();
                if(*next_it != my_base::condition_end) [[unlikely]]
                    my_base::throw_error(script_error_code::invalid_condition, next_it);

                ++next_it;
                return std::make_pair(
                    my_base::execute_op(op, var, var_2),
                    next_it
                );
            }

            my_base::throw_error(script_error_code::invalid_condition, next_it);
        }

        my_base::throw_error(script_error_code::invalid_condition, start);
    }

    static std::pair<format_arg_type, iterator> parse_field_name(
        parse_context& ctx, iterator start, iterator stop
    )
    {
        if(start == stop) [[unlikely]]
            my_base::throw_end_of_string();

        if(char32_t first_ch = *start; utf::is_digit(first_ch))
        {
            std::size_t idx = static_cast<std::size_t>(first_ch - U'0');
            ++start;

            while(start != stop)
            {
                char32_t ch = *start;
                if(!utf::is_digit(ch))
                    break;

                idx *= 10;
                idx += static_cast<std::size_t>(ch - U'0');
                ++start;
            }

            ctx.check_arg_id(idx);
            return std::make_pair(ctx.get_args()[static_cast<ssize_t>(idx)], start);
        }
        else if(my_base::is_field_name_ch(first_ch, true))
        {
            iterator str_start = start;
            ++start;
            iterator str_end = my_base::find_field_name_end(start, stop, false);

            string_ref_type name(str_start, str_end);

            return std::make_pair(
                ctx.get_args().get(string_view_type(name)),
                str_end
            );
        }
        else if(my_base::is_field_name_end_ch(first_ch)) // use default value
        {
            std::size_t idx = ctx.current_arg_id();
            ctx.next_arg_id();
            return std::make_pair(ctx.get_args()[static_cast<ssize_t>(idx)], start);
        }

        my_base::throw_error(script_error_code::invalid_field_name, start);
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

                iterator str_end = my_base::find_field_name_end(start, stop);

                string_ref_type attr_name(str_start, str_end);
                if(attr_name.empty())
                    my_base::throw_error(script_error_code::invalid_attribute, str_end);

                current = current.attribute(attr_name);

                start = str_end;
            }
            else if(first_ch == U'[')
            {
                ++start;
                auto [idx, next_it] = my_base::parse_indexing_value(start, stop);
                if(next_it == stop) [[unlikely]]
                    my_base::throw_end_of_string();
                if(*next_it != U']') [[unlikely]]
                    my_base::throw_error(script_error_code::invalid_index, next_it);
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
};

/// @}

/// @addtogroup Parse
/// @{

namespace detail
{
    class fmt_parser_base
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
    };

    class std_fmt_parser_base : public fmt_parser_base
    {
    protected:
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

    template <typename ParseContext>
    class fmt_parser_utils
    {
    public:
        using iterator = typename ParseContext::iterator;
        using interpreter_type = basic_interpreter<typename ParseContext::format_context_type>;

        /**
         * @brief Check if the iterator is at the end of the parsing context or at a closing brace.
         *
         * @param start The current iterator.
         * @param stop The end iterator.
         * @return True if the iterator is at the end of the parsing context or at a closing brace, otherwise false.
        */
        static bool check_stop(iterator start, iterator stop) noexcept
        {
            return start == stop || *start == U'}';
        }

        /**
         * @brief Parse a integer value from the format context.
         *
         * @tparam IsPrecision If true, then parse a precision, otherwise parse a width.
         * @param ctx The parsing context.
         * @return The parsed value and the parsing iterator.
        */
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

                auto var = variable(arg.to_variant());
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
} // namespace detail

PAPILIO_EXPORT template <typename ParseContext, bool UseLocale = false>
class simple_formatter_parser :
    detail::fmt_parser_base,
    detail::fmt_parser_utils<ParseContext>
{
    using utils = detail::fmt_parser_utils<ParseContext>;

public:
    using char_type = typename ParseContext::char_type;
    using iterator = typename ParseContext::iterator;

    using result_type = simple_formatter_data;
    using interpreter_type = basic_interpreter<typename ParseContext::format_context_type>;

    std::pair<result_type, iterator> parse(ParseContext& ctx)
    {
        result_type result;

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

        if(utils::check_stop(start, stop))
            goto parse_end;
        if(char32_t ch = *start; is_align_ch(ch))
        {
            result.align = get_align(ch);
            ++start;
        }

        if(utils::check_stop(start, stop))
            goto parse_end;
        if(char32_t ch = *start; utf::is_digit(ch))
        {
            if(ch == U'0')
                throw format_error("invalid format");

            ctx.advance_to(start);
            std::tie(result.width, start) = utils::template parse_value<false>(ctx);
        }
        else if(ch == U'{')
        {
            ctx.advance_to(start);
            std::tie(result.width, start) = utils::template parse_value<false>(ctx);
        }

        if constexpr(UseLocale)
        {
            if(utils::check_stop(start, stop))
                goto parse_end;
            if(*start == U'L')
            {
                result.use_locale = true;
                ++start;
            }
        }
        else
        {
            PAPILIO_ASSERT(result.use_locale == false);
        }

parse_end:
        ctx.advance_to(start);
        return std::make_pair(std::move(result), std::move(start));
    }
};

/**
 * @brief Parser for the standard format specification.
 *
 * @tparam ParseContext The parsing context.
 * @tparam EnablePrecision Enable parsing of precision.
*/
PAPILIO_EXPORT template <typename ParseContext, bool EnablePrecision = false>
class std_formatter_parser :
    detail::std_fmt_parser_base,
    detail::fmt_parser_utils<ParseContext>
{
    using my_base = detail::std_fmt_parser_base;
    using utils = detail::fmt_parser_utils<ParseContext>;

public:
    using char_type = typename ParseContext::char_type;
    using iterator = typename ParseContext::iterator;

    using result_type = std_formatter_data;

    std::pair<result_type, iterator> parse(ParseContext& ctx, std::u32string_view types)
    {
        result_type result;

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

        if(utils::check_stop(start, stop))
            goto parse_end;
        if(char32_t ch = *start; is_align_ch(ch))
        {
            result.align = get_align(ch);
            ++start;
        }

        if(utils::check_stop(start, stop))
            goto parse_end;
        if(char32_t ch = *start; is_sign_ch(ch))
        {
            result.sign = get_sign(ch);
            ++start;
        }

        if(utils::check_stop(start, stop))
            goto parse_end;
        if(*start == U'#')
        {
            result.alternate_form = true;
            ++start;
        }

        if(utils::check_stop(start, stop))
            goto parse_end;
        if(*start == U'0')
        {
            result.fill_zero = true;
            ++start;
        }

        if(utils::check_stop(start, stop))
            goto parse_end;
        if(char32_t ch = *start; utf::is_digit(ch))
        {
            if(ch == U'0')
                throw format_error("invalid format");

            ctx.advance_to(start);
            std::tie(result.width, start) = utils::template parse_value<false>(ctx);
        }
        else if(ch == U'{')
        {
            ctx.advance_to(start);
            std::tie(result.width, start) = utils::template parse_value<false>(ctx);
        }

        if(utils::check_stop(start, stop))
            goto parse_end;
        if(*start == U'.')
        {
            ++start;
            if(start == stop)
                throw format_error("invalid precision");

            ctx.advance_to(start);
            std::tie(result.precision, start) = utils::template parse_value<true>(ctx);
        }

        if(utils::check_stop(start, stop))
            goto parse_end;
        if(*start == U'L')
        {
            result.use_locale = true;
            ++start;
        }

        if(utils::check_stop(start, stop))
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
};

/// @}

/// @addtogroup Formatter
/// @{

class std_formatter_base
{
private:
    std_formatter_data m_data;

protected:
    std_formatter_data& data() noexcept
    {
        return m_data;
    }

    const std_formatter_data& data() const noexcept
    {
        return m_data;
    }

    /**
     * @brief Returns the left and right size for filling
     *
     * @param used Used length
     * @return std::pair<std::size_t, std::size_t> Left and right size to fill
     */
    std::pair<std::size_t, std::size_t> fill_size(std::size_t used) const noexcept
    {
        if(m_data.width <= used)
            return std::make_pair(0, 0);

        std::size_t remain = m_data.width - used;
        switch(m_data.align)
        {
        case format_align::right:
            return std::make_pair(remain, 0);

        case format_align::left:
            return std::make_pair(0, remain);

        case format_align::middle:
            return std::make_pair(
                remain / 2,
                remain / 2 + remain % 2 // ceil(remain / 2)
            );

        default:
        case format_align::default_align:
            PAPILIO_UNREACHABLE();
        }
    }

    template <typename FormatContext>
    void fill(FormatContext& ctx, std::size_t count) const
    {
        using context_t = format_context_traits<FormatContext>;

        PAPILIO_ASSERT(m_data.fill != U'\0');
        context_t::append(ctx, m_data.fill, count);
    }
};

/// @defgroup DigitMap Digit maps for implementing formatters
/// @{

inline constexpr char digit_map_lower[16] =
    {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
inline constexpr char digit_map_upper[16] =
    {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

template <typename CharT>
inline constexpr std::basic_string_view<CharT> inf_name_lower = PAPILIO_TSTRING_VIEW(CharT, "inf");
template <typename CharT>
inline constexpr std::basic_string_view<CharT> inf_name_upper = PAPILIO_TSTRING_VIEW(CharT, "INF");

template <typename CharT = char>
inline constexpr std::basic_string_view<CharT> nan_name_lower = PAPILIO_TSTRING_VIEW(CharT, "nan");
template <typename CharT = char>
inline constexpr std::basic_string_view<CharT> nan_name_upper = PAPILIO_TSTRING_VIEW(CharT, "NAN");

/// @}

/// @defgroup BaseFormatter Formatter bases
/// @brief Base classes for formatters, providing formatting support for common types.
/// @{

/**
 * @brief Base formatter for integral types.
 *
 * @tparam T Integral type except the character types
 * @tparam CharT Character type
 *
 * Accepted format types are: none, `b`,`B`,`x`,`X`,`o`,`d`.
 * - `b`: Binary format.
 * - `B`: Same as `b`, but the base prefix is `0B`.
 * - `x`: Hexadecimal format.
 * - `X`: Same as `x`, but the base prefix is `0X`.
 * - `o`: Octal format.
 * - `d`: Decimal format.
 * - none: Same as `d`.
 */
template <std::integral T, typename CharT>
requires(!char_like<T>)
class int_formatter : public std_formatter_base
{
public:
    using char_type = char;
    using facet_type = std::numpunct<CharT>;

    constexpr void set_data(const std_formatter_data& dt) noexcept
    {
        PAPILIO_ASSERT(dt.contains_type(U"BbXxod"));

        data() = dt;
        data().fill = dt.fill_or(U' ');
        data().type = dt.type_or(U'd');
        if(data().align != format_align::default_align)
            data().fill_zero = false;
        else
            data().align = format_align::right;
    }

    template <typename FormatContext>
    auto format(T val, FormatContext& ctx) const
        -> typename FormatContext::iterator
    {
        using context_t = format_context_traits<FormatContext>;

        if constexpr(context_t::use_locale())
        {
            if(data().use_locale)
            {
                return format_by_facet(val, ctx);
            }
        }

        CharT buf[sizeof(T) * 8];
        std::size_t buf_size = 0;

        auto [base, uppercase] = parse_type_ch(data().type);

        const auto& digits = uppercase ? digit_map_upper : digit_map_lower;

        const bool neg = val < 0;

        if constexpr(std::is_signed_v<T>)
        {
            do
            {
                const T digit = std::abs(val % static_cast<T>(base));
                buf[buf_size++] = static_cast<CharT>(digits[digit]);
                val /= static_cast<T>(base);
            } while(val);
        }
        else
        {
            do
            {
                const T digit = val % static_cast<T>(base);
                buf[buf_size++] = static_cast<CharT>(digits[digit]);
                val /= static_cast<T>(base);
            } while(val);
        }

        std::size_t used = buf_size;
        if(data().alternate_form)
            used += alt_prefix_width(base);
        switch(data().sign)
        {
        case format_sign::negative:
        case format_sign::default_sign:
            if(neg)
                ++used;
            break;

        case format_sign::positive:
        case format_sign::space:
            ++used;
            break;

        default:
            PAPILIO_UNREACHABLE();
        }

        auto [left, right] = data().fill_zero ?
                                 std::make_pair<std::size_t, std::size_t>(0, 0) :
                                 fill_size(used);

        fill(ctx, left);

        switch(data().sign)
        {
        case format_sign::negative:
        case format_sign::default_sign:
            if(neg)
                context_t::append(ctx, static_cast<CharT>('-'));
            break;

        case format_sign::positive:
            context_t::append(ctx, static_cast<CharT>(neg ? '-' : '+'));
            break;

        case format_sign::space:
            context_t::append(ctx, static_cast<CharT>(neg ? '-' : ' '));
            break;

        default:
            PAPILIO_UNREACHABLE();
        }

        if(data().alternate_form && base != 10)
        {
            context_t::append(ctx, '0');
            switch(base)
            {
            case 16:
                context_t::append(ctx, uppercase ? 'X' : 'x');
                break;

            case 2:
                context_t::append(ctx, uppercase ? 'B' : 'b');
                break;

            default:
                break;
            }
        }

        if(data().fill_zero)
        {
            if(used < data().width)
            {
                context_t::append(
                    ctx,
                    static_cast<CharT>('0'),
                    data().width - used
                );
            }
        }

        for(std::size_t i = buf_size; i > 0; --i)
            context_t::append(ctx, buf[i - 1]);

        fill(ctx, right);

        return context_t::out(ctx);
    }

private:
    // Returns the number base and whether to use uppercase.
    static std::pair<int, bool> parse_type_ch(char32_t ch) noexcept
    {
        int base = 10;
        bool uppercase = false;

        switch(ch)
        {
        case U'X':
            uppercase = true;
            [[fallthrough]];
        case U'x':
            base = 16;
            break;

        case U'B':
            uppercase = true;
            [[fallthrough]];
        case U'b':
            base = 2;
            break;

        case U'O':
            uppercase = true;
            [[fallthrough]];
        case U'o':
            base = 8;
            break;

        case U'D':
            uppercase = true;
            [[fallthrough]];
        case U'd':
            PAPILIO_ASSERT(base == 10);
            break;

        default:
            PAPILIO_UNREACHABLE();
        }

        return std::make_pair(base, uppercase);
    }

    // Get width of the prefix of alternate form
    static std::size_t alt_prefix_width(int base) noexcept
    {
        switch(base)
        {
        case 10:
            return 0;

        case 2:
        case 16:
            return 2; // "0b", "0B", "0x" and "0X"

        case 8:
            return 1; // "o"

        default:
            PAPILIO_UNREACHABLE();
        }
    }

    template <typename Context>
    auto format_by_facet(T val, Context& ctx) const
    {
        using context_t = format_context_traits<Context>;

        small_vector<CharT, 256> buf;
        const facet_type& facet = std::use_facet<facet_type>(context_t::getloc_ref(ctx));

        auto [base, uppercase] = parse_type_ch(data().type);

        const auto& digits = uppercase ? digit_map_upper : digit_map_lower;

        const bool neg = val < 0;

        std::size_t used = 0;
        std::size_t digit_count = 0;

        std::string grouping = facet.grouping();
        CharT sep = facet.thousands_sep();
        const std::size_t sep_width = utf::codepoint(static_cast<char32_t>(sep)).estimate_width();

        auto write_buf = [&, sep_idx = std::size_t(0), count_since_sep = std::size_t(0)](CharT ch) mutable
        {
            if(digit_count != 0)
            {
                char current_grouping_val = PAPILIO_NS index_grouping(grouping, sep_idx);
                if(count_since_sep >= std::size_t(current_grouping_val))
                {
                    buf.push_back(sep);
                    used += sep_width;
                    count_since_sep = 0;
                    ++sep_idx;
                }
            }

            buf.push_back(ch);
            ++digit_count;
            ++count_since_sep;
            ++used;
        };

        if constexpr(std::is_signed_v<T>)
        {
            do
            {
                const T digit = std::abs(val % static_cast<T>(base));
                write_buf(static_cast<CharT>(digits[digit]));
                val /= static_cast<T>(base);
            } while(val);
        }
        else
        {
            do
            {
                const T digit = val % static_cast<T>(base);
                write_buf(static_cast<CharT>(digits[digit]));
                val /= static_cast<T>(base);
            } while(val);
        }

        if(data().alternate_form)
            used += alt_prefix_width(base);
        switch(data().sign)
        {
        case format_sign::negative:
        case format_sign::default_sign:
            if(neg)
                ++used;
            break;

        case format_sign::positive:
        case format_sign::space:
            ++used;
            break;

        default:
            PAPILIO_UNREACHABLE();
        }

        auto [left, right] = data().fill_zero ?
                                 std::make_pair<std::size_t, std::size_t>(0, 0) :
                                 fill_size(used);

        fill(ctx, left);

        switch(data().sign)
        {
        case format_sign::negative:
        case format_sign::default_sign:
            if(neg)
                context_t::append(ctx, static_cast<CharT>('-'));
            break;

        case format_sign::positive:
            context_t::append(ctx, static_cast<CharT>(neg ? '-' : '+'));
            break;

        case format_sign::space:
            context_t::append(ctx, static_cast<CharT>(neg ? '-' : ' '));
            break;

        default:
            PAPILIO_UNREACHABLE();
        }

        if(data().alternate_form && base != 10)
        {
            context_t::append(ctx, '0');
            switch(base)
            {
            case 16:
                context_t::append(ctx, uppercase ? 'X' : 'x');
                break;

            case 2:
                context_t::append(ctx, uppercase ? 'B' : 'b');
                break;

            default:
                break;
            }
        }

        if(data().fill_zero)
        {
            if(digit_count < data().width)
            {
                std::size_t zeros = data().width - digit_count;
                for(std::size_t i = 0; i < zeros; ++i)
                    write_buf(CharT('0'));
            }
        }

        for(std::size_t i = buf.size(); i > 0; --i)
            context_t::append(ctx, buf[i - 1]);

        fill(ctx, right);

        return context_t::out(ctx);
    }
};

/**
 * @brief Base formatter for floating points.
 *
 * @tparam T Floating point type
 * @tparam CharT Character type
 *
 * Accepted format types are: none, `f`, `F`, `g`, `G`, `e`, `E`, `a`, and `A`.
 *
 * - `f`, `F`: Fixed format. The precision will be 6 if not specified.
 * - `g`, `G`: General format. The precision will be 6 if not specified.
 * - `e`, `E`: Scientific format. The precision will be 6 if not specified.
 * - `a`, `A`: Hexadecimal format.
 * - none: Same as the general (`g`) format.
 *
 * The uppercase types will replace all letters with their uppercase version.
 * For example, `E` format will use the letter E to indicate the exponent.
 *
 * The infinity and NaN will be formatted as `inf` and `nan`, respectively.
 */
template <std::floating_point T, typename CharT>
class float_formatter : public std_formatter_base
{
public:
    using char_type = CharT;
    using facet_type = std::numpunct<CharT>;

    void set_data(const std_formatter_data& dt)
    {
        PAPILIO_ASSERT(dt.contains_type(U"fFgGeEaA"));

        data() = dt;
        data().fill = dt.fill_or(U' ');
        if(data().align == format_align::default_align)
            data().align = format_align::right;
    }

    template <typename FormatContext>
    auto format(T val, FormatContext& ctx) const
        -> typename FormatContext::iterator
    {
        using context_t = format_context_traits<FormatContext>;

        small_vector<CharT, 256> buf;

        std::size_t used = 0;

        bool neg = std::signbit(val);
        val = std::abs(val);

        bool use_locale = false;
        if constexpr(context_t::use_locale())
        {
            if(data().use_locale)
            {
                use_locale = true;
                used += float_to_chars_reversed(
                            ctx.getloc_ref(), std::back_inserter(buf), val
                )
                            .second;
            }
        }

        if(!use_locale)
            used += float_to_chars(std::back_inserter(buf), val).second;

        switch(data().sign)
        {
        case format_sign::default_sign:
        case format_sign::negative:
            if(neg)
                ++used;
            break;

        case format_sign::positive:
        case format_sign::space:
            ++used;
            break;

        default:
            PAPILIO_UNREACHABLE();
        }

        auto [left, right] = fill_size(used);

        fill(ctx, left);

        switch(data().sign)
        {
        case format_sign::default_sign:
        case format_sign::negative:
            if(neg)
                context_t::append(ctx, static_cast<CharT>('-'));
            break;

        case format_sign::positive:
            context_t::append(ctx, static_cast<CharT>(neg ? '-' : '+'));
            break;

        case format_sign::space:
            context_t::append(ctx, static_cast<CharT>(neg ? '-' : ' '));
            break;

        default:
            PAPILIO_UNREACHABLE();
        }

        if(use_locale)
        {
            context_t::advance_to(
                ctx,
                std::reverse_copy(buf.begin(), buf.end(), ctx.out())
            );
        }
        else
            context_t::append(ctx, std::basic_string_view<CharT>(buf.data(), buf.size()));

        fill(ctx, right);

        return context_t::out(ctx);
    }

private:
    std::pair<std::chars_format, bool> get_chars_fmt() const
    {
        std::chars_format ch_fmt{};
        bool uppercase = false;

        switch(data().type)
        {
        case U'G':
            uppercase = true;
            [[fallthrough]];
        default:
        case U'\0':
        case U'g':
            ch_fmt = std::chars_format::general;
            break;

        case 'F':
            uppercase = true;
            [[fallthrough]];
        case 'f':
            ch_fmt = std::chars_format::fixed;
            break;

        case 'A':
            uppercase = true;
            [[fallthrough]];
        case 'a':
            ch_fmt = std::chars_format::hex;
            break;

        case 'E':
            uppercase = true;
            [[fallthrough]];
        case 'e':
            ch_fmt = std::chars_format::scientific;
            break;
        }

        return std::make_pair(ch_fmt, uppercase);
    }

    std::size_t call_char_conv(
        T val, char* buf, std::size_t buf_size, std::chars_format ch_fmt
    ) const
    {
        const std::u32string_view use_default_precision = U"fFeEgG";

        int precision = static_cast<int>(data().precision);
        if(precision == 0 &&
           use_default_precision.find(data().type) != std::u32string_view::npos)
        {
            precision = 6;
        }

        std::to_chars_result result;
        if(precision == 0)
            result = std::to_chars(buf, buf + buf_size, val, ch_fmt);
        else
            result = std::to_chars(buf, buf + buf_size, val, ch_fmt, precision);

        if(result.ec == std::errc::value_too_large) [[unlikely]]
        {
            throw format_error("value too large");
        }
        else if(result.ec == std::errc())
        {
            return static_cast<std::size_t>(result.ptr - buf);
        }

        PAPILIO_UNREACHABLE();
    }

    template <typename OutputIt>
    std::pair<OutputIt, std::size_t> float_to_chars(OutputIt out, T val) const
    {
        using namespace std::literals;

        auto [ch_fmt, uppercase] = get_chars_fmt();

        if(std::isinf(val)) [[unlikely]]
        {
            auto inf_name = uppercase ? inf_name_upper<char> : inf_name_lower<char>;
            out = std::copy(inf_name.begin(), inf_name.end(), out);
            return {std::move(out), 3};
        }
        else if(std::isnan(val)) [[unlikely]]
        {
            auto nan_name = uppercase ? nan_name_upper<char> : nan_name_lower<char>;
            out = std::copy(nan_name.begin(), nan_name.end(), out);
            return {std::move(out), 3};
        }

        char buf[256];
        std::size_t size = call_char_conv(val, buf, std::size(buf), ch_fmt);

        if(uppercase)
        {
            out = std::transform(
                buf,
                buf + size,
                out,
                [](char ch) -> CharT
                {
                    if('a' <= ch && ch <= 'z')
                        ch -= 'a' - 'A'; // to upper
                    return static_cast<CharT>(ch);
                }
            );
        }
        else
        {
            out = std::copy_n(buf, size, out);
        }

        return {std::move(out), size};
    }

    // Output in reversed order for easier implementation of locale support
    template <typename OutputIt>
    std::pair<OutputIt, std::size_t> float_to_chars_reversed(locale_ref loc, OutputIt out, T val) const
    {
        using namespace std::literals;

        auto [ch_fmt, uppercase] = get_chars_fmt();

        if(std::isinf(val)) [[unlikely]]
        {
            auto inf_name = uppercase ? inf_name_upper<char> : inf_name_lower<char>;
            out = std::copy(inf_name.begin(), inf_name.end(), out);
            return {std::move(out), 3};
        }
        else if(std::isnan(val)) [[unlikely]]
        {
            auto nan_name = uppercase ? nan_name_upper<char> : nan_name_lower<char>;
            out = std::copy(nan_name.begin(), nan_name.end(), out);
            return {std::move(out), 3};
        }

        const facet_type& facet = std::use_facet<facet_type>(loc);

        char buf[256];
        std::size_t size = call_char_conv(val, buf, std::size(buf), ch_fmt);
        std::size_t length = 0;

        CharT sep = facet.thousands_sep();
        const std::size_t sep_width = utf::codepoint(static_cast<char32_t>(sep)).estimate_width();
        std::string grouping = facet.grouping();

        std::size_t digit_count = 0;
        std::size_t sep_idx = 0;
        std::size_t count_since_sep = 0;
        bool point_reached = false;

        for(auto it = std::reverse_iterator(buf + size); it != std::reverse_iterator(buf); ++it)
        {
            char ch = *it;

            if(ch == '.') [[unlikely]]
            {
                CharT dp = facet.decimal_point();
                length += utf::codepoint(static_cast<char32_t>(dp)).estimate_width();
                point_reached = true;
                count_since_sep = 0;

                *out = dp;
                ++out;

                continue;
            }

            if(digit_count != 0 && point_reached)
            {
                char current_grouping_val = index_grouping(grouping, sep_idx);
                if(count_since_sep >= std::size_t(current_grouping_val))
                {
                    ++sep_idx;
                    count_since_sep = 0;
                    length += sep_width;

                    *out = sep;
                    ++out;
                }
            }

            if(uppercase)
            {
                if('a' <= ch && ch <= 'z')
                    ch -= 'a' - 'A'; // to upper
            }
            ++length;
            ++count_since_sep;
            ++digit_count;

            *out = static_cast<CharT>(ch);
            ++out;
        }

        return {std::move(out), length};
    }
};

/**
 * @brief Base formatter for Unicode code point.
 *
 * @sa utf::codepoint
 *
 * Accepted format types are: none, `c`, `?`.
 * - `c`: Output as a character.
 * - none: Same as `c`.
 * - `?`: Debug output, using escaped sequences if possible.
 */
class codepoint_formatter : public std_formatter_base
{
public:
    void set_data(const std_formatter_data& dt)
    {
        PAPILIO_ASSERT(dt.contains_type(U"c?"));

        data() = dt;
        data().type = dt.type_or(U'c');
        data().fill = dt.fill_or(U' ');
    }

    void set_debug_format() noexcept
    {
        data().type = U'?';
    }

    template <typename FormatContext>
    auto format(utf::codepoint cp, FormatContext& ctx)
    {
        using context_t = format_context_traits<FormatContext>;
        using char_type = typename FormatContext::char_type;

        if(data().type == U'?')
        {
            context_t::append(ctx, char_type('\''));
            context_t::append_escaped(ctx, cp, 1);
            context_t::append(ctx, char_type('\''));

            return context_t::out(ctx);
        }

        auto [left, right] = fill_size(cp.estimate_width());

        fill(ctx, left);
        context_t::append(ctx, cp);
        fill(ctx, right);

        return context_t::out(ctx);
    }
};

/**
 * @brief Base formatter for Unicode string.
 *
 * @sa utf::basic_string_ref
 *
 * @tparam CharT Charater type
 *
 * Accepted format types are: none, `s`, `?`.
 * - `s`: Copies the string to the output.
 * - none: Same as `s`.
 * - `?`: Debug output, using escaped sequences if possible.
 */
template <typename CharT>
class string_formatter : public std_formatter_base
{
public:
    using string_ref_type = utf::basic_string_ref<CharT>;
    using string_container_type = utf::basic_string_container<CharT>;

    void set_data(const std_formatter_data& dt)
    {
        PAPILIO_ASSERT(dt.contains_type(U"s?"));

        data() = dt;
        data().type = dt.type_or(U's');
        data().fill = dt.fill_or(U' ');
        if(data().align == format_align::default_align)
            data().align = format_align::left;
    }

    void set_debug_format() noexcept
    {
        data().type = U'?';
    }

    template <typename FormatContext>
    auto format(string_ref_type str, FormatContext& ctx)
        -> typename FormatContext::iterator
    {
        using context_t = format_context_traits<FormatContext>;

        if(data().type == '?')
        {
            context_t::append(ctx, CharT('"'));
            context_t::append_escaped(ctx, str);
            context_t::append(ctx, CharT('"'));
            return context_t::out(ctx);
        }

        std::size_t used = 0; // Used width

        // The "precision" for a string means the max width can be used.
        if(data().precision != 0)
        {
            for(auto it = str.begin(); it != str.end(); ++it)
            {
                std::size_t w = utf::codepoint(*it).estimate_width();
                if(used + w > data().precision)
                {
                    str.assign(str.begin(), it);
                    break;
                }
                used += w;
            }
        }
        else
        {
            for(utf::codepoint cp : str)
                used += cp.estimate_width();
        }

        auto [left, right] = fill_size(used);

        fill(ctx, left);
        context_t::append(ctx, str);
        fill(ctx, right);

        return context_t::out(ctx);
    }
};

/**
 * @brief Base formatter for enumerations.
 *
 * @tparam Enum The enumeration type
 * @tparam CharT Character type
 *
 * Accepted format types: none, `s`, `b`, `B`, `x`, `X`, `o`, `d`.
 * - none, `s`: Format as string. @sa string_formatter
 * - `b`, `B`, `x`, `X`, `o`, `d`: Format as integer. @sa int_formatter
 *
 * @warning The "enum-to-string" implementation has some limitations due to C++ lacking of reflection.
 *          It will fallback to integer representation on this occasion.
 * @sa enum_name
 */
template <typename Enum, typename CharT = char>
requires std::is_enum_v<Enum>
class enum_formatter
{
public:
    using char_type = char;

    template <typename ParseContext>
    auto parse(ParseContext& ctx)
        -> typename ParseContext::iterator
    {
        std_formatter_parser<ParseContext, true> parser;

        auto [result, it] = parser.parse(ctx, U"sBbXxod");

        m_data = result;

#ifndef PAPILIO_HAS_ENUM_NAME
        // No "enum-to-string" support, fallback to integer representation
        m_data.type = m_data.type_or(U'd');
#endif

        return it;
    }

    template <typename FormatContext>
    auto format(Enum e, FormatContext& ctx) const
        -> typename FormatContext::iterator
    {
        if(m_data.type_or(U's') != U's')
        {
            int_formatter<std::underlying_type_t<Enum>, CharT> fmt;
            fmt.set_data(m_data);
            return fmt.format(PAPILIO_NS to_underlying(e), ctx);
        }
#ifdef PAPILIO_HAS_ENUM_NAME
        else
        {
            string_formatter<CharT> fmt;
            fmt.set_data(m_data);
            if constexpr(char8_like<CharT>)
            {
                std::string_view name = enum_name<Enum>(e);
                auto name_conv = std::basic_string_view<CharT>(
                    std::bit_cast<const CharT*>(name.data()),
                    name.size()
                );
                return fmt.format(
                    name_conv,
                    ctx
                );
            }
            else
            {
                utf::string_ref name = enum_name<Enum>(e);
                auto name_conv = name.to_string<CharT>();
                return fmt.format(
                    name_conv,
                    ctx
                );
            }
        }
#else
        PAPILIO_UNREACHABLE();
#endif
    }

private:
    std_formatter_data m_data{.type = U's'};
};

/**
 * @brief Base formatter for ranges.
 *
 * @note Different from the standard one, whose first template parameter is @b underlying type.
 *
 * @tparam R @b Range type
 * @tparam CharT Character type
 */
template <typename R, typename CharT = char>
class range_formatter
{
public:
    using char_type = CharT;

    using underlying_type = std::remove_cvref_t<std::ranges::range_value_t<R>>;
    using string_view_type = std::basic_string_view<CharT>;

    range_formatter()
    {
        switch(get_kind())
        {
        default:
        case range_format::disabled:
            break;

        case range_format::set:
        case range_format::map:
            use_set_brackets();
            break;

        case range_format::sequence:
            use_seq_brackets();
            break;

        case range_format::string:
        case range_format::debug_string:
            clear_brackets();
            break;
        }
    }

    void set_separator(string_view_type sep) noexcept
    {
        m_sep = sep;
    }

    void set_brackets(string_view_type opening, string_view_type closing) noexcept
    {
        set_brackets_internal(opening, closing);
    }

    void set_debug_format() noexcept
    {
        m_debug = true;
    }

    template <typename ParseContext, typename FormatContext>
    auto format(const R& rng, ParseContext& parse_ctx, FormatContext& fmt_ctx) const
        -> typename FormatContext::iterator
    {
        using context_t = format_context_traits<FormatContext>;

        using underlying_formatter_type = typename context_t::template formatter_type<underlying_type>;
        underlying_formatter_type underlying_fmt{};
        using fmt_t = formatter_traits<underlying_formatter_type>;

        auto it = parse_ctx.begin();
        bool set_underlying_debug = true;
        if(it == parse_ctx.end()) [[unlikely]]
            goto end_parse;

        if(char32_t ch = *it; ch == U'n')
        {
            clear_brackets();
            ++it;
        }
        else if(ch == U'm')
        {
            use_set_brackets();
            fmt_t::try_set_brackets(underlying_fmt, string_view_type(), string_view_type());
            fmt_t::try_set_separator(underlying_fmt, PAPILIO_TSTRING_VIEW(CharT, ": "));
            ++it;
        }
        else if(ch == U'?')
        {
            m_debug = true;
            ++it;
            if(it == parse_ctx.end() || *it != U's')
                throw format_error("invalid range format");
            ++it;
            parse_ctx.advance_to(it);
            as_string(rng, fmt_ctx, m_debug);
            return fmt_ctx.out();
        }
        else if(ch == U's')
        {
            ++it;
            parse_ctx.advance_to(it);
            as_string(rng, fmt_ctx, m_debug);
            return fmt_ctx.out();
        }
        else if(ch != U':')
        {
            // End of specification / invalid specification
            goto end_parse;
        }

        if(it != parse_ctx.end() && *it == U':')
        {
            ++it;
            if constexpr(fmt_t::parsable())
            {
                parse_ctx.advance_to(it);
                it = underlying_fmt.parse(parse_ctx);
            }
            set_underlying_debug = false;
        }

end_parse:
        if(set_underlying_debug)
            fmt_t::try_set_debug_format(underlying_fmt);

        parse_ctx.advance_to(it);

        // Being formatting

        context_t::append(fmt_ctx, m_opening);

        // Possible implicit conversion when forwarding range values to the underlying formatter.
        // Suppress related compiler warnings.
#ifdef PAPILIO_COMPILER_CLANG
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wsign-conversion"
#endif

        bool first = true;
        for(auto&& i : rng)
        {
            if(!first)
            {
                context_t::append(fmt_ctx, m_sep);
            }
            first = false;

            fmt_t::format(underlying_fmt, i, fmt_ctx);
        }


#ifdef PAPILIO_COMPILER_CLANG
#    pragma clang diagnostic pop
#endif

        context_t::append(fmt_ctx, m_closing);

        return context_t::out(fmt_ctx);
    }

private:
    mutable string_view_type m_sep = PAPILIO_TSTRING_VIEW(CharT, ", ");
    mutable string_view_type m_opening;
    mutable string_view_type m_closing;
    mutable bool m_debug = false;

    template <typename FormatContext>
    static void as_string(const R& rng, FormatContext& ctx, bool debug_format)
    {
        using context_t = format_context_traits<FormatContext>;

        constexpr bool is_string_like_range =
            std::convertible_to<std::ranges::range_reference_t<R>, CharT> ||
            std::convertible_to<std::ranges::range_reference_t<R>, utf::codepoint>;

        if constexpr(!is_string_like_range)
            throw format_error("invalid range format");
        else
        {
            utf::basic_string_container<CharT> str;
            str.assign_range(rng);

            if(debug_format)
            {
                context_t::append(ctx, CharT('"'));
                context_t::append_escaped(ctx, str);
                context_t::append(ctx, CharT('"'));
            }
            else
                context_t::append(ctx, str);
        }
    }

    static constexpr range_format get_kind()
    {
        return format_kind<std::remove_cvref_t<R>>;
    }

    void set_brackets_internal(string_view_type opening, string_view_type closing) const noexcept
    {
        m_opening = opening;
        m_closing = closing;
    }

    void clear_brackets() const noexcept
    {
        set_brackets_internal(string_view_type(), string_view_type());
    }

    void use_set_brackets() const
    {
        set_brackets_internal(PAPILIO_TSTRING_VIEW(CharT, "{"), PAPILIO_TSTRING_VIEW(CharT, "}"));
    }

    void use_seq_brackets() const
    {
        set_brackets_internal(PAPILIO_TSTRING_VIEW(CharT, "["), PAPILIO_TSTRING_VIEW(CharT, "]"));
    }
};

/// @}

/**
 * @brief Formatter for integral types.
 *
 * @tparam T Integral type except Boolean type and character types
 * @tparam CharT Character type
 *
 * Accepted format types are: none, `b`, `B`, `x`, `X`, `o`, `d`, `c`.
 * -  none, `b`, `B`, `x`, `X`, `o`, `d`: Format as integer. @sa int_formatter
 * - `c`: Format as a Unicode code point. @sa codepoint_formatter
 *
 * @throw format_error If the integer value is to big for a Unicode code point for `c` type.
 */
PAPILIO_EXPORT template <std::integral T, typename CharT>
requires(!std::same_as<T, bool> && !char_like<T>)
class formatter<T, CharT>
{
public:
    template <typename ParseContext>
    auto parse(ParseContext& ctx) -> typename ParseContext::iterator
    {
        using namespace std::literals;

        using parser_t = std_formatter_parser<ParseContext, false>;

        parser_t parser;

        typename ParseContext::iterator it{};
        std::tie(m_data, it) = parser.parse(ctx, U"XxBbodc"sv);

        ctx.advance_to(it);
        return it;
    }

    template <typename FormatContext>
    auto format(T val, FormatContext& ctx) const -> typename FormatContext::iterator
    {
        if(m_data.type == U'c')
        {
            if(std::cmp_greater(val, std::numeric_limits<std::uint32_t>::max()))
                throw format_error("integer value out of range");

            codepoint_formatter fmt;
            fmt.set_data(m_data);
            return fmt.format(static_cast<char32_t>(val), ctx);
        }
        else
        {
            int_formatter<T, CharT> fmt;
            fmt.set_data(m_data);
            return fmt.format(val, ctx);
        }
    }

private:
    std_formatter_data m_data;
};

/**
 * @brief Formatter for floating points.
 *
 * @tparam T Floating point type
 * @tparam CharT Character type
 *
 * Accepted format types are: none, `f`, `F`, `g`, `G`, `e`, `E`, `a`, and `A`.
 * @sa float_formatter
 */
PAPILIO_EXPORT template <std::floating_point T, typename CharT>
class formatter<T, CharT>
{
public:
    template <typename ParseContext>
    auto parse(ParseContext& ctx) -> typename ParseContext::iterator
    {
        using namespace std::literals;

        using parser_t = std_formatter_parser<ParseContext, false>;

        parser_t parser;

        typename ParseContext::iterator it{};
        std::tie(m_data, it) = parser.parse(ctx, U"fFgGeEaA"sv);

        ctx.advance_to(it);
        return it;
    }

    template <typename FormatContext>
    auto format(T val, FormatContext& ctx) const -> typename FormatContext::iterator
    {
        float_formatter<T, CharT> fmt;
        fmt.set_data(m_data);
        return fmt.format(val, ctx);
    }

private:
    std_formatter_data m_data;
};

/**
 * @brief Formatter for Unicode code point.
 *
 * @sa utf::codepoint
 *
 * @tparam CharT Character type
 *
 * Accepted format types are: none, `c`, `?`, `b`,`B`,`x`,`X`,`o`,`d`.
 * - none, `c`, `?`: Format as a Unicode code point. @sa codepoint_formatter
 * - `b`,`B`,`x`,`X`,`o`,`d`: Format as integer. @sa int_formatter
 */
PAPILIO_EXPORT template <typename CharT>
class formatter<utf::codepoint, CharT>
{
public:
    void set_debug_format() noexcept
    {
        m_data.type = U'?';
    }

    template <typename ParseContext>
    auto parse(ParseContext& ctx) -> typename ParseContext::iterator
    {
        using namespace std::literals;

        using parser_t = std_formatter_parser<ParseContext, false>;

        parser_t parser;

        typename ParseContext::iterator it{};
        std::tie(m_data, it) = parser.parse(ctx, U"XxBbodc?"sv);

        ctx.advance_to(it);
        return it;
    }

    template <typename FormatContext>
    auto format(utf::codepoint cp, FormatContext& ctx) const -> typename FormatContext::iterator
    {
        if(!m_data.contains_type(U"c?"))
        {
            int_formatter<std::uint32_t, CharT> fmt;
            fmt.set_data(m_data);
            return fmt.format(static_cast<char32_t>(cp), ctx);
        }
        else
        {
            codepoint_formatter fmt;
            fmt.set_data(m_data);
            return fmt.format(cp, ctx);
        }
    }

private:
    std_formatter_data m_data;
};

/**
 * @brief Formatter for character type.
 * It simply redirect all calls to the Unicode code point formatter.
 *
 * @sa utf::codepoint
 *
 * @tparam CharT Character type
 *
 * Accepted format types are: none, `c`, `?`, `b`,`B`,`x`,`X`,`o`,`d`.
 * - none, `c`, `?`: Format as a Unicode code point. @sa codepoint_formatter
 * - `b`,`B`,`x`,`X`,`o`,`d`: Format as integer. @sa int_formatter
 */
PAPILIO_EXPORT template <typename CharT>
class formatter<CharT, CharT> : public formatter<utf::codepoint, CharT>
{
    using my_base = formatter<utf::codepoint, CharT>;

public:
    template <typename FormatContext>
    auto format(CharT ch, FormatContext& ctx) const
        -> typename FormatContext::iterator
    {
        return my_base::format(
            utf::codepoint(static_cast<char32_t>(ch)),
            ctx
        );
    }
};

/**
 * @brief Formatter for the Boolean type.
 *
 * @tparam CharT Character type
 *
 * Accepted format types are: none, `s`, `b`,`B`,`x`,`X`,`o`,`d`.
 * - none, `s`: Format as string representing the Boolean value. @sa string_formatter
 * - `b`,`B`,`x`,`X`,`o`,`d`: Format as integer. @sa int_formatter
 *
 * The default string representation is "true" and "false".
 * If the locale flag (`L`) is set, `truename()` and `falsename()` of `std::numpunct` will be used.
 * @sa locale_ref
 */
PAPILIO_EXPORT template <typename CharT>
class formatter<bool, CharT>
{
public:
    template <typename ParseContext>
    auto parse(ParseContext& ctx) -> typename ParseContext::iterator
    {
        using namespace std::literals;

        using parser_t = std_formatter_parser<ParseContext, false>;

        parser_t parser;

        typename ParseContext::iterator it{};
        std::tie(m_data, it) = parser.parse(ctx, U"sXxBbod"sv);

        ctx.advance_to(it);
        return it;
    }

    template <typename FormatContext>
    auto format(bool val, FormatContext& ctx) const -> typename FormatContext::iterator
    {
        if(!m_data.contains_type(U's'))
        {
            int_formatter<std::uint8_t, CharT> fmt;
            fmt.set_data(m_data);
            return fmt.format(static_cast<std::uint8_t>(val), ctx);
        }
        else
        {
            using context_t = format_context_traits<FormatContext>;

            string_formatter<CharT> fmt;
            fmt.set_data(m_data);
            const auto str = this->get_str<context_t::use_locale()>(
                val, context_t::getloc_ref(ctx)
            );
            return fmt.format(str, ctx);
        }
    }

private:
    std_formatter_data m_data;

    template <bool ContextUseLocale>
    utf::basic_string_container<CharT> get_str(bool val, locale_ref loc) const
    {
        std::basic_string_view<CharT> true_name = PAPILIO_TSTRING_VIEW(CharT, "true");
        std::basic_string_view<CharT> false_name = PAPILIO_TSTRING_VIEW(CharT, "false");

        if(!m_data.use_locale) [[likely]]
        {
            return val ? true_name : false_name;
        }
        else
        {
            if constexpr(!ContextUseLocale)
            {
                return val ? true_name : false_name;
            }
            else
            {
                const auto& facet = std::use_facet<std::numpunct<CharT>>(loc);
                return val ? facet.truename() : facet.falsename();
            }
        }
    }
};

/**
 * @brief Formatter for reference type of `std::vector<bool>`
 *
 * This formatter will forward the value to bool formatter.
 */
PAPILIO_EXPORT template <typename CharT>
class formatter<std::vector<bool>::reference, CharT> : public formatter<bool, CharT>
{};

/**
 * @brief Formatter for Unicode string.
 *
 * @sa utf::basic_string_container
 *
 * @tparam CharT Charater type
 *
 * Accepted format types are: none, `s`, `?`.
 * @sa string_formatter
 */
PAPILIO_EXPORT template <typename CharT>
class formatter<utf::basic_string_container<CharT>, CharT>
{
public:
    using string_container_type = utf::basic_string_container<CharT>;

    void set_debug_format() noexcept
    {
        m_data.type = U'?';
    }

    template <typename ParseContext>
    auto parse(ParseContext& ctx) -> typename ParseContext::iterator
    {
        using namespace std::literals;

        using parser_t = std_formatter_parser<ParseContext, true>;

        parser_t parser;

        typename ParseContext::iterator it{};
        std::tie(m_data, it) = parser.parse(ctx, U"s?"sv);

        ctx.advance_to(it);
        return it;
    }

    template <typename FormatContext>
    auto format(const string_container_type& str, FormatContext& ctx) const -> typename FormatContext::iterator
    {
        string_formatter<CharT> fmt;
        fmt.set_data(m_data);
        return fmt.format(str, ctx);
    }

private:
    std_formatter_data m_data;
};

PAPILIO_EXPORT template <typename CharT>
class formatter<CharT*, CharT> : public formatter<utf::basic_string_container<CharT>, CharT>
{};

PAPILIO_EXPORT template <typename CharT>
class formatter<const CharT*, CharT> : public formatter<utf::basic_string_container<CharT>, CharT>
{};

PAPILIO_EXPORT template <typename CharT>
class formatter<std::basic_string<CharT>, CharT> : public formatter<utf::basic_string_container<CharT>, CharT>
{};

PAPILIO_EXPORT template <typename CharT>
class formatter<std::basic_string_view<CharT>, CharT> : public formatter<utf::basic_string_container<CharT>, CharT>
{};

/**
 * @brief Formatter for pointers.
 *
 * @tparam CharT Character type
 *
 * Accepted format types: none, `p`, `P`.
 * - none, `p`: Format as if casted to `std::uintptr_t`,
 *   then format as integer in hexadecimal format with alternate form enabled. @sa int_formatter
 * - `P`: Same as `p`, but with `0X` prefix.
 */
PAPILIO_EXPORT template <typename CharT>
class formatter<const void*, CharT>
{
public:
    template <typename ParseContext>
    auto parse(ParseContext& ctx) -> typename ParseContext::iterator
    {
        using namespace std::literals;

        using parser_t = std_formatter_parser<ParseContext, true>;

        parser_t parser;

        typename ParseContext::iterator it{};
        std::tie(m_data, it) = parser.parse(ctx, U"pP"sv);

        if(m_data.use_locale)
            throw format_error("invalid format");

        switch(m_data.type)
        {
        case U'\0':
        case U'p':
            m_data.type = 'x';
            break;

        case U'P':
            m_data.type = 'X';
            break;

        default:
            PAPILIO_UNREACHABLE();
        }

        m_data.alternate_form = true;

        ctx.advance_to(it);
        return it;
    }

    template <typename FormatContext>
    auto format(const void* p, FormatContext& ctx) const -> typename FormatContext::iterator
    {
        int_formatter<std::uintptr_t, CharT> fmt;
        fmt.set_data(m_data);
        return fmt.format(reinterpret_cast<std::uintptr_t>(p), ctx);
    }

private:
    std_formatter_data m_data{.type = U'x', .alternate_form = true};
};

/**
 * @brief Formatter for enum values.
 *
 * @tparam T The enum type
 * @tparam CharT Character type
 *
 * Accepted format types: none, `s`, `b`, `B`, `x`, `X`, `o`, `d`.
 * @sa enum_formatter
 *
 * @warning The "enum-to-string" implementation has some limitations due to C++ lacking of reflection.
 *          It will fallback to integer representation on this occasion.
 * @sa enum_name
 */
template <typename T, typename CharT>
requires std::is_enum_v<T>
class formatter<T, CharT> : public enum_formatter<T, CharT>
{};

template <std::ranges::input_range R, typename CharT>
class formatter<R, CharT> :
    public std::conditional_t<formattable<std::ranges::range_value_t<R>, CharT>, range_formatter<R, CharT>, disabled_formatter>
{};

/// @}

/// @addtogroup Format
/// @{

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
        static_assert(std::same_as<OutputIt, typename Context::iterator>);

        basic_format_parse_context<Context> parse_ctx(fmt, args);
        Context fmt_ctx(loc, out, args);

        basic_interpreter<Context> intp;
        intp.format(parse_ctx, fmt_ctx);

        return fmt_ctx.out();
    }
} // namespace detail

PAPILIO_EXPORT template <typename OutputIt>
OutputIt vformat_to(
    OutputIt out,
    std::string_view fmt,
    const format_args_ref_for<OutputIt, char>& args
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
    const format_args_ref_for<OutputIt, char>& args
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
    const format_args_ref_for<OutputIt, wchar_t>& args
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
    const format_args_ref_for<OutputIt, wchar_t>& args
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

/// @}
} // namespace papilio

#include "core.inl"

#include "detail/suffix.hpp"

#endif
