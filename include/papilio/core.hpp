#pragma once

#include <string>
#include <variant>
#include <vector>
#include <map>
#include "macros.hpp"
#include "utility.hpp"
#include "container.hpp"
#include "utf/utf.hpp"
#include "locale.hpp"
#include "access.hpp"

#ifdef PAPILIO_COMPILER_MSVC
#    pragma warning(push)
#    pragma warning(disable : 26495) // uninitialized member variable
#endif

namespace papilio
{
// forward declarations
namespace detail
{
    template <typename CharT>
    using fmt_iter_for = std::back_insert_iterator<std::basic_string<CharT>>;
}

template <typename Context>
class basic_format_arg;

template <typename FormatContext>
class format_parse_context;

template <typename OutputIt, typename CharT = char>
class basic_format_context;

template <typename T, typename CharT = char>
class formatter;

using format_context = basic_format_context<detail::fmt_iter_for<char>, char>;
using wformat_context = basic_format_context<detail::fmt_iter_for<wchar_t>, wchar_t>;
using format_arg = basic_format_arg<format_context>;

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

class invalid_format : public std::invalid_argument
{
public:
    using invalid_argument::invalid_argument;
};

namespace detail
{
    template <typename T>
    concept acceptable_integral =
        !std::is_same_v<T, bool> &&
        std::integral<T> &&
        !char_like<T> &&
        sizeof(T) <= sizeof(unsigned long long int);

    template <acceptable_integral Integral>
    using convert_int_t = std::conditional_t<
        std::is_unsigned_v<Integral>,
        std::conditional_t<sizeof(Integral) <= sizeof(unsigned int), unsigned int, unsigned long long int>,
        std::conditional_t<sizeof(Integral) <= sizeof(int), int, long long int>>;

    template <typename T, typename CharT>
    concept use_handle =
        !std::is_same_v<T, bool> &&
        !std::is_same_v<T, utf::codepoint> &&
        !char_like<T> &&
        !acceptable_integral<T> &&
        !std::floating_point<T> &&
        !basic_string_like<T, CharT>;

    template <typename T>
    concept use_soo_handle =
        std::is_nothrow_copy_constructible_v<std::remove_cvref_t<T>> &&
        std::is_nothrow_move_constructible_v<std::remove_cvref_t<T>>;
} // namespace detail

template <typename Context>
class basic_format_arg
{
public:
    using char_type = typename Context::char_type;
    using string_type = std::basic_string<char_type>;
    using string_view_type = std::basic_string_view<char_type>;
    using string_container_type = utf::basic_string_container<char_type>;

    using indexing_value_type = basic_indexing_value<char_type>;
    using attribute_name_type = basic_attribute_name<char_type>;

    using parse_context = format_parse_context<Context>;

private:
    [[noreturn]]
    static void throw_unformattable()
    {
        throw invalid_format("unformattable");
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

        virtual void copy(void* mem) const noexcept = 0;

        virtual void move(void* mem) noexcept = 0;

        virtual bool has_ownership() const noexcept = 0;

        virtual bool is_formattable() const noexcept = 0;
    };

    template <typename T>
    class handle_impl final : public handle_impl_base
    {
    public:
        using value_type = std::remove_cvref_t<T>;

        handle_impl(const T& val) noexcept
            : m_ptr(std::addressof(val), false) {}

        template <typename Arg>
        handle_impl(independent_t, Arg&& val)
            : m_ptr(make_optional_unique<const value_type>(std::forward<Arg>(val)))
        {}

        handle_impl(const handle_impl& other) noexcept
            : m_ptr(other.m_ptr) {}

        handle_impl(handle_impl&& other) noexcept
            : m_ptr(std::move(other.m_ptr)) {}

        basic_format_arg index(const indexing_value_type& idx) const override
        {
            PAPILIO_ASSERT(m_ptr);

            using accessor_t = accessor_traits<value_type, char_type>;
            return accessor_t::template access<basic_format_arg>(*m_ptr, idx);
        }

        basic_format_arg attribute(const attribute_name_type& attr) const override
        {
            PAPILIO_ASSERT(m_ptr);

            using accessor_t = accessor_traits<value_type, char_type>;
            return accessor_t::template attribute<basic_format_arg>(*m_ptr, attr);
        }

        void format(parse_context& parse_ctx, Context& out_ctx) const override;

        void copy(void* mem) const noexcept override
        {
            new(mem) handle_impl(*this);
        }

        void move(void* mem) noexcept override
        {
            new(mem) handle_impl(std::move(*this));
        }

        bool has_ownership() const noexcept override
        {
            return m_ptr.has_ownership();
        }

        bool is_formattable() const noexcept override;

    private:
        optional_unique_ptr<const value_type> m_ptr;
    };

    template <typename T>
    class handle_impl_soo final : public handle_impl_base
    {
    public:
        using value_type = std::remove_cvref_t<T>;

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
            using accessor_t = accessor_traits<value_type, char_type>;
            return accessor_t::template access<basic_format_arg>(m_val, idx);
        }

        basic_format_arg attribute(const attribute_name_type& attr) const override
        {
            using accessor_t = accessor_traits<value_type, char_type>;
            return accessor_t::template attribute<basic_format_arg>(m_val, attr);
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

        bool is_formattable() const noexcept override;

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
        handle(T&& val) noexcept
            : handle()
        {
            construct<T>(val);
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

        basic_format_arg index(const indexing_value_type& idx) const
        {
            return ptr()->index(idx);
        }

        basic_format_arg attribute(const attribute_name_type& attr) const
        {
            return ptr()->attribute(attr);
        }

        void format(parse_context& parse_ctx, Context& out_ctx) const
        {
            ptr()->format(parse_ctx, out_ctx);
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

    private:
        static constexpr std::size_t storage_size = 32;
        mutable static_storage<storage_size> m_storage;

        handle_impl_base* ptr() const noexcept
        {
            return reinterpret_cast<handle_impl_base*>(m_storage.data());
        }

        template <typename T>
        void construct(T&& val) noexcept
        {
            using type = std::remove_cvref_t<T>;
            using impl_t = std::conditional_t<
                detail::use_soo_handle<type> && sizeof(handle_impl_soo<type>) <= storage_size,
                handle_impl_soo<type>,
                handle_impl<type>>;

            static_assert(sizeof(impl_t) <= m_storage.size());

            new(ptr()) impl_t(std::forward<T>(val));
        }

        template <typename T>
        void construct(independent_t, T&& val) noexcept
        {
            using type = std::remove_cvref_t<T>;
            using impl_t = std::conditional_t<
                detail::use_soo_handle<type> && sizeof(handle_impl_soo<type>) <= storage_size,
                handle_impl_soo<type>,
                handle_impl<type>>;

            static_assert(sizeof(impl_t) <= m_storage.size());

            new(ptr()) impl_t(independent, std::forward<T>(val));
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
    basic_format_arg(basic_format_arg&&) noexcept = default;

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

    template <std::floating_point Float>
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
    requires(!char_like<T>)
    basic_format_arg(const T* ptr) noexcept
        : m_val(std::in_place_type<const void*>, ptr)
    {}

    basic_format_arg(std::nullptr_t) noexcept
        : m_val(std::in_place_type<const void*>, nullptr) {}

    template <detail::use_handle<char_type> T>
    basic_format_arg(const T& val) noexcept
        : m_val(std::in_place_type<handle>, val)
    {}

    template <detail::use_handle<char_type> T>
    basic_format_arg(independent_t, T&& val) noexcept
        : m_val(std::in_place_type<handle>, independent, std::forward<T>(val))
    {}

    basic_format_arg& operator=(const basic_format_arg&) = default;
    basic_format_arg& operator=(basic_format_arg&&) noexcept = default;

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
                using target_type = std::remove_cvref_t<T>;

                if constexpr(std::is_same_v<target_type, handle>)
                {
                    return v.index(idx);
                }
                else
                {
                    using accessor_t = accessor_traits<target_type, char_type>;
                    return accessor_t::template access<basic_format_arg>(v, idx);
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
                using target_type = std::remove_cvref_t<T>;

                if constexpr(std::is_same_v<target_type, handle>)
                {
                    return v.attribute(attr);
                }
                else
                {
                    using accessor_t = accessor_traits<target_type, char_type>;
                    return accessor_t::template attribute<basic_format_arg>(v, attr);
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
        if constexpr(char_like<T>)
            return std::get<utf::codepoint>(val.m_val);
        else if constexpr(std::integral<T>)
            return std::get<detail::convert_int_t<T>>(val.m_val);
        else if constexpr(basic_string_like<T, char_type>)
            return std::get<string_container_type>(val.m_val);
        else
            return std::get<T>(val.m_val);
    }

    void format(parse_context& parse_ctx, Context& out_ctx) const;

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
        if constexpr(sizeof...(Ts) == 0)
            return 0;
        else
        {
            using tuple_t = std::tuple<Ts...>;
            using std::size_t;
            using std::tuple_element_t;

            return []<size_t... Is>(std::index_sequence<Is...>) -> size_t
            {
                return (count_if_named_arg<tuple_element_t<Is, tuple_t>>() + ...);
            }(std::make_index_sequence<sizeof...(Ts)>());
        }
    }

    template <typename... Ts>
    consteval std::size_t get_indexed_arg_count() noexcept
    {
        return sizeof...(Ts) - get_named_arg_count<Ts...>();
    }

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

        bool check(size_type i) const noexcept
        {
            return i < indexed_size();
        }

        virtual bool check(string_view_type key) const noexcept = 0;

        bool check(const indexing_value_type& idx) const noexcept
        {
            return idx.visit(
                [this]<typename T>(const T& v) -> bool
                {
                    if constexpr(std::is_same_v<T, indexing_value_type::index_type>)
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

        // clang-format off

        [[nodiscard]]
        const format_arg_type& operator[](const indexing_value_type& idx) const
        {
            return get(idx);
        }

        // clang-format on

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

template <
    std::size_t IndexedArgumentCount,
    std::size_t NamedArgumentCount,
    typename Context = format_context,
    typename CharT = typename Context::char_type>
class static_format_args final : public detail::format_args_base<Context, CharT>
{
    using base = detail::format_args_base<Context, CharT>;

public:
    using char_type = CharT;
    using size_type = std::size_t;
    using string_view_type = std::basic_string_view<char_type>;
    using format_arg_type = basic_format_arg<Context>;

    template <typename... Args>
    static_format_args(Args&&... args)
    {
        construct(std::forward<Args>(args)...);
    }

    const format_arg_type& get(size_type i) const override
    {
        if(i >= m_indexed_args.size())
            this->throw_index_out_of_range();
        return m_indexed_args[i];
    }

    const format_arg_type& get(string_view_type k) const override
    {
        auto it = m_named_args.find(k);
        if(it == m_named_args.end())
            this->throw_invalid_named_argument();
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
        PAPILIO_ASSERT(m_indexed_args.size() == IndexedArgumentCount);
        return IndexedArgumentCount;
    }

    size_type named_size() const noexcept override
    {
        PAPILIO_ASSERT(m_named_args.size() == NamedArgumentCount);
        return NamedArgumentCount;
    }

private:
    using vector_type = fixed_vector<
        format_arg_type,
        IndexedArgumentCount>;
    using map_type = fixed_flat_map<
        string_view_type,
        format_arg_type,
        NamedArgumentCount,
        std::less<>>;

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

template <typename Context, typename CharT = typename Context::char_type>
class basic_mutable_format_args final : public detail::format_args_base<Context, CharT>
{
    using base = detail::format_args_base<Context, CharT>;

public:
    using char_type = CharT;
    using string_type = std::basic_string<char_type>;
    using string_view_type = std::basic_string_view<char_type>;
    using size_type = std::size_t;
    using format_arg_type = basic_format_arg<Context>;

    basic_mutable_format_args() = default;
    basic_mutable_format_args(const basic_mutable_format_args&) = delete;
    basic_mutable_format_args(basic_mutable_format_args&&) = default;

    template <typename... Args>
    basic_mutable_format_args(Args&&... args)
    {
        push(std::forward<Args>(args)...);
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

    template <typename T, typename... Args>
    void push(T&& val, Args&&... args)
    {
        m_indexed_args.reserve(
            m_indexed_args.size() + detail::get_indexed_arg_count<T, Args...>()
        );

        push(std::forward<T>(val));
        if constexpr(sizeof...(Args))
            push(std::forward<Args>(args)...);
    }

    const format_arg_type& get(size_type i) const override
    {
        if(i >= m_indexed_args.size())
            this->throw_index_out_of_range();
        return m_indexed_args[i];
    }

    const format_arg_type& get(string_view_type key) const override
    {
        auto it = m_named_args.find(key);
        if(it == m_named_args.end())
            this->throw_invalid_named_argument();
        return it->second;
    }

    using base::get;

    bool check(string_view_type key) const noexcept override
    {
        return m_named_args.contains(key);
    }

    using base::check;

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
    using vector_type = std::vector<
        format_arg_type>;
    using map_type = std::map<
        string_type,
        format_arg_type,
        std::less<>>;

    vector_type m_indexed_args;
    map_type m_named_args;
};

using mutable_format_args = basic_mutable_format_args<format_context, char>;

// Type-erased format arguments.
template <typename Context, typename CharT = typename Context::char_type>
class dynamic_format_args final : public detail::format_args_base<Context, CharT>
{
    using base = detail::format_args_base<Context, CharT>;

public:
    using char_type = CharT;
    using string_view_type = std::basic_string_view<char_type>;
    using size_type = std::size_t;
    using format_arg_type = basic_format_arg<Context>;

    dynamic_format_args() = delete;
    constexpr dynamic_format_args(const dynamic_format_args&) noexcept = default;

    template <std::derived_from<base> T>
    constexpr dynamic_format_args(const T& args) noexcept
        : m_ptr(&args)
    {
        PAPILIO_ASSERT(m_ptr != this); // avoid circular reference
    }

    const format_arg_type& get(size_type i) const override
    {
        return m_ptr->get(i);
    }

    const format_arg_type& get(string_view_type k) const override
    {
        return m_ptr->get(k);
    }

    using base::get;

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

    using base::check;

    // WARNING: This function does not perform any runtime checks!
    template <std::derived_from<base> T>
    [[nodiscard]]
    constexpr const T& cast_to() const noexcept
    {
        PAPILIO_ASSERT(dynamic_cast<const T*>(m_ptr));
        return *static_cast<const T*>(m_ptr);
    }

private:
    const base* m_ptr;
};

template <typename Context = format_context, typename... Args>
auto make_format_args(Args&&... args)
{
    static_assert(
        std::conjunction_v<std::negation<std::is_base_of<detail::format_args_base<Context, char>, Args>>...>,
        "cannot use format_args as format argument"
    );

    using result_type = static_format_args<
        detail::get_indexed_arg_count<Args...>(),
        detail::get_named_arg_count<Args...>(),
        Context,
        char>;
    return result_type(std::forward<Args>(args)...);
}

template <typename Context = wformat_context, typename... Args>
auto make_wformat_args(Args&&... args)
{
    static_assert(
        std::conjunction_v<std::negation<std::is_base_of<detail::format_args_base<Context, wchar_t>, Args>>...>,
        "cannot use format_args as format argument"
    );

    using result_type = static_format_args<
        detail::get_indexed_arg_count<Args...>(),
        detail::get_named_arg_count<Args...>(),
        Context,
        wchar_t>;
    return result_type(std::forward<Args>(args)...);
}

template <typename OutputIt, typename CharT>
class basic_format_context
{
public:
    using char_type = CharT;
    using iterator = OutputIt;
    using format_args_type = dynamic_format_args<basic_format_context, char_type>;

    basic_format_context(iterator it, format_args_type args)
        : m_out(std::move(it)), m_args(args) {}

    basic_format_context(const std::locale& loc, iterator it, format_args_type args)
        : m_loc(loc), m_out(std::move(it)), m_args(args) {}

    basic_format_context(locale_ref loc, iterator it, format_args_type args)
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

template <typename Context>
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
        if constexpr(std::is_same_v<Char, char_type>)
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
        {
            advance_to(ctx, cp.append_to_as<char_type>(out(ctx)));
        }
    }
};

template <typename T, typename CharT = char>
concept formattable = requires() {
    PAPILIO_NS formatter<T, CharT>();
} && std::semiregular<formatter<T, CharT>>;

template <typename FormatContext>
class format_parse_context
{
public:
    using char_type = typename FormatContext::char_type;
    using string_type = std::basic_string<char_type>;
    using string_view_type = std::basic_string_view<char_type>;
    using string_ref_type = utf::basic_string_ref<char_type>;
    using iterator = string_ref_type::const_iterator;
    using size_type = std::size_t;
    using format_context_type = FormatContext;
    using format_args_type = dynamic_format_args<FormatContext, char_type>;

    format_parse_context() = delete;
    format_parse_context(const format_parse_context&) = delete;

    format_parse_context(string_ref_type str, format_args_type args) noexcept
        : m_ref(str), m_args(args)
    {
        m_it = m_ref.begin();
    }

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
        get_args().check(m_default_arg_idx);
        return m_default_arg_idx;
    }

    size_type check_arg_id(size_type i) const
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
        throw invalid_format("no default argument after an explicit argument");
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

} // namespace papilio

#ifdef PAPILIO_COMPILER_MSVC
#    pragma warning(pop)
#endif

#include "core.inl"
