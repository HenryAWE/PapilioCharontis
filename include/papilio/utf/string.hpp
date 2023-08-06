#pragma once

#include <string>
#include <algorithm>
#include "common.hpp"
#include "codepoint.hpp"


namespace papilio::utf
{
    template <typename CharT>
    class basic_string_ref;

    namespace detail
    {
        class str_base
        {
        public:
            using size_type = std::size_t;

            static constexpr size_type npos = utf::npos;
        };

        template <char_like CharT, typename Derived>
        class str_ref_impl_base : public str_base
        {
        public:
            using value_type = codepoint;
            using char_type = CharT;
            using size_type = std::size_t;
            using string_view_type = std::basic_string_view<CharT>;

        public:
            constexpr str_ref_impl_base& operator=(const str_ref_impl_base&) noexcept = default;

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
            constexpr codepoint operator[](size_type i) const noexcept
            {
                return as_derived().index(i);
            }
#ifdef __cpp_multidimensional_subscript
            // TODO: Support multidimensional subscript operator in C++ 23
            // codepoint operator[](reverse_index_t, size_type i) const noexcept
#endif

            [[nodiscard]]
            constexpr size_type size() const noexcept
            {
                return get_view().size();
            }
            [[nodiscard]]
            constexpr size_type length() const noexcept
            {
                return utf::strlen(get_view());
            }

            [[nodiscard]]
            constexpr bool empty() const noexcept
            {
                return size() == 0;
            }

            [[nodiscard]]
            constexpr codepoint front() const noexcept
            {
                return *as_derived().begin();
            }
            [[nodiscard]]
            constexpr codepoint back() const noexcept
            {
                return *--as_derived().end();
            }

            [[nodiscard]]
            constexpr auto find(Derived str, size_type pos = 0) const noexcept
            {
                return this->find_impl(str, pos);
            }
            [[nodiscard]]
            constexpr auto find(codepoint ch, size_type pos = 0) const noexcept
            {
                string_view_type v(ch);
                return find_impl(Derived(v), pos);
            }
            [[nodiscard]]
            constexpr auto find(const CharT* str, size_type pos, size_type count) const noexcept
            {
                return find_impl(
                    Derived(str, count), pos
                );
            }
            [[nodiscard]]
            constexpr auto find(const CharT* str, size_type pos = 0) const noexcept
            {
                return find_impl(
                    Derived(str), pos
                );
            }

            [[nodiscard]]
            constexpr bool contains(Derived str) const noexcept
            {
                return this->find(str) != this->end();
            }
            [[nodiscard]]
            constexpr bool contains(codepoint ch) const noexcept
            {
                return this->find(ch) != this->end();
            }

            constexpr void swap(str_ref_impl_base& other) noexcept
            {
                using std::swap;
                swap(m_str, other.m_str);
            }

            constexpr void remove_prefix(size_type n) noexcept
            {
                auto it = as_derived().cbegin();
                auto sentinel = as_derived().cend();
                for(size_type i = 0; i < n; ++i)
                {
                    if(it == sentinel)
                        break;
                    ++it;
                }

                set_view(string_view_type(it.to_address(), sentinel.to_address()));
            }
            constexpr void remove_suffix(size_type n) noexcept
            {
                auto it = as_derived().crbegin();
                auto sentinel = as_derived().crend();
                for(size_type i = 0; i < n; ++i)
                {
                    if(it == sentinel)
                        break;
                    ++it;
                }

                set_view(string_view_type(sentinel.base().to_address(), it.base().to_address()));
            }

            template <substr_behavior OnOutOfRange = substr_behavior::exception>
            [[nodiscard]]
            constexpr std::pair<Derived, size_type> substr_extended(size_type pos = 0, size_type count = npos)
                const noexcept(OnOutOfRange != substr_behavior::exception)
            {
                const auto sentinel = as_derived().cend();

                auto start = as_derived().cbegin();
                for(size_type i = 0; i < pos; ++i)
                {
                    if(start == sentinel)
                    {
                        // out of range
                        if constexpr(OnOutOfRange == substr_behavior::exception)
                            throw std::out_of_range("out of range");
                        else
                            return std::make_pair(Derived(), 0);
                    }

                    ++start;
                }

                size_type n = 0;
                auto stop = start;
                for(size_type i = 0; i < count; ++i)
                {
                    if(stop == sentinel)
                        break;
                    ++n;
                    ++stop;
                }

                return std::make_pair(Derived(start, stop), n);
            }
            template <substr_behavior OnOutOfRange = substr_behavior::exception>
            [[nodiscard]]
            constexpr Derived substr(size_type pos = 0, size_type count = npos)
                const noexcept(OnOutOfRange != substr_behavior::exception)
            {
                return substr_extended<OnOutOfRange>(pos, count).first;
            }

            [[nodiscard]]
            std::string to_string() const
            {
                return as_derived().template as_string<char>();
            }
            [[nodiscard]]
            std::u8string to_u8string() const
            {
                return as_derived().template as_string<char8_t>();
            }
            [[nodiscard]]
            std::u16string to_u16string() const
            {
                return as_derived().template as_string<char16_t>();
            }
            [[nodiscard]]
            std::u32string to_u32string() const
            {
                return as_derived().template as_string<char32_t>();
            }
            [[nodiscard]]
            std::wstring to_wstring() const
            {
                return as_derived().template as_string<wchar_t>();
            }

        protected:
            constexpr str_ref_impl_base() noexcept = default;
            constexpr str_ref_impl_base(const str_ref_impl_base&) noexcept = default;
            constexpr str_ref_impl_base(string_view_type str) noexcept
                : m_str(str) {}

            constexpr string_view_type get_view() const noexcept
            {
                return m_str;
            }
            constexpr void set_view(string_view_type new_str) noexcept
            {
                m_str = new_str;
            }

            constexpr Derived& as_derived() noexcept
            {
                return static_cast<Derived&>(*this);
            }
            constexpr const Derived& as_derived() const noexcept
            {
                return static_cast<const Derived&>(*this);
            }

            template <typename Other>
            constexpr int compare_impl(const Other& other) const noexcept
            {
                auto i = as_derived().cbegin();
                auto j = other.cbegin();

                while(true)
                {
                    if(i == as_derived().cend())
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

            constexpr auto find_impl(const Derived& v, size_type pos) const noexcept
            {
                auto it = as_derived().cbegin();
                auto sentinel = as_derived().cend();

                for(size_type n = 0; n < pos; ++n)
                {
                    if(it == sentinel)
                        return sentinel;
                    ++it;
                }

                auto v_begin = v.cbegin();
                auto v_end = v.cend();
                for(; it != as_derived().cend(); ++it)
                {
                    auto result = std::mismatch(it, sentinel, v_begin, v_end).second;
                    if(result == v_end)
                        break;
                }

                return it;
            }
            constexpr auto find_impl(codepoint cp, size_type pos) const noexcept
            {
                auto it = as_derived().cbegin();
                for(size_type n = 0; n < pos; ++n)
                {
                    auto sentinel = as_derived().cend();
                    if(it == sentinel)
                        return sentinel;
                    ++it;
                }

                return std::find(it, as_derived().cend(), cp);
            }

        private:
            string_view_type m_str;
        };

        template <typename CharT, typename Derived>
        class str_ref_impl;

        template <char_8b CharT, typename Derived>
        class str_ref_impl<CharT, Derived> : public str_ref_impl_base<CharT, Derived>
        {
            using base = str_ref_impl_base<CharT, Derived>;
        public:
            static_assert(sizeof(CharT) == 1);

            using value_type = codepoint;
            using char_type = CharT;
            using size_type = std::size_t;
            using string_view_type = std::basic_string_view<CharT>;

            class const_iterator
            {
            public:
                using size_type = std::size_t;
                using difference_type = std::ptrdiff_t;
                using value_type = codepoint;
                using reference = codepoint;

                using char_type = CharT;

                constexpr const_iterator() noexcept = default;
                constexpr const_iterator(const const_iterator&) noexcept = default;

            private:
                friend class str_ref_impl;
                constexpr const_iterator(string_view_type str, size_type offset, std::uint8_t len) noexcept
                    : m_str(str), m_offset(offset), m_len(len) {}

            public:
                constexpr const_iterator& operator=(const const_iterator&) noexcept = default;

                [[nodiscard]]
                constexpr bool operator==(const const_iterator& rhs) const noexcept
                {
                    return to_address() == rhs.to_address();
                }

                constexpr const_iterator& operator++() noexcept
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

                    return *this;
                }
                constexpr const_iterator operator++(int) noexcept
                {
                    const_iterator tmp(*this);
                    ++*this;
                    return tmp;
                }

                constexpr const_iterator& operator--() noexcept
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

                    return *this;
                }
                constexpr const_iterator operator--(int) noexcept
                {
                    const_iterator tmp(*this);
                    --*this;
                    return tmp;
                }

                constexpr const_iterator& operator+=(difference_type diff) noexcept
                {
                    if(diff > 0)
                    {
                        for(difference_type i = 0; i < diff; ++i)
                            ++*this;
                    }
                    else if(diff < 0)
                    {
                        diff = -diff;
                        for(difference_type i = 0; i < diff; ++i)
                            --*this;
                    }

                    return *this;
                }
                constexpr const_iterator& operator-=(difference_type diff) noexcept
                {
                    return *this += -diff;
                }

                [[nodiscard]]
                friend constexpr const_iterator operator+(const_iterator lhs, difference_type rhs) noexcept
                {
                    return lhs += rhs;
                }
                [[nodiscard]]
                friend constexpr const_iterator operator+(difference_type lhs, const_iterator rhs) noexcept
                {
                    return rhs += lhs;
                }

                [[nodiscard]]
                friend constexpr const_iterator operator-(const_iterator lhs, difference_type rhs) noexcept
                {
                    return lhs -= rhs;
                }


                [[nodiscard]]
                friend constexpr difference_type operator-(const const_iterator& lhs, const const_iterator& rhs) noexcept
                {
                    if(lhs.to_address() < rhs.to_address())
                    {
                        return -(rhs - *lhs);
                    }
                    else
                    {
                        difference_type diff = 0;

                        while(lhs != rhs && rhs)
                        {
                            ++rhs;
                            ++diff;
                        }

                        return diff;
                    }
                }

                [[nodiscard]]
                constexpr reference operator*() const noexcept
                {
                    return codepoint(to_address(), m_len);
                }

                explicit operator bool() const noexcept
                {
                    return !m_str.empty();
                }

                [[nodiscard]]
                constexpr const CharT* to_address() const noexcept
                {
                    return m_str.data() + m_offset;
                }
                // byte count
                [[nodiscard]]
                constexpr std::uint8_t size() const noexcept
                {
                    return m_len;
                }

                constexpr void swap(const_iterator& other) noexcept
                {
                    using std::swap;
                    swap(m_str, other.m_str);
                    swap(m_offset, other.m_offset);
                    swap(m_len, other.m_len);
                }
                friend constexpr void swap(const_iterator& lhs, const_iterator& rhs) noexcept
                {
                    lhs.swap(rhs);
                }

            private:
                string_view_type m_str;
                size_type m_offset = 0;
                std::uint8_t m_len = 0;
            };

            using iterator = const_iterator;
            using const_reverse_iterator = std::reverse_iterator<const_iterator>;
            using reverse_iterator = const_reverse_iterator;

            // for consistency
            [[nodiscard]]
            constexpr codepoint index(size_type i) const noexcept
            {
                std::size_t off = this->get_offset(i);
                PAPILIO_ASSUME(off != npos);

                string_view_type str = this->get_view();
                std::uint8_t len = utf::byte_count(str[off]);
                return codepoint(str.data() + off, len);
            }
            // pre-C++ 23 version
            [[nodiscard]]
            constexpr codepoint index(reverse_index_t, size_type i) const noexcept
            {
                std::size_t off = this->get_offset(reverse_index, i);
                PAPILIO_ASSUME(off != npos);

                string_view_type str = this->get_view();
                std::uint8_t len = utf::byte_count(str[off]);
                return codepoint(str.data() + off, len);
            }
            [[nodiscard]]
            constexpr codepoint index_or(size_type i, codepoint default_val) const noexcept
            {
                std::size_t off = this->get_offset(i);
                if(off == npos)
                    return default_val;

                string_view_type str = this->get_view();
                std::uint8_t len = utf::byte_count(str[off]);
                return codepoint(str.data(), len);
            }
            [[nodiscard]]
            constexpr codepoint index_or(reverse_index_t, size_type i, codepoint default_val) const noexcept
            {
                std::size_t off = this->get_offset(reverse_index, i);
                if(off == npos)
                    return default_val;

                string_view_type str = this->get_view();
                std::uint8_t len = utf::byte_count(str[off]);
                return codepoint(str.data(), len);
            }

            constexpr const_iterator cbegin() const noexcept
            {
                string_view_type str = this->get_view();
                if(this->empty())
                {
                    return make_iter(str, 0, 0);
                }
                else [[likely]]
                {
                    char8_t ch = str.data()[0];
                    std::uint8_t len =
                        is_leading_byte(ch) ?
                        byte_count(ch) : 1;
                    return make_iter(str, 0, len);
                }
            }
            constexpr const_iterator cend() const noexcept
            {
                string_view_type str = this->get_view();
                return make_iter(str, str.size(), 0);
            }

            constexpr iterator begin() const noexcept { return cbegin(); }
            constexpr iterator end() const noexcept { return cend(); }

            constexpr const_reverse_iterator crbegin() const noexcept
            {
                return const_reverse_iterator(cend());
            }
            constexpr const_reverse_iterator crend() const noexcept
            {
                return const_reverse_iterator(cbegin());
            }
            constexpr reverse_iterator rbegin() const noexcept { return crbegin(); }
            constexpr reverse_iterator rend() const noexcept { return crend(); }

            template <char_like To = CharT>
            [[nodiscard]]
            std::basic_string<To> as_string() const
            {
                if constexpr(std::is_same_v<To, char>)
                {
                    string_view_type sv = this->get_view();

                    if constexpr(std::is_same_v<CharT, char>)
                        return std::string(sv);
                    else // char8_t
                        return std::string(reinterpret_cast<const char*>(sv.data()), sv.size());
                }
                else if constexpr(std::is_same_v<To, char8_t>)
                {
                    string_view_type sv = this->get_view();

                    if constexpr(std::is_same_v<CharT, char>)
                        return std::u8string(reinterpret_cast<const char8_t*>(sv.data()), sv.size());
                    else // char8_t
                        return std::u8string(sv);
                }
                else if constexpr(std::is_same_v<To, char32_t>)
                {
                    std::u32string result;

                    for(auto&& cp : *this)
                    {
                        result.push_back(cp);
                    }

                    return std::move(result);
                }
                else // char16_t and wchar_t
                {
                    std::basic_string<To> result;

                    for(auto&& cp : *this)
                    {
                        auto ch = decoder<To>::from_codepoint(cp);
                        result.append(ch);
                    }

                    return result;
                }
            }

        protected:
            using base::base;

            static constexpr const_iterator make_iter(string_view_type str, size_type offset, std::uint8_t len) noexcept
            {
                return const_iterator(str, offset, len);
            }

        private:
            using base::as_derived;
        };

        template <char_16b CharT, typename Derived>
        class str_ref_impl<CharT, Derived> : public str_ref_impl_base<CharT, Derived>
        {
            using base = str_ref_impl_base<CharT, Derived>;
        public:
            using value_type = codepoint;
            using char_type = CharT;
            using size_type = std::size_t;
            using string_view_type = std::basic_string_view<CharT>;

            class const_iterator
            {
            public:
                using size_type = std::size_t;
                using difference_type = std::ptrdiff_t;
                using value_type = codepoint;
                using reference = codepoint;

                using char_type = CharT;

                constexpr const_iterator() noexcept = default;
                constexpr const_iterator(const const_iterator&) noexcept = default;

            private:
                friend class str_ref_impl;
                constexpr const_iterator(string_view_type str, size_type offset, std::uint8_t len) noexcept
                    : m_str(str), m_offset(offset), m_len(len) {}

            public:
                constexpr const_iterator& operator=(const const_iterator&) noexcept = default;

                [[nodiscard]]
                constexpr bool operator==(const const_iterator& rhs) const noexcept
                {
                    return to_address() == rhs.to_address();
                }

                constexpr const_iterator& operator++() noexcept
                {
                    size_type next_offset = m_offset + m_len;
                    if(next_offset < m_str.size())
                    {
                        m_offset = next_offset;
                        std::uint16_t ch = m_str[next_offset];
                        if(is_high_surrogate(ch))
                            m_len = 2;
                        else
                            m_len = 1;
                    }
                    else
                    {
                        m_offset = m_str.size();
                        m_len = 0;
                    }

                    return *this;
                }
                constexpr const_iterator operator++(int) noexcept
                {
                    const_iterator tmp(*this);
                    ++*this;
                    return tmp;
                }

                constexpr const_iterator& operator--() noexcept
                {
                    PAPILIO_ASSUME(m_offset != 0);
                    --m_offset;
                    while(!is_low_surrogate(m_str[m_offset]))
                        --m_offset;

                    return *this;
                }
                constexpr const_iterator operator--(int) noexcept
                {
                    const_iterator tmp(*this);
                    --*this;
                    return tmp;
                }

                constexpr const_iterator& operator+=(difference_type diff) noexcept
                {
                    if(diff > 0)
                    {
                        for(difference_type i = 0; i < diff; ++i)
                            ++*this;
                    }
                    else if(diff < 0)
                    {
                        diff = -diff;
                        for(difference_type i = 0; i < diff; ++i)
                            --*this;
                    }

                    return *this;
                }
                constexpr const_iterator& operator-=(difference_type diff) noexcept
                {
                    return *this += -diff;
                }

                [[nodiscard]]
                friend constexpr const_iterator operator+(const_iterator lhs, difference_type rhs) noexcept
                {
                    return lhs += rhs;
                }
                [[nodiscard]]
                friend constexpr const_iterator operator+(difference_type lhs, const_iterator rhs) noexcept
                {
                    return rhs += lhs;
                }
                [[nodiscard]]
                friend constexpr const_iterator operator-(const_iterator lhs, difference_type rhs) noexcept
                {
                    return lhs -= rhs;
                }
                [[nodiscard]]
                friend constexpr const_iterator operator-(difference_type lhs, const_iterator rhs) noexcept
                {
                    return rhs -= lhs;
                }

                [[nodiscard]]
                constexpr difference_type operator-(const_iterator rhs) const noexcept
                {
                    if(to_address() < rhs.to_address())
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

                [[nodiscard]]
                constexpr reference operator*() const
                {
                    return decoder<CharT>::to_codepoint(
                        m_str.substr(m_offset, m_len)
                    ).first;
                }

                explicit operator bool() const noexcept
                {
                    return !m_str.empty();
                }

                [[nodiscard]]
                constexpr const CharT* to_address() const noexcept
                {
                    return m_str.data() + m_offset;
                }
                // byte count
                [[nodiscard]]
                constexpr std::uint8_t size() const noexcept
                {
                    return m_len;
                }

                constexpr void swap(const_iterator& other) noexcept
                {
                    using std::swap;
                    swap(m_str, other.m_str);
                    swap(m_offset, other.m_offset);
                    swap(m_len, other.m_len);
                }
                friend constexpr void swap(const_iterator& lhs, const_iterator& rhs) noexcept
                {
                    lhs.swap(rhs);
                }

            private:
                string_view_type m_str;
                std::size_t m_offset = 0;
                std::uint8_t m_len = 0;
            };

            using iterator = const_iterator;
            using const_reverse_iterator = std::reverse_iterator<const_iterator>;
            using reverse_iterator = const_reverse_iterator;

            // for consistency
            [[nodiscard]]
            constexpr codepoint index(size_type i) const noexcept
            {
                std::size_t off = this->get_offset(i);
                PAPILIO_ASSUME(off != npos);

                string_view_type str = this->get_view();
                std::uint8_t len = utf::is_high_surrogate(str[off]) ? 2 : 1;
                return decoder<CharT>::to_codepoint(
                    str.substr(off, len)
                ).first;
            }
            // pre-C++ 23 version
            [[nodiscard]]
            constexpr codepoint index(reverse_index_t, size_type i) const noexcept
            {
                std::size_t off = this->get_offset(reverse_index, i);
                PAPILIO_ASSUME(off != npos);

                string_view_type str = this->get_view();
                std::uint8_t len = utf::is_high_surrogate(str[off]) ? 2 : 1;
                return decoder<CharT>::to_codepoint(
                    str.substr(off, len)
                ).first;
            }
            [[nodiscard]]
            constexpr codepoint index_or(size_type i, codepoint default_val) const noexcept
            {
                std::size_t off = this->get_offset(i);
                if(off == npos)
                    return default_val;

                string_view_type str = this->get_view();
                std::uint8_t len = utf::is_high_surrogate(str[off]) ? 2 : 1;
                return decoder<CharT>::to_codepoint(
                    str.substr(off, len)
                ).first;
            }
            [[nodiscard]]
            constexpr codepoint index_or(reverse_index_t, size_type i, codepoint default_val) const noexcept
            {
                std::size_t off = this->get_offset(reverse_index, i);
                if(off == npos)
                    return default_val;

                string_view_type str = this->get_view();
                std::uint8_t len = utf::is_high_surrogate(str[off]) ? 2 : 1;
                return decoder<CharT>::to_codepoint(
                    str.substr(off, len)
                ).first;
            }

            constexpr const_iterator cbegin() const noexcept
            {
                string_view_type str = this->get_view();
                if(this->empty())
                {
                    return make_iter(str, 0, 0);
                }
                else [[likely]]
                {
                    std::uint16_t ch = str.data()[0];
                    std::uint8_t len = is_high_surrogate(ch) ? 2 : 1;
                    return make_iter(str, 0, len);
                }
            }
            constexpr const_iterator cend() const noexcept
            {
                string_view_type str = this->get_view();
                return make_iter(str, str.size(), 0);
            }

            constexpr iterator begin() const noexcept { return cbegin(); }
            constexpr iterator end() const noexcept { return cend(); }

            constexpr const_reverse_iterator crbegin() const noexcept
            {
                return const_reverse_iterator(cend());
            }
            constexpr const_reverse_iterator crend() const noexcept
            {
                return const_reverse_iterator(cbegin());
            }
            constexpr reverse_iterator rbegin() const noexcept { return crbegin(); }
            constexpr reverse_iterator rend() const noexcept { return crend(); }

            template <char_like To = CharT>
            std::basic_string<To> as_string() const
            {
                using result_t = std::basic_string<To>;
                using view_t = std::basic_string_view<To>;

                if constexpr(char_16b<To>)
                {
                    string_view_type str = this->get_view();

                    if constexpr(std::is_same_v<CharT, To>)
                        return result_t(this->get_view());
                    else
                    {
                        return result_t(
                            std::bit_cast<const To*>(str.data()),
                            str.size()
                        );
                    }
                }
                else if constexpr(char_8b<To>)
                {
                    result_t result;

                    for(auto&& i : as_derived())
                        result.append(view_t(i));

                    return result;
                }
                else if constexpr(char_32b<To>)
                {
                    result_t result;

                    for(auto&& i : as_derived())
                        result += (static_cast<char32_t>(i));

                    return result;
                }
            }

        protected:
            using base::base;

            static constexpr const_iterator make_iter(string_view_type str, size_type offset, std::uint8_t len) noexcept
            {
                return const_iterator(str, offset, len);
            }

        private:
            using base::as_derived;
        };

        template <char_32b CharT, typename Derived>
        class str_ref_impl<CharT, Derived> : public str_ref_impl_base<CharT, Derived>
        {
            using base = str_ref_impl_base<CharT, Derived>;
        public:
            using value_type = codepoint;
            using char_type = CharT;
            using size_type = std::size_t;
            using string_view_type = std::basic_string_view<CharT>;

            class const_iterator
            {
            public:
                using size_type = std::size_t;
                using difference_type = std::ptrdiff_t;
                using value_type = codepoint;
                using reference = codepoint;

                using char_type = CharT;

                constexpr const_iterator() noexcept = default;
                constexpr const_iterator(const const_iterator&) noexcept = default;

            private:
                friend str_ref_impl;
                constexpr const_iterator(string_view_type::const_iterator iter) noexcept
                    : m_iter(iter) {}

            public:
                constexpr const_iterator& operator=(const const_iterator&) noexcept = default;

                [[nodiscard]]
                constexpr bool operator==(const const_iterator& rhs) const noexcept
                {
                    return to_address() == rhs.to_address();
                }

                constexpr const_iterator& operator++() noexcept
                {
                    ++m_iter;
                    return *this;
                }
                constexpr const_iterator operator++(int) noexcept
                {
                    const_iterator tmp(*this);
                    ++*this;
                    return tmp;
                }

                constexpr const_iterator& operator--() noexcept
                {
                    --m_iter;
                    return *this;
                }
                constexpr const_iterator operator--(int) noexcept
                {
                    const_iterator tmp(*this);
                    --*this;
                    return tmp;
                }

                constexpr const_iterator& operator+=(difference_type diff) noexcept
                {
                    if(diff > 0)
                    {
                        for(difference_type i = 0; i < diff; ++i)
                            ++*this;
                    }
                    else if(diff < 0)
                    {
                        diff = -diff;
                        for(difference_type i = 0; i < diff; ++i)
                            --*this;
                    }

                    return *this;
                }
                constexpr const_iterator& operator-=(difference_type diff) noexcept
                {
                    return *this += -diff;
                }

                [[nodiscard]]
                friend constexpr const_iterator operator+(const_iterator lhs, difference_type rhs) noexcept
                {
                    return lhs += rhs;
                }
                [[nodiscard]]
                friend constexpr const_iterator operator+(difference_type lhs, const_iterator rhs) noexcept
                {
                    return rhs += lhs;
                }
                [[nodiscard]]
                friend constexpr const_iterator operator-(const_iterator lhs, difference_type rhs) noexcept
                {
                    return lhs -= rhs;
                }
                [[nodiscard]]
                friend constexpr const_iterator operator-(difference_type lhs, const_iterator rhs) noexcept
                {
                    return rhs -= lhs;
                }

                [[nodiscard]]
                constexpr difference_type operator-(const_iterator rhs) const noexcept
                {
                    if(to_address() < rhs.to_address())
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

                [[nodiscard]]
                constexpr reference operator*() const
                {
                    return decoder<char32_t>::to_codepoint(*m_iter).first;
                }

                explicit operator bool() const noexcept
                {
                    return static_cast<bool>(m_iter);
                }

                [[nodiscard]]
                constexpr const CharT* to_address() const noexcept
                {
                    return std::to_address(m_iter);
                }
                // byte count
                [[nodiscard]]
                constexpr std::uint8_t size() const noexcept
                {
                    return 1;
                }

                constexpr void swap(const_iterator& other) noexcept
                {
                    using std::swap;
                    swap(m_iter, other.m_iter);
                }
                friend constexpr void swap(const_iterator& lhs, const_iterator& rhs) noexcept
                {
                    lhs.swap(rhs);
                }

            private:
                string_view_type::const_iterator m_iter;
            };

            using iterator = const_iterator;
            using const_reverse_iterator = std::reverse_iterator<const_iterator>;
            using reverse_iterator = const_reverse_iterator;

            // for consistency
            [[nodiscard]]
            constexpr codepoint index(size_type i) const noexcept
            {
                string_view_type str = this->get_view();
                PAPILIO_ASSUME(i < str.size());
                return decoder<char32_t>::to_codepoint(
                    str[i]
                ).first;
            }
            // pre-C++ 23 version
            [[nodiscard]]
            constexpr codepoint index(reverse_index_t, size_type i) const noexcept
            {
                string_view_type str = this->get_view();
                PAPILIO_ASSUME(i < str.size());

                return decoder<char32_t>::to_codepoint(
                    str[str.size() - 1 - i]
                ).first;
            }
            [[nodiscard]]
            constexpr codepoint index_or(size_type i, codepoint default_val) const noexcept
            {
                string_view_type str = this->get_view();
                if(i >= str.size())
                    return default_val;
                return decoder<char32_t>::to_codepoint(
                    str[i]
                ).first;
            }
            [[nodiscard]]
            constexpr codepoint index_or(reverse_index_t, size_type i, codepoint default_val) const noexcept
            {
                string_view_type str = this->get_view();
                if(i >= str.size())
                    return default_val;
                return decoder<char32_t>::to_codepoint(
                    str[str.size() - 1 - i]
                ).first;
            }

            constexpr const_iterator cbegin() const noexcept
            {
                return const_iterator(this->get_view().begin());
            }
            constexpr const_iterator cend() const noexcept
            {
                return const_iterator(this->get_view().end());
            }

            constexpr iterator begin() const noexcept { return cbegin(); }
            constexpr iterator end() const noexcept { return cend(); }

            constexpr const_reverse_iterator crbegin() const noexcept
            {
                return const_reverse_iterator(cend());
            }
            constexpr const_reverse_iterator crend() const noexcept
            {
                return const_reverse_iterator(cbegin());
            }
            constexpr reverse_iterator rbegin() const noexcept { return crbegin(); }
            constexpr reverse_iterator rend() const noexcept { return crend(); }

            template <char_like To>
            std::basic_string<To> as_string() const
            {
                using result_t = std::basic_string<To>;

                if constexpr(char_32b<To>)
                {
                    string_view_type str = this->get_view();

                    if constexpr(std::is_same_v<CharT, To>)
                        return result_t(this->get_view());
                    else
                    {
                        return result_t(
                            std::bit_cast<const To*>(str.data()),
                            str.size()
                        );
                    }
                }
                else if constexpr(char_8b<To>)
                {
                    result_t result;

                    for(auto&& i : as_derived())
                        result.append(std::basic_string_view<To>(i));

                    return result;
                }
                else if constexpr(char_16b<To>)
                {
                    result_t result;

                    for(auto&& i : as_derived())
                    {
                        auto ch = decoder<To>::from_codepoint(i);
                        result.append(std::basic_string_view<To>(ch));
                    }

                    return result;
                }
            }

        protected:
            using base::base;

        private:
            using base::as_derived;
        };

        template <char_8b To, char_8b From>
        constexpr std::basic_string_view<To> conv_narrow_str(const From* src) noexcept
        {
            if constexpr(std::is_same_v<To, From>)
                return src;
            else
                return std::bit_cast<const To*>(src);
        }
        template <char_8b To, char_8b From>
        constexpr std::basic_string_view<To> conv_narrow_str(const From* src, std::size_t size) noexcept
        {
            if constexpr(std::is_same_v<To, From>)
                return src;
            else
            {
                return std::basic_string_view<To>(
                    std::bit_cast<const To*>(src),
                    size
                );
            }
        }
        template <char_8b To, char_8b From>
        constexpr std::basic_string_view<To> conv_narrow_str(std::basic_string_view<From> src) noexcept
        {
            if constexpr(std::is_same_v<To, From>)
                return src;
            {
                return std::basic_string_view<To>(
                    std::bit_cast<const To*>(src.data()),
                    src.size()
                );
            }
        }
        template <char_8b To, char_8b From>
        constexpr std::basic_string_view<To> conv_narrow_str(const std::basic_string<From>& src) noexcept
        {
            if constexpr(std::is_same_v<To, From>)
                return src;
            else
            {
                return std::basic_string_view<To>(
                    std::bit_cast<const To*>(src.data()),
                    src.size()
                );
            }
        }
    }

#define PAPILIO_UTF_STRING_REF_COMAPRE() \
    template <char_like U>\
    constexpr int compare(basic_string_ref<U> other) const noexcept\
    {\
        return this->compare_impl(other);\
    }\
    template <char_like U>\
    constexpr int compare(const U* other) const noexcept\
    {\
        return this->compare_impl(basic_string_ref<U>(other));\
    }\
    template <char_like U>\
    constexpr int compare(std::basic_string_view<U> other) const noexcept\
    {\
        return this->compare_impl(basic_string_ref<U>(other));\
    }\
    template <char_like U>\
    constexpr int compare(const std::basic_string<U>& other) const noexcept\
    {\
        return this->compare_impl(basic_string_ref<U>(other));\
    }

    template <char_8b CharT>
    class basic_string_ref<CharT> : public detail::str_ref_impl<CharT, basic_string_ref<CharT>>
    {
        using base = detail::str_ref_impl<CharT, basic_string_ref>;
    public:
        using char_type = CharT;
        using value_type = base::value_type;
        using size_type = std::size_t;
        using string_type = std::basic_string<CharT>;
        using string_view_type = std::basic_string_view<CharT>;

        using const_iterator = base::const_iterator;
        using iterator = base::iterator;
        using const_reverse_iterator = base::const_reverse_iterator;
        using reverse_iterator = base::reverse_iterator;

        constexpr basic_string_ref() noexcept = default;
        template <char_8b U>
        explicit(!std::is_same_v<CharT, U>) constexpr basic_string_ref(const basic_string_ref<U>& other) noexcept
            : base(detail::conv_narrow_str<CharT>(other.data(), other.size())) {}
        constexpr basic_string_ref(std::nullptr_t) = delete;

        constexpr basic_string_ref(const CharT* str, size_type count) noexcept
            : base(detail::conv_narrow_str<CharT>(str, count)) {}
        template <char_8b U>
        explicit(!std::is_same_v<CharT, U>) constexpr basic_string_ref(const U* str) noexcept
            : base(detail::conv_narrow_str<CharT>(str)) {}
        template <char_8b U>
        explicit(!std::is_same_v<CharT, U>) constexpr basic_string_ref(std::basic_string_view<U> str) noexcept
            : base(detail::conv_narrow_str<CharT>(str)) {}
        template <char_8b U>
        explicit(!std::is_same_v<CharT, U>) constexpr basic_string_ref(const std::basic_string<U>& str) noexcept
            : base(detail::conv_narrow_str<CharT>(str)) {}
        template <typename Iterator, typename Sentinel>
            requires std::is_same_v<std::iter_value_t<Iterator>, CharT>
        constexpr basic_string_ref(Iterator start, Sentinel stop)
            : base(string_view_type(start, stop)) {}
        constexpr basic_string_ref(const_iterator start, const_iterator stop) noexcept
            : base(string_view_type(start.to_address(), stop.to_address())) {}

        constexpr basic_string_ref& operator=(const basic_string_ref&) noexcept = default;
        constexpr basic_string_ref& operator=(std::nullptr_t) noexcept = delete;
        constexpr basic_string_ref& operator=(const CharT* str) noexcept
        {
            PAPILIO_ASSUME(str != nullptr);
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

        PAPILIO_UTF_STRING_REF_COMAPRE();

        constexpr operator string_view_type() const noexcept
        {
            return this->get_view();
        }

        template <char_8b U>
        explicit(!std::is_same_v<CharT, U>) constexpr operator std::span<const U>() const noexcept
        {
            string_view_type str = this->get_view();
            if constexpr(std::is_same_v<CharT, U>)
                return str;
            else
            {
                return std::span<const U>(
                    std::bit_cast<const U*>(str.data()),
                    str.size()
                );
            }
        }
    };

    template <char_16b CharT>
    class basic_string_ref<CharT> : public detail::str_ref_impl<CharT, basic_string_ref<CharT>>
    {
        using base = detail::str_ref_impl<CharT, basic_string_ref<CharT>>;
    public:
        using char_type = CharT;
        using value_type = base::value_type;
        using size_type = std::size_t;
        using string_type = std::basic_string<CharT>;
        using string_view_type = std::basic_string_view<CharT>;

        using const_iterator = base::const_iterator;
        using iterator = base::iterator;
        using const_reverse_iterator = base::const_reverse_iterator;
        using reverse_iterator = base::reverse_iterator;

        constexpr basic_string_ref() noexcept = default;
        constexpr basic_string_ref(const basic_string_ref& other) noexcept = default;
        constexpr basic_string_ref(std::nullptr_t) = delete;

        constexpr basic_string_ref(const CharT* str, size_type count) noexcept
            : base(string_view_type(str, count)) {}
        constexpr basic_string_ref(const CharT* str) noexcept
            : base(string_view_type(str)) {}
        constexpr basic_string_ref(string_view_type str) noexcept
            : base(str) {}
        constexpr basic_string_ref(const string_type& str) noexcept
            : base(str) {}
        template <typename Iterator, typename Sentinel>
            requires std::is_same_v<std::iter_value_t<Iterator>, CharT>
        constexpr basic_string_ref(Iterator start, Sentinel stop)
            : base(string_view_type(start, stop)) {}
        constexpr basic_string_ref(const_iterator start, const_iterator stop) noexcept
            : base(string_view_type(start.to_address(), stop.to_address())) {}

        PAPILIO_UTF_STRING_REF_COMAPRE();

        constexpr operator string_view_type() const noexcept
        {
            return this->get_view();
        }
    };

    template <char_32b CharT>
    class basic_string_ref<CharT> : public detail::str_ref_impl<CharT, basic_string_ref<CharT>>
    {
        using base = detail::str_ref_impl<CharT, basic_string_ref<CharT>>;
    public:
        using char_type = CharT;
        using value_type = base::value_type;
        using size_type = std::size_t;
        using string_type = std::basic_string<CharT>;
        using string_view_type = std::basic_string_view<CharT>;

        constexpr basic_string_ref() noexcept = default;
        constexpr basic_string_ref(const basic_string_ref& other) noexcept = default;
        constexpr basic_string_ref(std::nullptr_t) = delete;

        constexpr basic_string_ref(const CharT* str, size_type count) noexcept
            : base(string_view_type(str, count)) {}
        constexpr basic_string_ref(const CharT* str) noexcept
            : base(string_view_type(str)) {}
        constexpr basic_string_ref(string_view_type str) noexcept
            : base(str) {}
        constexpr basic_string_ref(const string_type& str) noexcept
            : base(str) {}
        template <typename Iterator, typename Sentinel>
            requires std::is_same_v<std::iter_value_t<Iterator>, CharT>
        constexpr basic_string_ref(Iterator start, Sentinel stop)
            : base(string_view_type(start, stop)) {}

        PAPILIO_UTF_STRING_REF_COMAPRE();

        constexpr operator string_view_type() const noexcept
        {
            return this->get_view();
        }
    };

    // global functions

    template <char_like T, char_like U>
    constexpr bool operator==(basic_string_ref<T> lhs, basic_string_ref<U> rhs) noexcept(std::is_same_v<T, U>)
    {
        if constexpr(std::is_same_v<T, U>)
            return std::basic_string_view<T>(lhs) == std::basic_string_view<U>(rhs);
        else
        {
            return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
        }
    }
    template <char_like T, char_like U>
    constexpr bool operator==(basic_string_ref<T> lhs, const U* rhs) noexcept(std::is_same_v<T, U>)
    {
        return lhs == basic_string_ref<U>(rhs);
    }
    template <char_like T, char_like U>
    constexpr bool operator==(const T* lhs, basic_string_ref<U> rhs) noexcept(std::is_same_v<T, U>)
    {
        return basic_string_ref<T>(lhs) == rhs;
    }
    template <char_like T, char_like U>
    constexpr bool operator==(basic_string_ref<T> lhs, std::basic_string_view<U> rhs) noexcept(std::is_same_v<T, U>)
    {
        return lhs == basic_string_ref<U>(rhs);
    }
    template <char_like T, char_like U>
    constexpr bool operator==(std::basic_string_view<T> lhs, basic_string_ref<U> rhs) noexcept(std::is_same_v<T, U>)
    {
        return basic_string_ref<T>(lhs) == rhs;
    }
    template <char_like T, char_like U>
    constexpr bool operator==(basic_string_ref<T> lhs, const std::basic_string<U>& rhs) noexcept(std::is_same_v<T, U>)
    {
        return lhs == basic_string_ref<U>(rhs);
    }
    template <char_like T, char_like U>
    constexpr bool operator==(const std::basic_string<T>& lhs, basic_string_ref<U> rhs) noexcept(std::is_same_v<T, U>)
    {
        return basic_string_ref<T>(lhs) == rhs;
    }

    template <char_like T, char_like U>
    constexpr std::strong_ordering operator<=>(basic_string_ref<T> lhs, basic_string_ref<U> rhs) noexcept
    {
        return lhs.compare(rhs) <=> 0;
    }
    template <char_like T, char_like U>
    constexpr std::strong_ordering operator<=>(basic_string_ref<T> lhs, const U* rhs) noexcept
    {
        return lhs.compare(rhs) <=> 0;
    }
    template <char_like T, char_like U>
    constexpr std::strong_ordering operator<=>(const T* lhs, basic_string_ref<U> rhs) noexcept
    {
        return basic_string_ref(lhs).compare(rhs) <=> 0;
    }
    template <char_like T, char_like U>
    constexpr std::strong_ordering operator<=>(basic_string_ref<T> lhs, std::basic_string_view<U> rhs) noexcept
    {
        return lhs.compare(rhs) <=> 0;
    }
    template <char_like T, char_like U>
    constexpr std::strong_ordering operator<=>(std::basic_string_view<T> lhs, basic_string_ref<U> rhs) noexcept
    {
        return basic_string_ref(lhs).compare(rhs) <=> 0;
    }
    template <char_like T, char_like U>
    constexpr std::strong_ordering operator<=>(basic_string_ref<T> lhs, const std::basic_string<U>& rhs) noexcept
    {
        return lhs.compare(rhs) <=> 0;
    }
    template <char_like T, char_like U>
    constexpr std::strong_ordering operator<=>(const std::basic_string<T>& lhs, basic_string_ref<U> rhs) noexcept
    {
        return basic_string_ref(lhs).compare(rhs) <=> 0;
    }

    using string_ref = basic_string_ref<char>;
    using u8string_ref = basic_string_ref<char8_t>;
    using u16string_ref = basic_string_ref<char16_t>;
    using u32string_ref = basic_string_ref<char32_t>;
    using wstring_ref = basic_string_ref<wchar_t>;

    inline namespace literals
    {
        constexpr string_ref operator""_sr(const char* str, std::size_t size) noexcept
        {
            return string_ref(str, size);
        }
        constexpr u8string_ref operator""_sr(const char8_t* str, std::size_t size) noexcept
        {
            return u8string_ref(str, size);
        }
        constexpr u16string_ref operator""_sr(const char16_t* str, std::size_t size) noexcept
        {
            return u16string_ref(str, size);
        }
        constexpr u32string_ref operator""_sr(const char32_t* str, std::size_t size) noexcept
        {
            return u32string_ref(str, size);
        }
        constexpr wstring_ref operator""_sr(const wchar_t* str, std::size_t size) noexcept
        {
            return wstring_ref(str, size);
        }
    }
}
