#pragma once

#include <cassert>
#include <utility>
#include <type_traits>
#include <typeinfo>
#include <string>
#include <sstream>
#include <vector>
#include <variant>
#include <map>
#include <memory>


namespace papilio::script
{
    struct nullvar_t {};
    constexpr nullvar_t nullvar{};

    namespace helper
    {
        struct less
        {
            constexpr bool operator()(std::partial_ordering order) const noexcept
            {
                return order < 0;
            }
        };
        struct less_equal
        {
            constexpr bool operator()(std::partial_ordering order) const noexcept
            {
                return order <= 0;
            }
        };
        struct greater
        {
            constexpr bool operator()(std::partial_ordering order) const noexcept
            {
                return order > 0;
            }
        };
        struct greater_equal
        {
            constexpr bool operator()(std::partial_ordering order) const noexcept
            {
                return order >= 0;
            }
        };
        struct equal
        {
            constexpr bool operator()(std::partial_ordering order) const noexcept
            {
                return order == 0;
            }
        };
        struct not_equal
        {
            constexpr bool operator()(std::partial_ordering order) const noexcept
            {
                return order != 0;
            }
        };
    }

    namespace detailed
    {
        template <typename CharT>
        class variable_impl_base
        {
        public:
            typedef CharT char_type;
            typedef std::basic_string<CharT> string_type;

            virtual ~variable_impl_base() = default;

            template <typename T>
            T to() const noexcept = delete;
            template <>
            string_type to<string_type>() const { return to_string(); }
            template <>
            std::uint64_t to<std::uint64_t>() const { return to_uint64_t(); }
            template <>
            std::int64_t to<std::int64_t>() const { return to_int64_t(); }
            template <>
            std::uint32_t to<std::uint32_t>() const { return to_uint32_t(); }
            template <>
            std::int32_t to<std::int32_t>() const { return to_int32_t(); }
            template <>
            std::uint16_t to<std::uint16_t>() const { return to_uint16_t(); }
            template <>
            std::int16_t to<std::int16_t>() const { return to_int16_t(); }
            template <>
            std::uint8_t to<std::uint8_t>() const { return to_uint8_t(); }
            template <>
            std::int8_t to<std::int8_t>() const { return to_int8_t(); }
            template <>
            char_type to<char_type>() const { return to_char(); }
            template <>
            bool to<bool>() const { return to_char(); }
            template <>
            float to<float>() const { return to_float(); }
            template <>
            double to<double>() const { return to_double(); }

            virtual const std::type_info& type() const noexcept = 0;

            // UNSAFE!
            // output: Point to raw memory for placement new
            // Please make sure there is enough memory space!
            virtual void duplicate(void* output) const = 0;
            // UNSAFE!
            // output: Point to raw memory for placement new
            // Please make sure there is enough memory space!
            // Difference from duplicate():
            // This function will deep copy its stored/referred value when it is necessary
            virtual void copy(void* output) const { duplicate(output); }

            // *this <=> other
            virtual std::partial_ordering compare(const variable_impl_base& other) const
            {
                return std::partial_ordering::unordered;
            }
            virtual bool equal(const variable_impl_base& var) const
            {
                return false;
            }

        private:
            virtual string_type to_string() const = 0;
            virtual std::uint64_t to_uint64_t() const = 0;
            virtual std::int64_t to_int64_t() const = 0;
            virtual std::uint32_t to_uint32_t() const = 0;
            virtual std::int32_t to_int32_t() const = 0;
            virtual std::uint16_t to_uint16_t() const = 0;
            virtual std::int16_t to_int16_t() const = 0;
            virtual std::uint8_t to_uint8_t() const = 0;
            virtual std::int8_t to_int8_t() const = 0;
            virtual char_type to_char() const = 0;
            virtual bool to_bool() const = 0;
            virtual float to_float() const = 0;
            virtual double to_double() const = 0;
        };

        template <typename CharT>
        class null_variable_impl : public variable_impl_base<CharT>
        {
        public:
            typedef CharT char_type;
            typedef std::basic_string<CharT> string_type;

            const std::type_info& type() const noexcept final { return typeid(nullvar); }

            bool equal(const variable_impl_base<CharT>& var) const final
            {
                return dynamic_cast<const null_variable_impl*>(&var);
            }

        private:
            string_type to_string() const final { return string_type(); }
            std::uint64_t to_uint64_t() const final { return 0; }
            std::int64_t to_int64_t() const final { return 0; }
            std::uint32_t to_uint32_t() const final { return 0; }
            std::int32_t to_int32_t() const final { return 0; }
            std::uint16_t to_uint16_t() const final { return 0; }
            std::int16_t to_int16_t() const final { return 0; }
            std::uint8_t to_uint8_t() const final { return 0; }
            std::int8_t to_int8_t() const final { return 0; }
            char_type to_char() const final { return char_type('\0'); }
            bool to_bool() const final { return false; }
            float to_float() const final { return std::numeric_limits<double>::quiet_NaN(); }
            double to_double() const final { return std::numeric_limits<double>::quiet_NaN(); }
        };
        template <typename Derived, typename CharT>
        class arithmetic_variable_impl : public variable_impl_base<CharT>
        {
        public:
            typedef CharT char_type;
            typedef variable_impl_base<CharT>::string_type string_type;

            const std::type_info& type() const noexcept final { return typeid(derived().get()); }

            std::partial_ordering compare(const variable_impl_base<CharT>& other) const
            {
                using value_type = std::remove_cvref_t<decltype(derived().get())>;
                return derived().get() <=> other.to<value_type>();
            }
            bool equal(const variable_impl_base<CharT>& var) const final
            {
                using value_type = std::remove_cvref_t<decltype(derived().get())>;
                return derived().get() == var.to<value_type>();
            }

        private:
            constexpr const Derived& derived() const { return static_cast<const Derived&>(*this); }

            string_type to_string() const final
            {
                std::basic_stringstream<char_type> ss;
                ss << derived().get();
                return ss.str();
            }
            std::uint64_t to_uint64_t() const final
            {
                return static_cast<std::uint64_t>(derived().get());
            }
            std::int64_t to_int64_t() const final
            {
                return static_cast<std::int64_t>(derived().get());
            }
            std::uint32_t to_uint32_t() const final
            {
                return static_cast<std::uint32_t>(derived().get());
            }
            std::int32_t to_int32_t() const final
            {
                return static_cast<std::int32_t>(derived().get());
            }
            std::uint16_t to_uint16_t() const final
            {
                return static_cast<std::uint16_t>(derived().get());
            }
            std::int16_t to_int16_t() const final
            {
                return static_cast<std::int16_t>(derived().get());
            }
            std::uint8_t to_uint8_t() const final
            {
                return static_cast<std::uint8_t>(derived().get());
            }
            std::int8_t to_int8_t() const final
            {
                return static_cast<std::int8_t>(derived().get());
            }
            char_type to_char() const final
            {
                return static_cast<char_type>(derived().get());
            }
            bool to_bool() const final
            {
                return static_cast<bool>(derived().get());
            }
            float to_float() const final
            {
                return static_cast<float>(derived().get());
            }
            double to_double() const final
            {
                return static_cast<double>(derived().get());
            }
        };
        template <typename Derived, typename CharT>
        class string_variable_impl : public variable_impl_base<CharT>
        {
        public:
            typedef CharT char_type;
            typedef variable_impl_base<CharT>::string_type string_type;
            typedef std::basic_string_view<CharT> string_view_type;

            const std::type_info& type() const noexcept override { return typeid(string_type); }

            std::partial_ordering compare(const variable_impl_base<CharT>& other) const
            {
                return derived().get() <=> other.to<string_type>();
            }
            bool equal(const variable_impl_base<CharT>& var) const final
            {
                return derived().get() == var.to<string_type>();
            }

        private:
            constexpr const Derived& derived() const { return static_cast<const Derived&>(*this); }

            string_type to_string() const final { return string_type(derived().get()); }
            std::uint64_t to_uint64_t() const final { return std::stoull(string_type(derived().get())); }
            std::int64_t to_int64_t() const final { return std::stoll(string_type(derived().get())); }
            std::uint32_t to_uint32_t() const final { return std::stoul(string_type(derived().get())); }
            std::int32_t to_int32_t() const final { return std::stol(string_type(derived().get())); }
            std::uint16_t to_uint16_t() const final
            {
                auto v = std::stoi(string_type(derived().get()));
                if(v > UINT16_MAX)
                    throw std::out_of_range("out of range");
                return static_cast<std::int16_t>(v);
            }
            std::int16_t to_int16_t() const final
            {
                auto v = std::stoi(string_type(derived().get()));
                if(v > INT16_MAX)
                    throw std::out_of_range("out of range");
                return static_cast<std::int16_t>(v);
            }
            std::uint8_t to_uint8_t() const final
            {
                auto v = std::stoi(string_type(derived().get()));
                if(v > INT8_MAX)
                    throw std::out_of_range("out of range");
                return static_cast<std::uint8_t>(v);
            }
            std::int8_t to_int8_t() const final
            {
                auto v = std::stoi(string_type(derived().get()));
                if(v > UINT8_MAX)
                    throw std::out_of_range("out of range");
                return static_cast<std::int8_t>(v);
            }
            char_type to_char() const final
            {
                string_view_type sv = derived().get();
                if(sv.size() > 1)
                    throw std::out_of_range("out of range");
                return sv.empty() ? char_type('\0') : sv[0];
            }
            bool to_bool() const final
            {
                return !derived().get().empty();
            }
            float to_float() const final { return std::stof(string_type(derived().get())); }
            double to_double() const final { return std::stod(string_type(derived().get())); }
        };

        // General implementation for arithmetic type
        template <typename T, typename CharT>
        class value_impl : public arithmetic_variable_impl<value_impl<T, CharT>, CharT>
        {
            static_assert(std::is_arithmetic_v<T>);
        public:
            typedef CharT char_type;
            typedef variable_impl_base<CharT>::string_type string_type;
            typedef T value_type;

            value_impl(T val) : m_val(val) {}

            constexpr const value_type& get() const noexcept { return m_val; }

            void duplicate(void* output) const final
            {
                new(output) value_impl(m_val);
            }

        private:
            value_type m_val;
        };
        template <typename CharT>
        class value_impl<std::basic_string<CharT>, CharT> final :
            public string_variable_impl<value_impl<std::basic_string<CharT>, CharT>, CharT>
        {
        public:
            typedef CharT char_type;
            typedef std::basic_string<char_type> string_type;
            typedef string_type value_type;

            value_impl() noexcept : m_val() {}
            value_impl(string_type val) noexcept : m_val(std::move(val)) {}

            constexpr const value_type& get() const noexcept { return m_val; }

            void duplicate(void* output) const final
            {
                new(output) value_impl(m_val);
            }

        private:
            string_type m_val;
        };
        template <typename CharT>
        class value_impl<nullvar_t, CharT> final : public null_variable_impl<CharT>
        {
        public:
            typedef CharT char_type;
            typedef variable_impl_base<CharT>::string_type string_type;
            typedef nullvar_t value_type;

            constexpr value_type get() const noexcept { return nullvar; }

            void duplicate(void* output) const final
            {
                new(output) value_impl();
            }
        };

        // Does not hold the ownership of the value
        // General implementation for arithmetic type
        template <typename T, typename CharT>
        class argument_impl final : public arithmetic_variable_impl<argument_impl<T, CharT>, CharT>
        {
            static_assert(std::is_arithmetic_v<T>);
        public:
            typedef CharT char_type;
            typedef T value_type;

            argument_impl() = delete;
            argument_impl(const argument_impl&) = delete;
            argument_impl(const T& value) noexcept
                : m_ref(std::cref(value)) {}

            void duplicate(void* output) const final
            {
                new(output) argument_impl(m_ref.get());
            }
            void copy(void* output) const final
            {
                new(output) value_impl<value_type, char_type>(m_ref.get());
            }

            constexpr const value_type& get() const noexcept { return m_ref.get(); }

        private:
            std::reference_wrapper<const value_type> m_ref;
        };
        // Does not hold the ownership of the value
        template <typename CharT>
        class argument_impl<std::basic_string_view<CharT>, CharT> final
            : public string_variable_impl<argument_impl<std::basic_string_view<CharT>, CharT>, CharT>
        {
        public:
            typedef CharT char_type;
            typedef variable_impl_base<CharT>::string_type string_type;
            typedef std::basic_string_view<CharT> string_view_type;

            argument_impl() = delete;
            argument_impl(const argument_impl&) = delete;
            argument_impl(string_view_type sv) noexcept
                : m_ref(sv) {}

            void duplicate(void* output) const final
            {
                new(output) argument_impl(m_ref);
            }
            void copy(void* output) const final
            {
                new(output) value_impl<string_type, char_type>(string_type(m_ref));
            }

            constexpr string_view_type get() const noexcept { return m_ref; }

        private:
            string_view_type m_ref;
        };
        // Does not hold the ownership of the value
        template <typename CharT>
        class argument_impl<nullvar_t, CharT> : public null_variable_impl<CharT>
        {
        public:
            typedef CharT char_type;
            typedef nullvar_t value_type;

            void duplicate(void* output) const final
            {
                new(output) argument_impl();
            }
            void copy(void* output) const final
            {
                new(output) value_impl<nullvar_t, char_type>();
            }

            constexpr nullvar_t get() const noexcept { return nullvar; }
        };
    }

    template <typename CharT>
    class basic_context
    {
    public:
        typedef CharT char_type;
        typedef CharT value_type;
        typedef std::basic_string<CharT> string_type;
        typedef std::basic_string_view<CharT> string_view_type;

        class value;

        class argument
        {
            friend value;

            typedef detailed::variable_impl_base<char_type> impl_base;
            template <typename T>
            using impl = detailed::argument_impl<T, char_type>;
            static constexpr std::size_t cache_size = std::max(
                sizeof(impl<string_view_type>),
                sizeof(impl<std::uint64_t>)
            );

        public:
            argument()
            {
                construct<nullvar_t>();
            }
            argument(const argument& other) noexcept
            {
                other.get().duplicate(&m_buf);
            }
            /* WARNING!!! Argument class only holds the reference, be careful with dangling reference */
            template <typename T>
            explicit argument(const T& value)
            {
                construct<T>(value);
            }

            ~argument()
            {
                destroy();
            }

            template <typename T>
            void assign(const T& value) noexcept
            {
                destroy();
                construct<T>(value);
            }
            void assign(nullvar_t) noexcept
            {
                destroy();
                construct<nullvar_t>();
            }
            void assign(const argument& arg) noexcept
            {
                destroy();
                arg.get().duplicate(&m_buf);
            }
            template <typename T>
            argument& operator=(const T& value) noexcept
            {
                *this = value;
                return *this;
            }

            template <typename T>
            auto compare(T value)
            {
                return *this <=> value;
            }
            auto compare(const argument& other) const
            {
                return *this <=> other;
            }

            template <typename T>
            friend auto operator<=>(T lhs, const argument& rhs)->std::enable_if_t<std::is_integral_v<T>, std::strong_ordering>
            {
                return lhs <=> rhs.as<T>();
            }
            template <typename T>
            friend auto operator<=>(const argument& lhs, T rhs)->std::enable_if_t<std::is_integral_v<T>, std::strong_ordering>
            {
                return lhs.as<T>() <=> rhs;
            }
            template <typename T>
            friend auto operator<=>(T lhs, const argument& rhs)->std::enable_if_t<std::is_floating_point_v<T>, std::partial_ordering>
            {
                return lhs <=> rhs.as<T>();
            }
            template <typename T>
            friend auto operator<=>(const argument& lhs, T rhs)->std::enable_if_t<std::is_floating_point_v<T>, std::partial_ordering>
            {
                return lhs.as<T>() <=> rhs;
            }
            friend auto operator<=>(string_view_type lhs, const argument& rhs)->std::strong_ordering
            {
                return lhs <=> rhs.as<string_type>();
            }
            friend auto operator<=>(const argument& lhs, string_view_type rhs)->std::strong_ordering
            {
                return lhs.as<string_type>() <=> rhs;
            }
            auto operator<=>(const argument& rhs) const
            {
                return get().compare(rhs.get());
            }

            template <typename T>
            friend auto operator==(const argument& lhs, T rhs)->std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, bool>
            {
                return lhs.as<T>() == rhs;
            }
            template <typename T>
            friend auto operator==(T lhs, const argument& rhs)->std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, bool>
            {
                return lhs == rhs.as<T>();
            }
            friend bool operator==(string_view_type lhs, const argument& rhs)
            {
                return lhs == rhs.as<string_type>();
            }
            friend bool operator==(const argument& lhs, string_view_type rhs)
            {
                return lhs.as<string_type>() == rhs;
            }
            bool operator==(const argument& rhs) const
            {
                return get().equal(rhs.get());
            }

            bool empty() const noexcept
            {
                return type() == typeid(nullvar_t);
            }
            void clear() noexcept
            {
                assign(nullvar);
            }

            template <typename T>
            T as() const { return get().to<T>(); }

            const std::type_info& type() const noexcept { return get().type();  }

        private:
            std::aligned_storage_t<cache_size> m_buf;

            impl_base& get() noexcept
            {
                return *reinterpret_cast<impl_base*>(&m_buf);
            }
            const impl_base& get() const noexcept
            {
                return *reinterpret_cast<const impl_base*>(&m_buf);
            }

            template <typename T, typename... Args>
            void construct(Args&&... args)
            {
                if constexpr(
                    std::is_same_v<std::remove_cvref_t<T>, string_type> ||
                    (std::is_array_v<T> && std::is_same_v<CharT, std::remove_extent_t<T>>)
                ) {
                    static_assert(sizeof(impl<string_view_type>) <= cache_size);
                    new(&m_buf) impl<string_view_type>(std::forward<Args>(args)...);
                }
                else
                {
                    static_assert(sizeof(impl<T>) <= cache_size);
                    new(&m_buf) impl<T>(std::forward<Args>(args)...);
                }
            }
            void destroy() noexcept
            {
                get().~variable_impl_base();
            }
        };

        class value
        {
            typedef detailed::variable_impl_base<char_type> impl_base;
            template <typename T>
            using impl = detailed::value_impl<T, char_type>;

        public:
            value()
            {
                construct<nullvar_t>();
            }
            value(const value& other)
            {
                other.get().copy(&m_buf);
            }
            template <typename T>
            explicit value(T&& val)
            {
                if constexpr(std::is_same_v<std::remove_cvref_t<T>, argument>)
                    val.get().copy(&m_buf);
                else
                    construct<T>(std::move(val));
            }

            template <typename T>
            void assign(const T& val)
            {
                destroy();
                if constexpr(std::is_same_v<std::remove_cvref_t<T>, argument>)
                    val.get().copy(&m_buf);
                else
                    construct<T>(std::move(val));
            }
            void assign(nullvar_t) noexcept
            {
                destroy();
                construct<nullvar_t>();
            }
            void assign(const value& arg)
            {
                destroy();
                arg.get().duplicate(&m_buf);
            }

            template <typename T>
            auto compare(T val) const
            {
                return *this <=> val;
            }
            auto compare(const value& other) const
            {
                return *this <=> other;
            }

            template <typename T>
            friend auto operator<=>(T lhs, const value& rhs)->std::enable_if_t<std::is_integral_v<T>, std::strong_ordering>
            {
                return lhs <=> rhs.as<T>();
            }
            template <typename T>
            friend auto operator<=>(const value& lhs, T rhs)->std::enable_if_t<std::is_integral_v<T>, std::strong_ordering>
            {
                return lhs.as<T>() <=> rhs;
            }
            template <typename T>
            friend auto operator<=>(T lhs, const value& rhs)->std::enable_if_t<std::is_floating_point_v<T>, std::partial_ordering>
            {
                return lhs <=> rhs.as<T>();
            }
            template <typename T>
            friend auto operator<=>(const value& lhs, T rhs)->std::enable_if_t<std::is_floating_point_v<T>, std::partial_ordering>
            {
                return lhs.as<T>() <=> rhs;
            }
            friend auto operator<=>(string_view_type lhs, const value& rhs)->std::strong_ordering
            {
                return lhs <=> rhs.as<string_type>();
            }
            friend auto operator<=>(const value& lhs, string_view_type rhs)->std::strong_ordering
            {
                return lhs.as<string_type>() <=> rhs;
            }
            auto operator<=>(const value& rhs) const
            {
                return get().compare(rhs.get());
            }

            template <typename T>
            friend auto operator==(const value& lhs, T rhs)->std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, bool>
            {
                return lhs.as<T>() == rhs;
            }
            template <typename T>
            friend auto operator==(T lhs, const value& rhs)->std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, bool>
            {
                return lhs == rhs.as<T>();
            }
            friend bool operator==(string_view_type lhs, const value& rhs)
            {
                return lhs == rhs.as<string_type>();
            }
            friend bool operator==(const value& lhs, string_view_type rhs)
            {
                return lhs.as<string_type>() == rhs;
            }
            bool operator==(const value& rhs) const
            {
                return get().equal(rhs.get());
            }

            bool empty() const noexcept
            {
                return type() == typeid(nullvar_t);
            }
            void clear() noexcept
            {
                assign(nullvar);
            }

            template <typename T>
            T as() const { return get().to<T>(); }

            const std::type_info& type() const noexcept { return get().type(); }

        private:
            static constexpr std::size_t cache_size = std::max(
                sizeof(impl<std::uint64_t>),
                sizeof(impl<string_type>)
            );
            std::aligned_storage_t<cache_size> m_buf;

            impl_base& get() noexcept
            {
                return *reinterpret_cast<impl_base*>(&m_buf);
            }
            const impl_base& get() const noexcept
            {
                return *reinterpret_cast<const impl_base*>(&m_buf);
            }

            template <typename T, typename... Args>
            void construct(Args&&... args)
            {
                if constexpr(
                    std::is_same_v<std::remove_cvref_t<T>, string_view_type> ||
                    (std::is_array_v<T> && std::is_same_v<CharT, std::remove_extent_t<T>>)
                    ) {
                    static_assert(sizeof(impl<string_type>) <= cache_size);
                    new(&m_buf) impl<string_type>(std::forward<Args>(args)...);
                }
                else
                {
                    static_assert(sizeof(impl<T>) <= cache_size);
                    new(&m_buf) impl<T>(std::forward<Args>(args)...);
                }
            }
            void destroy() noexcept
            {
                get().~variable_impl_base();
            }
        };

        class script
        {
        public:
            virtual ~script() = default;

            virtual value invoke(basic_context& ctx) = 0;
        };
        template <typename Comp>
        class script_compare final : public script
        {
        public:
            value invoke(basic_context& ctx) override
            {
                return value(static_cast<bool>(m_comp(
                    left_operand->invoke(ctx) <=> right_operand->invoke(ctx)
                )));
            }
            
            std::unique_ptr<script> left_operand;
            std::unique_ptr<script> right_operand;

        private:
            Comp m_comp;
        };
        class script_if : public script
        {
        public:
            value invoke(basic_context& ctx) override
            {
                if(condition->invoke(ctx).as<bool>())
                {
                    return on_true->invoke(ctx);
                }
                else if(on_false)
                {
                    return on_false->invoke(ctx);
                }
                else
                {
                    return value();
                }
            }

            std::unique_ptr<script> condition;
            std::unique_ptr<script> on_true;
            std::unique_ptr<script> on_false;
        };
        template <typename T>
        class script_argument : public script
        {
            typedef std::variant<std::size_t, string_type> id_t;
        public:
            script_argument(std::size_t idx) : m_id(idx) {}
            script_argument(string_type name) : m_id(std::move(name)) {}

            value invoke(basic_context& ctx)
            {
                return value(
                    std::visit([&ctx](auto& id) { return ctx.arg(id); }, m_id).as<T>()
                );
            }

        private:
            id_t m_id;
        };
        class script_argument_any : public script
        {
            typedef std::variant<std::size_t, string_type> id_t;
        public:
            script_argument_any(std::size_t idx) : m_id(idx) {}
            script_argument_any(string_type name) : m_id(std::move(name)) {}

            value invoke(basic_context& ctx)
            {
                return value(
                    std::visit([&ctx](auto& id) { return ctx.arg(id); }, m_id)
                );
            }

        private:
            id_t m_id;
        };
        class script_literal : public script
        {
        public:
            template <typename T>
            script_literal(T val) : m_val(std::move(val)) {}
            script_literal(value val) : m_val(val) {}

            value invoke(basic_context& ctx) override
            {
                return m_val;
            }

        private:
            value m_val;
        };

        std::size_t push_arg(argument arg)
        {
            m_args.emplace_back(arg);
            return m_args.size() - 1;
        }
        void set_named_arg(string_type name, argument arg)
        {
            m_named_args.insert_or_assign(std::move(name), arg);
        }
        void clear_arg() noexcept
        {
            m_args.clear();
            m_named_args.clear();
        }

        argument arg(std::size_t index) const noexcept
        {
            if(index < m_args.size())
                return m_args[index];
            else
                return argument();
        }
        argument arg(string_view_type id) const noexcept
        {
            auto it = m_named_args.find(id);
            if(it == m_named_args.end())
                return argument();
            else
                return it->second;
        }

    private:
        std::vector<argument> m_args;
        // Heterogeneous lookup comparator for avoiding memory allocation when comparing
        struct named_args_comp
        {
            using is_transparent = void;

            bool operator()(string_view_type lhs, string_view_type rhs) const noexcept
            {
                return lhs < rhs;
            }
        };
        std::map<string_type, argument, named_args_comp> m_named_args;
    };

    typedef basic_context<char> context;
    typedef basic_context<wchar_t> wcontext;
    typedef basic_context<char16_t> u16context;
    typedef basic_context<char32_t> u32context;
    typedef basic_context<char8_t> u8context;
}
