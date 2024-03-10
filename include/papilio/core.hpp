#ifndef PAPILIO_CORE_HPP
#define PAPILIO_CORE_HPP

#pragma once

#include <variant>
#include <typeinfo>
#include <vector>
#include <map>
#include <span>
#include <array>
#include "macros.hpp"
#include "fmtfwd.hpp"
#include "utility.hpp"
#include "container.hpp"
#include "utf.hpp"
#include "locale.hpp"
#include "access.hpp"

#ifdef PAPILIO_COMPILER_CLANG
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wc++98-compat"
#    pragma clang diagnostic ignored "-Wc++98-compat-pedantic"
#    pragma clang diagnostic ignored "-Wc++20-compat"
#endif

namespace papilio
{
PAPILIO_EXPORT enum class format_align : std::uint8_t
{
    default_align = 0,
    left,
    middle,
    right
};

PAPILIO_EXPORT enum class format_sign : std::uint8_t
{
    default_sign = 0,
    positive,
    negative,
    space
};

PAPILIO_EXPORT class format_error : public std::runtime_error
{
public:
    using runtime_error::runtime_error;
};

PAPILIO_EXPORT class bad_handle_cast : public std::bad_cast
{
public:
    const char* what() const noexcept override
    {
        return "bad handle cast";
    }
};

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

// Derive your formatter from this class to explicitly prevent a type from being formatted.
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
        !std::is_same_v<std::remove_cv_t<T>, bool> &&
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
        std::is_same_v<std::remove_cv_t<T>, float> ||
        std::is_same_v<std::remove_cv_t<T>, double> ||
        std::is_same_v<std::remove_cv_t<T>, long double>;

    template <typename T, typename CharT>
    concept use_handle =
        !std::is_same_v<std::remove_cv_t<T>, bool> &&
        !std::is_same_v<std::remove_cv_t<T>, utf::codepoint> &&
        !char_like<T> &&
        !acceptable_integral<T> &&
        !acceptable_fp<T> &&
        !std::is_pointer_v<T> &&
        !std::is_bounded_array_v<T> &&
        !basic_string_like<T, CharT>;

    template <typename T>
    concept use_soo_handle =
        std::is_nothrow_copy_constructible_v<std::remove_cvref_t<T>> &&
        std::is_nothrow_move_constructible_v<std::remove_cvref_t<T>>;
} // namespace detail

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
            static_assert(detail::use_handle<typename Impl::value_type, char_type>);
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
        return std::is_same_v<std::remove_cvref_t<T>, handle>;
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

    template <detail::use_handle<char_type> T>
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

    template <detail::use_handle<char_type> T>
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
        if constexpr(char_like<T> || std::is_same_v<std::remove_cvref_t<T>, utf::codepoint>)
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

#ifdef PAPILIO_COMPILER_CLANG_CL
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif

    template <typename Context, typename CharT>
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

        [[nodiscard]]
        virtual const format_arg_type& get(size_type i) const = 0;
        [[nodiscard]]
        virtual const format_arg_type& get(string_view_type key) const = 0;

        [[nodiscard]]
        virtual const format_arg_type& get(const indexing_value_type& idx) const
        {
            return idx.visit(
                [&]<typename T>(const T& v) -> const format_arg_type&
                {
                    if constexpr(std::is_same_v<T, typename indexing_value_type::index_type>)
                    {
                        if(v < 0)
                            throw_index_out_of_range();
                        size_type i = static_cast<size_type>(v);
                        return get(i);
                    }
                    else if constexpr(std::is_same_v<T, string_container_type>)
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
        bool check(size_type i) const noexcept
        {
            return i < indexed_size();
        }

        [[nodiscard]]
        virtual bool check(string_view_type key) const noexcept = 0;

        [[nodiscard]]
        bool check(const indexing_value_type& idx) const noexcept
        {
            return idx.visit(
                [this]<typename T>(const T& v) -> bool
                {
                    if constexpr(std::is_same_v<T, typename indexing_value_type::index_type>)
                    {
                        if(v < 0)
                            return false;
                        return check(static_cast<size_type>(v));
                    }
                    else if constexpr(std::is_same_v<T, string_container_type>)
                    {
                        return check(string_view_type(v));
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
} // namespace detail

#ifdef PAPILIO_COMPILER_CLANG_CL
#    pragma clang diagnostic pop
#endif

PAPILIO_EXPORT template <typename T, typename Context = format_context>
struct is_format_args :
    std::bool_constant<std::is_base_of_v<detail::format_args_base<Context, typename Context::char_type>, T>>
{};

PAPILIO_EXPORT template <typename T, typename Context = format_context>
constexpr inline bool is_format_args_v = is_format_args<T, Context>::value;

PAPILIO_EXPORT template <
    std::size_t IndexedArgumentCount,
    std::size_t NamedArgumentCount,
    typename Context = format_context,
    typename CharT = typename Context::char_type>
class static_format_args final : public detail::format_args_base<Context, CharT>
{
    using my_base = detail::format_args_base<Context, CharT>;

public:
    static_assert(std::is_same_v<typename Context::char_type, CharT>);

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
    bool check(string_view_type key) const noexcept override
    {
        return m_named_args.contains(key);
    }

    using my_base::check;

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

        (push(std::forward<Args>(args)), ...);
    }

    vector_type m_indexed_args;
    map_type m_named_args;

    template <typename T>
    requires(!is_named_arg_v<T>)
    void push(T&& val) noexcept(std::is_nothrow_constructible_v<format_arg_type, T>)
    {
        m_indexed_args.emplace_back(std::forward<T>(val));
    }

    template <typename T>
    requires(is_named_arg_v<T> && std::is_same_v<char_type, typename T::char_type>)
    void push(T&& na) noexcept(std::is_nothrow_constructible_v<format_arg_type, typename T::value_type>)
    {
        m_named_args.insert_or_assign(
            na.name,
            PAPILIO_NS forward_like<T>(na.value)
        );
    }
};

PAPILIO_EXPORT template <typename Context, typename CharT = typename Context::char_type>
class basic_dynamic_format_args final : public detail::format_args_base<Context, CharT>
{
    using my_base = detail::format_args_base<Context, CharT>;

public:
    static_assert(std::is_same_v<typename Context::char_type, CharT>);

    using char_type = CharT;
    using string_type = std::basic_string<char_type>;
    using string_view_type = std::basic_string_view<char_type>;
    using size_type = std::size_t;
    using format_arg_type = basic_format_arg<Context>;

    using vector_type = std::vector<
        format_arg_type>;
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
        push_tuple(std::forward<Args>(args)...);
    }

    template <typename T>
    void push(T&& val)
    {
        if constexpr(is_named_arg_v<std::remove_cvref_t<T>>)
        {
            static_assert(
                std::is_same_v<char_type, typename T::char_type>,
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
    void push_tuple(Args&&... args)
    {
        m_indexed_args.reserve(
            m_indexed_args.size() + detail::get_indexed_arg_count<Args...>()
        );

        (push(std::forward<Args>(args)), ...);
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
    bool check(string_view_type key) const noexcept override
    {
        return m_named_args.contains(key);
    }

    using my_base::check;

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

// Type-erased format arguments.
PAPILIO_EXPORT template <typename Context, typename CharT = typename Context::char_type>
class basic_format_args_ref final : public detail::format_args_base<Context, CharT>
{
    using my_base = detail::format_args_base<Context, CharT>;

public:
    using char_type = CharT;
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
    bool check(string_view_type k) const noexcept override
    {
        return m_ptr->check(k);
    }

    using my_base::check;

private:
    const my_base* m_ptr;
};

PAPILIO_EXPORT using format_args_ref = basic_format_args_ref<format_context, char>;
PAPILIO_EXPORT using wformat_args_ref = basic_format_args_ref<wformat_context, wchar_t>;

PAPILIO_EXPORT template <typename Context = format_context, typename... Args>
auto make_format_args(Args&&... args)
{
    static_assert(
        std::conjunction_v<std::negation<is_format_args<Args, Context>>...>,
        "cannot use format_args as format argument"
    );

    using result_type = static_format_args<
        detail::get_indexed_arg_count<Args...>(),
        detail::get_named_arg_count<Args...>(),
        Context,
        char>;
    return result_type(std::forward<Args>(args)...);
}

PAPILIO_EXPORT template <typename Context = wformat_context, typename... Args>
auto make_wformat_args(Args&&... args)
{
    static_assert(
        std::conjunction_v<std::negation<is_format_args<Args, Context>>...>,
        "cannot use format_args as format argument"
    );

    using result_type = static_format_args<
        detail::get_indexed_arg_count<Args...>(),
        detail::get_named_arg_count<Args...>(),
        Context,
        wchar_t>;
    return result_type(std::forward<Args>(args)...);
}

namespace detail
{
    template <typename T, typename FormatContext>
    concept check_adl_format_simple = requires(T&& val, FormatContext fmt_ctx) {
        {
            format(val, fmt_ctx)
        } -> std::same_as<typename FormatContext::iterator>;
    };

    template <typename T, typename FormatContext>
    concept check_adl_format_complex = requires(
        T&& val, basic_format_parse_context<FormatContext> parse_ctx, FormatContext fmt_ctx
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
    void invoke_adl_format(T&& val, ParseContext& parse_ctx, FormatContext& fmt_ctx)
    {
        if constexpr(detail::check_adl_format_simple<T, FormatContext>)
        {
            fmt_ctx.advance_to(
                format(val, fmt_ctx)
            );
        }
        else
        {
            fmt_ctx.advance_to(
                format(val, parse_ctx, fmt_ctx)
            );
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
    using parse_context = basic_format_parse_context<FormatContext>;

    auto format(const T& val, parse_context& parse_ctx, FormatContext& fmt_ctx) const
        -> typename FormatContext::iterator
    {
        detail::invoke_adl_format(val, parse_ctx, fmt_ctx);

        return fmt_ctx.out();
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
    template <typename ParseContext>
    auto parse(ParseContext& ctx)
    {
        auto it = ctx.begin();
        if(*it == U'L')
        {
            m_use_locale = true;
            ++it;
        }

        return it;
    }

    template <typename Context>
    auto format(const T& val, Context& ctx) const
    {
        using os_t = basic_oiterstream<
            CharT,
            typename Context::iterator>;

        os_t os(ctx.out());

        if(m_use_locale)
            os.imbue(ctx.getloc());

        os << val;

        return os.base();
    }

private:
    bool m_use_locale = false;
};

namespace detail
{
    template <typename T, typename CharT>
    concept is_formatter_disabled = std::is_base_of_v<
        disabled_formatter,
        formatter<T, CharT>>;

    template <typename T, typename Context, typename CharT>
    struct select_formatter
    {
        using type = formatter<T, CharT>;
    };

    template <typename T, typename Context, typename CharT>
    requires(!is_formatter_disabled<T, CharT> && !std::semiregular<formatter<T, CharT>> && has_adl_format_v<T, Context>)
    struct select_formatter<T, Context, CharT>
    {
        using type = adl_format_adaptor<T, Context>;
    };

    template <typename T, typename Context, typename CharT>
    requires(!is_formatter_disabled<T, CharT> && !std::semiregular<formatter<T, CharT>> && !has_adl_format_v<T, Context> && streamable<T, CharT>)
    struct select_formatter<T, Context, CharT>
    {
        using type = streamable_formatter<T, CharT>;
    };

    template <typename T, typename Context>
    using select_formatter_t =
        typename select_formatter<T, Context, typename Context::char_type>::type;
} // namespace detail

PAPILIO_EXPORT template <typename OutputIt, typename CharT>
class basic_format_context
{
public:
    using char_type = CharT;
    using iterator = OutputIt;
    using format_args_type = basic_format_args_ref<basic_format_context, char_type>;

    template <typename T>
    using formatter_type = detail::select_formatter_t<T, basic_format_context>;

    basic_format_context(iterator it, format_args_type args)
        : m_out(std::move(it)), m_args(args), m_loc(nullptr) {}

    basic_format_context(const std::locale& loc, iterator it, format_args_type args)
        : m_out(std::move(it)), m_args(args), m_loc(loc) {}

    basic_format_context(locale_ref loc, iterator it, format_args_type args)
        : m_out(std::move(it)), m_args(args), m_loc(loc) {}

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
    const format_args_type& get_args() const noexcept
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
    format_args_type m_args;
    locale_ref m_loc;
};

PAPILIO_EXPORT template <typename Context>
class format_context_traits
{
public:
    using char_type = typename Context::char_type;
    using string_type = std::basic_string<char_type>;
    using string_view_type = std::basic_string_view<char_type>;
    using context_type = Context;
    using iterator = typename Context::iterator;
    using format_arg_type = format_arg;
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

    static void append(context_type& ctx, utf::codepoint cp, std::size_t count = 1)
    {
        for(std::size_t i = 0; i < count; ++i)
        {
            advance_to(ctx, cp.append_to_as<char_type>(out(ctx)));
        }
    }
};

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
        if(!get_args().check(i))
            throw format_error("invalid arg id");
    }

    void check_arg_id(string_view_type name) const
    {
        if(!get_args().check(name))
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

    // Default implementation of skipping unused format specification.
    // WARNING: This function cannot skip specification that contains unbalanced braces ("{" and "}")!
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

PAPILIO_EXPORT using format_parse_context = basic_format_parse_context<format_context>;
PAPILIO_EXPORT using wformat_parse_context = basic_format_parse_context<wformat_context>;

PAPILIO_EXPORT using dynamic_format_args = basic_dynamic_format_args<format_context>;
PAPILIO_EXPORT using wdynamic_format_args = basic_dynamic_format_args<wformat_context>;

namespace detail
{
    template <typename Formatter, typename ParseContext>
    concept check_parse_method = requires(Formatter& f, ParseContext parse_ctx) {
        {
            f.parse(parse_ctx)
        } -> std::same_as<typename ParseContext::iterator>;
    };
} // namespace detail

PAPILIO_EXPORT template <typename Formatter>
struct formatter_traits
{
    // Returns true if the formatter has a standalone parse function
    template <typename ParseContext = format_parse_context>
    static constexpr bool parsable() noexcept
    {
        return detail::check_parse_method<Formatter, ParseContext>;
    }

    template <typename T, typename ParseContext, typename FormatContext>
    static void format(T&& val, ParseContext& parse_ctx, FormatContext& fmt_ctx)
    {
        using context_t = format_context_traits<FormatContext>;

        Formatter fmt{};

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
};

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
    concept check_format_method_combined = requires(const Formatter& cf, T&& val, FormatContext fmt_ctx, ParseContext parse_ctx) {
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
        typename Formatter = typename Context::template formatter_type<std::remove_const_t<T>>>
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
    basic_format_context<detail::fmt_iter_for<CharT>, CharT>>;

PAPILIO_EXPORT template <pointer_like T>
const void* ptr(const T& p) noexcept
{
    return std::to_address(p);
}
} // namespace papilio

#include "core.inl"

#ifdef PAPILIO_COMPILER_CLANG
#    pragma clang diagnostic pop
#endif

#endif
