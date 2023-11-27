#pragma once

#include <string>
#include <variant>
#include <vector>
#include <map>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <typeinfo>
#include "macros.hpp"
#include "type.hpp"
#include "container.hpp"
#include "utf/utf.hpp"
#include "error.hpp"
#include "locale.hpp"
#include "script/variable.hpp"


namespace papilio
{
    // forward declarations
    class indexing_value;
    class attribute_name;
    class format_arg;
    template <typename OutputIt>
    class basic_format_context;
    class dynamic_format_context;
    class format_spec_parse_context;

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

    class indexing_value
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;
        using index_type = ssize_t;
        using underlying_type = std::variant<
            index_type,
            slice,
            utf::string_container
        >;

        indexing_value() = delete;
        indexing_value(const indexing_value&) = default;
        indexing_value(indexing_value&&) = default;
        indexing_value(index_type index)
            : m_val(index) {}
        indexing_value(slice s)
            : m_val(s) {}
        indexing_value(utf::string_container key)
            : m_val(std::move(key)) {}
        template <string_like String>
        indexing_value(String&& key)
            : m_val(std::in_place_type<utf::string_container>, std::forward<String>(key)) {}
        template <string_like String>
        indexing_value(independent_t, String&& key)
            : m_val(std::in_place_type<utf::string_container>, independent, std::forward<String>(key)) {}

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
            return holds<utf::string_container>();
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
        const utf::string_container& as_key() const noexcept
        {
            return *std::get_if<utf::string_container>(&m_val);
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
        template <typename... Args> requires(std::constructible_from<utf::string_container, Args...>)
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
        const utf::string_container& name() const noexcept
        {
            return m_name;
        }
        [[nodiscard]]
        operator string_view_type() const noexcept
        {
            return static_cast<string_view_type>(m_name);
        }

        [[nodiscard]]
        static bool validate(utf::string_ref name) noexcept;

    private:
        utf::string_container m_name;
    };
    class invalid_attribute : public std::invalid_argument
    {
    public:
        invalid_attribute(attribute_name attr)
            : invalid_argument("invalid attribute name \"" + attr.name().str() + '\"'),
            m_attr(independent, attr.name()) {}

        [[nodiscard]]
        const attribute_name& attr() const noexcept
        {
            return m_attr;
        }

    private:
        attribute_name m_attr;
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

        template <typename T>
        concept has_ostream_support_helper = requires(std::ostream os, const T& val)
        {
            os << val;
        };

        template <typename T>
        struct formatter_selector_helper
        {
            using type = std::remove_const_t<T>;
        };

        template <std::integral T> requires(!char_like<T> && !std::is_same_v<T, bool>)
        struct formatter_selector_helper<T>
        {
            using type = best_int_type_t<T>;
        };
        template <>
        struct formatter_selector_helper<bool>
        {
            using type = bool;
        };

        template <char_like T>
        struct formatter_selector_helper<T>
        {
            using type = utf::codepoint;
        };

        template <typename T> requires(!char_like<T>)
        struct formatter_selector_helper<T*>
        {
            using type = const T*;
        };
        template <typename T> requires(!char_like<T>)
        struct formatter_selector_helper<const T*>
        {
            using type = const T*;
        };

        template <std::floating_point T>
        struct formatter_selector_helper<T>
        {
            using type = T;
        };

        template <string_like T>
        struct formatter_selector_helper<T>
        {
            using type = utf::string_container;
        };
    }

    template <typename T>
    class formatter;

    template <typename T>
    class formatter_traits
    {
    public:
        using type = detail::formatter_selector_helper<T>::type;
        using formatter_type = formatter<type>;

        template <typename Context = dynamic_format_context>
        [[nodiscard]]
        static constexpr bool has_formatter() noexcept;

        // TODO: rename
        template <typename Context>
        static void format(format_spec_parse_context& spec, const type& val, Context& ctx);
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
        static decltype(auto) index_handler(U&& object, const utf::string_container& str)
        {
            return index_handler(std::forward<U>(object), std::string_view(str));
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
        class handle_impl_base
        {
        public:
            handle_impl_base() = default;
            handle_impl_base(const handle_impl_base&) = delete;

            virtual ~handle_impl_base() = default;

            virtual format_arg index(const indexing_value& idx) const = 0;
            virtual format_arg attribute(const attribute_name& attr) const = 0;

            virtual void format(format_spec_parse_context& spec, dynamic_format_context& ctx) const = 0;

            virtual void reset(handle_impl_base* mem) const = 0;
        };

        template <typename T>
        class handle_impl final : public handle_impl_base
        {
        public:
            handle_impl(const T& val) noexcept
                : m_ptr(&val) {}

            format_arg index(const indexing_value& idx) const override;
            format_arg attribute(const attribute_name& attr) const override;

            void format(format_spec_parse_context& spec, dynamic_format_context& ctx) const override;

            void reset(handle_impl_base* mem) const override
            {
                new(mem) handle_impl(*m_ptr);
            }

        private:
            const T* m_ptr;
        };
        // used for calculating storage space
        template <>
        class handle_impl<void> final : public handle_impl_base
        {
        public:
            handle_impl() = delete;

        private:
            const void* m_ptr;
        };

        template <typename T>
        concept integral_type =
            std::integral<T> &&
            !char_like<T>;

        template <typename T>
        concept use_handle =
            !std::is_same_v<T, utf::codepoint> &&
            !char_like<T> &&
            !integral_type<T> &&
            !std::floating_point<T> &&
            !std::is_same_v<T, utf::string_container> &&
            !string_like<T>;
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
            handle() noexcept
                : m_storage{}, m_has_value(false) {}
            handle(const handle& other) noexcept
                : handle()
            {
                copy(other);
            }
            template <typename T>
            handle(const T& val)
                : handle()
            {
                construct<T>(val);
            }

            handle& operator=(const handle& rhs) noexcept
            {
                if(this == &rhs)
                    return *this;
                destroy();
                copy(rhs);
                return *this;
            }

            format_arg index(const indexing_value& idx) const
            {
                return ptr()->index(idx);
            }
            format_arg attribute(const attribute_name& attr) const
            {
                return ptr()->attribute(attr);
            }

            void format(format_spec_parse_context& spec, dynamic_format_context& ctx) const
            {
                ptr()->format(spec, ctx);
            }

        private:
            char m_storage[sizeof(detail::handle_impl<void>)]{};
            bool m_has_value = false;

            detail::handle_impl_base* ptr() noexcept
            {
                return reinterpret_cast<detail::handle_impl_base*>(m_storage);
            }
            const detail::handle_impl_base* ptr() const noexcept
            {
                return reinterpret_cast<const detail::handle_impl_base*>(m_storage);
            }

            template <typename T>
            void construct(const T& val) noexcept
            {
                static_assert(sizeof(detail::handle_impl<T>) <= sizeof(m_storage));
                new(ptr()) detail::handle_impl<T>(val);
                m_has_value = true;
            }
            void copy(const handle& other) noexcept;
            void destroy() noexcept;
        };

        using underlying_type = std::variant<
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
            utf::string_container,
            const void*,
            handle
        >;

        format_arg() noexcept
            : m_val() {}
        format_arg(const format_arg&) noexcept = default;
        format_arg(format_arg&&) noexcept = default;
        format_arg(bool val) noexcept
            : m_val(std::in_place_type<bool>, val) {}
        format_arg(utf::codepoint cp) noexcept
            : m_val(std::in_place_type<utf::codepoint>, cp) {}
        template <char_like Char>
        format_arg(Char ch) noexcept
            : m_val(std::in_place_type<utf::codepoint>, char32_t(ch)) {}
        template <detail::integral_type Integral>
        format_arg(Integral val) noexcept
            : m_val(static_cast<detail::best_int_type_t<Integral>>(val)) {}
        template <std::floating_point Float>
        format_arg(Float val) noexcept
            : m_val(val) {}
        format_arg(utf::string_container str) noexcept
            : m_val(std::in_place_type<utf::string_container>, std::move(str)) {}
        template <string_like String>
        format_arg(String&& str)
            : m_val(std::in_place_type<utf::string_container>, std::forward<String>(str)) {}
        template <string_like String>
        format_arg(independent_t, String&& str)
            : m_val(std::in_place_type<utf::string_container>, independent, std::forward<String>(str)) {}
        template <typename T, typename... Args>
        format_arg(std::in_place_type_t<T>, Args&&... args)
            : m_val(std::in_place_type<T>, std::forward<Args>(args)...) {}
        template <typename T> requires(!char_like<T>)
        format_arg(const T* ptr) noexcept
            : m_val(std::in_place_type<const void*>, ptr) {}
        format_arg(std::nullptr_t) noexcept
            : m_val(std::in_place_type<const void*>, nullptr) {}
        template <detail::use_handle T>
        format_arg(const T& val) noexcept
            : m_val(std::in_place_type<handle>, val) {}

        format_arg& operator=(const format_arg&) = default;
        format_arg& operator=(format_arg&&) noexcept = default;

        [[nodiscard]]
        format_arg index(const indexing_value& idx) const;
        [[nodiscard]]
        format_arg attribute(const attribute_name& attr) const;

        template <typename T>
        [[nodiscard]]
        bool holds() const noexcept
        {
            return std::holds_alternative<T>(m_val);
        }
        [[nodiscard]]
        bool empty() const noexcept
        {
            return holds<std::monostate>();
        }

        template <typename T>
        [[nodiscard]]
        friend const auto& get(const format_arg& val)
        {
            if constexpr(char_like<T>)
                return std::get<utf::codepoint>(val.m_val);
            else if constexpr(std::integral<T>)
                return std::get<detail::best_int_type_t<T>>(val.m_val);
            else if constexpr(std::is_same_v<T, utf::string_container> || string_like<T>)
                return std::get<utf::string_container>(val.m_val);
            else
                return std::get<T>(val.m_val);
        }

        script::variable as_variable() const;

        template <typename Context>
        void format(format_spec_parse_context& spec, Context& ctx);

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
        using member_storage = small_vector<member_type, 2>;

        format_arg_access() noexcept : m_members() {}
        format_arg_access(const format_arg_access&) = delete;
        format_arg_access(format_arg_access&&) noexcept = default;
        format_arg_access(member_storage members)
            : m_members(std::move(members)) {}

        [[nodiscard]]
        format_arg access(format_arg arg) const;

        [[nodiscard]]
        bool empty() const noexcept
        {
            return m_members.empty();
        }

    private:
        member_storage m_members;
    };

    namespace detail
    {
        template <typename T>
        consteval std::size_t count_if_index_arg() noexcept
        {
            using type = std::remove_cvref_t<T>;
            if constexpr(!is_named_arg_v<type>)
                return 1;
            else
                return 0;
        }
        template <typename T>
        consteval std::size_t count_if_named_arg() noexcept
        {
            using type = std::remove_cvref_t<T>;
            if constexpr(is_named_arg_v<type>)
                return 1;
            else
                return 0;
        }

        template <typename... Ts>
        consteval std::size_t get_index_arg_count() noexcept
        {
            if constexpr(sizeof...(Ts) == 0)
                return 0;
            else
            {
                using tuple_type = std::tuple<Ts...>;

                auto helper = []<std::size_t... Is> (std::index_sequence<Is...>)
                {
                    return (count_if_index_arg<std::tuple_element_t<Is, tuple_type>>() + ...);
                };

                return helper(std::make_index_sequence<sizeof...(Ts)>());
            }
        }
        template <typename... Ts>
        consteval std::size_t get_named_arg_count() noexcept
        {
            if constexpr(sizeof...(Ts) == 0)
                return 0;
            else
            {
                using tuple_type = std::tuple<Ts...>;

                auto helper = []<std::size_t... Is> (std::index_sequence<Is...>)
                {
                    return (count_if_named_arg<std::tuple_element_t<Is, tuple_type>>() + ...);
                };

                return helper(std::make_index_sequence<sizeof...(Ts)>());
            }
        }

        class format_args_base
        {
        public:
            using char_type = char;
            using string_type = std::basic_string<char_type>;
            using string_view_type = std::basic_string_view<char_type>;
            using size_type = std::size_t;

            [[nodiscard]]
            virtual const format_arg& get(size_type i) const = 0;
            [[nodiscard]]
            virtual const format_arg& get(string_view_type key) const = 0;
            [[nodiscard]]
            virtual const format_arg& get(const indexing_value& idx) const;

            bool check(size_type i) const noexcept;
            virtual bool check(string_view_type key) const noexcept = 0;
            bool check(const indexing_value& idx) const noexcept;

            virtual size_type size() const noexcept = 0;
            virtual size_type named_size() const noexcept = 0;

            [[nodiscard]]
            const format_arg& operator[](const indexing_value& idx) const
            {
                return get(idx);
            }

        protected:
            [[noreturn]]
            static void raise_index_out_of_range();
            [[noreturn]]
            static void raise_invalid_named_argument();
        };
    }

    template <std::size_t ArgumentCount, std::size_t NamedArgumentCount>
    class static_format_arg_store final : public detail::format_args_base
    {
    public:
        template <typename... Args>
        static_format_arg_store(Args&&... args)
        {
            construct(std::forward<Args>(args)...);
        }

        const format_arg& get(size_type i) const override
        {
            if(i >= m_args.size())
                raise_index_out_of_range();
            return m_args[i];
        }
        const format_arg& get(string_view_type k) const override
        {
            auto it = m_named_args.find(k);
            if(it == m_named_args.end())
                raise_invalid_named_argument();
            return it->second;
        }
        using format_args_base::get;

        bool check(string_view_type key) const noexcept
        {
            return m_named_args.contains(key);
        }
        using format_args_base::check;

        size_type size() const noexcept override
        {
            PAPILIO_ASSERT(m_args.size() == ArgumentCount);
            return ArgumentCount;
        }
        size_type named_size() const noexcept override
        {
            PAPILIO_ASSERT(m_named_args.size() == NamedArgumentCount);
            return NamedArgumentCount;
        }

    private:
        template <typename... Args>
        void construct(Args&&... args)
        {
            static_assert(
                detail::get_index_arg_count<Args...>() == ArgumentCount,
                "invalid argument count"
            );
            static_assert(
                detail::get_named_arg_count<Args...>() == NamedArgumentCount,
                "invalid named argument count"
            );

            (push(std::forward<Args>(args)), ...);
        }

        fixed_vector<format_arg, ArgumentCount> m_args;
        fixed_flat_map<std::string_view, format_arg, NamedArgumentCount> m_named_args;

        template <typename T>
        void push(T&& val) requires(!is_named_arg_v<T>)
        {
            m_args.emplace_back(std::forward<T>(val));
        }
        template <typename T>
        void push(named_arg<T> na)
        {
            m_named_args.insert_or_assign(
                na.name,
                na.value
            );
        }
    };

    class mutable_format_args final : public detail::format_args_base
    {
    public:
        mutable_format_args() = default;
        mutable_format_args(const mutable_format_args&) = delete;
        mutable_format_args(mutable_format_args&&) = default;
        template <typename... Args>
        mutable_format_args(Args&&... args)
        {
            push(std::forward<Args>(args)...);
        }

        template <typename... Args>
        void push(Args&&... args)
        {
            auto helper = [&]<typename T> (T&& arg)
            {
                if constexpr(requires() { typename T::named_arg_tag; })
                {
                    m_named_args.emplace(std::make_pair(arg.name, std::forward<T>(arg).value));
                }
                else
                {
                    m_args.emplace_back(std::forward<T>(arg));
                }
            };

            (helper(std::forward<Args>(args)), ...);
        }

        const format_arg& get(size_type i) const override;
        const format_arg& get(string_view_type key) const override;
        using format_args_base::get;

        bool check(string_view_type key) const noexcept override;
        using format_args_base::check;

        [[nodiscard]]
        size_type size() const noexcept override
        {
            return m_args.size();
        }
        [[nodiscard]]
        size_type named_size() const noexcept override
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

    // type-erased argument store
    class dynamic_format_args final : public detail::format_args_base
    {
    public:
        dynamic_format_args() = delete;
        dynamic_format_args(const dynamic_format_args&) noexcept = default;
        dynamic_format_args(const detail::format_args_base& store) noexcept
            : m_ref(&store)
        {
            PAPILIO_ASSERT(m_ref != this); // circular reference
        }

        const format_arg& get(size_type i) const override
        {
            return m_ref->get(i);
        }
        const format_arg& get(string_view_type k) const override
        {
            return m_ref->get(k);
        }
        using format_args_base::get;

        size_type size() const noexcept override
        {
            return m_ref->size();
        }
        size_type named_size() const noexcept override
        {
            return m_ref->named_size();
        }

        bool check(string_view_type k) const noexcept override
        {
            return m_ref->check(k);
        }
        using format_args_base::check;

        [[nodiscard]]
        const detail::format_args_base& to_underlying() const noexcept
        {
            return *m_ref;
        }

    private:
        const detail::format_args_base* m_ref;
    };

    template <typename T>
    concept format_arg_store = std::is_base_of_v<detail::format_args_base, T>;

    template <typename... Args>
    auto make_format_args(Args&&... args)
    {
        static_assert(
            (!format_arg_store<Args> && ...),
            "cannot use format_arg_store as format argument"
        );

        using store_type = static_format_arg_store<
            detail::get_index_arg_count<Args...>(),
            detail::get_named_arg_count<Args...>()
        >;
        return store_type(std::forward<Args>(args)...);
    }

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
        format_parse_context(string_view_type str, const dynamic_format_args& store);

        const dynamic_format_args& get_store() const noexcept
        {
            return m_store;
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
        dynamic_format_args m_store;

        void enable_manual_indexing() const noexcept
        {
            m_manual_indexing = true;
        }
    };

    template <typename OutputIt>
    class basic_format_context
    {
    public:
        using char_type = char;
        using iterator = OutputIt;
        using store_type = dynamic_format_args;

        basic_format_context(iterator it, const store_type& store)
            : m_out(std::move(it)), m_store(store) {}
        basic_format_context(const std::locale& loc, iterator it, const store_type& store)
            : m_loc(loc), m_out(std::move(it)), m_store(store) {}

        iterator out()
        {
            return m_out;
        }
        void advance_to(iterator it)
        {
            m_out = std::move(it);
        }

        const store_type& get_store() const noexcept
        {
            return m_store;
        }
        std::locale getloc() const
        {
            return m_loc.get();
        }

        // internal API
        locale_ref getloc_ref() const noexcept
        {
            return m_loc;
        }

    private:
        iterator m_out;
        store_type m_store;
        locale_ref m_loc;
    };

    namespace detail
    {
        class dynamic_format_context_impl_base
        {
        public:
            using char_type = char;

            virtual void push_back(char_type ch) = 0;
            virtual const dynamic_format_args& get_store() const noexcept = 0;
            virtual locale_ref getloc_ref() const noexcept = 0;

            std::locale getloc() const
            {
                return getloc_ref().get();
            }
        };
        template <typename OutputIt>
        class dynamic_format_context_impl final : public dynamic_format_context_impl_base
        {
        public:
            using iterator = OutputIt;
            using store_type = dynamic_format_args;

            dynamic_format_context_impl(basic_format_context<OutputIt>& ctx) noexcept
                : m_out(ctx.out()), m_store(ctx.get_store()), m_loc(ctx.getloc_ref()) {}

            void push_back(char_type ch) override
            {
                *m_out = ch;
                ++m_out;
            }
            const dynamic_format_args& get_store() const noexcept override
            {
                return m_store;
            }
            locale_ref getloc_ref() const noexcept override
            {
                return m_loc;
            }
        private:
            iterator m_out;
            dynamic_format_args m_store;
            locale_ref m_loc;
        };
    }

    class dynamic_format_context
    {
    public:
        using char_type = char;
        using iterator = std::back_insert_iterator<dynamic_format_context>;
        using store_type = dynamic_format_args;
        using value_type = char_type;

        dynamic_format_context() = delete;
        template <typename OutputIt>
        dynamic_format_context(basic_format_context<OutputIt>& ctx)
            : m_impl(std::make_unique<detail::dynamic_format_context_impl<OutputIt>>(ctx)) {}

        iterator out()
        {
            return std::back_inserter(*this);
        }
        void advance_to(iterator) {}

        const store_type& get_store() const noexcept
        {
            return m_impl->get_store();
        }

        void push_back(char_type ch)
        {
            m_impl->push_back(ch);
        }

        std::locale getloc() const
        {
            return m_impl->getloc();
        }

        // internal API
        locale_ref getloc_ref() const noexcept
        {
            return m_impl->getloc_ref();
        }

    private:
        std::unique_ptr<detail::dynamic_format_context_impl_base> m_impl;
    };

    template <typename Context>
    class format_context_traits
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;
        using context_type = Context;
        using iterator = typename Context::iterator;
        using store_type = typename Context::store_type;

        format_context_traits() = delete;

        [[nodiscard]]
        static iterator out(context_type& ctx)
        {
            return ctx.out();
        }
        static void advance_to(context_type& ctx, iterator it)
        {
            ctx.advance_to(std::move(it));
        }

        [[nodiscard]]
        static const store_type& get_store(context_type& ctx) noexcept
        {
            return ctx.get_store();
        }

        template <typename InputIt>
        static void append(context_type& ctx, InputIt begin, InputIt end)
        {
            advance_to(ctx, std::copy(begin, end, out(ctx)));
        }
        static void append(context_type& ctx, string_view_type str)
        {
            append(ctx, str.begin(), str.end());
        }
        template <char_like Char>
        static void append(context_type& ctx, Char ch, std::size_t count = 1)
        {
            if constexpr(std::is_same_v<Char, char> || std::is_same_v<Char, char8_t>)
            {
                advance_to(ctx, std::fill_n(out(ctx), count, static_cast<char>(ch)));
            }
            else
            {
                utf::codepoint cp(static_cast<char32_t>(ch));
                append(ctx, cp, count);
            }
        }
        static void append(context_type& ctx, utf::codepoint cp, std::size_t count = 1)
        {
            for(std::size_t i = 0; i < count; ++i)
                append(ctx, string_view_type(cp));
        }
    };
}

#include "core.inl"
