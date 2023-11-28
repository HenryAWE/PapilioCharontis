#pragma once

#include <string>
#include <variant>
#include <stdexcept>
#include <limits>
#include "../utf/utf.hpp"


namespace papilio::script
{
    class bad_variable_access : public std::bad_variant_access {};

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
    }

    template <typename CharT>
    class basic_variable : public detail::variable_base
    {
    public:
        using char_type = CharT;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;
        using string_container_type = utf::basic_string_container<CharT>;
        using int_type = std::int64_t;
        using float_type = long double;

        using variant_type = std::variant<
            bool,
            int_type,
            float_type,
            string_container_type
        >;

        basic_variable() = delete;
        basic_variable(const basic_variable&) = default;
        basic_variable(basic_variable&&) noexcept = default;
        basic_variable(bool v)
            : m_var(v) {}
        template <std::integral T>
        basic_variable(T i)
            : m_var(std::in_place_type<int_type>, i) {}
        template <std::floating_point T>
        basic_variable(T f)
            : m_var(std::in_place_type<float_type>, f) {}
        basic_variable(string_container_type str)
            : m_var(std::move(str)) {}
        template <string_like String>
        basic_variable(String&& str)
            : m_var(std::in_place_type<string_container_type>, std::forward<String>(str)) {}
        template <string_like String>
        basic_variable(independent_t, String&& str)
            : m_var(std::in_place_type<string_container_type>, independent, std::forward<String>(str)) {}

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

        bool holds_bool() const noexcept
        {
            return holds<bool>();
        }
        bool holds_int() const noexcept
        {
            return holds<int_type>();
        }
        bool holds_float() const noexcept
        {
            return holds<float_type>();
        }
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

        template <typename T>
        [[nodiscard]]
        T as() const
        {
            auto visitor = [](auto&& v) -> T
            {
                using std::is_same_v;
                using U = std::remove_cvref_t<decltype(v)>;

                if constexpr(is_same_v<T, bool>)
                {
                    if constexpr(is_same_v<U, string_container_type>)
                    {
                        return !v.empty();
                    }
                    else
                    {
                        return static_cast<bool>(v);
                    }
                }
                else if constexpr(std::integral<T> || std::floating_point<T>)
                {
                    if constexpr(is_same_v<U, string_container_type>)
                    {
                        throw_invalid_conversion();
                    }
                    else // ind_type and float_type
                    {
                        return static_cast<T>(v);
                    }
                }
                else if constexpr(is_same_v<T, string_container_type> || string_like<T>)
                {
                    if constexpr(is_same_v<U, string_container_type>)
                    {
                        return T(string_view_type(v));
                    }
                    else
                    {
                        throw_invalid_conversion();
                    }
                }
                else
                {
                    static_assert(!sizeof(T), "invalid type");
                }
            };

            return std::visit(visitor, m_var);
        }

        variant_type& to_underlying() noexcept
        {
            return m_var;
        }
        const variant_type& to_underlying() const noexcept
        {
            return m_var;
        }

        [[nodiscard]]
        std::partial_ordering compare(const basic_variable& var) const noexcept
        {
            auto visitor = [](auto&& lhs, auto&& rhs) -> std::partial_ordering
            {
                using std::is_same_v;
                using T = std::remove_cvref_t<decltype(lhs)>;
                using U = std::remove_cvref_t<decltype(rhs)>;

                if constexpr(is_same_v<T, U>)
                {
                    return lhs <=> rhs;
                }
                else if constexpr(is_same_v<T, string_container_type> || is_same_v<U, string_container_type>)
                {
                    return std::partial_ordering::unordered;
                }
                else
                {
                    using R = std::common_type_t<T, U>;
                    return static_cast<R>(lhs) <=> static_cast<R>(rhs);
                }
            };

            return std::visit(visitor, m_var, var.m_var);
        }

        [[nodiscard]]
        std::partial_ordering operator<=>(const basic_variable& rhs) const
        {
            return compare(rhs);
        }

        [[nodiscard]]
        bool equal(
            const basic_variable& var,
            float_type epsilon = std::numeric_limits<float_type>::epsilon()
        ) const noexcept {
            auto visitor = [epsilon](auto&& lhs, auto&& rhs)->bool
            {
                using std::is_same_v;
                using T = std::remove_cvref_t<decltype(lhs)>;
                using U = std::remove_cvref_t<decltype(rhs)>;

                if constexpr(is_same_v<T, U>)
                {
                    if constexpr(std::floating_point<T>)
                    {
                        return std::abs(lhs - rhs) < epsilon;
                    }
                    else
                    {
                        return lhs == rhs;
                    }
                }
                else
                {
                    if constexpr(is_same_v<T, string_container_type> || is_same_v<U, string_container_type>)
                    {
                        return false;
                    }
                    else
                    {
                        using R = std::common_type_t<T, U>;
                        if constexpr(std::floating_point<R>)
                        {
                            return std::abs(static_cast<R>(lhs) - static_cast<R>(rhs)) < epsilon;
                        }
                        else
                        {
                            return static_cast<R>(lhs) == static_cast<R>(rhs);
                        }
                    }
                }
            };

            return std::visit(visitor, m_var, var.m_var);
        }

        [[nodiscard]]
        bool operator==(const basic_variable& rhs) const noexcept
        {
            return equal(rhs);
        }

    private:
        variant_type m_var;
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
        public std::bool_constant<basic_variable_storable<T, CharT>> {};
    template <typename T, typename CharT>
    inline constexpr bool is_basic_variable_storable_v =
        is_basic_variable_storable<T, CharT>::value;

    template <typename T>
    using is_variable_storable = is_basic_variable_storable<T, char>;
    template <typename T>
    inline constexpr bool is_variable_storable_v = is_variable_storable<T>::value;
}
