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
    // forward declarations
    class slice;
    class indexing_value;
    class attribute_name;
    class format_arg;

    template <typename T, typename CharT>
    concept basic_string_like =
        std::is_same_v<std::decay_t<T>, CharT*> ||
        std::is_same_v<std::decay_t<T>, const CharT*> ||
        std::is_same_v<T, std::basic_string<CharT>> ||
        std::is_same_v<T, std::basic_string_view<CharT>>;
    template <typename T>
    concept string_like = basic_string_like<T, char>;

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

        template <typename T>
        struct is_char : std::false_type {};
        template <>
        struct is_char<char> : std::true_type {};
        template <>
        struct is_char<wchar_t> : std::true_type {};
        template <>
        struct is_char<char16_t> : std::true_type {};
        template <>
        struct is_char<char32_t> : std::true_type {};
        template <>
        struct is_char<char8_t> : std::true_type {};

        template <typename T>
        inline constexpr bool is_char_v = is_char<T>::value;
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
                string_container
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
            variable(string_container str)
                : m_var(std::move(str)) {}
            template <string_like String>
            variable(String&& str)
                : m_var(std::in_place_type<string_container>, std::forward<String>(str)) {}
            template <string_like String>
            variable(independent_t, String&& str)
                : m_var(std::in_place_type<string_container>, independent, std::forward<String>(str)) {}

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
            variable& operator=(string_container str)
            {
                m_var.emplace<string_container>(std::move(str));
                return *this;
            }
            template <string_like String>
            variable& operator=(String&& str)
            {
                m_var.emplace<string_container>(std::forward<String>(str));
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
                        if constexpr(is_same_v<U, string_container>)
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
                        if constexpr(is_same_v<U, string_container>)
                        {
                            invalid_conversion();
                        }
                        else // ind_type and float_type
                        {
                            return static_cast<T>(v);
                        }
                    }
                    else if constexpr(is_same_v<T, string_container> || string_like<T>)
                    {
                        if constexpr(is_same_v<U, string_container>)
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
                std::is_same_v<T, string_container>;
        };
        template <typename T>
        inline constexpr bool is_variable_type_v = is_variable_type<T>::value;
    }

    enum class format_align : std::uint8_t
    {
        default_align = 0,
        left,
        middle,
        right
    };
    enum class format_sign
    {
        default_sign = 0,
        positive,
        negative,
        space
    };

    class format_spec_parse_context;

    namespace detail
    {
        struct std_format_spec
        {
            using char_type = char;

            char_type fill = ' ';
            format_align align = format_align::default_align;
            format_sign sign = format_sign::default_sign;
            bool alternate_form = false; // specified by '#'
            bool fill_zero = false;
            std::size_t width = 0;
            bool use_locale = false;
            char_type type_char = '\0';
        };

        std_format_spec parse_std_format_spec(format_spec_parse_context& ctx);
    }

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
            string_container
        >;

        indexing_value() = delete;
        indexing_value(const indexing_value&) = default;
        indexing_value(indexing_value&&) = default;
        indexing_value(index_type index)
            : m_val(index) {}
        indexing_value(slice s)
            : m_val(s) {}
        indexing_value(string_container key)
            : m_val(std::move(key)) {}
        template <string_like String>
        indexing_value(String&& key)
            : m_val(std::in_place_type<string_container>, std::forward<String>(key)) {}
        template <string_like String>
        indexing_value(independent_t, String&& key)
            : m_val(std::in_place_type<string_container>, independent, std::forward<String>(key)) {}

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
            return holds<string_container>();
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
        const string_container& as_key() const noexcept
        {
            return *std::get_if<string_container>(&m_val);
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
        friend bool operator==(const attribute_name& lhs, const string_type& rhs) noexcept
        {
            return lhs.m_name == rhs;
        }
        [[nodiscard]]
        friend bool operator==(const string_type& lhs, const attribute_name& rhs) noexcept
        {
            return lhs == rhs.m_name;
        }
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
        friend bool operator==(const attribute_name& lhs, const char* rhs) noexcept
        {
            return lhs.m_name == rhs;
        }
        [[nodiscard]]
        friend bool operator==(const char* lhs, const attribute_name& rhs) noexcept
        {
            return lhs == rhs.m_name;
        }

        [[nodiscard]]
        const string_type& name() const noexcept
        {
            return m_name;
        }

        [[nodiscard]]
        static bool validate(string_view_type name) noexcept;

    private:
        string_type m_name;
    };
    class invalid_attribute : public std::invalid_argument
    {
    public:
        invalid_attribute(attribute_name attr)
            : invalid_argument("invalid attribute name \"" + attr.name() + '\"'),
            m_attr(std::move(attr)) {}

        [[nodiscard]]
        const attribute_name& attr() const noexcept
        {
            return m_attr;
        }

    private:
        attribute_name m_attr;
    };

    template <typename T>
    struct accessor {};

    template <typename T>
    class accessor_traits
    {
    public:
        using type = T;
        using accessor_type = accessor<type>;

        [[nodiscard]]
        static constexpr bool has_index() noexcept
        {
            return requires() { typename accessor_type::has_index; };
        }
        [[nodiscard]]
        static constexpr bool has_custom_index() noexcept
        {
            if constexpr(has_index())
            {
                return requires(T object, indexing_value::index_type i) { accessor_type::get(object, i); };
            }
            else
                return false;
        }

        [[nodiscard]]
        static constexpr bool has_key() noexcept
        {
            return requires() { typename accessor_type::has_key; };
        }
        [[nodiscard]]
        static constexpr bool has_custom_key() noexcept
        {
            if constexpr(has_key())
            {
                return requires(T object, indexing_value::string_view_type str) { accessor_type::get(object, str); };
            }
            else
                return false;
        }

        [[nodiscard]]
        static constexpr bool has_slice() noexcept
        {
            return requires() { typename accessor_type::has_slice; };
        }
        [[nodiscard]]
        static constexpr bool has_custom_slice() noexcept
        {
            if constexpr(has_slice())
            {
                return requires(T object, slice s) { accessor_type::get(object, s); };
            }
            else
                return false;
        }

        [[noreturn]]
        constexpr static void index_unavailable()
        {
            throw std::runtime_error("index unavailable");
        }
        [[noreturn]]
        constexpr static void key_unavailable()
        {
            throw std::runtime_error("key unavailable");
        }
        [[noreturn]]
        constexpr static void slice_unavailable()
        {
            throw std::runtime_error("slice unavailable");
        }

        template <typename U>
        static format_arg get_arg(U&& object, const indexing_value& idx);

        template <typename U>
        static decltype(auto) get(U&& object, indexing_value::index_type i)
        {
            return index_handler(std::forward<U>(object), i);
        }
        template <typename U>
        static decltype(auto) get(U&& object, indexing_value::string_view_type str)
        {
            return index_handler(std::forward<U>(object), str);
        }
        template <typename U>
        static decltype(auto) get(U&& object, slice s)
        {
            return index_handler(std::forward<U>(object), s);
        }

        template <typename U>
        static format_arg get_attr(U&& object, const attribute_name& attr);

    private:
        template <typename U>
        static decltype(auto) index_handler(U&& object, indexing_value::index_type i)
        {
            if constexpr(!has_index())
            {
                index_unavailable();
            }
            else if constexpr(has_custom_index())
            {
                return accessor_type::get(std::forward<U>(object), i);
            }
            else
            {
                if(i < 0)
                    throw std::runtime_error("reverse index unavailable");
                return object[static_cast<std::size_t>(i)];
            }
        }
        template <typename U>
        static decltype(auto) index_handler(U&& object, indexing_value::string_view_type str)
        {
            if constexpr(!has_key())
            {
                key_unavailable();
            }
            else if constexpr(has_custom_key())
            {
                using Accessor = accessor<T>;
                return Accessor::get(std::forward<U>(object), str);
            }
            else
            {
                return object[str];
            }
        }
        template <typename U>
        static decltype(auto) index_handler(U&& object, slice s)
        {
            if constexpr(!has_slice())
            {
                slice_unavailable();
            }
            else if constexpr(has_custom_slice())
            {
                using Accessor = accessor<T>;
                return Accessor::get(std::forward<U>(object), s);
            }
            else
            {
                return object[s];
            }
        }
    };

    namespace detail
    {
        template <std::integral Integral>
        struct best_int_type
        {
            using type = std::conditional_t<
                std::is_unsigned_v<Integral>,
                std::conditional_t<(sizeof(Integral) <= sizeof(unsigned int)), unsigned int, unsigned long long int>,
                std::conditional_t<(sizeof(Integral) <= sizeof(int)), int, long long int>
            >;
        };

        template <std::integral Integral>
        using best_int_type_t = best_int_type<Integral>::type;
    }

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
            utf8::codepoint,
            int,
            unsigned int,
            long long int,
            unsigned long long int,
            float,
            double,
            long double,
            string_container,
            handle
        >;

        format_arg()
            : m_val() {}
        format_arg(const format_arg&) = default;
        format_arg(format_arg&&) noexcept = default;
        format_arg(underlying_type val)
            : m_val(std::move(val)) {}
        format_arg(utf8::codepoint cp)
            : m_val(std::in_place_type<utf8::codepoint>, cp) {}
        template <typename Char>
        format_arg(Char ch) requires detail::is_char_v<Char>
            : m_val(std::in_place_type<utf8::codepoint>, char32_t(ch)) {}
        template <std::integral Integral> requires(!detail::is_char_v<Integral>)
        format_arg(Integral val)
            : m_val(static_cast<detail::best_int_type_t<Integral>>(val)) {}
        template <std::floating_point Float>
        format_arg(Float val)
            : m_val(val) {}
        template <string_like String>
        format_arg(String&& str)
            : m_val(std::in_place_type<string_container>, std::forward<String>(str)) {}
        template <string_like String>
        format_arg(independent_t, String&& str)
            : m_val(std::in_place_type<string_container>, independent, std::forward<String>(str)) {}
        template <typename T, typename... Args>
        format_arg(std::in_place_type_t<T>, Args&&... args)
            : m_val(std::in_place_type<T>, std::forward<Args>(args)...) {}

        format_arg& operator=(const format_arg&) = default;
        format_arg& operator=(format_arg&&) noexcept = default;

        [[nodiscard]]
        format_arg index(const indexing_value& idx) const;
        [[nodiscard]]
        format_arg attribute(attribute_name name) const;

        template <typename T>
        [[nodiscard]]
        friend const auto& get(const format_arg& val)
        {
#ifdef __GNUC__
            // handle std::size_t
            if constexpr(std::is_same_v<T, unsigned long>)
                return std::get<unsigned long long int>(val.m_val);
            else
                return std::get<T>(val.m_val);
#else
            return std::get<T>(val.m_val);
#endif
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
        format_arg_access(format_arg_access&&) noexcept = default;
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

    template <typename T>
    class formatter
    {
    public:
        static_assert(!sizeof(T), "You need to specialize formatter for this type");
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
        bool check(const indexing_value& idx) const noexcept;

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

    // WARNING: This class only holds the view of string and reference of arguments
    // The invoker needs to handle lifetimes manually
    class format_parse_context
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;
        using iterator = string_view_type::iterator;

        format_parse_context() = delete;
        format_parse_context(const format_parse_context&) = delete;
        format_parse_context(string_view_type str, const dynamic_format_arg_store& store);

        const dynamic_format_arg_store& get_store() const noexcept
        {
            return *m_store;
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
            return m_view.end();
        }

        std::size_t current_arg_id() const
        {
            if(m_manual_indexing)
                invalid_default_argument();
            return m_default_arg_idx;
        }
        std::size_t next_arg_id()
        {
            if(m_manual_indexing)
                invalid_default_argument();
            return ++m_default_arg_idx;
        }
        std::size_t check_arg_id(std::size_t i) const
        {
            enable_manual_indexing();
            return get_store().check(i);
        }

        [[nodiscard]]
        bool manual_indexing() const noexcept
        {
            return m_manual_indexing;
        }

        [[noreturn]]
        static void invalid_default_argument()
        {
            throw std::runtime_error("no default argument after an explicit argument");
        }

    private:
        string_view_type m_view;
        iterator m_it;
        mutable bool m_manual_indexing = false;
        std::size_t m_default_arg_idx = 0;
        const dynamic_format_arg_store* m_store;

        void enable_manual_indexing() const noexcept
        {
            m_manual_indexing = true;
        }
    };

    class format_context
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;

        format_context() = default;
        format_context(const format_context&) = delete;

        void append(const string_type& str)
        {
            m_result.append(str);
        }
        void append(string_view_type str)
        {
            m_result.append(str);
        }
        void append(const char_type* str)
        {
            m_result.append(str);
        }
        void append(char_type ch, std::size_t count = 1)
        {
            m_result.append(count, ch);
        }
        template <typename InputIt>
        void append(InputIt begin, InputIt end)
        {
            m_result.append(begin, end);
        }

        // compatible with std::back_inserter
        void push_back(char_type ch)
        {
            m_result.push_back(ch);
        }

        [[nodiscard]]
        const string_type& str() const& noexcept
        {
            return m_result;
        }
        [[nodiscard]]
        string_type str() && noexcept
        {
            return m_result;
        }

    private:
        string_type m_result;
    };

    // WARNING: This class only holds the view of string and reference of arguments
    // The invoker needs to handle lifetimes manually
    class format_spec_parse_context
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;
        using iterator = string_view_type::const_iterator;
        using reverse_iterator = string_view_type::const_reverse_iterator;

        format_spec_parse_context() = delete;
        format_spec_parse_context(const format_spec_parse_context&) = delete;
        constexpr format_spec_parse_context(string_view_type str) noexcept
            : m_spec_str(str), m_store(nullptr) {}
        constexpr format_spec_parse_context(string_view_type str, const dynamic_format_arg_store& store) noexcept
            : m_spec_str(str), m_store(&store) {}

        [[nodiscard]]
        constexpr iterator begin() const noexcept
        {
            return m_spec_str.begin();
        }
        [[nodiscard]]
        constexpr iterator end() const noexcept
        {
            return m_spec_str.end();
        }
        [[nodiscard]]
        constexpr reverse_iterator rbegin() const noexcept
        {
            return m_spec_str.rbegin();
        }
        [[nodiscard]]
        constexpr reverse_iterator rend() const noexcept
        {
            return m_spec_str.rend();
        }

        [[nodiscard]]
        constexpr operator string_view_type() const noexcept
        {
            return m_spec_str;
        }

        [[nodiscard]]
        constexpr bool has_store() const noexcept
        {
            return m_store != nullptr;
        }
        [[nodiscard]]
        constexpr const dynamic_format_arg_store& get_store() const noexcept
        {
            return *m_store;
        }

    private:
        string_view_type m_spec_str;
        const dynamic_format_arg_store* m_store;
    };
}

#include "core.inl"
