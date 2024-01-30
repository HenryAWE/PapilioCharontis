#pragma once

#include <iosfwd>
#include <string>
#include <algorithm>
#include <variant>
#include "stralgo.hpp"
#include "codepoint.hpp"
#include "../memory.hpp"

namespace papilio::utf
{
PAPILIO_EXPORT template <typename CharT>
class codepoint_iterator;

PAPILIO_EXPORT template <typename CharT>
class basic_string_ref;
PAPILIO_EXPORT template <typename CharT>
class basic_string_container;

namespace detail
{
    class cp_iter_impl_base
    {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
    };

    template <typename CharT>
    class cp_iter_impl;

    template <char8_like CharT>
    class cp_iter_impl<CharT> : public cp_iter_impl_base
    {
    public:
        using char_type = CharT;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using value_type = codepoint;
        using reference = codepoint;
        using string_view_type = std::basic_string_view<CharT>;

        constexpr cp_iter_impl() noexcept = default;
        constexpr cp_iter_impl(const cp_iter_impl&) noexcept = default;

        constexpr cp_iter_impl& operator=(const cp_iter_impl&) noexcept = default;

        constexpr cp_iter_impl(string_view_type str, size_type offset, std::uint8_t len) noexcept
            : m_str(str), m_offset(offset), m_len(len) {}

    protected:
        constexpr void next() noexcept
        {
            size_type next_offset = m_offset + m_len;
            if(next_offset < m_str.size())
            {
                m_offset = next_offset;
                char8_t ch = m_str[next_offset];
                if(is_leading_byte(ch))
                    m_len = byte_count(ch);
                else
                    m_len = 1;
            }
            else
            {
                m_offset = m_str.size();
                m_len = 0;
            }
        }

        constexpr void prev() noexcept
        {
            PAPILIO_ASSUME(m_offset != 0);
            --m_offset;
            size_type next_offset = m_offset;
            while(true)
            {
                char8_t ch = m_str[next_offset];

                if(m_offset - next_offset > 3) [[unlikely]]
                {
                    m_len = 1;
                    break;
                }
                else if(is_leading_byte(ch))
                {
                    m_offset = next_offset;
                    m_len = byte_count(ch);
                    break;
                }
                else if(next_offset == 0)
                {
                    m_len = 1;
                    break;
                }

                --next_offset;
            }
        }

    public:
        // clang-format off

        [[nodiscard]]
        constexpr reference operator*() const noexcept
        {
            return codepoint(base(), m_len);
        }

        // clang-format on

        explicit constexpr operator bool() const noexcept
        {
            return !m_str.empty();
        }

        [[nodiscard]]
        constexpr const CharT* base() const noexcept
        {
            return m_str.data() + m_offset;
        }

        // byte count
        [[nodiscard]]
        constexpr std::uint8_t size() const noexcept
        {
            return m_len;
        }

        constexpr void swap(cp_iter_impl& other) noexcept
        {
            using std::swap;
            swap(m_str, other.m_str);
            swap(m_offset, other.m_offset);
            swap(m_len, other.m_len);
        }

    private:
        string_view_type m_str;
        size_type m_offset = 0;
        std::uint8_t m_len = 0;
    };

    template <char16_like CharT>
    class cp_iter_impl<CharT> : public cp_iter_impl_base
    {
    public:
        using char_type = CharT;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using value_type = codepoint;
        using reference = codepoint;
        using string_view_type = std::basic_string_view<CharT>;

        constexpr cp_iter_impl() noexcept = default;
        constexpr cp_iter_impl(const cp_iter_impl&) noexcept = default;

        constexpr cp_iter_impl& operator=(const cp_iter_impl&) noexcept = default;

        constexpr cp_iter_impl(string_view_type str, size_type offset, std::uint8_t len) noexcept
            : m_str(str), m_offset(offset), m_len(len) {}

    protected:
        constexpr void next() noexcept
        {
            size_type next_offset = m_offset + m_len;
            if(next_offset < m_str.size())
            {
                m_offset = next_offset;
                std::uint16_t ch = m_str[next_offset];
                if(PAPILIO_NS utf::is_high_surrogate(ch))
                    m_len = 2;
                else
                    m_len = 1;
            }
            else
            {
                m_offset = m_str.size();
                m_len = 0;
            }
        }

        constexpr void prev() noexcept
        {
            PAPILIO_ASSUME(m_offset != 0);
            --m_offset;
            while(PAPILIO_NS utf::is_low_surrogate(m_str[m_offset]))
                --m_offset;
        }

    public:
        // clang-format off

        [[nodiscard]]
        constexpr reference operator*() const
        {
            return decoder<CharT>::to_codepoint(m_str.substr(m_offset, m_len)).first;
        }

        // clang-format on

        explicit constexpr operator bool() const noexcept
        {
            return !m_str.empty();
        }

        [[nodiscard]]
        constexpr const CharT* base() const noexcept
        {
            return m_str.data() + m_offset;
        }

        // byte count
        [[nodiscard]]
        constexpr std::uint8_t size() const noexcept
        {
            return m_len;
        }

        constexpr void swap(cp_iter_impl& other) noexcept
        {
            using std::swap;
            swap(m_str, other.m_str);
            swap(m_offset, other.m_offset);
            swap(m_len, other.m_len);
        }

    private:
        string_view_type m_str;
        std::size_t m_offset = 0;
        std::uint8_t m_len = 0;
    };

    template <char32_like CharT>
    class cp_iter_impl<CharT> : public cp_iter_impl_base
    {
    public:
        using char_type = CharT;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using value_type = codepoint;
        using reference = codepoint;
        using string_view_type = std::basic_string_view<CharT>;

        using base_iter_t = typename string_view_type::const_iterator;

        constexpr cp_iter_impl() noexcept = default;
        constexpr cp_iter_impl(const cp_iter_impl&) noexcept = default;

        constexpr cp_iter_impl& operator=(const cp_iter_impl&) noexcept = default;

        constexpr cp_iter_impl(base_iter_t iter) noexcept
            : m_iter(iter) {}

    protected:
        constexpr void next() noexcept
        {
            ++m_iter;
        }

        constexpr void prev() noexcept
        {
            --m_iter;
        }

    public:
        // clang-format off

        [[nodiscard]]
        constexpr reference operator*() const
        {
            return decoder<char32_t>::to_codepoint(*m_iter).first;
        }

        // clang-format on

        explicit operator bool() const noexcept
        {
            return static_cast<bool>(m_iter);
        }

        [[nodiscard]]
        constexpr const CharT* base() const noexcept
        {
            return std::to_address(m_iter);
        }

        // byte count
        [[nodiscard]]
        constexpr std::uint8_t size() const noexcept
        {
            return 1;
        }

        constexpr void swap(cp_iter_impl& other) noexcept
        {
            using std::swap;
            swap(m_iter, other.m_iter);
        }

    private:
        base_iter_t m_iter;
    };
} // namespace detail

PAPILIO_EXPORT template <typename CharT>
class codepoint_iterator : public detail::cp_iter_impl<CharT>
{
public:
    using my_base = detail::cp_iter_impl<CharT>;

public:
    using char_type = CharT;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using value_type = codepoint;
    using reference = codepoint;
    using string_view_type = std::basic_string_view<CharT>;

    constexpr codepoint_iterator() noexcept = default;
    constexpr codepoint_iterator(const codepoint_iterator&) noexcept = default;

private:
    using my_base::my_base;

public:
    constexpr codepoint_iterator& operator=(const codepoint_iterator&) noexcept = default;

    constexpr bool operator==(const codepoint_iterator& rhs) const noexcept
    {
        return this->base() == rhs.base();
    }

    constexpr codepoint_iterator& operator++() noexcept
    {
        this->next();
        return *this;
    }

    constexpr codepoint_iterator operator++(int) noexcept
    {
        codepoint_iterator tmp(*this);
        ++*this;
        return tmp;
    }

    constexpr codepoint_iterator& operator--() noexcept
    {
        this->prev();
        return *this;
    }

    constexpr codepoint_iterator operator--(int) noexcept
    {
        codepoint_iterator tmp(*this);
        --*this;
        return tmp;
    }

    constexpr codepoint_iterator& operator+=(difference_type diff) noexcept
    {
        if(diff > 0)
        {
            for(difference_type i = 0; i < diff; ++i) ++*this;
        }
        else if(diff < 0)
        {
            diff = -diff;
            for(difference_type i = 0; i < diff; ++i) --*this;
        }

        return *this;
    }

    constexpr codepoint_iterator& operator-=(difference_type diff) noexcept
    {
        return *this += -diff;
    }

    friend constexpr codepoint_iterator operator+(codepoint_iterator lhs, difference_type rhs) noexcept
    {
        return lhs += rhs;
    }

    friend constexpr codepoint_iterator operator+(difference_type lhs, codepoint_iterator rhs) noexcept
    {
        return rhs += lhs;
    }

    friend constexpr codepoint_iterator operator-(codepoint_iterator lhs, difference_type rhs) noexcept
    {
        return lhs -= rhs;
    }

    friend constexpr codepoint_iterator operator-(difference_type lhs, codepoint_iterator rhs) noexcept
    {
        return rhs -= lhs;
    }

    constexpr difference_type operator-(codepoint_iterator rhs) const noexcept
    {
        if(this->to_address() < rhs.to_address())
        {
            return -(rhs - *this);
        }
        else
        {
            difference_type diff = 0;

            while(rhs != *this && rhs)
            {
                ++rhs;
                ++diff;
            }

            return diff;
        }
    }

    friend constexpr void swap(codepoint_iterator& lhs, codepoint_iterator& rhs) noexcept
    {
        lhs.swap(rhs);
    }
};

namespace detail
{
    class str_static_base
    {
    public:
        using size_type = std::size_t;

        static constexpr size_type npos = utf::npos;

    protected:
        [[noreturn]]
        static void throw_out_of_range(const char* msg = "out of range")
        {
            throw std::out_of_range(msg);
        }
    };

    template <typename CharT, typename Derived>
    class str_base : public str_static_base
    {
    public:
        using value_type = codepoint;
        using char_type = CharT;
        using size_type = std::size_t;
        using string_view_type = std::basic_string_view<CharT>;

        using const_iterator = codepoint_iterator<CharT>;

        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        constexpr const_iterator cbegin() const noexcept
        {
            string_view_type str = this->get_view();

            if constexpr(!char32_like<CharT>) // char8_like and char16_like
            {
                if(str.empty())
                    return make_c_iter(str, size_type(0), std::uint8_t(0));
            }

            if constexpr(char8_like<CharT>)
            {
                std::uint8_t ch = str[0];
                std::uint8_t ch_size = is_leading_byte(ch) ? byte_count(ch) : 1;
                return make_c_iter(str, size_type(0), ch_size);
            }
            else if constexpr(char16_like<CharT>)
            {
                std::uint16_t ch = str[0];
                std::uint8_t ch_size = is_high_surrogate(ch) ? 2 : 1;
                return make_c_iter(str, size_type(0), ch_size);
            }
            else // char32_like
            {
                return make_c_iter(str.begin());
            }
        }

        const_iterator cend() const noexcept
        {
            string_view_type str = this->get_view();

            if constexpr(!char32_like<CharT>) // char8_like and char16_like
            {
                return make_c_iter(str, str.size(), std::uint8_t(0));
            }
            else
            {
                return make_c_iter(str.end());
            }
        }

        constexpr const_iterator begin() const noexcept
        {
            return cbegin();
        }

        constexpr const_iterator end() const noexcept
        {
            return cend();
        }

        constexpr const_reverse_iterator crbegin() const noexcept
        {
            return const_reverse_iterator(cend());
        }

        constexpr const_reverse_iterator crend() const noexcept
        {
            return const_reverse_iterator(cbegin());
        }

        constexpr const_reverse_iterator rbegin() const noexcept
        {
            return crbegin();
        }

        constexpr const_reverse_iterator rend() const noexcept
        {
            return crend();
        }

        // clang-format off

        [[nodiscard]]
        constexpr codepoint operator[](size_type i) const noexcept
        {
            return index(i);
        }

#ifdef PAPILIO_HAS_MULTIDIMENSIONAL_SUBSCRIPT

        [[nodiscard]]
        constexpr codepoint operator[](reverse_index_t, size_type i) const noexcept
        {
            return index(reverse_index, i);
        }

#endif

        // clang-format on

        // for consistency
        [[nodiscard]]
        constexpr codepoint index(size_type i) const noexcept
        {
            return cp_from_off(get_offset(i));
        }

        // for pre-C++ 23
        [[nodiscard]]
        constexpr codepoint index(reverse_index_t, size_type i) const noexcept
        {
            return cp_from_off(get_offset(reverse_index, i));
        }

        [[nodiscard]]
        constexpr codepoint at(size_type i) const
        {
            size_type off = get_offset(i);
            if(off == npos) throw_out_of_range();
            return cp_from_off(off);
        }

        [[nodiscard]]
        constexpr codepoint at(reverse_index_t, size_type i) const
        {
            size_type off = get_offset(reverse_index, i);
            if(off == npos) throw_out_of_range();
            return cp_from_off(off);
        }

        constexpr codepoint index_or(size_type i, codepoint default_val) const noexcept
        {
            size_type off = get_offset(i);
            if(off == npos) return default_val;
            return cp_from_off(off);
        }

        constexpr codepoint index_or(reverse_index_t, size_type i, codepoint default_val) const noexcept
        {
            size_type off = get_offset(reverse_index, i);
            if(off == npos) return default_val;
            return cp_from_off(off);
        }

        [[nodiscard]]
        constexpr const CharT* data() const noexcept
        {
            return get_view().data();
        }

        [[nodiscard]]
        constexpr size_type get_offset(size_type i) const noexcept
        {
            return index_offset(i, get_view());
        }

        [[nodiscard]]
        constexpr size_type get_offset(reverse_index_t, size_type i) const noexcept
        {
            return index_offset(reverse_index, i, get_view());
        }

        [[nodiscard]]
        constexpr size_type length() const noexcept
        {
            return utf::strlen(get_view());
        }

        [[nodiscard]]
        constexpr size_type size() const noexcept
        {
            return get_view().size();
        }

        [[nodiscard]]
        constexpr bool empty() const noexcept
        {
            return as_derived().size() == 0;
        }

        [[nodiscard]]
        constexpr codepoint front() const noexcept
        {
            return index(0);
        }

        [[nodiscard]]
        constexpr codepoint back() const noexcept
        {
            return index(reverse_index, 0);
        }

        [[nodiscard]]
        const_iterator find(codepoint ch, size_type pos = 0) const noexcept
        {
            auto it = as_derived().cbegin();
            const auto sentinel = as_derived().cend();
            for(size_type n = 0; n < pos; ++n)
            {
                if(it == sentinel) return sentinel;
                ++it;
            }

            return std::find(it, sentinel, ch);
        }

        [[nodiscard]]
        constexpr bool contains(codepoint ch) const noexcept
        {
            return find(ch) != as_derived().cend();
        }

        constexpr bool starts_with(string_view_type str) const noexcept
        {
            return get_view().substr(0, str.size()) == str;
        }

        [[nodiscard]]
        constexpr bool starts_with(const CharT* str) const noexcept
        {
            return starts_with(string_view_type(str));
        }

        [[nodiscard]]
        constexpr bool starts_with(codepoint cp) const noexcept
        {
            return !empty() && front() == cp;
        }

        template <substr_behavior OnOutOfRange = substr_behavior::exception>
        [[nodiscard]]
        constexpr std::pair<Derived, size_type> substr_extended(
            size_type pos = 0, size_type count = npos
        ) const noexcept(OnOutOfRange != substr_behavior::exception)
        {
            const auto sentinel = cend();

            auto start = cbegin();
            for(size_type i = 0; i < pos; ++i)
            {
                if(start == sentinel)
                {
                    // out of range
                    if constexpr(OnOutOfRange == substr_behavior::exception)
                        this->throw_out_of_range();
                    else
                        return std::make_pair(Derived(), 0);
                }

                ++start;
            }

            size_type n = 0;
            auto stop = start;
            for(size_type i = 0; i < count; ++i)
            {
                if(stop == sentinel) break;
                ++n;
                ++stop;
            }

            return std::make_pair(Derived(start, stop), n);
        }

        template <substr_behavior OnOutOfRange = substr_behavior::exception>
        [[nodiscard]]
        constexpr Derived substr(size_type pos = 0, size_type count = npos) const noexcept(OnOutOfRange != substr_behavior::exception)
        {
            return substr_extended<OnOutOfRange>(pos, count).first;
        }

        template <substr_behavior OnOutOfRange = substr_behavior::exception>
        [[nodiscard]]
        constexpr Derived substr(slice s) const
        {
            auto get_iter = [this](slice::index_type idx) -> const_iterator
            {
                if(idx >= 0)
                {
                    const auto sentinel = as_derived().cend();

                    const_iterator it = as_derived().cbegin();
                    for(slice::index_type i = 0; i < idx; ++i)
                    {
                        if(it == sentinel)
                        {
                            if constexpr(OnOutOfRange == substr_behavior::exception)
                                throw_out_of_range();
                            else
                                return it;
                        }

                        ++it;
                    }

                    return it;
                }
                else
                {
                    idx = -idx; // abs(idx)

                    const auto sentinel = cbegin();

                    const_iterator it = cend();
                    for(slice::index_type i = 0; i < idx; ++i)
                    {
                        if(it == sentinel)
                        {
                            if constexpr(OnOutOfRange == substr_behavior::exception)
                                throw_out_of_range();
                            else
                                return it;
                        }

                        --it;
                    }

                    return it;
                }
            };

            auto start = get_iter(s.begin());
            auto stop = s.end() == slice::npos ? as_derived().cend() : get_iter(s.end());

            if(start.base() >= stop.base()) [[unlikely]]
                return Derived();

            return Derived(start, stop);
        }

        friend std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const Derived& str)
        {
            os << static_cast<string_view_type>(str);
            return os;
        }

    protected:
        constexpr string_view_type get_view() const noexcept
        {
            return string_view_type(as_derived());
        }

        template <typename... Args>
        constexpr const_iterator make_c_iter(Args&&... args) const noexcept
        {
            return const_iterator(std::forward<Args>(args)...);
        }

        // Calculates how many characters are required by a codepoint.
        static constexpr std::uint8_t ch_size_for_cp(CharT ch) noexcept
        {
            if constexpr(char8_like<CharT>)
            {
                return utf::byte_count(ch);
            }
            else if constexpr(char16_like<CharT>)
            {
                return utf::is_high_surrogate(ch) ? 2 : 1;
            }
            else // char32_like
            {
                return 1;
            }
        }

        constexpr codepoint cp_from_off(size_type off) const noexcept
        {
            string_view_type str = get_view();
            PAPILIO_ASSERT(off < str.size());

            if constexpr(char8_like<CharT>)
            {
                std::uint8_t ch_size = ch_size_for_cp(str[off]);
                return codepoint(str.data() + off, ch_size);
            }
            else if constexpr(char16_like<CharT>)
            {
                std::uint8_t ch_size = ch_size_for_cp(str[off]) ? 2 : 1;
                return decoder<CharT>::to_codepoint(str.substr(off, ch_size)).first;
            }
            else // char32_like
            {
                return decoder<char32_t>::to_codepoint(str[off]).first;
            }
        }

    private:
        constexpr Derived& as_derived() noexcept
        {
            return static_cast<Derived&>(*this);
        }

        constexpr const Derived& as_derived() const noexcept
        {
            return static_cast<const Derived&>(*this);
        }
    };
} // namespace detail

PAPILIO_EXPORT template <char_like CharT>
class basic_string_ref<CharT> : public detail::str_base<CharT, basic_string_ref<CharT>>
{
    using my_base = detail::str_base<CharT, basic_string_ref<CharT>>;

public:
    using char_type = CharT;
    using value_type = codepoint;
    using size_type = std::size_t;
    using string_type = std::basic_string<CharT>;
    using string_view_type = std::basic_string_view<CharT>;

    using const_iterator = typename my_base::const_iterator;
    using iterator = const_iterator;
    using const_reverse_iterator = typename my_base::const_reverse_iterator;
    using reverse_iterator = const_reverse_iterator;

    constexpr basic_string_ref() noexcept = default;
    constexpr basic_string_ref(const basic_string_ref& other) noexcept = default;
    constexpr basic_string_ref(std::nullptr_t) = delete;

    constexpr basic_string_ref(const CharT* str, size_type count) noexcept
        : m_str(string_view_type(str, count)) {}

    constexpr basic_string_ref(const CharT* str) noexcept
        : m_str(string_view_type(str)) {}

    constexpr basic_string_ref(string_view_type str) noexcept
        : m_str(str) {}

    constexpr basic_string_ref(const string_type& str) noexcept
        : m_str(str) {}

    template <std::contiguous_iterator Iterator, typename Sentinel>
    requires(std::is_same_v<std::iter_value_t<Iterator>, CharT> && !std::is_convertible_v<Sentinel, size_type>)
    constexpr basic_string_ref(Iterator start, Sentinel stop)
        : m_str(string_view_type(start, stop))
    {}

    constexpr basic_string_ref(const_iterator start, const_iterator stop) noexcept
        : m_str(string_view_type(start.base(), stop.base()))
    {}

    constexpr basic_string_ref& operator=(const basic_string_ref&) noexcept = default;
    constexpr basic_string_ref& operator=(std::nullptr_t) noexcept = delete;

    constexpr basic_string_ref& operator=(const CharT* str) noexcept
    {
        PAPILIO_ASSERT(str != nullptr);
        this->set_view(str);
        return *this;
    }

    constexpr basic_string_ref& operator=(string_view_type str) noexcept
    {
        this->set_view(str);
        return *this;
    }

    basic_string_ref& operator=(const string_type& str) noexcept
    {
        this->set_view(str);
        return *this;
    }

    using my_base::find;

    [[nodiscard]]
    constexpr auto find(basic_string_ref str, size_type pos = 0) const noexcept
    {
        return find_impl(str, pos);
    }

    [[nodiscard]]
    constexpr auto find(const CharT* str, size_type pos, size_type count) const noexcept
    {
        return find_impl(basic_string_ref(str, count), pos);
    }

    [[nodiscard]]
    constexpr auto find(const CharT* str, size_type pos = 0) const noexcept
    {
        return find_impl(basic_string_ref(str), pos);
    }

    using my_base::contains;

    [[nodiscard]]
    constexpr bool contains(basic_string_ref str) const noexcept
    {
        return this->find(str) != this->cend();
    }

    using my_base::starts_with;

    [[nodiscard]]
    constexpr bool starts_with(basic_string_ref str) const noexcept
    {
        return starts_with(string_view_type(str));
    }

    [[nodiscard]]
    constexpr bool ends_with(basic_string_ref str) const noexcept
    {
        string_view_type view = this->get_view();
        if(view.size() < str.size()) return false;
        return view.substr(view.size() - str.size(), str.size()) == string_view_type(str);
    }

    [[nodiscard]]
    constexpr bool ends_with(const CharT* str) const noexcept
    {
        return ends_with(basic_string_ref(str));
    }

    [[nodiscard]]
    constexpr bool ends_with(codepoint cp) const noexcept
    {
        return !this->empty() && this->back() == cp;
    }

    constexpr void swap(basic_string_ref& other) noexcept
    {
        using std::swap;
        swap(m_str, other.m_str);
    }

    constexpr void remove_prefix(size_type n) noexcept
    {
        auto it = this->cbegin();
        auto sentinel = this->cend();
        for(size_type i = 0; i < n; ++i)
        {
            if(it == sentinel)
                break;
            ++it;
        }

        set_view(it, sentinel);
    }

    constexpr void remove_suffix(size_type n) noexcept
    {
        auto it = this->crbegin();
        auto sentinel = this->crend();
        for(size_type i = 0; i < n; ++i)
        {
            if(it == sentinel)
                break;
            ++it;
        }

        set_view(sentinel.base(), it.base());
    }

    template <char_like To>
    [[nodiscard]]
    std::basic_string<To> to_string_as() const
    {
        std::basic_string<To> result;

        for(codepoint cp : *this)
            cp.append_to(result);

        return result;
    }

    [[nodiscard]]
    std::string to_string() const
    {
        return to_string_as<char>();
    }

    [[nodiscard]]
    std::u8string to_u8string() const
    {
        return to_string_as<char8_t>();
    }

    [[nodiscard]]
    std::u16string to_u16string() const
    {
        return to_string_as<char16_t>();
    }

    [[nodiscard]]
    std::u32string to_u32string() const
    {
        return to_string_as<char32_t>();
    }

    [[nodiscard]]
    std::wstring to_wstring() const
    {
        return to_string_as<wchar_t>();
    }

    constexpr operator string_view_type() const noexcept
    {
        return m_str;
    }

    template <char_like U>
    constexpr int compare(basic_string_ref<U> other) const noexcept
    {
        return this->compare_impl(other);
    }

    template <char_like U>
    constexpr int compare(const U* other) const noexcept
    {
        return this->compare_impl(basic_string_ref<U>(other));
    }

    template <char_like U>
    constexpr int compare(std::basic_string_view<U> other) const noexcept
    {
        return this->compare_impl(basic_string_ref<U>(other));
    }

    template <char_like U>
    constexpr int compare(const std::basic_string<U>& other) const noexcept
    {
        return this->compare_impl(basic_string_ref<U>(other));
    }

private:
    string_view_type m_str;

    constexpr string_view_type get_view() const noexcept
    {
        return m_str;
    }

    constexpr void set_view(string_view_type new_str) noexcept
    {
        m_str = new_str;
    }

    constexpr void set_view(const_iterator start, const_iterator stop) noexcept
    {
        set_view(string_view_type(start.base(), stop.base()));
    }

    template <typename Other>
    constexpr int compare_impl(const Other& other) const noexcept
    {
        auto i = this->cbegin();
        auto j = other.cbegin();

        while(true)
        {
            if(i == this->cend())
                return j == other.cend() ? 0 : -1;
            if(j == other.cend())
                return 1; // obviously, i != cend()

            auto order = *i <=> *j;
            if(order == std::strong_ordering::less) return -1;
            if(order == std::strong_ordering::greater) return 1;

            ++i;
            ++j;
        }
    }

    constexpr auto find_impl(const basic_string_ref& v, size_type pos) const noexcept
    {
        auto it = this->cbegin();
        auto sentinel = this->cend();

        for(size_type n = 0; n < pos; ++n)
        {
            if(it == sentinel)
                return sentinel;
            ++it;
        }

        auto v_begin = v.cbegin();
        auto v_end = v.cend();
        for(; it != this->cend(); ++it)
        {
            if(basic_string_ref(it, sentinel).starts_with(v))
                break;
        }

        return it;
    }
};

// global functions

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(basic_string_ref<T> lhs, basic_string_ref<U> rhs) noexcept(std::is_same_v<T, U>)
{
    if constexpr(std::is_same_v<T, U>)
        return std::basic_string_view<T>(lhs) == std::basic_string_view<U>(rhs);
    else
    {
        return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(basic_string_ref<T> lhs, const U* rhs) noexcept(std::is_same_v<T, U>)
{
    return lhs == basic_string_ref<U>(rhs);
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(const T* lhs, basic_string_ref<U> rhs) noexcept(std::is_same_v<T, U>)
{
    return basic_string_ref<T>(lhs) == rhs;
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(basic_string_ref<T> lhs, std::basic_string_view<U> rhs) noexcept(std::is_same_v<T, U>)
{
    return lhs == basic_string_ref<U>(rhs);
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(std::basic_string_view<T> lhs, basic_string_ref<U> rhs) noexcept(std::is_same_v<T, U>)
{
    return basic_string_ref<T>(lhs) == rhs;
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(basic_string_ref<T> lhs, const std::basic_string<U>& rhs) noexcept(std::is_same_v<T, U>)
{
    return lhs == basic_string_ref<U>(rhs);
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(const std::basic_string<T>& lhs, basic_string_ref<U> rhs) noexcept(std::is_same_v<T, U>)
{
    return basic_string_ref<T>(lhs) == rhs;
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr std::strong_ordering operator<=>(basic_string_ref<T> lhs, basic_string_ref<U> rhs) noexcept
{
    return lhs.compare(rhs) <=> 0;
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr std::strong_ordering operator<=>(basic_string_ref<T> lhs, const U* rhs) noexcept
{
    return lhs.compare(rhs) <=> 0;
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr std::strong_ordering operator<=>(const T* lhs, basic_string_ref<U> rhs) noexcept
{
    return basic_string_ref(lhs).compare(rhs) <=> 0;
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr std::strong_ordering operator<=>(basic_string_ref<T> lhs, std::basic_string_view<U> rhs) noexcept
{
    return lhs.compare(rhs) <=> 0;
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr std::strong_ordering operator<=>(std::basic_string_view<T> lhs, basic_string_ref<U> rhs) noexcept
{
    return basic_string_ref(lhs).compare(rhs) <=> 0;
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr std::strong_ordering operator<=>(basic_string_ref<T> lhs, const std::basic_string<U>& rhs) noexcept
{
    return lhs.compare(rhs) <=> 0;
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr std::strong_ordering operator<=>(const std::basic_string<T>& lhs, basic_string_ref<U> rhs) noexcept
{
    return basic_string_ref(lhs).compare(rhs) <=> 0;
}

PAPILIO_EXPORT using string_ref = basic_string_ref<char>;
PAPILIO_EXPORT using u8string_ref = basic_string_ref<char8_t>;
PAPILIO_EXPORT using u16string_ref = basic_string_ref<char16_t>;
PAPILIO_EXPORT using u32string_ref = basic_string_ref<char32_t>;
PAPILIO_EXPORT using wstring_ref = basic_string_ref<wchar_t>;

inline namespace literals
{
    PAPILIO_EXPORT constexpr string_ref operator""_sr(
        const char* str, std::size_t size
    ) noexcept
    {
        return string_ref(str, size);
    }

    PAPILIO_EXPORT constexpr u8string_ref operator""_sr(
        const char8_t* str, std::size_t size
    ) noexcept
    {
        return u8string_ref(str, size);
    }

    PAPILIO_EXPORT constexpr u16string_ref operator""_sr(
        const char16_t* str, std::size_t size
    ) noexcept
    {
        return u16string_ref(str, size);
    }

    PAPILIO_EXPORT constexpr u32string_ref operator""_sr(
        const char32_t* str, std::size_t size
    ) noexcept
    {
        return u32string_ref(str, size);
    }

    PAPILIO_EXPORT constexpr wstring_ref operator""_sr(
        const wchar_t* str, std::size_t size
    ) noexcept
    {
        return wstring_ref(str, size);
    }
} // namespace literals

PAPILIO_EXPORT template <char_like CharT>
class basic_string_container<CharT> : public detail::str_base<CharT, basic_string_container<CharT>>
{
    using my_base = detail::str_base<CharT, basic_string_container<CharT>>;

public:
    using size_type = std::size_t;
    using value_type = codepoint;
    using char_type = CharT;
    using string_view_type = std::basic_string_view<CharT>;
    using string_type = std::basic_string<CharT>;
    using string_ref_type = basic_string_ref<CharT>;

    using const_iterator = typename my_base::const_iterator;
    using const_reverse_iterator = typename my_base::const_reverse_iterator;

    basic_string_container() noexcept = default;
    basic_string_container(std::nullptr_t) = delete;

    basic_string_container(const basic_string_container& other) noexcept
        : m_data(std::in_place_type<string_view_type>, other.get_view()) {}

    basic_string_container(independent_t, const basic_string_container& str)
        : m_data(std::in_place_type<string_type>, string_view_type(str)) {}

    basic_string_container(basic_string_container&&) noexcept = default;

    basic_string_container(string_type&& str) noexcept
        : m_data(std::in_place_type<string_type>, std::move(str)) {}

    basic_string_container(const string_type& str) noexcept
        : m_data(std::in_place_type<string_view_type>, str) {}

    basic_string_container(independent_t, const string_type& str)
        : m_data(std::in_place_type<string_type>, str) {}

    basic_string_container(string_view_type str) noexcept
        : m_data(std::in_place_type<string_view_type>, str) {}

    basic_string_container(const CharT* str) noexcept
        : m_data(std::in_place_type<string_view_type>, str) {}

    constexpr basic_string_container(const CharT* str, size_type count) noexcept
        : m_data(std::in_place_type<string_view_type>, str, count) {}

    basic_string_container(independent_t, const CharT* str) noexcept
        : m_data(std::in_place_type<string_type>, str) {}

    constexpr basic_string_container(independent_t, const CharT* str, size_type count) noexcept
        : m_data(std::in_place_type<string_type>, str, count) {}

    basic_string_container(const_iterator start, const_iterator stop) noexcept
        : m_data(std::in_place_type<string_view_type>, start.base(), stop.base())
    {}

    basic_string_container(size_type count, CharT ch)
    {
        assign(count, ch);
    }

    basic_string_container(size_type count, codepoint cp)
    {
        assign(count, cp);
    }

    template <typename Iterator, typename Sentinel>
    requires std::is_same_v<std::iter_value_t<Iterator>, CharT>
    basic_string_container(Iterator start, Sentinel stop)
        : m_data(std::in_place_type<string_type>, start, stop)
    {}

    basic_string_container& assign(size_type count, CharT ch)
    {
        string_type& str = to_str();
        str.assign(count, ch);
    }

    basic_string_container& assign(size_type count, codepoint cp)
    {
        string_type& str = to_str();
        if constexpr(char8_like<CharT>)
        {
            for(size_type i = 0; i < count; ++i)
            {
                str.append(
                    std::bit_cast<const CharT*>(cp.data()),
                    cp.size_bytes()
                );
            }
        }
        else
        {
            for(size_type i = 0; i < count; ++i)
                cp.append_to(str);
        }

        return *this;
    }

    basic_string_container& assign(const CharT* str) noexcept
    {
        emplace_data<string_view_type>(str);
        return *this;
    }

    basic_string_container& assign(const CharT* str, size_type count) noexcept
    {
        emplace_data<string_view_type>(str, count);
        return *this;
    }

    basic_string_container& assign(string_view_type str)
    {
        emplace_data<string_view_type>(str);
        return *this;
    }

    basic_string_container& assign(independent_t, string_view_type str)
    {
        emplace_data<string_type>(str);
        return *this;
    }

    basic_string_container& assign(const string_type& str) noexcept
    {
        emplace_data<string_view_type>(str);
        return *this;
    }

    basic_string_container& assign(independent_t, const string_type& str)
    {
        emplace_data<string_type>(str);
        return *this;
    }

    basic_string_container& assign(string_type&& str) noexcept
    {
        emplace_data<string_type>(std::move(str));
        return *this;
    }

    basic_string_container& assign(const_iterator start, const_iterator stop) noexcept
    {
        emplace_data<string_view_type>(start.base(), stop.base());
        return *this;
    }

    constexpr basic_string_container& operator=(const basic_string_container&) = default;
    constexpr basic_string_container& operator=(basic_string_container&&) noexcept = default;

    basic_string_container& operator=(string_view_type str)
    {
        return assign(str);
    }

    basic_string_container& operator=(independent_proxy<string_view_type> str)
    {
        return assign(independent, str);
    }

    [[nodiscard]]
    constexpr bool empty() const noexcept
    {
        return std::visit(
            [](auto&& v)
            { return v.empty(); },
            m_data
        );
    }

    [[nodiscard]]
    constexpr const CharT* c_str() const
    {
        return str().c_str();
    }

    [[nodiscard]]
    constexpr const CharT* data() const noexcept
    {
        return std::visit(
            [](auto&& v) -> const CharT*
            { return v.data(); },
            m_data
        );
    }

    [[nodiscard]]
    constexpr size_type capacity() const noexcept
    {
        if(const string_type* p = std::get_if<string_type>(&m_data); p)
            return p->capacity();
        else
            return 0;
    }

    bool null_terminated() const noexcept
    {
        auto v = this->get_view();
        return *(v.data() + v.size()) == static_cast<CharT>(0);
    }

    [[nodiscard]]
    constexpr size_type size() const noexcept
    {
        return std::visit(
            [](auto&& v) -> size_type
            { return v.size(); },
            m_data
        );
    }

    using my_base::find;

    [[nodiscard]]
    const_iterator find(string_view_type str, size_type pos = 0) const noexcept
    {
        auto it = this->cbegin();
        const auto sentinel = this->cend();

        for(size_type n = 0; n < pos; ++n)
        {
            if(it == sentinel)
                return sentinel;
            ++it;
        }

        for(; it != sentinel; ++it)
        {
            if(create_ref(it, sentinel).starts_with(str))
                break;
        }

        return it;
    }

    using my_base::contains;

    [[nodiscard]]
    constexpr bool contains(string_view_type str) const noexcept
    {
        return find(str) != this->cend();
    }

    [[nodiscard]]
    bool has_ownership() const noexcept
    {
        return std::holds_alternative<string_type>(m_data);
    }

    void obtain_ownership()
    {
        to_str();
    }

    string_type&& str() &&
    {
        to_str();
        return std::move(*std::get_if<string_type>(&m_data));
    }

    const string_type& str() const&
    {
        to_str();
        return *std::get_if<string_type>(&m_data);
    }

    constexpr void swap(basic_string_container& other) noexcept
    {
        using std::swap;
        swap(m_data, other.m_data);
    }

    constexpr operator string_view_type() const noexcept
    {
        return std::visit(
            [](auto&& v)
            { return string_view_type(v); },
            std::as_const(m_data)
        );
    }

    constexpr operator string_ref_type() const noexcept
    {
        return static_cast<string_view_type>(*this);
    }

    template <char_like U>
    constexpr int compare(const basic_string_container<U>& other) const noexcept
    {
        return this->compare_impl(basic_string_ref<U>(other));
    }

    template <char_like U>
    constexpr int compare(basic_string_ref<U> other) const noexcept
    {
        return this->compare_impl(other);
    }

    template <char_like U>
    constexpr int compare(const U* other) const noexcept
    {
        return this->compare_impl(basic_string_ref<U>(other));
    }

    template <char_like U>
    constexpr int compare(std::basic_string_view<U> other) const noexcept
    {
        return this->compare_impl(basic_string_ref<U>(other));
    }

    template <char_like U>
    constexpr int compare(const std::basic_string<U>& other) const noexcept
    {
        return this->compare_impl(basic_string_ref<U>(other));
    }

    void push_back(CharT ch)
    {
        to_str().push_back(ch);
    }

    void push_back(codepoint cp)
    {
        cp.append_to(to_str());
    }

    using my_base::begin;
    using my_base::end;

    // TODO: iterator and begin() end() pair

private:
    using string_store = std::variant<std::basic_string<CharT>, std::basic_string_view<CharT>>;

    mutable string_store m_data;

    string_type& to_str() const
    {
        string_view_type* p_str = std::get_if<string_view_type>(&m_data);
        if(p_str)
        {
            string_type tmp(*p_str);
            return m_data.template emplace<string_type>(std::move(tmp));
        }
        else
        {
            return *std::get_if<string_type>(&m_data);
        }
    }

    template <typename T, typename... Args>
    T& emplace_data(Args&&... args) const noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        return m_data.template emplace<T>(std::forward<Args>(args)...);
    }

    static constexpr string_ref_type create_ref(const_iterator start, const_iterator stop) noexcept
    {
        return string_view_type(start.base(), stop.base());
    }

    template <typename U>
    constexpr int compare_impl(basic_string_ref<U> other) const noexcept
    {
        auto i = this->cbegin();
        auto j = other.cbegin();

        while(true)
        {
            if(i == this->cend())
                return j == other.cend() ? 0 : -1;
            if(j == other.cend())
                return 1; // obviously, i != cend()

            auto order = *i <=> *j;
            if(order == std::strong_ordering::less)
                return -1;
            if(order == std::strong_ordering::greater)
                return 1;

            ++i;
            ++j;
        }
    }
};

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(const basic_string_container<T>& lhs, const basic_string_container<U>& rhs) noexcept(std::is_same_v<T, U>)
{
    return basic_string_ref<T>(lhs) == basic_string_ref<U>(rhs);
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(const basic_string_container<T>& lhs, basic_string_ref<U> rhs) noexcept(std::is_same_v<T, U>)
{
    return basic_string_ref<T>(lhs) == rhs;
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(basic_string_ref<T> lhs, const basic_string_container<U>& rhs) noexcept(std::is_same_v<T, U>)
{
    return lhs == basic_string_ref<U>(rhs);
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(const basic_string_container<T>& lhs, const U* rhs) noexcept(std::is_same_v<T, U>)
{
    return lhs == basic_string_ref<U>(rhs);
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(const T* lhs, const basic_string_container<U>& rhs) noexcept(std::is_same_v<T, U>)
{
    return basic_string_ref<T>(lhs) == rhs;
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(const basic_string_container<T>& lhs, std::basic_string_view<U> rhs) noexcept(std::is_same_v<T, U>)
{
    return lhs == basic_string_ref<U>(rhs);
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(std::basic_string_view<T> lhs, const basic_string_container<U>& rhs) noexcept(std::is_same_v<T, U>)
{
    return basic_string_ref<T>(lhs) == rhs;
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(const basic_string_container<T>& lhs, const std::basic_string<U>& rhs) noexcept(std::is_same_v<T, U>)
{
    return lhs == basic_string_ref<U>(rhs);
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(const std::basic_string<T>& lhs, const basic_string_container<U>& rhs) noexcept(std::is_same_v<T, U>)
{
    return basic_string_ref<T>(lhs) == rhs;
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr std::strong_ordering operator<=>(const basic_string_container<T>& lhs, const basic_string_container<U>& rhs) noexcept
{
    return lhs.compare(rhs) <=> 0;
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr std::strong_ordering operator<=>(const basic_string_container<T>& lhs, const U* rhs) noexcept
{
    return lhs.compare(rhs) <=> 0;
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr std::strong_ordering operator<=>(const T* lhs, const basic_string_container<U>& rhs) noexcept
{
    return basic_string_ref(lhs).compare(rhs) <=> 0;
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr std::strong_ordering operator<=>(const basic_string_container<T>& lhs, std::basic_string_view<U> rhs) noexcept
{
    return lhs.compare(rhs) <=> 0;
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr std::strong_ordering operator<=>(std::basic_string_view<T> lhs, const basic_string_container<U>& rhs) noexcept
{
    return basic_string_ref(lhs).compare(rhs) <=> 0;
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr std::strong_ordering operator<=>(const basic_string_container<T>& lhs, const std::basic_string<U>& rhs) noexcept
{
    return lhs.compare(rhs) <=> 0;
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr std::strong_ordering operator<=>(const std::basic_string<T>& lhs, const basic_string_container<U>& rhs) noexcept
{
    return basic_string_ref(lhs).compare(rhs) <=> 0;
}

PAPILIO_EXPORT using string_container = basic_string_container<char>;
PAPILIO_EXPORT using u8string_container = basic_string_container<char8_t>;
PAPILIO_EXPORT using u16string_container = basic_string_container<char16_t>;
PAPILIO_EXPORT using u32string_container = basic_string_container<char32_t>;
PAPILIO_EXPORT using wstring_container = basic_string_container<wchar_t>;

PAPILIO_EXPORT std::istream& operator>>(std::istream& is, string_container& str);
PAPILIO_EXPORT std::wistream& operator>>(std::wistream& is, wstring_container& str);

inline namespace literals
{
    PAPILIO_EXPORT constexpr string_container operator""_sc(
        const char* str, std::size_t size
    ) noexcept
    {
        return string_container(str, size);
    }

    PAPILIO_EXPORT constexpr u8string_container operator""_sc(
        const char8_t* str, std::size_t size
    ) noexcept
    {
        return u8string_container(str, size);
    }

    PAPILIO_EXPORT constexpr u16string_container operator""_sc(
        const char16_t* str, std::size_t size
    ) noexcept
    {
        return u16string_container(str, size);
    }

    PAPILIO_EXPORT constexpr u32string_container operator""_sc(
        const char32_t* str, std::size_t size
    ) noexcept
    {
        return u32string_container(str, size);
    }

    PAPILIO_EXPORT constexpr wstring_container operator""_sc(
        const wchar_t* str, std::size_t size
    ) noexcept
    {
        return wstring_container(str, size);
    }
} // namespace literals
} // namespace papilio::utf
