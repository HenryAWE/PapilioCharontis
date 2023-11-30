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
#include "utility.hpp"
#include "container.hpp"
#include "utf/utf.hpp"
#include "error.hpp"
#include "locale.hpp"
#include "access.hpp"
#include "script/variable.hpp"


namespace papilio
{
    // forward declarations
    class format_arg;

    class format_parse_context;

    template <typename OutputIt>
    class basic_format_context;

    class dynamic_format_context;

    using format_context = basic_format_context<
        std::back_insert_iterator<std::string>
    >;

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
        static constexpr bool has_formatter() noexcept
        {
            return requires(formatter<T> f, format_parse_context & parse_ctx, const type & val, Context & out_ctx)
            {
                typename formatter<T>;
                f.parse(parse_ctx);
                f.format(val, out_ctx);
            };
        }

        static void parse(format_parse_context& ctx)
        {

        }

        template <typename Context>
        static void format(const type& val, Context& ctx)
        {

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

            virtual void parse(format_parse_context& ctx) const = 0;

            virtual void format(dynamic_format_context& ctx) const = 0;

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

            void parse(format_parse_context& ctx) const override
            {
                // TODO
            }

            void format(dynamic_format_context& ctx) const override
            {
                // TODO
            }

            void reset(handle_impl_base* mem) const override
            {
                new(mem) handle_impl(*m_ptr);
            }

        private:
            const T* m_ptr;
        };
        // Used to calculate storage space
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

            void parse(format_parse_context& ctx) const
            {
                // TODO
            }

            void format(dynamic_format_context& ctx) const
            {
                // TODO
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
            void copy(const handle& other) noexcept
            {
                other.ptr()->reset(ptr());
                m_has_value = true;
            }
            void destroy() noexcept
            {
                ptr()->~handle_impl_base();
                std::memset(m_storage, 0, sizeof(m_storage));
                m_has_value = false;
            }
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

        void parse(format_parse_context& ctx);

        template <typename Context>
        void format(Context& ctx);

    private:
        underlying_type m_val;
    };

    // access members of format argument
    class chained_access
    {
    public:
        using member_type = std::variant<
            indexing_value,
            attribute_name
        >;
        using member_storage = small_vector<member_type, 2>;

        chained_access() noexcept : m_members() {}
        chained_access(const chained_access&) = delete;
        chained_access(chained_access&&) noexcept = default;
        chained_access(member_storage members)
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
        consteval std::size_t get_indexed_arg_count() noexcept
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

            virtual size_type indexed_size() const noexcept = 0;
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
    class static_format_args final : public detail::format_args_base
    {
        using base = detail::format_args_base;
    public:
        template <typename... Args>
        static_format_args(Args&&... args)
        {
            construct(std::forward<Args>(args)...);
        }

        const format_arg& get(size_type i) const override
        {
            if(i >= m_indexed_args.size())
                raise_index_out_of_range();
            return m_indexed_args[i];
        }
        const format_arg& get(string_view_type k) const override
        {
            auto it = m_named_args.find(k);
            if(it == m_named_args.end())
                raise_invalid_named_argument();
            return it->second;
        }
        using base::get;

        bool check(string_view_type key) const noexcept
        {
            return m_named_args.contains(key);
        }
        using base::check;

        size_type indexed_size() const noexcept override
        {
            PAPILIO_ASSERT(m_indexed_args.size() == ArgumentCount);
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
                detail::get_indexed_arg_count<Args...>() == ArgumentCount,
                "invalid indexed argument count"
            );
            static_assert(
                detail::get_named_arg_count<Args...>() == NamedArgumentCount,
                "invalid named argument count"
            );

            (push(std::forward<Args>(args)), ...);
        }

        fixed_vector<format_arg, ArgumentCount> m_indexed_args;
        fixed_flat_map<std::string_view, format_arg, NamedArgumentCount> m_named_args;

        template <typename T>
        void push(T&& val) requires(!is_named_arg_v<T>)
        {
            m_indexed_args.emplace_back(std::forward<T>(val));
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
        size_type indexed_size() const noexcept override
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

    // Type-erased format arguments.
    class dynamic_format_args final : public detail::format_args_base
    {
        using base = detail::format_args_base;
    public:
        dynamic_format_args() = delete;
        constexpr dynamic_format_args(const dynamic_format_args&) noexcept = default;
        constexpr dynamic_format_args(const detail::format_args_base& args) noexcept
            : m_ptr(&args)
        {
            PAPILIO_ASSERT(m_ptr != this); // avoid circular reference
        }

        const format_arg& get(size_type i) const override
        {
            return m_ptr->get(i);
        }
        const format_arg& get(string_view_type k) const override
        {
            return m_ptr->get(k);
        }
        using format_args_base::get;

        size_type indexed_size() const noexcept override
        {
            return m_ptr->indexed_size();
        }
        size_type named_size() const noexcept override
        {
            return m_ptr->named_size();
        }

        bool check(string_view_type k) const noexcept override
        {
            return m_ptr->check(k);
        }
        using format_args_base::check;

        // WARNING: This function does not perform any runtime checks!
        template <std::derived_from<base> T>
        [[nodiscard]]
        constexpr const T& cast_to() const noexcept
        {
            PAPILIO_ASSERT(dynamic_cast<const T*>(m_ptr));
            return *static_cast<const T*>(m_ptr);
        }

    private:
        const detail::format_args_base* m_ptr;
    };

    template <typename T>
    concept format_args = std::is_base_of_v<detail::format_args_base, T>;

    template <typename... Args>
    auto make_format_args(Args&&... args)
    {
        static_assert(
            (!format_args<Args> && ...),
            "cannot use format_args as format argument"
        );

        using result_type = static_format_args<
            detail::get_indexed_arg_count<Args...>(),
            detail::get_named_arg_count<Args...>()
        >;
        return result_type(std::forward<Args>(args)...);
    }

    class format_parse_context
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;
        using string_ref_type = utf::basic_string_ref<char_type>;
        using iterator = string_ref_type::const_iterator;

        format_parse_context() = delete;
        format_parse_context(const format_parse_context&) = delete;
        format_parse_context(string_ref_type str, dynamic_format_args args) noexcept
            : m_ref(str), m_args(args)
        {
            m_it = m_ref.begin();
        }

        const dynamic_format_args& get_args() const noexcept
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
            return get_args().check(i);
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
        string_ref_type m_ref;
        iterator m_it;
        dynamic_format_args m_args;
        std::size_t m_default_arg_idx = 0;
        mutable bool m_manual_indexing = false;

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
        using format_args_type = dynamic_format_args;

        basic_format_context(iterator it, dynamic_format_args args)
            : m_out(std::move(it)), m_args(args) {}
        basic_format_context(const std::locale& loc, iterator it, dynamic_format_args args)
            : m_loc(loc), m_out(std::move(it)), m_args(args) {}

        [[nodiscard]]
        iterator out()
        {
            return m_out;
        }

        void advance_to(iterator it)
        {
            m_out = std::move(it);
        }

        [[nodiscard]]
        const dynamic_format_args& get_args() const noexcept
        {
            return m_args;
        }

        [[nodiscard]]
        std::locale getloc() const
        {
            return m_loc.get();
        }

        [[nodiscard]]
        locale_ref getloc_ref() const noexcept
        {
            return m_loc;
        }

    private:
        iterator m_out;
        dynamic_format_args m_args;
        locale_ref m_loc;
    };

    class dynamic_format_context
    {
    public:
        using char_type = char;
        using format_args_type = dynamic_format_args;
        using value_type = char_type;

        class iterator
        {
        public:
            explicit iterator(dynamic_format_context& ctx)
                : m_ctx(&ctx) {}

            iterator& operator=(char_type val)
            {
                m_ctx->m_handle->write(val);
                return *this;
            }

            iterator& operator*() { return *this; }
            iterator& operator++() { return *this; }
            iterator& operator++(int) { return *this; }

        private:
            dynamic_format_context* m_ctx;
        };

    private:
        class handle_impl_base
        {
        public:
            virtual ~handle_impl_base() = default;

            virtual void write(char_type ch) = 0;

            virtual const dynamic_format_args& get_args() const noexcept = 0;

            virtual locale_ref getloc_ref() const noexcept = 0;

            virtual std::size_t size_bytes() const noexcept = 0;
            // Copy handle data to uninitialized memory
            virtual void copy(handle_impl_base* mem) const = 0;
        };

        template <typename OutputIt>
        class handle_impl final : public handle_impl_base
        {
        public:
            handle_impl(OutputIt it, dynamic_format_args args, locale_ref loc) noexcept
                : m_out(std::move(it)), m_args(args), m_loc(loc) {}

            void write(char_type ch) override
            {
                *m_out = ch;
                ++m_out;
            }

            const dynamic_format_args& get_args() const noexcept override
            {
                return m_args;
            }

            locale_ref getloc_ref() const noexcept override
            {
                return m_loc;
            }

            std::size_t size_bytes() const noexcept override
            {
                return sizeof(*this);
            }
            void copy(handle_impl_base* mem) const override
            {
                new(mem) handle_impl(m_out, m_args, m_loc);
            }

        private:
            OutputIt m_out;
            dynamic_format_args m_args;
            locale_ref m_loc;
        };

        class handle
        {
        public:
            handle() = delete;
            handle(const handle& other)
            {
                other.copy(*this);
            }
            template <typename OutputIt>
            handle(basic_format_context<OutputIt>& ctx)
            {
                construct<OutputIt>(ctx);
            }

            ~handle()
            {
                destroy();
                if(m_use_ptr)
                    deallocate();
            }

            handle_impl_base* operator->() const
            {
                return get();
            }

        private:
            static constexpr std::size_t storage_size = 48;

            union handle_data_t
            {
                handle_impl_base* ptr;
                mutable static_storage<storage_size> storage;
            };

            handle_data_t m_data;
            bool m_use_ptr = false;

            handle_impl_base* get() const noexcept
            {
                if(m_use_ptr)
                    return m_data.ptr;
                else
                    return reinterpret_cast<handle_impl_base*>(m_data.storage.data());
            }

            void allocate(std::size_t mem_size)
            {
                m_use_ptr = true;
                m_data.ptr = reinterpret_cast<handle_impl_base*>(new std::byte[mem_size]);
            }

            void deallocate() noexcept
            {
                PAPILIO_ASSERT(m_use_ptr);
                delete[] reinterpret_cast<std::byte*>(m_data.ptr);
            }

            template <typename OutputIt>
            void construct(basic_format_context<OutputIt>& ctx)
            {
                if constexpr(sizeof(ctx) > storage_size)
                {
                    allocate(sizeof(handle_impl<OutputIt>));
                    m_data.ptr = new handle_impl<OutputIt>(
                        ctx.out(), ctx.get_args(), ctx.getloc_ref()
                    );
                }
                else
                {
                    new(m_data.storage.data()) handle_impl<OutputIt>(
                        ctx.out(), ctx.get_args(), ctx.getloc_ref()
                    );
                }
            }

            // Copy this handle to another uninitialized handle
            void copy(handle& dst) const
            {
                PAPILIO_ASSERT(!dst.m_use_ptr);

                handle_impl_base* ptr = get();
                if(std::size_t size = ptr->size_bytes(); size > storage_size)
                    dst.allocate(size);
                ptr->copy(dst.get());
            }

            void destroy() noexcept
            {
                get()->~handle_impl_base();
            }
        };

    public:
        dynamic_format_context() = delete;
        dynamic_format_context(const dynamic_format_context&) = default;
        template <typename OutputIt>
        dynamic_format_context(basic_format_context<OutputIt>& ctx)
            : m_handle(ctx) {}

        [[nodiscard]]
        iterator out()
        {
            return iterator(*this);
        }
        void advance_to(iterator)
        {
           // no effect
        }

        [[nodiscard]]
        const dynamic_format_args& get_args() const noexcept
        {
            return m_handle->get_args();
        }

        void push_back(char_type ch)
        {
            m_handle->write(ch);
        }

        [[nodiscard]]
        std::locale getloc() const
        {
            return getloc_ref();
        }

        [[nodiscard]]
        locale_ref getloc_ref() const noexcept
        {
            return m_handle->getloc_ref();
        }

    private:
        handle m_handle;
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
        using format_args_type = typename Context::format_args_type;

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
        static const format_args_type& get_args(context_type& ctx) noexcept
        {
            return ctx.get_args();
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
