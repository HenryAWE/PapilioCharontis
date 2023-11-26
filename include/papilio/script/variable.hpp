#pragma once

#include <string>
#include <variant>
#include <stdexcept>
#include <limits>
#include "../utf/utf.hpp"


namespace papilio::script
{
    class variable
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;
        using int_type = std::int64_t;
        using float_type = long double;

        using underlying_type = std::variant<
            bool,
            int_type,
            float_type,
            utf::string_container
        >;

        variable() = delete;
        variable(const variable&) = default;
        variable(variable&&) noexcept = default;
        variable(bool v)
            : m_var(v) {}
        template <std::integral T>
        variable(T i)
            : m_var(static_cast<int_type>(i)) {}
        template <std::floating_point T>
        variable(T f)
            : m_var(static_cast<float_type>(f)) {}
        variable(utf::string_container str)
            : m_var(std::move(str)) {}
        template <string_like String>
        variable(String&& str)
            : m_var(std::in_place_type<utf::string_container>, std::forward<String>(str)) {}
        template <string_like String>
        variable(independent_t, String&& str)
            : m_var(std::in_place_type<utf::string_container>, independent, std::forward<String>(str)) {}

        variable& operator=(const variable&) = default;
        variable& operator=(bool v)
        {
            m_var = v;
            return *this;
        }
        template <std::integral T>
        variable& operator=(T i)
        {
            m_var = static_cast<int_type>(i);
            return *this;
        }
        template <std::floating_point T>
        variable& operator=(T f)
        {
            m_var = static_cast<float_type>(f);
            return *this;
        }
        variable& operator=(string_type str)
        {
            m_var = std::move(str);
            return *this;
        }
        variable& operator=(utf::string_container str)
        {
            m_var.emplace<utf::string_container>(std::move(str));
            return *this;
        }
        template <string_like String>
        variable& operator=(String&& str)
        {
            m_var.emplace<utf::string_container>(std::forward<String>(str));
            return *this;
        }

        template <typename T>
        [[nodiscard]]
        bool holds() const noexcept
        {
            return std::holds_alternative<T>(m_var);
        }

        template <typename T>
        [[nodiscard]]
        const T& get() const noexcept
        {
            assert(holds<T>());
            return *std::get_if<T>(&m_var);
        }

        template <typename T>
        [[nodiscard]]
        T as() const
        {
            auto visitor = [](auto&& v)->T
            {
                using std::is_same_v;
                using U = std::remove_cvref_t<decltype(v)>;

                if constexpr(is_same_v<T, bool>)
                {
                    if constexpr(is_same_v<U, utf::string_container>)
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
                    if constexpr(is_same_v<U, utf::string_container>)
                    {
                        invalid_conversion();
                    }
                    else // ind_type and float_type
                    {
                        return static_cast<T>(v);
                    }
                }
                else if constexpr(is_same_v<T, utf::string_container> || string_like<T>)
                {
                    if constexpr(is_same_v<U, utf::string_container>)
                    {
                        return T(string_view_type(v));
                    }
                    else
                    {
                        invalid_conversion();
                    }
                }
                else
                {
                    static_assert(!sizeof(T), "invalid type");
                }
            };

            return std::visit(visitor, m_var);
        }

        underlying_type& to_underlying() noexcept
        {
            return m_var;
        }
        const underlying_type& to_underlying() const noexcept
        {
            return m_var;
        }

        [[nodiscard]]
        std::partial_ordering compare(const variable& var) const;
        [[nodiscard]]
        std::partial_ordering operator<=>(const variable& rhs) const
        {
            return compare(rhs);
        }

        [[nodiscard]]
        bool equal(
            const variable& var,
            float_type epsilon = std::numeric_limits<float_type>::epsilon()
        ) const noexcept;
        [[nodiscard]]
        bool operator==(const variable& rhs) const noexcept
        {
            return equal(rhs);
        }

        [[noreturn]]
        static void invalid_conversion()
        {
            throw std::runtime_error("invalid conversion");
        }

    private:
        underlying_type m_var;
    };

    template <typename T>
    struct is_variable_type
    {
        static constexpr bool value =
            std::is_same_v<T, bool> ||
            std::is_same_v<T, variable::int_type> ||
            std::is_same_v<T, variable::float_type> ||
            std::is_same_v<T, utf::string_container>;
    };
    template <typename T>
    inline constexpr bool is_variable_type_v = is_variable_type<T>::value;
}
