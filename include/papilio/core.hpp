#pragma once

#include <format>
#include <string>
#include <variant>
#include <concepts>
#include <stdexcept>
#include <vector>
#include <map>
#include <charconv>
#include <limits>
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
                            std::basic_stringstream<char_type> ss;
                            ss << v;
                            T result;
                            ss >> result;

                            return result;
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
                            using std::to_string;
                            return to_string(v);
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

            std::partial_ordering compare(const variable& var) const
            {
                auto visitor = [](auto&& lhs, auto&& rhs)->std::partial_ordering
                {
                    using std::is_same_v;
                    using T = std::remove_cvref_t<decltype(lhs)>;
                    using U = std::remove_cvref_t<decltype(rhs)>;

                    if constexpr(is_same_v<T, U>)
                    {
                        return lhs <=> rhs;
                    }
                    else if constexpr(is_same_v<T, string_type> || is_same_v<U, string_type>)
                    {
                        throw std::invalid_argument("invalid argument");
                    }
                    else
                    {
                        using R = std::common_type_t<T, U>;
                        return static_cast<R>(lhs) <=> static_cast<R>(rhs);
                    }
                };

                return std::visit(visitor, m_var, var.to_underlying());
            }
            std::partial_ordering operator<=>(const variable& rhs) const
            {
                return compare(rhs);
            }

            bool equal(
                const variable& var,
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
                        if constexpr(is_same_v<T, string_type> || is_same_v<U, string_type>)
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

                return std::visit(visitor, m_var, var.to_underlying());
            }
            bool operator==(const variable& rhs) const noexcept
            {
                return equal(rhs);
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

    class indexing_value
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;
        using size_type = std::size_t;
        using underlying_type = std::variant<
            size_type,
            string_type
        >;

        indexing_value() = delete;
        indexing_value(const indexing_value&) = default;
        indexing_value(indexing_value&&) = default;
        indexing_value(size_type index)
            : m_val(index) {}
        indexing_value(string_type key)
            : m_val(std::move(key)) {}

        bool is_index() const
        {
            return m_val.index() == 0;
        }
        bool is_key() const
        {
            return m_val.index() == 1;
        }

        std::size_t as_index() const noexcept
        {
            // use std::get_if to avoid exception
            return *std::get_if<size_type>(&m_val);
        }
        const std::string& as_key() const noexcept
        {
            return *std::get_if<string_type>(&m_val);
        }

        [[nodiscard]]
        underlying_type& get() noexcept
        {
            return m_val;
        }
        [[nodiscard]]
        const underlying_type& get() const noexcept
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

        friend bool operator==(const attribute_name& lhs, string_view_type rhs)
        {
            return lhs.m_name == rhs;
        }
        friend bool operator==(string_view_type lhs, const attribute_name& rhs)
        {
            return lhs == rhs.m_name;
        }

        [[nodiscard]]
        static bool validate(string_view_type name) noexcept
        {
            bool first = true;
            for(char_type c : name)
            {
                if(!detail::is_identifier_ch(c, first))
                    return false;
                if(first)
                    first = false;
            }

            return true;
        }

    private:
        string_type m_name;
    };


    class format_parse_context
    {
    public:
        using char_type = char;

    private:
    };

    template <std::integral T>
    class formatter
    {
    public:
        using value_type = T;
        using char_type = char;
        using iterator = std::basic_string_view<char_type>::iterator;
        using const_iterator = std::basic_string_view<char_type>::const_iterator;

        void parse(format_parse_context ctx)
        {

        }

        template <typename FormatContext>
        void format(T val, FormatContext& ctx)
        {
            char_type buf[100];
            auto result = std::to_chars(buf, buf + 100, 10);

            if(result.ec)
            {
                throw std::system_error(result.ec);
            }

            ctx.advance_to(std::copy(buf, result.ptr, ctx.out()));
        }

    private:
        int m_base;
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
        format_arg(underlying_type val)
            : m_val(std::move(val)) {}
        template <typename T, typename... Args>
        format_arg(std::in_place_type_t<T>, Args&&... args)
            : m_val(std::in_place_type<T>, std::forward<Args>(args)...) {}

        format_arg index(const indexing_value& idx) const
        {
            auto visitor = [&](auto&& v)->format_arg
            {
                using T = std::remove_cvref_t<decltype(v)>;

                if constexpr(string_like<T>)
                {
                    if(!idx.is_index())
                        invalid_index();
                    auto i = idx.as_index();

                    string_view_type sv(v);
                    if(i >= sv.size())
                        invalid_index();
                    return format_arg(sv[i]);
                }

                invalid_index();
            };
            return std::visit(visitor, m_val);
        }
        format_arg attribute(attribute_name name) const
        {
            auto visitor = [&](auto&& v)->format_arg
            {
                using T = std::remove_cvref_t<decltype(v)>;

                if constexpr(string_like<T>)
                {
                    string_view_type sv(v);
                    if(name == "length")
                    {
                        return format_arg(utf8::strlen(sv));
                    }
                    else if(name == "size")
                    {
                        return format_arg(sv.size());
                    }
                }

                invalid_attribute();
            };
            return std::visit(visitor, m_val);
        }

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
        friend const T& get(const format_arg& val)
        {
            return std::get<T>(val.m_val);
        }

        script::variable as_variable() const
        {
            using script::variable;

            auto visitor = [](auto&& v)->script::variable
            {
                using T = std::remove_cvref_t<decltype(v)>;
                
                if constexpr(std::is_same_v<T, char_type>)
                {
                    return string_type(1, v);
                }
                if constexpr(std::integral<T>)
                {
                    return static_cast<variable::int_type>(v);
                }
                else if constexpr(std::floating_point<T>)
                {
                    return static_cast<variable::float_type>(v);
                }
                else if constexpr(string_like<T>)
                {
                    return string_view_type(v);
                }
                else
                {
                    throw std::invalid_argument("invalid type");
                }
            };

            return std::visit(visitor, m_val);
        }

    private:
        underlying_type m_val;
    };

    class dynamic_format_arg_store
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;
        using size_type = std::size_t;

        dynamic_format_arg_store() = default;
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

        const format_arg& get(size_type i) const
        {
            if(i >= m_args.size())
                throw std::out_of_range("index out of range");
            return m_args[i];
        }
        const format_arg& get(string_view_type key) const
        {
            auto it = m_named_args.find(key);
            if(it == m_named_args.end())
                throw std::out_of_range("invalid named argument");
            return it->second;
        }
        const format_arg& get(const indexing_value& idx) const
        {
            auto visitor = [&](auto&& v)->const format_arg&
            {
                using std::is_same_v;
                using T = std::remove_cvref_t<decltype(v)>;

                if constexpr(is_same_v<T, size_type>)
                {
                    return m_args[v];
                }
                else if constexpr(is_same_v<T, string_type>)
                {
                    auto it = m_named_args.find(v);
                    if(it == m_named_args.end())
                        throw std::out_of_range("invalid named argument");
                    return it->second;
                }
            };

            return std::visit(visitor, idx.get());
        }

        const format_arg& operator[](size_type i) const
        {
            return get(i);
        }
        const format_arg& operator[](string_view_type key) const
        {
            return get(key);
        }
        const format_arg& operator[](const indexing_value& idx) const
        {
            return get(idx);
        }

        size_type size() const noexcept
        {
            return m_args.size();
        }
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


    template <typename OutputIt>
    class format_context
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;
        using iterator = OutputIt;

        iterator out()
        {
            return std::move(m_out);
        }
        void advance_to(iterator it)
        {
            m_out = it;
        }

    private:
        iterator m_out;
        
    };

    namespace detail
    {
        template <typename T, typename CharT = char>
        concept has_std_formatter = requires
        {
            typename std::formatter<T, CharT>;
        };

        struct format_spec
        {

        };
    }

    
}