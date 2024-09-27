/**
 * @file utility.hpp
 * @author HenryAWE
 * @brief Header of concepts, type traits, tags, and auxiliary types.
 */

#ifndef PAPILIO_UTILITY_HPP
#define PAPILIO_UTILITY_HPP

#pragma once

#include <utility>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <concepts>
#include <string>
#include <array>
#include <iterator>
#include <iostream>
#include "macros.hpp"
#include "detail/compat.hpp"
#include "detail/prefix.hpp"

namespace papilio
{
/// @defgroup Utility Utilities
/// @{

PAPILIO_EXPORT template <typename T>
concept char_like =
    std::is_same_v<std::remove_cv_t<T>, char> ||
    std::is_same_v<std::remove_cv_t<T>, wchar_t> ||
    std::is_same_v<std::remove_cv_t<T>, char16_t> ||
    std::is_same_v<std::remove_cv_t<T>, char32_t> ||
    std::is_same_v<std::remove_cv_t<T>, char8_t>;

PAPILIO_EXPORT template <typename CharT>
concept char8_like = char_like<CharT> && sizeof(CharT) == 1;
PAPILIO_EXPORT template <typename CharT>
concept char16_like = char_like<CharT> && sizeof(CharT) == 2;
PAPILIO_EXPORT template <typename CharT>
concept char32_like = char_like<CharT> && sizeof(CharT) == 4;

PAPILIO_EXPORT template <typename T, typename CharT>
concept basic_string_like =
    std::is_same_v<std::decay_t<T>, CharT*> ||
    std::is_same_v<std::decay_t<T>, const CharT*> ||
    std::is_same_v<std::remove_cv_t<T>, std::basic_string<CharT>> ||
    std::is_same_v<std::remove_cv_t<T>, std::basic_string_view<CharT>> ||
    std::is_convertible_v<T, std::basic_string_view<CharT>>;

PAPILIO_EXPORT template <typename T>
concept string_like = basic_string_like<T, char>;
PAPILIO_EXPORT template <typename T>
concept u8string_like = basic_string_like<T, char8_t>;
PAPILIO_EXPORT template <typename T>
concept u16string_like = basic_string_like<T, char16_t>;
PAPILIO_EXPORT template <typename T>
concept u32string_like = basic_string_like<T, char32_t>;
PAPILIO_EXPORT template <typename T>
concept wstring_like = basic_string_like<T, wchar_t>;

PAPILIO_EXPORT template <typename T>
concept pointer_like = requires(T ptr) { static_cast<bool>(ptr); } &&
                       (requires(T ptr) { *ptr; ptr.operator->(); } || requires(T ptr, std::size_t i) { ptr[i]; });

/**
 * @brief Get the raw pointer from a pointer-like object, e.g., `std::shared_ptr`.
 *
 * @param p The pointer-like object
 * @return The raw pointer
 */
PAPILIO_EXPORT template <pointer_like T>
constexpr const void* ptr(const T& p) noexcept
{
    return std::to_address(p);
}

namespace detail
{
    template <typename Tuple, std::size_t... Is>
    consteval bool check_tuple_like(std::index_sequence<Is...>)
    {
        return (requires(Tuple tp) {
            typename std::tuple_element_t<Is, Tuple>;
            get<Is>(tp);
        } && ...);
    }

    template <typename T>
    concept tuple_like_helper =
        requires() { typename std::tuple_size<T>; } &&
        check_tuple_like<T>(std::make_index_sequence<std::tuple_size<T>::value>());

    template <typename T>
    concept pair_like_helper =
        tuple_like_helper<T> &&
        std::tuple_size<T>::value == 2;
} // namespace detail

PAPILIO_EXPORT template <typename T>
concept tuple_like = detail::tuple_like_helper<std::remove_cvref_t<T>>;
PAPILIO_EXPORT template <typename T>
concept pair_like = detail::pair_like_helper<std::remove_cvref_t<T>>;

namespace detail
{
    template <typename MapType, typename Key, typename T>
    concept map_like_impl =
        requires(MapType m, const MapType cm, const Key& k) {
            typename MapType::key_compare;
            {
                m.find(k)
            } -> std::convertible_to<typename MapType::iterator>;
            {
                m.end()
            } -> std::equality_comparable_with<typename MapType::iterator>;
            {
                cm.find(k)
            } -> std::convertible_to<typename MapType::const_iterator>;
            {
                cm.end()
            } -> std::equality_comparable_with<typename MapType::const_iterator>;
        };
} // namespace detail

PAPILIO_EXPORT template <typename MapType>
concept map_like = detail::map_like_impl<
    std::remove_cv_t<MapType>,
    typename MapType::key_type,
    typename MapType::mapped_type>;

// ^^^ concepts ^^^ / vvv tags vvv

PAPILIO_EXPORT struct reverse_index_t
{};

PAPILIO_EXPORT constexpr reverse_index_t reverse_index = {};

// ^^^ tags ^^^ / vvv auxiliary types vvv

/**
 * @brief Signed size type
 */
PAPILIO_EXPORT using ssize_t = std::make_signed_t<std::size_t>;

// [begin, end) range
// Negative value means reverse index like Python.
// For example, -1 refers to the last element, and -2 refers to the second to last element.
PAPILIO_EXPORT class slice : public std::pair<ssize_t, ssize_t>
{
public:
    using size_type = std::size_t;
    using index_type = ssize_t;

    static constexpr index_type npos = std::numeric_limits<index_type>::max();

    constexpr slice() noexcept
        : slice(0, npos) {}

    constexpr slice(const slice&) noexcept = default;

    constexpr explicit slice(index_type start, index_type stop = npos) noexcept
        : pair(start, stop) {}

    constexpr slice& operator=(const slice&) noexcept = default;

    constexpr bool operator==(const slice&) const noexcept = default;

    constexpr void normalize(std::in_place_t, size_type len) noexcept
    {
        PAPILIO_ASSERT(len <= static_cast<size_type>(std::numeric_limits<index_type>::max()));
        index_type slen = static_cast<index_type>(len);

        if(first < 0)
            first = slen + first;

        if(second < 0)
            second = slen + second;
        else if(second == npos)
            second = slen;
    }

    [[nodiscard]]
    constexpr slice normalize(size_type len) const noexcept
    {
        slice result = *this;
        result.normalize(std::in_place, len);
        return result;
    }

    [[nodiscard]]
    constexpr index_type begin() const noexcept
    {
        return first;
    }

    [[nodiscard]]
    constexpr index_type end() const noexcept
    {
        return second;
    }

    [[nodiscard]]
    constexpr size_type length() const noexcept
    {
        PAPILIO_ASSERT(first > 0);
        PAPILIO_ASSERT(second > 0);
        PAPILIO_ASSERT(second != npos);
        return static_cast<size_type>(second - first);
    }
};

PAPILIO_EXPORT template <typename CharT, typename T>
struct basic_named_arg
{
    static_assert(!std::is_reference_v<T>, "T cannot be a reference");

    using named_arg_tag = void;

    using char_type = CharT;
    using string_view_type = std::basic_string_view<CharT>;
    using value_type = T;
    using reference = std::add_lvalue_reference_t<T>;

    string_view_type name;
    reference value;

    basic_named_arg() = delete;
    constexpr basic_named_arg(const basic_named_arg&) noexcept = default;

    constexpr basic_named_arg(string_view_type arg_name, reference arg_value) noexcept
        : name(arg_name), value(arg_value) {}

    basic_named_arg& operator=(const basic_named_arg&) = delete;

    [[nodiscard]]
    constexpr reference get() const noexcept
    {
        return value;
    }

    constexpr operator reference() const noexcept
    {
        return value;
    }
};

PAPILIO_EXPORT template <typename T>
using named_arg = basic_named_arg<char, T>;
PAPILIO_EXPORT template <typename T>
using wnamed_arg = basic_named_arg<wchar_t, T>;

namespace detail
{
    template <typename T>
    concept is_named_arg_helper = requires() { typename T::named_arg_tag; };
} // namespace detail

PAPILIO_EXPORT template <typename T>
struct is_named_arg : public std::bool_constant<detail::is_named_arg_helper<T>>
{};

PAPILIO_EXPORT template <typename T>
constexpr inline bool is_named_arg_v = is_named_arg<T>::value;

PAPILIO_EXPORT template <typename CharT, typename T>
constexpr auto arg(const CharT* name, T&& value) noexcept
{
    return basic_named_arg<CharT, std::remove_reference_t<T>>(name, value);
}

PAPILIO_EXPORT template <typename CharT, typename T>
constexpr auto arg(const std::basic_string_view<CharT> name, T&& value) noexcept
{
    return basic_named_arg<CharT, std::remove_reference_t<T>>(name, value);
}

namespace detail
{
    template <typename CharT>
    struct named_arg_proxy
    {
        using string_view_type = std::basic_string_view<CharT>;

        string_view_type name;

        named_arg_proxy() = delete;
        named_arg_proxy(const named_arg_proxy&) = delete;

        constexpr named_arg_proxy(const CharT* arg_name, std::size_t size) noexcept
            : name(arg_name, size) {}

        named_arg_proxy& operator=(const named_arg_proxy&) = delete;

        // clang-format off

        template <typename T>
        [[nodiscard]]
        constexpr auto operator=(T&& value) noexcept
        {
            return PAPILIO_NS arg(name, std::forward<T>(value));
        }

        // clang-format on
    };
} // namespace detail

inline namespace literals
{
    inline namespace named_arg_literals
    {
        PAPILIO_EXPORT constexpr auto operator""_a(
            const char* name, std::size_t size
        ) noexcept
        {
            return PAPILIO_NS detail::named_arg_proxy<char>(name, size);
        }

        PAPILIO_EXPORT constexpr auto operator""_a(
            const wchar_t* name, std::size_t size
        ) noexcept
        {
            return PAPILIO_NS detail::named_arg_proxy<wchar_t>(name, size);
        }
    } // namespace named_arg_literals
} // namespace literals

PAPILIO_EXPORT template <typename T>
struct independent_proxy : public std::reference_wrapper<T>
{
    using std::reference_wrapper<T>::reference_wrapper;
};

PAPILIO_EXPORT struct independent_t
{
    template <typename T>
    using proxy = independent_proxy<T>;

    // clang-format off

    template <typename T>
    [[nodiscard]]
    constexpr proxy<T> operator()(T& v) const noexcept
    {
        return proxy<T>(v);
    }

    template <typename T>
    [[nodiscard]]
    constexpr proxy<T> operator()(proxy<T> v) const noexcept
    {
        return v;
    }

    template <typename T>
    [[nodiscard]]
    constexpr proxy<const T> operator()(const T& v) const noexcept
    {
        return proxy<const T>(v);
    }

    // clang-format on
};

PAPILIO_EXPORT inline constexpr independent_t independent{};

namespace detail
{
    template <typename T>
    concept cp_is_empty = !std::is_final_v<T> && std::is_empty_v<T>;

    template <typename T1, typename T2>
    inline consteval int get_cp_impl_id()
    {
        using namespace std;
        if constexpr(!cp_is_empty<T1> && !cp_is_empty<T2>)
            return 0; // normal pair
        else if constexpr(cp_is_empty<T1> && !cp_is_empty<T2>)
            return 1; // only T1 is empty
        else if constexpr(!cp_is_empty<T1> && cp_is_empty<T2>)
            return 2; // only T2 is empty
        else
        {
            // both are empty
            if constexpr(!is_same_v<T1, T2>)
                return 3; // T1 != T2
            else
                return 1; // T1 == T2, fallback to implementation for only T1 is empty
        }
    }

    template <typename T1, typename T2, int Id>
    class compressed_pair_impl;

#ifdef PAPILIO_COMPILER_MSVC
#    pragma warning(push)
#    pragma warning(disable : 26495)
#endif

    // normal pair
    template <typename T1, typename T2>
    class compressed_pair_impl<T1, T2, 0>
    {
    public:
        using first_type = T1;
        using second_type = T2;

        constexpr compressed_pair_impl() = default;
        constexpr compressed_pair_impl(const compressed_pair_impl&) = default;
        constexpr compressed_pair_impl(compressed_pair_impl&&) = default;

        constexpr compressed_pair_impl(const T1& v1, const T2& v2)
            : m_first(v1), m_second(v2) {}

        template <typename U1, typename U2>
        constexpr compressed_pair_impl(U1&& v1, U2&& v2)
            : m_first(std::forward<U1>(v1)), m_second(std::forward<U2>(v2))
        {}


    protected:
        template <typename Tuple1, typename Tuple2, std::size_t... Indices1, std::size_t... Indices2>
        constexpr compressed_pair_impl(
            Tuple1& args1, Tuple2& args2, std::index_sequence<Indices1...>, std::index_sequence<Indices2...>
        )
            : m_first(std::get<Indices1>(std::move(args1))...), m_second(std::get<Indices2>(std::move(args2))...)
        {}

    public:
        [[nodiscard]]
        constexpr first_type& first() noexcept
        {
            return m_first;
        }

        [[nodiscard]]
        constexpr const first_type& first() const noexcept
        {
            return m_first;
        }

        [[nodiscard]]
        constexpr second_type& second() noexcept
        {
            return m_second;
        }

        [[nodiscard]]
        constexpr const second_type& second() const noexcept
        {
            return m_second;
        }

        // clang-format off

        constexpr void swap(
            compressed_pair_impl& other
        ) noexcept(std::is_nothrow_swappable_v<T1> && std::is_nothrow_swappable_v<T2>)
        {
            using std::swap;
            swap(m_first, other.first());
            swap(m_second, other.second());
        }

        // clang-format on

    private:
        first_type m_first;
        second_type m_second;
    };

    // T1 is empty
    template <typename T1, typename T2>
    class compressed_pair_impl<T1, T2, 1> : private std::remove_cv_t<T1>
    {
    public:
        using first_type = T1;
        using second_type = T2;

        constexpr compressed_pair_impl() = default;
        constexpr compressed_pair_impl(const compressed_pair_impl&) = default;
        constexpr compressed_pair_impl(compressed_pair_impl&&) = default;

        constexpr compressed_pair_impl(const T1& v1, const T2& v2)
            : T1(v1), m_second(v2) {}

        template <typename U1, typename U2>
        constexpr compressed_pair_impl(U1&& v1, U2&& v2)
            : T1(std::forward<U1>(v1)), m_second(std::forward<U2>(v2))
        {}


    protected:
        template <typename Tuple1, typename Tuple2, std::size_t... Indices1, std::size_t... Indices2>
        constexpr compressed_pair_impl(
            Tuple1& args1, Tuple2 args2, std::index_sequence<Indices1...>, std::index_sequence<Indices2...>
        )
            : T1(std::get<Indices1>(std::move(args1))...), m_second(std::get<Indices2>(std::move(args2))...)
        {}

    public:
        [[nodiscard]]
        constexpr first_type& first() noexcept
        {
            return *this;
        }

        [[nodiscard]]
        constexpr const first_type& first() const noexcept
        {
            return *this;
        }

        [[nodiscard]]
        constexpr second_type& second() noexcept
        {
            return m_second;
        }

        [[nodiscard]]
        constexpr const second_type& second() const noexcept
        {
            return m_second;
        }

        constexpr void swap(compressed_pair_impl& other) noexcept(std::is_nothrow_swappable_v<T2>)
        {
            using std::swap;
            swap(m_second, other.second());
        }

    private:
        second_type m_second;
    };

    // T2 is empty
    template <typename T1, typename T2>
    class compressed_pair_impl<T1, T2, 2> : private std::remove_cv_t<T2>
    {
    public:
        using first_type = T1;
        using second_type = T2;

        constexpr compressed_pair_impl() = default;
        constexpr compressed_pair_impl(const compressed_pair_impl&) = default;
        constexpr compressed_pair_impl(compressed_pair_impl&&) = default;

        constexpr compressed_pair_impl(const T1& v1, const T2& v2)
            : T2(v2), m_first(v1) {}

        template <typename U1, typename U2>
        constexpr compressed_pair_impl(U1&& v1, U2&& v2)
            : T2(std::forward<U1>(v2)), m_first(std::forward<U2>(v1))
        {}


    protected:
        template <typename Tuple1, typename Tuple2, std::size_t... Indices1, std::size_t... Indices2>
        constexpr compressed_pair_impl(
            Tuple1& args1, Tuple2& args2, std::index_sequence<Indices1...>, std::index_sequence<Indices2...>
        )
            : T2(std::get<Indices2>(std::move(args2))...), m_first(std::get<Indices1>(std::move(args1))...)
        {}

    public:
        [[nodiscard]]
        constexpr first_type& first() noexcept
        {
            return m_first;
        }

        [[nodiscard]]
        constexpr const first_type& first() const noexcept
        {
            return m_first;
        }

        [[nodiscard]]
        constexpr second_type& second() noexcept
        {
            return *this;
        }

        [[nodiscard]]
        constexpr const second_type& second() const noexcept
        {
            return *this;
        }

        constexpr void swap(compressed_pair_impl& other) noexcept(std::is_nothrow_swappable_v<T1>)
        {
            using std::swap;
            swap(m_first, other.first());
        }

    private:
        first_type m_first;
    };

    // T1 != T2, both are empty
    template <typename T1, typename T2>
    class compressed_pair_impl<T1, T2, 3> : private std::remove_cv_t<T1>, private std::remove_cv_t<T2>
    {
    public:
        using first_type = T1;
        using second_type = T2;

        constexpr compressed_pair_impl() = default;
        constexpr compressed_pair_impl(const compressed_pair_impl&) = default;
        constexpr compressed_pair_impl(compressed_pair_impl&&) = default;

        constexpr compressed_pair_impl(const T1& v1, const T2& v2)
            : T1(v1), T2(v2) {}

        template <typename U1, typename U2>
        constexpr compressed_pair_impl(U1&& v1, U2&& v2)
            : T1(std::forward<U1>(v1)), T2(std::forward<U2>(v2))
        {}

    protected:
        template <typename Tuple1, typename Tuple2, std::size_t... Indices1, std::size_t... Indices2>
        constexpr compressed_pair_impl(
            Tuple1& args1, Tuple2& args2, std::index_sequence<Indices1...>, std::index_sequence<Indices2...>
        )
            : T1(std::get<Indices1>(std::move(args1))...), T2(std::get<Indices2>(std::move(args2))...)
        {}

    public:
        [[nodiscard]]
        constexpr first_type& first() noexcept
        {
            return *this;
        }

        [[nodiscard]]
        constexpr const first_type& first() const noexcept
        {
            return *this;
        }

        [[nodiscard]]
        constexpr second_type& second() noexcept
        {
            return *this;
        }

        [[nodiscard]]
        constexpr const second_type& second() const noexcept
        {
            return *this;
        }

        constexpr void swap(compressed_pair_impl&) noexcept
        {
            // empty
        }
    };

#ifdef PAPILIO_COMPILER_MSVC
#    pragma warning(pop)
#endif
} // namespace detail

/**
 * @brief Compressed pair optimized for empty types.
 *
 * @tparam T1 First type
 * @tparam T2 Second type
 */
PAPILIO_EXPORT template <typename T1, typename T2>
class compressed_pair :
    public detail::compressed_pair_impl<T1, T2, detail::get_cp_impl_id<T1, T2>()>
{
    using my_base = detail::compressed_pair_impl<T1, T2, detail::get_cp_impl_id<T1, T2>()>;

public:
    using first_type = T1;
    using second_type = T2;

    constexpr compressed_pair() = default;
    constexpr compressed_pair(const compressed_pair&) = default;
    constexpr compressed_pair(compressed_pair&&) = default;

    constexpr compressed_pair(const T1& v1, const T2& v2)
        : my_base(v1, v2) {}

    template <typename U1, typename U2>
    constexpr compressed_pair(U1&& v1, U2&& v2)
        : my_base(std::forward<U1>(v1), std::forward<U2>(v2))
    {}

    template <typename... Args1, typename... Args2>
    constexpr compressed_pair(std::piecewise_construct_t, std::tuple<Args1...> args1, std::tuple<Args2...> args2)
        : my_base(
              args1,
              args2,
              std::make_index_sequence<sizeof...(Args1)>(),
              std::make_index_sequence<sizeof...(Args2)>()
          )
    {}

    template <std::size_t Index>
    requires(Index < 2)
    friend decltype(auto) get(compressed_pair& cp)
    {
        if constexpr(Index == 0)
            return cp.first();
        else
            return cp.second();
    }

    template <std::size_t Index>
    requires(Index < 2)
    friend decltype(auto) get(compressed_pair&& cp)
    {
        if constexpr(Index == 0)
            return cp.first();
        else
            return cp.second();
    }

    template <std::size_t Index>
    requires(Index < 2)
    friend decltype(auto) get(const compressed_pair& cp)
    {
        if constexpr(Index == 0)
            return cp.first();
        else
            return cp.second();
    }

    // The specializations of tuple_element and tuple_size are at the end of this file.
};

/// @defgroup IterStream I/O stream wrapper for input/output iterator
/// @{

namespace detail
{
    template <bool EnableBuf, typename CharT>
    struct basic_iterbuf_base : public std::basic_streambuf<CharT>
    {
        [[nodiscard]]
        CharT* gbuf_ptr() noexcept
        {
            return &m_buf_ch;
        }

    private:
        CharT m_buf_ch = static_cast<CharT>(0);
    };

    template <typename CharT>
    class basic_iterbuf_base<false, CharT> : public std::basic_streambuf<CharT>
    {
        // empty
    };
} // namespace detail

/**
 * @brief Stream buffer wrapper for input/output iterator.
 *
 * @tparam CharT Character type
 * @tparam Iterator Input or output iterator
 */
PAPILIO_EXPORT template <
    typename CharT,
    std::input_or_output_iterator Iterator>
class basic_iterbuf :
    public detail::basic_iterbuf_base<std::input_iterator<Iterator>, CharT>
{
    using my_base = detail::basic_iterbuf_base<std::input_iterator<Iterator>, CharT>;

public:
    using char_type = CharT;
    using iterator = Iterator;
    using int_type = typename my_base::int_type;
    using traits_type = typename my_base::traits_type;

    basic_iterbuf() = default;

    basic_iterbuf(Iterator iter) noexcept(std::is_nothrow_move_constructible_v<Iterator>)
        : basic_iterbuf(std::in_place, std::move(iter)) {}

    template <typename... Args>
    basic_iterbuf(std::in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<Iterator, Args...>)
        : m_iter(std::forward<Args>(args)...)
    {}

    [[nodiscard]]
    Iterator base() const
    {
        return m_iter;
    }

protected:
    int_type underflow() override
    {
        if constexpr(std::input_iterator<Iterator>)
        {
            CharT c = *m_iter;
            ++m_iter;

            *this->gbuf_ptr() = c;
            intput_setg();
            return traits_type::to_int_type(c);
        }
        else
        {
            return my_base::underflow();
        }
    }

    int_type overflow(int_type c) override
    {
        if constexpr(std::output_iterator<Iterator, CharT>)
        {
            *m_iter = static_cast<CharT>(c);
            ++m_iter;
            return c;
        }
        else
        {
            return my_base::overflow(c);
        }
    }

private:
    Iterator m_iter;

    void intput_setg()
    {
        if constexpr(std::input_iterator<Iterator>)
        {
            CharT* ptr = this->gbuf_ptr();
            this->setg(ptr, ptr, ptr + 1);
        }
    }
};

PAPILIO_EXPORT template <std::input_or_output_iterator Iterator>
using iterbuf = basic_iterbuf<char, Iterator>;
PAPILIO_EXPORT template <std::input_or_output_iterator Iterator>
using witerbuf = basic_iterbuf<wchar_t, Iterator>;

PAPILIO_EXPORT template <
    typename CharT,
    std::output_iterator<CharT> Iterator>
class basic_oiterstream : public std::basic_ostream<CharT>
{
    using my_base = std::basic_ostream<CharT>;

public:
    using iterator = Iterator;

    basic_oiterstream() = default;

    basic_oiterstream(Iterator iter)
        : my_base(&m_buf), m_buf(std::move(iter)) {}

    [[nodiscard]]
    Iterator base() const
    {
        return m_buf.base();
    }

private:
    basic_iterbuf<CharT, Iterator> m_buf;
};

PAPILIO_EXPORT template <std::output_iterator<char> Iterator>
using oiterstream = basic_oiterstream<char, Iterator>;
PAPILIO_EXPORT template <std::output_iterator<wchar_t> Iterator>
using woiterstream = basic_oiterstream<wchar_t, Iterator>;

/// @}

/// @defgroup MagicEnum Utilities for enumerations
/// @brief If the macro `PAPILIO_HAS_ENUM_NAME` is defined, it means current compiler has sufficient extension to
///        obtain the string representation of static enum value.
///
/// These functions have limitations due to C++ lacking of reflection.
/// The content of macro `PAPILIO_HAS_ENUM_NAME` is how the string representation is obtained.
/// - For MSVC, it is `__FUNCSIG__`.
/// - For GCC / Clang, it is `__PRETTY_FUNCTION__` of corresponding format.
/// - Otherwise, this macro is not defined, which means current compiler does not have required extensions.
/// @{

namespace detail
{
    template <auto Value>
    constexpr std::string_view static_enum_name_impl()
    {
        std::string_view name;

#if defined PAPILIO_COMPILER_GCC || defined PAPILIO_COMPILER_CLANG
        name = __PRETTY_FUNCTION__;

        std::size_t start = name.find("Value = ") + 8;

#    ifdef PAPILIO_COMPILER_CLANG
#        define PAPILIO_HAS_ENUM_NAME "__PRETTY_FUNCTION__ (Clang)"

        std::size_t end = name.find_last_of(']');
#    else // GCC
#        define PAPILIO_HAS_ENUM_NAME "__PRETTY_FUNCTION__ (GCC)"

        std::size_t end = std::min(name.find(';', start), name.find_last_of(']'));
#    endif

        return std::string_view(name.data() + start, end - start);

#elif defined PAPILIO_COMPILER_MSVC
#    define PAPILIO_HAS_ENUM_NAME "__FUNCSIG__"

        name = __FUNCSIG__;
        std::size_t start = name.find("static_enum_name_impl<") + 22;
        std::size_t end = name.find_last_of('>');
        return std::string_view(name.data() + start, end - start);

#else
        static_assert(false, "unsupported compiler");

        return name; // placeholder
#endif
    }
} // namespace detail

#if defined PAPILIO_COMPILER_CLANG
#    pragma clang diagnostic push
#    if __clang_major__ >= 16
#        pragma clang diagnostic ignored "-Wenum-constexpr-conversion"
#    endif
#endif

/**
 * @brief Convert an enum value to string at compile time.
 *
 * @tparam Value The enum value
 *
 * @warning This function has some limitations.
 * - For multiple enum with same value, the result will be one of them, depending on the compiler.
 * - For invalid enum value, the result is undefined.
 */
PAPILIO_EXPORT template <auto Value>
constexpr std::string_view static_enum_name()
{
    constexpr std::string_view name = detail::static_enum_name_impl<Value>();

    // Remove qualifier
    std::size_t qual_end = name.rfind("::");
    if(qual_end != std::string_view::npos)
    {
        qual_end += 2; // skip "::"
        return name.substr(qual_end);
    }

    return name;
}

/**
 * @brief Convert an enum value to string at runtime.
 *
 * @param value The enum value.
 *
 * @warning This function has some limitations.
 * - It can only convert valid enum values ranging from -128 to 128.
 *
 * @sa static_enum_name
 */
PAPILIO_EXPORT template <typename T>
requires std::is_enum_v<T>
constexpr std::string_view enum_name(T value) noexcept
{
    auto names = [=]<std::size_t... Is>(std::index_sequence<Is...>)
    {
        return std::array<std::string_view, 256>{
            static_enum_name<static_cast<T>(static_cast<ssize_t>(Is) - 128)>()...
        };
    }(std::make_index_sequence<256>());

    return names[static_cast<std::size_t>(value) + 128];
}

#if defined PAPILIO_COMPILER_CLANG
#    pragma clang diagnostic pop
#endif

/// @}

namespace detail
{
    template <std::size_t Idx, typename Tuple, typename Func>
    constexpr void tuple_call(Tuple&& tp, Func&& func)
    {
        func(get<Idx>(std::forward<Tuple>(tp)));
    }
} // namespace detail

/**
 * @brief `for_each` for tuple-like types.
 *
 * @param tp Tuple-like object
 * @param func Function to apply on each element
 *
 * @code{.cpp}
 * std::tuple<char, int, float> tp{'c', 1, 1.1f};
 * std::stringstream ss;
 *
 * tuple_for_each(
 *     tp,
 *     [&](auto&& v)
 *     { ss << v << " "; }
 * );
 * // ss.str() == "c 1 1.1 "
 * @endcode
 */
PAPILIO_EXPORT template <tuple_like Tuple, typename Func>
constexpr void tuple_for_each(Tuple&& tp, Func&& func)
{
    constexpr std::size_t tp_size = std::tuple_size_v<std::remove_cvref_t<Tuple>>;

    [&]<std::size_t... Is>(std::index_sequence<Is...>)
    {
        (detail::tuple_call<Is>(std::forward<Tuple>(tp), func), ...);
    }(std::make_index_sequence<tp_size>());
}

namespace detail
{
    template <typename CharT>
    class joiner_base
    {
    public:
        using char_type = CharT;
        using string_view_type = std::basic_string_view<CharT>;

        inline static const string_view_type default_sep =
            PAPILIO_TSTRING_VIEW(CharT, ", ");
    };
} // namespace detail

/// @defgroup Join String joiner
/// @{

PAPILIO_EXPORT template <std::ranges::range R, typename CharT = char>
class joiner : public detail::joiner_base<CharT>
{
    using my_base = detail::joiner_base<CharT>;

public:
    using char_type = CharT;
    using string_view_type = std::basic_string_view<CharT>;
    using range_type = R;

    joiner() = delete;

    joiner(const joiner&) noexcept = default;

    explicit joiner(R& rng, string_view_type sep = my_base::default_sep)
        : m_p_rng(std::addressof(rng)), m_sep(sep) {}

    friend std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const joiner& j)
    {
        bool first = true;

        for(auto&& i : *j.m_p_rng)
        {
            if(!first)
            {
                os << j.m_sep;
            }
            first = false;

            os << i;
        }

        return os;
    }

    auto begin() const
    {
        return std::ranges::begin(*m_p_rng);
    }

    auto end() const
    {
        return std::ranges::end(*m_p_rng);
    }

    [[nodiscard]]
    string_view_type separator() const noexcept
    {
        return m_sep;
    }

private:
    R* m_p_rng;
    string_view_type m_sep;
};

PAPILIO_EXPORT template <typename CharT = char, std::ranges::range R>
auto join(R& rng)
{
    return joiner<R, CharT>(rng);
}

PAPILIO_EXPORT template <typename CharT = char, std::ranges::range R>
auto join(R& rng, std::basic_string_view<CharT> sep)
{
    return joiner<R, CharT>(rng, sep);
}

PAPILIO_EXPORT template <typename CharT = char, std::ranges::range R>
auto join(R& rng, const CharT* sep)
{
    return joiner<R, CharT>(rng, sep);
}

/// @}

/// @}
} // namespace papilio

namespace std
{
template <typename T1, typename T2>
struct tuple_element<0, ::papilio::compressed_pair<T1, T2>>
{
    using type = T1;
};

template <typename T1, typename T2>
struct tuple_element<1, ::papilio::compressed_pair<T1, T2>>
{
    using type = T2;
};

template <typename T1, typename T2>
struct tuple_size<::papilio::compressed_pair<T1, T2>> :
    integral_constant<size_t, 2>
{};
} // namespace std

#include "detail/suffix.hpp"

#endif
