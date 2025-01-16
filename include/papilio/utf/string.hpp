#ifndef PAPILIO_UTF_STRING_HPP
#define PAPILIO_UTF_STRING_HPP

#pragma once

#include <iosfwd>
#include <string>
#include <algorithm>
#include <variant>
#include "../fmtfwd.hpp"
#include "stralgo.hpp"
#include "codepoint.hpp"
#include "../detail/prefix.hpp"

namespace papilio::utf
{
/**
 * @brief Base of all string classes
 */
class string_base
{
public:
    using size_type = std::size_t;

    /**
     * @brief Special position value. Its actual meaning depends on API returning this value.
     */
    static constexpr size_type npos = utf::npos;

protected:
#ifndef PAPILIO_DOXYGEN // Don't generate documentation for internal APIs

    [[noreturn]]
    static void throw_out_of_range(const char* msg = "out of range")
    {
        throw std::out_of_range(msg);
    }

#endif
};

/**
 * @brief CRTP base of string classes
 *
 * Implements common functionalities
 */
template <typename CharT, typename Derived>
class implement_string : public string_base
{
public:
    using size_type = std::size_t;
    using value_type = codepoint;
    using char_type = CharT;
    using string_view_type = std::basic_string_view<CharT>;
    using string_type = std::basic_string<CharT>;
    using string_ref_type = basic_string_ref<CharT>;

    using const_iterator = codepoint_iterator<CharT>;

    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    using const_reference = typename codepoint_iterator<CharT>::const_reference;

    constexpr const_iterator cbegin() const noexcept
    {
        return PAPILIO_NS utf::codepoint_begin(get_view());
    }

    constexpr const_iterator cend() const noexcept
    {
        return PAPILIO_NS utf::codepoint_end(get_view());
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

private:
    constexpr const_reference make_const_reference(size_type off) const noexcept
    {
        const CharT* p = data() + off;
        return const_reference(p, ch_size_for_cp(*p));
    }

public:
    constexpr const_reference operator[](size_type i) const noexcept
    {
        return index(i);
    }

#ifdef PAPILIO_HAS_MULTIDIMENSIONAL_SUBSCRIPT

    constexpr const_reference operator[](reverse_index_t, size_type i) const noexcept
    {
        return index(reverse_index, i);
    }

#endif

    // for consistency
    [[nodiscard]]
    constexpr const_reference index(size_type i) const noexcept
    {
        return make_const_reference(get_offset(i));
    }

    // for pre-C++ 23
    [[nodiscard]]
    constexpr const_reference index(reverse_index_t, size_type i) const noexcept
    {
        return make_const_reference(get_offset(reverse_index, i));
    }

    constexpr codepoint at(size_type i) const
    {
        size_type off = get_offset(i);
        if(off == npos) throw_out_of_range();
        return cp_from_off(off);
    }

    constexpr codepoint at(reverse_index_t, size_type i) const
    {
        size_type off = get_offset(reverse_index, i);
        if(off == npos) throw_out_of_range();
        return cp_from_off(off);
    }

    [[nodiscard]]
    constexpr codepoint index_or(size_type i, codepoint default_val) const noexcept
    {
        size_type off = get_offset(i);
        if(off == npos) return default_val;
        return cp_from_off(off);
    }

    [[nodiscard]]
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

    [[nodiscard]]
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
    constexpr Derived substr(index_range s) const
    {
        auto get_iter = [this](index_range::index_type idx) -> const_iterator
        {
            if(idx >= 0)
            {
                const auto sentinel = as_derived().cend();

                const_iterator it = as_derived().cbegin();
                for(index_range::index_type i = 0; i < idx; ++i)
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
                for(index_range::index_type i = 0; i < idx; ++i)
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
        auto stop = s.end() == index_range::npos ? as_derived().cend() : get_iter(s.end());

        if(start >= stop) [[unlikely]]
            return Derived();

        return Derived(start, stop);
    }

    friend std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const Derived& str)
    {
        auto sv = str.to_string_view();
        os.write(sv.data(), static_cast<std::streamsize>(sv.size()));
        return os;
    }

protected:
#ifndef PAPILIO_DOXYGEN // Don't generate documentation for internal APIs

    constexpr string_view_type get_view() const noexcept
    {
        return as_derived().to_string_view();
    }

    // Calculates how many characters are required by a codepoint.
    static constexpr std::uint8_t ch_size_for_cp(CharT ch) noexcept
    {
        if constexpr(char8_like<CharT>)
        {
            return utf::byte_count(std::uint8_t(ch));
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
            char32_t ch = static_cast<char32_t>(str[off]);
            return decoder<char32_t>::to_codepoint(ch).first;
        }
    }

#endif

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

/**
 * @brief Reference to a string without ownership, similar to `std::string_view`
 */
PAPILIO_EXPORT template <char_like CharT>
class basic_string_ref<CharT> : public implement_string<CharT, basic_string_ref<CharT>>
{
    using my_base = implement_string<CharT, basic_string_ref<CharT>>;

public:
    using size_type = std::size_t;
    using value_type = codepoint;
    using char_type = CharT;
    using string_view_type = std::basic_string_view<CharT>;
    using string_type = std::basic_string<CharT>;
    using string_ref_type = basic_string_ref<CharT>;

    using const_iterator = typename my_base::const_iterator;
    using iterator = const_iterator;
    using const_reverse_iterator = typename my_base::const_reverse_iterator;
    using reverse_iterator = const_reverse_iterator;

    using const_reference = typename my_base::const_reference;
    using reference = const_reference;

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
    requires(std::same_as<std::iter_value_t<Iterator>, CharT> && !std::is_convertible_v<Sentinel, size_type>)
    constexpr basic_string_ref(Iterator start, Sentinel stop)
        : m_str(string_view_type(start, stop))
    {}

    constexpr basic_string_ref(const_iterator start, const_iterator stop) noexcept
        : m_str(string_view_type(start.base(), stop.base()))
    {}

    template <std::convertible_to<const_iterator> Iterator>
    constexpr basic_string_ref(Iterator start, Iterator stop) noexcept
        : m_str(string_view_type(const_iterator(start).base(), const_iterator(stop).base()))
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

    constexpr basic_string_ref& operator=(const string_type& str) noexcept
    {
        this->set_view(str);
        return *this;
    }

    constexpr basic_string_ref& assign(const_iterator start, const_iterator stop) noexcept
    {
        this->set_view(start, stop);
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

    template <char_like To = char>
    [[nodiscard]]
    std::basic_string<To> to_string() const
    {
        if constexpr(std::same_as<To, CharT>)
        {
            return std::basic_string<To>(get_view());
        }
        else if constexpr(sizeof(To) == sizeof(CharT))
        {
            string_view_type raw_view = get_view();
            std::basic_string_view<To> target_view(
                reinterpret_cast<const To*>(raw_view.data()),
                raw_view.size()
            );

            return std::basic_string<To>(target_view);
        }
        else
        {
            std::basic_string<To> result;

            for(codepoint cp : *this)
                cp.append_to(result);

            return result;
        }
    }

    [[nodiscard]]
    std::u8string to_u8string() const
    {
        return to_string<char8_t>();
    }

    [[nodiscard]]
    std::u16string to_u16string() const
    {
        return to_string<char16_t>();
    }

    [[nodiscard]]
    std::u32string to_u32string() const
    {
        return to_string<char32_t>();
    }

    [[nodiscard]]
    std::wstring to_wstring() const
    {
        return to_string<wchar_t>();
    }

    constexpr string_view_type to_string_view() const noexcept
    {
        return m_str;
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
        PAPILIO_ASSERT(start <= stop);
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
constexpr bool operator==(basic_string_ref<T> lhs, basic_string_ref<U> rhs) noexcept(std::same_as<T, U>)
{
    if constexpr(std::same_as<T, U>)
        return std::basic_string_view<T>(lhs) == std::basic_string_view<U>(rhs);
    else
    {
        return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(basic_string_ref<T> lhs, const U* rhs) noexcept(std::same_as<T, U>)
{
    return lhs == basic_string_ref<U>(rhs);
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(const T* lhs, basic_string_ref<U> rhs) noexcept(std::same_as<T, U>)
{
    return basic_string_ref<T>(lhs) == rhs;
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(basic_string_ref<T> lhs, std::basic_string_view<U> rhs) noexcept(std::same_as<T, U>)
{
    return lhs == basic_string_ref<U>(rhs);
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(std::basic_string_view<T> lhs, basic_string_ref<U> rhs) noexcept(std::same_as<T, U>)
{
    return basic_string_ref<T>(lhs) == rhs;
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(basic_string_ref<T> lhs, const std::basic_string<U>& rhs) noexcept(std::same_as<T, U>)
{
    return lhs == basic_string_ref<U>(rhs);
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(const std::basic_string<T>& lhs, basic_string_ref<U> rhs) noexcept(std::same_as<T, U>)
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

/**
 * @brief Copy-on-write string container
 *
 * This string class not only can refer to a string without owning it,
 * but also has ability to obtain ownership when necessary.
 */
PAPILIO_EXPORT template <char_like CharT>
class basic_string_container<CharT> : public implement_string<CharT, basic_string_container<CharT>>
{
    using my_base = implement_string<CharT, basic_string_container<CharT>>;

public:
    using size_type = std::size_t;
    using value_type = codepoint;
    using char_type = CharT;
    using string_view_type = std::basic_string_view<CharT>;
    using string_type = std::basic_string<CharT>;
    using string_ref_type = basic_string_ref<CharT>;

    using const_iterator = typename my_base::const_iterator;
    using const_reverse_iterator = typename my_base::const_reverse_iterator;

    using const_reference = typename my_base::const_reference;

    basic_string_container() noexcept = default;
    basic_string_container(std::nullptr_t) = delete;

    basic_string_container(const basic_string_container& other) noexcept
        : m_data(std::in_place_type<string_view_type>, other.get_view()) {}

    basic_string_container(independent_t, const basic_string_container& str)
        : m_data(std::in_place_type<string_type>, str.to_string_view()) {}

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

    template <typename Iterator, std::sentinel_for<Iterator> Sentinel>
    requires(std::same_as<std::iter_value_t<Iterator>, CharT>)
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

    template <std::ranges::input_range R>
    basic_string_container& assign_range(R&& r) noexcept
    {
        if constexpr(std::convertible_to<std::ranges::range_reference_t<R>, CharT>)
        {
            string_type buf;
            if constexpr(std::ranges::sized_range<R>)
                buf.reserve(std::ranges::size(r));
            for(auto&& ch : r)
                buf.push_back(static_cast<CharT>(ch));

            assign(std::move(buf));
        }
        else if constexpr(std::convertible_to<std::ranges::range_reference_t<R>, utf::codepoint>)
        {
            string_type buf;
            for(auto&& cp : r)
                utf::codepoint(cp).append_to(buf);

            assign(std::move(buf));
        }
        else
        {
            static_assert(!sizeof(R), "Invalid range");
        }

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

    basic_string_container& operator=(string_type&& str)
    {
        return assign(std::move(str));
    }

    basic_string_container& operator=(const string_type& str)
    {
        return assign(str);
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
            if(string_ref_type(it, sentinel).starts_with(str))
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

    constexpr string_view_type to_string_view() const noexcept
    {
        return std::visit(
            [](auto&& v)
            { return string_view_type(v); },
            std::as_const(m_data)
        );
    }

    constexpr operator string_view_type() const noexcept
    {
        return to_string_view();
    }

    constexpr operator string_ref_type() const noexcept
    {
        return string_ref_type(to_string_view());
    }

    template <char_like To = char>
    [[nodiscard]]
    std::basic_string<To> to_string() const
    {
        return string_ref_type(*this).template to_string<To>();
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

    void clear() noexcept
    {
        if(string_type* p = std::get_if<string_type>(&m_data); p)
            p->clear();
        else
            emplace_data<string_view_type>();
    }

    using my_base::begin;
    using my_base::end;

    class reference_proxy
    {
        friend basic_string_container;

        constexpr reference_proxy(basic_string_container* p, size_type off) noexcept
            : m_str(p), m_offset(off) {}

    public:
        reference_proxy(const reference_proxy&) = delete;

        constexpr bool operator==(const reference_proxy& rhs) const noexcept
        {
            return static_cast<char32_t>(*this) == static_cast<char32_t>(rhs);
        }

        constexpr operator codepoint() const noexcept
        {
            return m_str->cp_from_off(m_offset);
        }

        constexpr operator char32_t() const noexcept
        {
            return static_cast<codepoint>(*this);
        }

        constexpr const CharT* operator&() const noexcept
        {
            return m_str->data() + m_offset;
        }

        constexpr const reference_proxy& operator=(const reference_proxy& rhs) const
        {
            assign_cp(rhs);
            return *this;
        }

        constexpr const reference_proxy& operator=(codepoint cp) const
        {
            assign_cp(cp);
            return *this;
        }

    private:
        basic_string_container* m_str;
        size_type m_offset;

        void assign_cp(const codepoint& cp) const
        {
            std::uint8_t len = m_str->ch_size_for_cp(*&*this);

            string_type& str = m_str->to_str();
            cp.replace(str, m_offset, len);
        }
    };

    using reference = reference_proxy;

private:
    constexpr reference make_reference(size_type off)
    {
        return reference(this, off);
    }

public:
    using my_base::operator[];

    constexpr reference operator[](size_type idx) noexcept
    {
        size_type off = this->get_offset(idx);
        PAPILIO_ASSERT(off != npos);
        return make_reference(off);
    }

#ifdef PAPILIO_HAS_MULTIDIMENSIONAL_SUBSCRIPT

    constexpr reference operator[](reverse_index_t, size_type i) noexcept
    {
        return index(reverse_index, i);
    }

#endif

    // for consistency
    [[nodiscard]]
    constexpr reference index(size_type i) noexcept
    {
        return make_reference(this->get_offset(i));
    }

    // for pre-C++ 23
    [[nodiscard]]
    constexpr reference index(reverse_index_t, size_type i) noexcept
    {
        return make_reference(this->get_offset(reverse_index, i));
    }

    class iterator
    {
        friend class basic_string_container;

        iterator(basic_string_container* str, size_type offset)
            : m_str(str), m_offset(offset) {}

    public:
        using iterator_category = std::bidirectional_iterator_tag;

        using char_type = CharT;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using value_type = codepoint;
        using reference = reference_proxy;
        using const_reference = codepoint;

        iterator() noexcept
            : m_str(nullptr), m_offset(0) {}

        constexpr iterator(const iterator&) noexcept = default;

        constexpr iterator& operator=(const iterator&) noexcept = default;

        constexpr bool operator==(const iterator& rhs) const noexcept
        {
            return to_address() == rhs.to_address();
        }

        constexpr void swap(iterator& other) noexcept
        {
            using std::swap;
            swap(m_str, other.m_str);
            swap(m_offset, other.m_offset);
        }

        constexpr const CharT* to_address() const noexcept
        {
            if(!*this)
                return nullptr;
            return m_str->data() + m_offset;
        }

        constexpr operator codepoint_iterator<CharT>() const noexcept
        {
            if(!*this)
                return codepoint_iterator<CharT>();
            return codepoint_begin<CharT>(m_str->get_view().substr(m_offset));
        }

        constexpr reference operator*() const noexcept
        {
            PAPILIO_ASSERT(*this);
            return m_str->make_reference(m_offset);
        }

        constexpr operator bool() const noexcept
        {
            return m_str != nullptr;
        }

        constexpr iterator& operator++()
        {
            next_pos();
            return *this;
        }

        constexpr iterator& operator--()
        {
            prev_pos();
            return *this;
        }

        constexpr iterator operator++(int)
        {
            iterator tmp(*this);
            ++*this;
            return tmp;
        }

        constexpr iterator operator--(int)
        {
            iterator tmp(*this);
            --*this;
            return tmp;
        }

    private:
        basic_string_container* m_str;
        size_type m_offset;

        constexpr void next_pos()
        {
            PAPILIO_ASSERT(*this);
            m_offset += m_str->ch_size_for_cp(to_address()[m_offset]);
            if(m_offset > m_str->size())
                m_offset = m_str->size();
        }

        constexpr void prev_pos()
        {
            PAPILIO_ASSERT(*this);
            if constexpr(char8_like<CharT>)
            {
                const CharT* p = m_str->data();

                PAPILIO_ASSERT(m_offset != 0);
                --m_offset;
                size_type next_offset = m_offset;
                while(true)
                {
                    char8_t ch = static_cast<char8_t>(p[next_offset]);

                    if(m_offset - next_offset > 3) [[unlikely]]
                    {
                        break;
                    }
                    else if(PAPILIO_NS utf::is_leading_byte(ch))
                    {
                        m_offset = next_offset;
                        break;
                    }
                    else if(next_offset == 0)
                    {
                        break;
                    }

                    --next_offset;
                }
            }
            else if constexpr(char16_like<CharT>)
            {
                const CharT* p = m_str->data();

                PAPILIO_ASSERT(m_offset != 0);
                --m_offset;
                while(PAPILIO_NS utf::is_low_surrogate(p[m_offset]))
                    --m_offset;
            }
            else
            {
                --m_offset;
            }
        }
    };

    constexpr iterator begin()
    {
        return iterator(this, 0);
    }

    constexpr iterator end()
    {
        return iterator(this, size());
    }

    using reverse_iterator = std::reverse_iterator<iterator>;

    using my_base::rbegin;
    using my_base::rend;

    constexpr reverse_iterator rbegin()
    {
        return reverse_iterator(end());
    }

    constexpr reverse_iterator rend()
    {
        return reverse_iterator(begin());
    }

    template <typename Operation>
    constexpr void resize_and_overwrite(size_type count, Operation op)
    {
        string_type& str = to_str();

#if defined(__cpp_lib_string_resize_and_overwrite) && __cpp_lib_string_resize_and_overwrite >= 202110L
        str.resize_and_overwrite(count, std::move(op));

#else
        str.resize(count);
        size_type result = std::move(op)(str.data(), str.size());
        str.resize(result);

#endif
    }

private:
    // String data type
    using string_store = std::variant<
        std::basic_string<CharT>, // has ownership
        std::basic_string_view<CharT>>; // no ownership

    mutable string_store m_data;

    constexpr string_type& to_str() const
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
    constexpr T& emplace_data(Args&&... args) const noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        return m_data.template emplace<T>(std::forward<Args>(args)...);
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
constexpr bool operator==(const basic_string_container<T>& lhs, const basic_string_container<U>& rhs) noexcept(std::same_as<T, U>)
{
    return basic_string_ref<T>(lhs) == basic_string_ref<U>(rhs);
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(const basic_string_container<T>& lhs, basic_string_ref<U> rhs) noexcept(std::same_as<T, U>)
{
    return basic_string_ref<T>(lhs) == rhs;
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(basic_string_ref<T> lhs, const basic_string_container<U>& rhs) noexcept(std::same_as<T, U>)
{
    return lhs == basic_string_ref<U>(rhs);
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(const basic_string_container<T>& lhs, const U* rhs) noexcept(std::same_as<T, U>)
{
    return lhs == basic_string_ref<U>(rhs);
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(const T* lhs, const basic_string_container<U>& rhs) noexcept(std::same_as<T, U>)
{
    return basic_string_ref<T>(lhs) == rhs;
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(const basic_string_container<T>& lhs, std::basic_string_view<U> rhs) noexcept(std::same_as<T, U>)
{
    return lhs == basic_string_ref<U>(rhs);
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(std::basic_string_view<T> lhs, const basic_string_container<U>& rhs) noexcept(std::same_as<T, U>)
{
    return basic_string_ref<T>(lhs) == rhs;
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(const basic_string_container<T>& lhs, const std::basic_string<U>& rhs) noexcept(std::same_as<T, U>)
{
    return lhs == basic_string_ref<U>(rhs);
}

PAPILIO_EXPORT template <char_like T, char_like U>
constexpr bool operator==(const std::basic_string<T>& lhs, const basic_string_container<U>& rhs) noexcept(std::same_as<T, U>)
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
    PAPILIO_EXPORT inline string_container operator""_sc(
        const char* str, std::size_t size
    ) noexcept
    {
        return string_container(str, size);
    }

    PAPILIO_EXPORT inline u8string_container operator""_sc(
        const char8_t* str, std::size_t size
    ) noexcept
    {
        return u8string_container(str, size);
    }

    PAPILIO_EXPORT inline u16string_container operator""_sc(
        const char16_t* str, std::size_t size
    ) noexcept
    {
        return u16string_container(str, size);
    }

    PAPILIO_EXPORT inline u32string_container operator""_sc(
        const char32_t* str, std::size_t size
    ) noexcept
    {
        return u32string_container(str, size);
    }

    PAPILIO_EXPORT inline wstring_container operator""_sc(
        const wchar_t* str, std::size_t size
    ) noexcept
    {
        return wstring_container(str, size);
    }
} // namespace literals
} // namespace papilio::utf

#include "../detail/suffix.hpp"

#endif
