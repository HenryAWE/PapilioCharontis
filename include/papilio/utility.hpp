// concepts, type traits, tags, auxiliary types

#pragma once

#include <iosfwd>
#include <utility>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <concepts>
#include <string>
#include <iterator>
#include "macros.hpp"


namespace papilio
{
    template <typename T>
    concept char_like =
        std::is_same_v<T, char> ||
        std::is_same_v<T, wchar_t> ||
        std::is_same_v<T, char16_t> ||
        std::is_same_v<T, char32_t> ||
        std::is_same_v<T, char8_t>;

    template <typename CharT>
    concept char8_like = std::is_same_v<CharT, char> || std::is_same_v<CharT, char8_t>;
    template <typename CharT>
    concept char16_like =
        std::is_same_v<CharT, char16_t> ||
        (sizeof(wchar_t) == 2 && std::is_same_v<CharT, wchar_t>);
    template <typename CharT>
    concept char32_like =
        std::is_same_v<CharT, char32_t> ||
        (sizeof(wchar_t) == 4 && std::is_same_v<CharT, wchar_t>);

    template <typename T, typename CharT>
    concept basic_string_like =
        std::is_same_v<std::decay_t<T>, CharT*> ||
        std::is_same_v<std::decay_t<T>, const CharT*> ||
        std::is_same_v<T, std::basic_string<CharT>> ||
        std::is_same_v<T, std::basic_string_view<CharT>> ||
        std::is_convertible_v<T, std::basic_string_view<CharT>>;

    template <typename T>
    concept string_like = basic_string_like<T, char>;
    template <typename T>
    concept u8string_like = basic_string_like<T, char8_t>;
    template <typename T>
    concept u16string_like = basic_string_like<T, char16_t>;
    template <typename T>
    concept u32string_like = basic_string_like<T, char32_t>;
    template <typename T>
    concept wstring_like = basic_string_like<T, wchar_t>;

    template <typename T>
    concept pointer_like =
        (requires(T ptr) { *ptr; ptr.operator->(); } || requires(T ptr, std::size_t i) { ptr[i]; })
        && requires(T ptr) { static_cast<bool>(ptr); };

    // ^^^ concepts ^^^ / vvv tags vvv

    struct reverse_index_t {};
    constexpr reverse_index_t reverse_index = {};

    // ^^^ tags ^^^ / vvv auxiliary types vvv

    // Signed size type
    using ssize_t = std::make_signed_t<std::size_t>;

    // [begin, end) range
    // Negative value means reverse index like Python.
    // For example, -1 refers to the last element, and -2 refers to the second to last element.
    class slice : public std::pair<ssize_t, ssize_t>
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
            PAPILIO_ASSERT(len <= std::numeric_limits<index_type>::max());

            if(first < 0)
                first = len + first;

            if(second < 0)
                second = len + second;
            else if(second == npos)
                second = len;
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
            return second - first;
        }
    };

    template <typename CharT, typename T>
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

    template <typename T>
    using named_arg = basic_named_arg<char, T>;
    template <typename T>
    using wnamed_arg = basic_named_arg<wchar_t, T>;

    namespace detail
    {
        template <typename T>
        concept is_named_arg_helper = requires() { typename T::named_arg_tag; };
    }

    template <typename T>
    struct is_named_arg : public std::bool_constant<detail::is_named_arg_helper<T>> {};

    template <typename T>
    constexpr inline bool is_named_arg_v = is_named_arg<T>::value;

    template <typename CharT, typename T>
    constexpr auto arg(const CharT* name, T&& value) noexcept
    {
        return basic_named_arg<CharT, std::remove_reference_t<T>>(name, value);
    }
    template <typename CharT, typename T>
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

            template <typename T>
            [[nodiscard]]
            constexpr auto operator=(T&& value) noexcept
            {
                return PAPILIO_NS arg(name, std::forward<T>(value));
            }
        };
    }

    inline namespace literals
    {
        inline namespace named_arg_literals
        {
            constexpr auto operator""_a(const char* name, std::size_t size) noexcept
            {
                return PAPILIO_NS detail::named_arg_proxy<char>(name, size);
            }

            constexpr auto operator""_a(const wchar_t* name, std::size_t size) noexcept
            {
                return PAPILIO_NS detail::named_arg_proxy<wchar_t>(name, size);
            }
        }
    }

    template <typename T>
    struct independent_proxy : public std::reference_wrapper<T> {};

    struct independent_t
    {
        template <typename T>
        using proxy = independent_proxy<T>;

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
    };

    inline constexpr independent_t independent{};

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
            constexpr compressed_pair_impl(const T1& v1, const T2 v2)
                : m_first(v1), m_second(v2) {}
            template <typename U1, typename U2>
            constexpr compressed_pair_impl(U1&& v1, U2&& v2)
                : m_first(std::forward<U1>(v1)), m_second(std::forward<U2>(v2)) {}

            [[nodiscard]]
            constexpr first_type& first() noexcept { return m_first; }
            [[nodiscard]]
            constexpr const first_type& first() const noexcept { return m_first; }
            [[nodiscard]]
            constexpr second_type& second() noexcept { return m_second; }
            [[nodiscard]]
            constexpr const second_type& second() const noexcept { return m_second; }

            constexpr void swap(compressed_pair_impl& other)
                noexcept(std::is_nothrow_swappable_v<T1>&& std::is_nothrow_swappable_v<T2>)
            {
                using std::swap;
                swap(m_first, other.first());
                swap(m_second, other.second());
            }

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
            constexpr compressed_pair_impl(const T1& v1, const T2 v2)
                : T1(v1), m_second(v2) {}
            template <typename U1, typename U2>
            constexpr compressed_pair_impl(U1&& v1, U2&& v2)
                : T1(std::forward<U1>(v1)), m_second(std::forward<U2>(v2)) {}

            [[nodiscard]]
            constexpr first_type& first() noexcept { return *this; }
            [[nodiscard]]
            constexpr const first_type& first() const noexcept { return *this; }
            [[nodiscard]]
            constexpr second_type& second() noexcept { return m_second; }
            [[nodiscard]]
            constexpr const second_type& second() const noexcept { return m_second; }

            constexpr void swap(compressed_pair_impl& other)
                noexcept(std::is_nothrow_swappable_v<T2>)
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
            constexpr compressed_pair_impl(const T1& v1, const T2 v2)
                : T2(v2), m_first(v1) {}
            template <typename U1, typename U2>
            constexpr compressed_pair_impl(U1&& v1, U2&& v2)
                : T2(std::forward<U1>(v2)), m_first(std::forward<U2>(v1)) {}

            [[nodiscard]]
            constexpr first_type& first() noexcept { return m_first; }
            [[nodiscard]]
            constexpr const first_type& first() const noexcept { return m_first; }
            [[nodiscard]]
            constexpr second_type& second() noexcept { return *this; }
            [[nodiscard]]
            constexpr const second_type& second() const noexcept { return *this; }

            constexpr void swap(compressed_pair_impl& other)
                noexcept(std::is_nothrow_swappable_v<T1>)
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
            constexpr compressed_pair_impl(const T1& v1, const T2 v2)
                : T1(v1), T2(v2) {}
            template <typename U1, typename U2>
            constexpr compressed_pair_impl(U1&& v1, U2&& v2)
                : T1(std::forward<U1>(v1)), T2(std::forward<U2>(v2)) {}

            [[nodiscard]]
            constexpr first_type& first() noexcept { return *this; }
            [[nodiscard]]
            constexpr const first_type& first() const noexcept { return *this; }
            [[nodiscard]]
            constexpr second_type& second() noexcept { return *this; }
            [[nodiscard]]
            constexpr const second_type& second() const noexcept { return *this; }

            constexpr void swap(compressed_pair_impl& other) noexcept
            {
                // empty
            }
        };
    }

    template <typename T1, typename T2>
    class compressed_pair :
        public detail::compressed_pair_impl<T1, T2, detail::get_cp_impl_id<T1, T2>()>
    {
        using base = detail::compressed_pair_impl<T1, T2, detail::get_cp_impl_id<T1, T2>()>;
    public:
        using base::base;
    };

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
        class basic_iterbuf_base<false, CharT> : public std::basic_streambuf<CharT> {};
    }

    // output iterator stream buffer
    template <typename CharT, std::input_or_output_iterator Iterator>
    class basic_iterbuf :
        public detail::basic_iterbuf_base<std::input_iterator<Iterator>, CharT>
    {
        using base = detail::basic_iterbuf_base<std::input_iterator<Iterator>, CharT>;
    public:
        using char_type = CharT;
        using iterator = Iterator;
        using int_type = base::int_type;
        using traits_type = base::traits_type;

        basic_iterbuf() = default;
        basic_iterbuf(Iterator iter) noexcept(std::is_nothrow_move_constructible_v<Iterator>)
            : basic_iterbuf(std::in_place, std::move(iter)) {}
        template <typename... Args>
        basic_iterbuf(std::in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<Iterator, Args...>)
            : m_iter(std::forward<Args>(args)...) {}

        [[nodiscard]]
        Iterator get() const
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
                return base::underflow();
            }
        }

        int_type overflow(int_type c) override
        {
            if constexpr(std::output_iterator<Iterator, CharT>)
            {
                *m_iter = c;
                ++m_iter;
                return c;
            }
            else
            {
                return base::overflow(c);
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

    template <std::input_or_output_iterator Iterator>
    using iterbuf = basic_iterbuf<char, Iterator>;
    template <std::input_or_output_iterator Iterator>
    using witerbuf = basic_iterbuf<wchar_t, Iterator>;

    template <typename CharT, std::output_iterator<CharT> Iterator>
    class basic_oiterstream : public std::basic_ostream<CharT>
    {
        using base = std::basic_ostream<CharT>;
    public:
        using iterator = Iterator;

        basic_oiterstream() = default;
        basic_oiterstream(Iterator iter)
            : base(&m_buf), m_buf(std::move(iter)) {}
        template <typename... Args>
        basic_oiterstream(std::in_place_t, Args&&... args)
            : base(&m_buf), m_buf(std::in_place, std::forward<Args>(args)...) {}

        [[nodiscard]]
        Iterator get() const
        {
            return m_buf.get();
        }

    private:
        basic_iterbuf<CharT, Iterator> m_buf;
    };

    template <std::output_iterator<char> Iterator>
    using oiterstream = basic_oiterstream<char, Iterator>;
    template <std::output_iterator<wchar_t> Iterator>
    using woiterstream = basic_oiterstream<wchar_t, Iterator>;
}
