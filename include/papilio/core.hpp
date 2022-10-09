#pragma once

#include <string>
#include <variant>
#include <concepts>
#include <stdexcept>
#include <vector>
#include <map>
#include <charconv>
#include <limits>
#include <iterator>
#include "utf8.hpp"
#include "error.hpp"


namespace papilio
{
    namespace detail
    {
        template <typename CharT>
        [[nodiscard]]
        constexpr bool is_digit(CharT ch) noexcept
        {
            return '0' <= ch && ch <= '9';
        }

        template <typename CharT>
        [[nodiscard]]
        constexpr bool is_identifier_ch(CharT ch, bool first = false) noexcept
        {
            bool digit = is_digit(ch);
            if(first && digit)
                return false;

            return ('A' <= ch && ch <= 'z') || digit || ch == '_';
        }
    }

    namespace script
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
                string_type
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
            variable(string_type str)
                : m_var(std::move(str)) {}
            variable(string_view_type str)
                : m_var(std::in_place_type<string_type>, str) {}
            variable(const char_type* str)
                : m_var(std::in_place_type<string_type>, str) {}

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
            variable& operator=(string_view_type str)
            {
                m_var.emplace<string_type>(str);
                return *this;
            }
            variable& operator=(const char_type* str)
            {
                m_var.emplace<string_type>(str);
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
                        if constexpr(is_same_v<U, string_type>)
                        {
                            return !v.empty();
                        }
                        else
                        {
                            return static_cast<bool>(v);
                        }
                    }
                    else if constexpr(std::integral<T> ||std::floating_point<T>)
                    {
                        if constexpr(is_same_v<U, string_type>)
                        {
                            invalid_conversion();
                        }
                        else // ind_type and float_type
                        {
                            return static_cast<T>(v);
                        }
                    }
                    else if constexpr(is_same_v<T, string_type>)
                    {
                        if constexpr(is_same_v<U, string_type>)
                        {
                            return v;
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
        concept is_variable_type =
            std::is_same_v<T, bool> ||
            std::is_same_v<T, variable::int_type> ||
            std::is_same_v<T, variable::float_type> ||
            std::is_same_v<T, variable::string_type>;

    }

    template <typename T, typename CharT>
    concept basic_string_like =
        std::is_same_v<std::decay_t<T>, CharT*> ||
        std::is_same_v<std::decay_t<T>, const CharT*> ||
        std::is_same_v<T, std::basic_string<CharT>> ||
        std::is_same_v<T, std::basic_string_view<CharT>>;
    template <typename T>
    concept string_like = basic_string_like<T, char>;

    enum class align : std::uint8_t
    {
        left,
        middle,
        right
    };

    template <typename T>
    struct named_arg
    {
        using named_arg_tag = void;

        const char* name;
        const T& value;

        named_arg() = delete;
        constexpr named_arg(const named_arg&) noexcept = default;
        constexpr named_arg(const char* name_, const T& value_) noexcept
            : name(name_), value(value_) {}

        named_arg& operator=(const named_arg&) = delete;
    };

    template <typename T>
    constexpr named_arg<T> arg(const char* name, const T& value) noexcept
    {
        return named_arg<T>(name, value);
    }

    inline namespace literals
    {
        struct named_arg_proxy
        {
            const char* name;

            named_arg_proxy() = delete;
            named_arg_proxy(const named_arg_proxy&) = delete;
            constexpr named_arg_proxy(const char* name_) noexcept
                : name(name_) {}

            template <typename T>
            constexpr named_arg<T> operator=(const T& value) noexcept
            {
                return named_arg(name, value);
            }
        };

        [[nodiscard]]
        constexpr named_arg_proxy operator""_a(const char* name, std::size_t) noexcept
        {
            return named_arg_proxy(name);
        }
    }

    class slice
    {
    public:
        using index_type = std::make_signed_t<std::size_t>; // ssize_t

        static constexpr index_type npos = std::numeric_limits<index_type>::max();

        constexpr slice() noexcept
            : m_begin(0), m_end(npos) {}
        constexpr slice(const slice&) noexcept = default;
        constexpr explicit slice(index_type begin_, index_type end_) noexcept
            : m_begin(begin_), m_end(end_) {}

        slice& operator=(const slice&) noexcept = default;

        constexpr void normalize(index_type length) noexcept
        {
            if(m_begin < 0)
                m_begin = length + m_begin;
            if(m_end)
                m_end = length + m_end;
        }

        [[nodiscard]]
        constexpr index_type begin() const noexcept
        {
            return m_begin;
        }
        [[nodiscard]]
        constexpr index_type end() const noexcept
        {
            return m_end;
        }

    private:
        index_type m_begin;
        index_type m_end;
    };

    class indexing_value
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;
        using index_type = std::make_signed_t<std::size_t>; // ssize_t
        using underlying_type = std::variant<
            index_type,
            slice,
            string_type
        >;

        indexing_value() = delete;
        indexing_value(const indexing_value&) = default;
        indexing_value(indexing_value&&) = default;
        indexing_value(index_type index)
            : m_val(index) {}
        indexing_value(slice s)
            : m_val(s) {}
        indexing_value(string_type key)
            : m_val(std::move(key)) {}

        template <typename T>
        [[nodiscard]]
        bool holds() const noexcept
        {
            return std::holds_alternative<T>(m_val);
        }

        [[nodiscard]]
        bool is_index() const noexcept
        {
            return holds<index_type>();
        }
        [[nodiscard]]
        bool is_slice() const noexcept
        {
            return holds<slice>();
        }
        [[nodiscard]]
        bool is_key() const noexcept
        {
            return holds<string_type>();
        }

        [[nodiscard]]
        index_type as_index() const noexcept
        {
            // use std::get_if to avoid exception
            return *std::get_if<index_type>(&m_val);
        }
        [[nodiscard]]
        const slice& as_slice() const noexcept
        {
            return *std::get_if<slice>(&m_val);
        }
        [[nodiscard]]
        const std::string& as_key() const noexcept
        {
            return *std::get_if<string_type>(&m_val);
        }

        [[nodiscard]]
        underlying_type& to_underlying() noexcept
        {
            return m_val;
        }
        [[nodiscard]]
        const underlying_type& to_underlying() const noexcept
        {
            return m_val;
        }

    private:
        underlying_type m_val;
    };
    class attribute_name
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;

        attribute_name() = delete;
        attribute_name(const attribute_name&) = default;
        attribute_name(attribute_name&&) noexcept = default;
        template <typename... Args> requires(std::constructible_from<string_type, Args...>)
        attribute_name(Args&&... args)
            : m_name(std::forward<Args>(args)...) {}

        [[nodiscard]]
        bool operator==(const attribute_name& rhs) const noexcept = default;
        [[nodiscard]]
        friend bool operator==(const attribute_name& lhs, string_view_type rhs) noexcept
        {
            return lhs.m_name == rhs;
        }
        [[nodiscard]]
        friend bool operator==(string_view_type lhs, const attribute_name& rhs) noexcept
        {
            return lhs == rhs.m_name;
        }

        [[nodiscard]]
        static bool validate(string_view_type name) noexcept;

    private:
        string_type m_name;
    };

    class format_arg
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;

        class handle
        {
        public:
            

        private:
        };

        using underlying_type = std::variant<
            std::monostate,
            char_type,
            int,
            unsigned int,
            long long int,
            unsigned long long int,
            float,
            double,
            long double,
            const char_type*,
            string_view_type,
            string_type,
            handle
        >;

        format_arg()
            : m_val() {}
        format_arg(const format_arg&) = default;
        format_arg(format_arg&&) noexcept = default;
        format_arg(underlying_type val)
            : m_val(std::move(val)) {}
        template <typename T, typename... Args>
        format_arg(std::in_place_type_t<T>, Args&&... args)
            : m_val(std::in_place_type<T>, std::forward<Args>(args)...) {}

        format_arg& operator=(const format_arg&) = default;
        format_arg& operator=(format_arg&&) noexcept = default;

        format_arg index(const indexing_value& idx) const;
        format_arg attribute(attribute_name name) const;

        [[noreturn]]
        static void invalid_index()
        {
            throw std::out_of_range("invalid index");
        }
        [[noreturn]]
        static void invalid_attribute()
        {
            throw std::invalid_argument("invalid attribute");
        }

        template <typename T>
        [[nodiscard]]
        friend const T& get(const format_arg& val)
        {
            return std::get<T>(val.m_val);
        }

        script::variable as_variable() const;

    private:
        underlying_type m_val;
    };

    // access members of format argument
    class format_arg_access
    {
    public:
        using member_type = std::variant<
            indexing_value,
            attribute_name
        >;

        format_arg_access() noexcept : m_members() {}
        format_arg_access(const format_arg_access&) = delete;
        format_arg_access(std::vector<member_type> members)
            : m_members(std::move(members)) {}

        [[nodiscard]]
        format_arg access(format_arg arg) const;

        [[nodiscard]]
        bool empty() const noexcept
        {
            return m_members.empty();
        }

    private:
        std::vector<member_type> m_members;
    };

    class dynamic_format_arg_store
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;
        using size_type = std::size_t;

        dynamic_format_arg_store() = default;
        dynamic_format_arg_store(const dynamic_format_arg_store&) = delete;
        dynamic_format_arg_store(dynamic_format_arg_store&&) = default;
        template <typename... Args>
        dynamic_format_arg_store(Args&&... args)
        {
            emplace(std::forward<Args>(args)...);
        }

        template <typename... Args>
        void emplace(Args&&... args)
        {
            auto helper = [&]<typename T> (T&& arg)
            {
                if constexpr(requires() { typename T::named_arg_tag; })
                {
                    m_named_args.emplace(std::make_pair(arg.name, arg.value));
                }
                else
                {
                    m_args.emplace_back(arg);
                }
            };

            (helper(std::forward<Args>(args)), ...);
        }

        [[nodiscard]]
        const format_arg& get(size_type i) const;
        [[nodiscard]]
        const format_arg& get(string_view_type key) const;
        [[nodiscard]]
        const format_arg& get(const indexing_value& idx) const;

        [[nodiscard]]
        bool check(size_type i) const noexcept
        {
            return i < m_args.size();
        }
        [[nodiscard]]
        bool check(string_view_type key) const noexcept
        {
            return m_named_args.contains(key);
        }
        [[nodiscard]]
        bool check(const indexing_value& idx) const noexcept;

        [[nodiscard]]
        const format_arg& operator[](size_type i) const
        {
            return get(i);
        }
        [[nodiscard]]
        const format_arg& operator[](string_view_type key) const
        {
            return get(key);
        }
        [[nodiscard]]
        const format_arg& operator[](const indexing_value& idx) const
        {
            return get(idx);
        }

        [[nodiscard]]
        size_type size() const noexcept
        {
            return m_args.size();
        }
        [[nodiscard]]
        size_type named_size() const noexcept
        {
            return m_named_args.size();
        }

        void clear() noexcept
        {
            m_args.clear();
            m_named_args.clear();
        }

    private:
        std::vector<format_arg> m_args;
        std::map<string_type, format_arg, std::less<>> m_named_args;
    };
}
