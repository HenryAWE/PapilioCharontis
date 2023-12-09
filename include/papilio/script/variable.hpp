#pragma once

#include <string>
#include <variant>
#include <stdexcept>
#include <limits>
#include "../utf/utf.hpp"

namespace papilio::script
{
class bad_variable_access : public std::bad_variant_access
{};

class invalid_conversion : public std::invalid_argument
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

template <typename CharT>
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

    basic_variable() = delete;
    basic_variable(const basic_variable&) = default;
    basic_variable(basic_variable&&) noexcept = default;

    basic_variable(bool v)
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

    template <string_like String>
    basic_variable(String&& str)
        : m_var(std::in_place_type<string_container_type>, std::forward<String>(str))
    {}

    template <string_like String>
    basic_variable(independent_t, String&& str)
        : m_var(std::in_place_type<string_container_type>, independent, std::forward<String>(str))
    {}

    basic_variable& operator=(const basic_variable&) = default;

    basic_variable& operator=(bool v)
    {
        emplace<bool>(v);
        return *this;
    }

    template <std::integral T>
    basic_variable& operator=(T i)
    {
        emplace<int_type>(i);
        return *this;
    }

    template <std::floating_point T>
    basic_variable& operator=(T f)
    {
        emplace<float_type>(f);
        return *this;
    }

    basic_variable& operator=(string_type str)
    {
        emplace<string_container_type>(std::move(str));
        return *this;
    }

    basic_variable& operator=(string_container_type str)
    {
        emplace<string_container_type>(std::move(str));
        return *this;
    }

    template <string_like String>
    basic_variable& operator=(String&& str)
    {
        emplace<utf::string_container>(std::forward<String>(str));
        return *this;
    }

    template <typename T, typename... Args>
    auto& emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        return m_var.template emplace<T>(std::forward<Args>(args)...);
    }

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
    variant_type& to_variant() noexcept
    {
        return m_var;
    }

    [[nodiscard]]
    const variant_type& to_variant() const noexcept
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

using variable = basic_variable<char>;
using wvariable = basic_variable<wchar_t>;

template <typename T, typename CharT>
concept basic_variable_storable =
    std::is_same_v<T, bool> ||
    std::is_same_v<T, variable::int_type> ||
    std::is_same_v<T, variable::float_type> ||
    std::is_same_v<T, utf::basic_string_container<CharT>>;

template <typename T, typename CharT>
struct is_basic_variable_storable :
    public std::bool_constant<basic_variable_storable<T, CharT>>
{};

template <typename T, typename CharT>
inline constexpr bool is_basic_variable_storable_v =
    is_basic_variable_storable<T, CharT>::value;

template <typename T>
using is_variable_storable = is_basic_variable_storable<T, char>;
template <typename T>
inline constexpr bool is_variable_storable_v = is_variable_storable<T>::value;
} // namespace papilio::script
