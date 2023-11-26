#pragma once

#include <string>
#include <algorithm>
#include <variant>
#include "common.hpp"
#include "codepoint.hpp"
#include "../memory.hpp"


namespace papilio::utf
{
    template <typename CharT>
    class basic_string_ref;
    template <typename CharT>
    class basic_string_container;

    namespace detail
    {
        template <typename CharT>
        class const_str_iter_impl;

        template <char_8b CharT>
        class const_str_iter_impl<CharT>
        {
        public:
            using char_type = CharT;
            using size_type = std::size_t;
            using difference_type = std::ptrdiff_t;
            using value_type = codepoint;
            using reference = codepoint;
            using string_view_type = std::basic_string_view<CharT>;

            constexpr const_str_iter_impl() noexcept = default;
            constexpr const_str_iter_impl(const const_str_iter_impl&) noexcept = default;

        protected:
            constexpr const_str_iter_impl(string_view_type str, size_type offset, std::uint8_t len) noexcept
                : m_str(str), m_offset(offset), m_len(len) {}

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

                    if (m_offset - next_offset > 3) [[unlikely]]
                    {
                        m_len = 1;
                        break;
                    }
                    else if (is_leading_byte(ch))
                    {
                        m_offset = next_offset;
                        m_len = byte_count(ch);
                        break;
                    }
                    else if (next_offset == 0)
                    {
                        m_len = 1;
                        break;
                    }

                    --next_offset;
                }
            }

        public:
            [[nodiscard]]
            constexpr reference operator*() const noexcept
            {
                return codepoint(to_address(), m_len);
            }

            explicit constexpr operator bool() const noexcept
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

            constexpr void swap(const_str_iter_impl& other) noexcept
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

        template <char_16b CharT>
        class const_str_iter_impl<CharT>
        {
        public:
            using char_type = CharT;
            using size_type = std::size_t;
            using difference_type = std::ptrdiff_t;
            using value_type = codepoint;
            using reference = codepoint;
            using string_view_type = std::basic_string_view<CharT>;

            constexpr const_str_iter_impl() noexcept = default;
            constexpr const_str_iter_impl(const const_str_iter_impl&) noexcept = default;

        protected:
            constexpr const_str_iter_impl(string_view_type str, size_type offset, std::uint8_t len) noexcept
                : m_str(str), m_offset(offset), m_len(len) {}

            constexpr void next() noexcept
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
            }

            constexpr void prev() noexcept
            {
                PAPILIO_ASSUME(m_offset != 0);
                --m_offset;
                while(!is_low_surrogate(m_str[m_offset]))
                    --m_offset;
            }

        public:
            [[nodiscard]]
            constexpr reference operator*() const
            {
                return decoder<CharT>::to_codepoint(
                    m_str.substr(m_offset, m_len)
                ).first;
            }

            explicit constexpr operator bool() const noexcept
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

            constexpr void swap(const_str_iter_impl& other) noexcept
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

        template <char_32b CharT>
        class const_str_iter_impl<CharT>
        {
        public:
            using char_type = CharT;
            using size_type = std::size_t;
            using difference_type = std::ptrdiff_t;
            using value_type = codepoint;
            using reference = codepoint;
            using string_view_type = std::basic_string_view<CharT>;

            constexpr const_str_iter_impl() noexcept = default;
            constexpr const_str_iter_impl(const const_str_iter_impl&) noexcept = default;

        protected:
            constexpr const_str_iter_impl(string_view_type::const_iterator iter) noexcept
                : m_iter(iter) {}

            constexpr void next() noexcept
            {
                ++m_iter;
            }

            constexpr void prev() noexcept
            {
                --m_iter;
            }

        public:
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

            constexpr void swap(const_str_iter_impl& other) noexcept
            {
                using std::swap;
                swap(m_iter, other.m_iter);
            }

        private:
            string_view_type::const_iterator m_iter;
        };

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

            class const_iterator : public const_str_iter_impl<CharT>
            {
            public:
                using base = const_str_iter_impl<CharT>;
            public:
                using char_type = CharT;
                using size_type = std::size_t;
                using difference_type = std::ptrdiff_t;
                using value_type = codepoint;
                using reference = codepoint;
                using string_view_type = std::basic_string_view<CharT>;

                constexpr const_iterator() noexcept = default;
                constexpr const_iterator(const const_iterator&) noexcept = default;

            private:
                friend str_base;
                using base::base;

            public:
                constexpr const_iterator& operator=(const const_iterator&) noexcept = default;

                [[nodiscard]]
                constexpr bool operator==(const const_iterator& rhs) const noexcept
                {
                    return this->to_address() == rhs.to_address();
                }

                constexpr const_iterator& operator++() noexcept
                {
                    this->next();
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
                    this->prev();
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

                friend constexpr void swap(const_iterator& lhs, const_iterator& rhs) noexcept
                {
                    lhs.swap(rhs);
                }
            };

            using const_reverse_iterator = std::reverse_iterator<const_iterator>;

            constexpr const_iterator begin() const noexcept { return as_derived().cbegin(); }
            constexpr const_iterator end() const noexcept { return as_derived().cend(); }

            constexpr const_reverse_iterator crbegin() const noexcept
            {
                return const_reverse_iterator(as_derived().cend());
            }
            constexpr const_reverse_iterator crend() const noexcept
            {
                return const_reverse_iterator(as_derived().cbegin());
            }
            constexpr const_reverse_iterator rbegin() const noexcept { return crbegin(); }
            constexpr const_reverse_iterator rend() const noexcept { return crend(); }

            [[nodiscard]]
            constexpr codepoint operator[](size_type i) const noexcept
            {
                return index(i);
            }

#if __cpp_multidimensional_subscript >= 202110L

            [[nodiscard]]
            constexpr codepoint operator[](reverse_index_t, size_type i) const noexcept
            {
                return index(reverse_index, i);
            }

#endif

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
                if(off == npos)
                    throw_out_of_range();
                return cp_from_off(off);
            }
            [[nodiscard]]
            constexpr codepoint at(reverse_index_t, size_type i) const
            {
                size_type off = get_offset(reverse_index, i);
                if(off == npos)
                    throw_out_of_range();
                return cp_from_off(off);
            }

            constexpr codepoint index_or(size_type i, codepoint default_val) const noexcept
            {
                size_type off = get_offset(i);
                if(off == npos)
                    return default_val;
                return cp_from_off(off);
            }
            constexpr codepoint index_or(reverse_index_t, size_type i, codepoint default_val) const noexcept
            {
                size_type off = get_offset(reverse_index, i);
                if(off == npos)
                    return default_val;
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
                    if(it == sentinel)
                        return sentinel;
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
            constexpr std::pair<Derived, size_type> substr_extended(size_type pos = 0, size_type count = npos)
                const noexcept(OnOutOfRange != substr_behavior::exception)
            {
                const auto sentinel = this->as_derived().cend();

                auto start = this->as_derived().cbegin();
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

                        const auto sentinel = as_derived().cbegin();

                        const_iterator it = as_derived().cend();
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

                if(start.to_address() >= stop.to_address()) [[unlikely]]
                    return Derived();

                return Derived(start, stop);
            }

        protected:
            constexpr string_view_type get_view() const noexcept
            {
                return string_view_type(static_cast<const Derived&>(*this));
            }

            constexpr Derived& as_derived() noexcept
            {
                return static_cast<Derived&>(*this);
            }
            constexpr const Derived& as_derived() const noexcept
            {
                return static_cast<const Derived&>(*this);
            }

            template <typename... Args>
            constexpr const_iterator make_c_iter(Args&&... args) const noexcept
            {
                return const_iterator(std::forward<Args>(args)...);
            }

            constexpr codepoint cp_from_off(size_type off) const noexcept
            {
                string_view_type str = get_view();
                PAPILIO_ASSERT(off < str.size());

                if constexpr(char_8b<CharT>)
                {
                    std::uint8_t len = utf::byte_count(str[off]);
                    return codepoint(str.data() + off, len);
                }
                else if constexpr(char_16b<CharT>)
                {
                    std::uint8_t len = utf::is_high_surrogate(str[off]) ? 2 : 1;
                    return decoder<CharT>::to_codepoint(str.substr(off, len)).first;
                }
                else if constexpr(char_32b<CharT>)
                {
                    return decoder<char32_t>::to_codepoint(str[off]).first;
                }
            }
        };

        template <char_like CharT, typename Derived>
        class string_ref_base : public str_base<CharT, Derived>
        {
            using base = str_base<CharT, Derived>;
        public:
            using value_type = codepoint;
            using char_type = CharT;
            using size_type = std::size_t;
            using string_view_type = std::basic_string_view<CharT>;

            using const_iterator = base::const_iterator;

        public:
            constexpr string_ref_base& operator=(const string_ref_base&) noexcept = default;

            using base::find;
            [[nodiscard]]
            constexpr auto find(Derived str, size_type pos = 0) const noexcept
            {
                return this->find_impl(str, pos);
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

            using base::contains;
            [[nodiscard]]
            constexpr bool contains(Derived str) const noexcept
            {
                return this->find(str) != as_derived().end();
            }

            using base::starts_with;
            [[nodiscard]]
            constexpr bool starts_with(Derived str) const noexcept
            {
                return starts_with(string_view_type(str));
            }

            [[nodiscard]]
            constexpr bool ends_with(Derived str) const noexcept
            {
                string_view_type view = this->get_view();
                if(view.size() < str.size())
                    return false;
                return view.substr(view.size() - str.size(), str.size()) == string_view_type(str);
            }
            [[nodiscard]]
            constexpr bool ends_with(const CharT* str) const noexcept
            {
                return ends_with(Derived(str));
            }
            [[nodiscard]]
            constexpr bool ends_with(codepoint cp) const noexcept
            {
                return !this->empty() && this->back() == cp;
            }

            constexpr void swap(string_ref_base& other) noexcept
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

                set_view(it, sentinel);
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

                set_view(sentinel.base(), it.base());
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

            constexpr operator string_view_type() const noexcept
            {
                return m_str;
            }

        protected:
            constexpr string_ref_base() noexcept = default;
            constexpr string_ref_base(const string_ref_base&) noexcept = default;
            constexpr string_ref_base(string_view_type str) noexcept
                : m_str(str) {}

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
                set_view(string_view_type(start.to_address(), stop.to_address()));
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
                    if(Derived(it, sentinel).starts_with(v))
                        break;
                }

                return it;
            }

        protected:
            using base::as_derived;

        private:
            string_view_type m_str;
        };

        // conversion between char and char8_t for compatibility
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

#define PAPILIO_UTF_STRING_REF_OP_ASSIGN() \
    constexpr basic_string_ref& operator=(const basic_string_ref&) noexcept = default;\
    constexpr basic_string_ref& operator=(std::nullptr_t) noexcept = delete;\
    constexpr basic_string_ref& operator=(const CharT* str) noexcept\
    {\
        PAPILIO_ASSUME(str != nullptr);\
        this->set_view(str);\
        return *this;\
    }\
    constexpr basic_string_ref& operator=(string_view_type str) noexcept\
    {\
        this->set_view(str);\
        return *this;\
    }\
    basic_string_ref& operator=(const string_type& str) noexcept\
    {\
        this->set_view(str);\
        return *this;\
    }

#define PAPILIO_UTF_STRING_REF_COMPARE() \
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
    class basic_string_ref<CharT> : public detail::string_ref_base<CharT, basic_string_ref<CharT>>
    {
        using base = detail::string_ref_base<CharT, basic_string_ref>;
    public:
        using char_type = CharT;
        using value_type = codepoint;
        using size_type = std::size_t;
        using string_type = std::basic_string<CharT>;
        using string_view_type = std::basic_string_view<CharT>;

        using const_iterator = base::const_iterator;
        using iterator = const_iterator;
        using const_reverse_iterator = base::const_reverse_iterator;
        using reverse_iterator = const_reverse_iterator;

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

        PAPILIO_UTF_STRING_REF_OP_ASSIGN();

        PAPILIO_UTF_STRING_REF_COMPARE();

        constexpr const_iterator cbegin() const noexcept
        {
            string_view_type str = this->get_view();
            if(this->empty())
            {
                return this->make_c_iter(str, 0, 0);
            }
            else [[likely]]
            {
                char8_t ch = str.data()[0];
                std::uint8_t len =
                    is_leading_byte(ch) ?
                    byte_count(ch) : 1;
                return this->make_c_iter(str, 0, len);
            }
        }
        constexpr const_iterator cend() const noexcept
        {
            string_view_type str = this->get_view();
            return this->make_c_iter(str, str.size(), 0);
        }

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
    };

    template <char_16b CharT>
    class basic_string_ref<CharT> : public detail::string_ref_base<CharT, basic_string_ref<CharT>>
    {
        using base = detail::string_ref_base<CharT, basic_string_ref<CharT>>;
    public:
        using char_type = CharT;
        using value_type = codepoint;
        using size_type = std::size_t;
        using string_type = std::basic_string<CharT>;
        using string_view_type = std::basic_string_view<CharT>;

        using const_iterator = base::const_iterator;
        using iterator = const_iterator;
        using const_reverse_iterator = base::const_reverse_iterator;
        using reverse_iterator = const_reverse_iterator;

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

        PAPILIO_UTF_STRING_REF_OP_ASSIGN();

        PAPILIO_UTF_STRING_REF_COMPARE();

        constexpr const_iterator cbegin() const noexcept
        {
            string_view_type str = this->get_view();
            if(this->empty())
            {
                return this->make_c_iter(str, 0, 0);
            }
            else [[likely]]
            {
                std::uint16_t ch = str.data()[0];
                std::uint8_t len = is_high_surrogate(ch) ? 2 : 1;
                return this->make_c_iter(str, 0, len);
            }
        }
        constexpr const_iterator cend() const noexcept
        {
            string_view_type str = this->get_view();
            return this->make_c_iter(str, str.size(), 0);
        }

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

                for(auto&& i : *this)
                    result.append(view_t(i));

                return result;
            }
            else if constexpr(char_32b<To>)
            {
                result_t result;

                for(auto&& i : *this)
                    result += static_cast<char32_t>(i);

                return result;
            }
        }
    };

    template <char_32b CharT>
    class basic_string_ref<CharT> : public detail::string_ref_base<CharT, basic_string_ref<CharT>>
    {
        using base = detail::string_ref_base<CharT, basic_string_ref<CharT>>;
    public:
        using char_type = CharT;
        using value_type = codepoint;
        using size_type = std::size_t;
        using string_type = std::basic_string<CharT>;
        using string_view_type = std::basic_string_view<CharT>;

        using const_iterator = base::const_iterator;
        using iterator = const_iterator;
        using const_reverse_iterator = base::const_reverse_iterator;
        using reverse_iterator = const_reverse_iterator;

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

        PAPILIO_UTF_STRING_REF_OP_ASSIGN();

        PAPILIO_UTF_STRING_REF_COMPARE();

        constexpr const_iterator cbegin() const noexcept
        {
            return this->make_c_iter(this->get_view().begin());
        }
        constexpr const_iterator cend() const noexcept
        {
            return  this->make_c_iter(this->get_view().end());
        }

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

                for(auto&& i : *this)
                    result.append(std::basic_string_view<To>(i));

                return result;
            }
            else if constexpr(char_16b<To>)
            {
                result_t result;

                for(auto&& i : *this)
                {
                    auto ch = decoder<To>::from_codepoint(i);
                    result.append(std::basic_string_view<To>(ch));
                }

                return result;
            }
        }
    };

#undef PAPILIO_UTF_STRING_REF_COMAPRE
#undef PAPILIO_UTF_STRING_REF_OP_ASSIGN

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

    namespace detail
    {
        template <char_like CharT, typename Derived>
        class string_container_base : public str_base<CharT, Derived>
        {
            using base = str_base<CharT, Derived>;
        public:
            using size_type = std::size_t;
            using value_type = codepoint;
            using char_type = CharT;
            using string_view_type = std::basic_string_view<CharT>;
            using string_type = std::basic_string<CharT>;
            using string_ref_type = basic_string_ref<CharT>;

            using const_iterator = base::const_iterator;

            constexpr string_container_base& operator=(const string_container_base&) = default;
            constexpr string_container_base& operator=(string_container_base&&) noexcept = default;

            [[nodiscard]]
            constexpr bool empty() const noexcept
            {
                return std::visit([](auto&& v) { return v.empty(); }, m_data);
            }

            [[nodiscard]]
            constexpr const CharT* c_str() const
            {
                return str().c_str();
            }
            [[nodiscard]]
            constexpr const CharT* data() const noexcept
            {
                return std::visit([](auto&& v) { return v.data(); }, m_data);
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
                return std::visit([](auto&& v) { return v.size(); }, m_data);
            }

            using base::find;
            [[nodiscard]]
            const_iterator find(string_view_type str, size_type pos = 0) const noexcept
            {
                auto it = as_derived().cbegin();
                auto sentinel = as_derived().cend();

                for(size_type n = 0; n < pos; ++n)
                {
                    if(it == sentinel)
                        return sentinel;
                    ++it;
                }

                for(; it != as_derived().cend(); ++it)
                {
                    if(create_ref(it, sentinel).starts_with(str))
                        break;
                }

                return it;
            }

            using base::contains;
            [[nodiscard]]
            constexpr bool contains(string_view_type str) const noexcept
            {
                return find(str) != as_derived().cend();
            }

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

            constexpr void swap(string_container_base& other) noexcept
            {
                using std::swap;
                swap(m_data, other.m_data);
            }

            constexpr operator string_view_type() const noexcept
            {
                return std::visit(
                    [](auto&& v) { return string_view_type(v); },
                    std::as_const(m_data)
                );
            }

            constexpr operator string_ref_type() const noexcept
            {
                return static_cast<string_view_type>(*this);
            }

        protected:
            using string_store = std::variant<
                std::basic_string<CharT>,
                std::basic_string_view<CharT>
            >;

            mutable string_store m_data;

            constexpr string_container_base() noexcept
                : m_data() {}
            constexpr string_container_base(const string_container_base&) = default;
            constexpr string_container_base(string_container_base&&) noexcept = default;

            template <typename T, typename... Args>
            string_container_base(std::in_place_type_t<T>, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
                : m_data(std::in_place_type<T>, std::forward<Args>(args)...) {}

            string_type& to_str() const
            {
                string_view_type* p_str = std::get_if<string_view_type>(&m_data);
                if(p_str)
                {
                    return m_data.template emplace<string_type>(*p_str);
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
                return string_view_type(start.to_address(), stop.to_address());
            }

            template <typename U>
            constexpr int compare_impl(basic_string_ref<U> other) const noexcept
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

        private:
            using base::as_derived;
        };
    }

#define PAPILIO_UTF_STRING_CONTAINER_COMPARE() \
    template <char_like U>\
    constexpr int compare(const basic_string_container<U>& other) const noexcept\
    {\
        return this->compare_impl(basic_string_ref<U>(other));\
    }\
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
    class basic_string_container<CharT> :
        public detail::string_container_base<CharT, basic_string_container<CharT>>
    {
        using base = detail::string_container_base<CharT, basic_string_container<CharT>>;
    public:
        using size_type = std::size_t;
        using value_type = codepoint;
        using char_type = CharT;
        using string_view_type = std::basic_string_view<CharT>;
        using string_type = std::basic_string<CharT>;
        using string_ref_type = basic_string_ref<CharT>;

        using const_iterator = base::const_iterator;
        using const_reverse_iterator = base::const_reverse_iterator;

        basic_string_container() noexcept = default;
        basic_string_container(std::nullptr_t) = delete;
        basic_string_container(const basic_string_container&) = default;
        basic_string_container(basic_string_container&&) noexcept = default;

        basic_string_container(string_type&& str) noexcept
            : base(std::in_place_type<string_type>, std::move(str)) {}
        basic_string_container(const string_type& str) noexcept
            : base(std::in_place_type<string_view_type>, str) {}
        basic_string_container(independent_t, const string_type& str)
            : base(std::in_place_type<string_type>, str) {}

        basic_string_container(string_view_type str) noexcept
            : base(std::in_place_type<string_view_type>, str) {}

        basic_string_container(independent_t, const basic_string_container& str)
            : base(std::in_place_type<string_type>, string_view_type(str)) {}
        basic_string_container(const CharT* str) noexcept
            : base(std::in_place_type<string_view_type>, str) {}
        constexpr basic_string_container(const CharT* str, size_type count) noexcept
            : base(std::in_place_type<string_view_type>, str, count) {}
        basic_string_container(const_iterator start, const_iterator stop) noexcept
            : base(std::in_place_type<string_view_type>, start.to_address(), stop.to_address()) {}

        basic_string_container(size_type count, codepoint ch)
        {
            assign(count, ch);
        }

        template <typename Iterator, typename Sentinel>
            requires std::is_same_v<std::iter_value_t<Iterator>, CharT>
        basic_string_container(Iterator start, Sentinel stop)
            : base(std::in_place_type<string_type>, start, stop) {}

        basic_string_container& assign(size_type count, codepoint ch) noexcept
        {
            string_type& str = this->to_str();
            for(size_type i = 0; i < count; ++i)
                str.append(ch.data(), ch.size_bytes());

            return *this;
        }
        basic_string_container& assign(const CharT* str) noexcept
        {
            this->template emplace_data<string_view_type>(str);
            return *this;
        }
        basic_string_container& assign(const CharT* str, size_type count) noexcept
        {
            this->template emplace_data<string_view_type>(str, count);
            return *this;
        }

        basic_string_container& assign(string_view_type str)
        {
            this->template emplace_data<string_view_type>(str);
            return *this;
        }

        basic_string_container& assign(independent_t, string_view_type str)
        {
            this->template emplace_data<string_type>(str);
            return *this;
        }

        constexpr basic_string_container& operator=(const basic_string_container&) = default;
        constexpr basic_string_container& operator=(basic_string_container&&) noexcept = default;

        basic_string_container& operator=(string_view_type str)
        {
            return this->assign(str);
        }
        basic_string_container& operator=(independent_proxy<string_view_type> str)
        {
            return this->assign(independent, str);
        }

        PAPILIO_UTF_STRING_CONTAINER_COMPARE();

        using base::begin;
        using base::end;

        constexpr const_iterator cbegin() const noexcept
        {
            string_view_type str = this->get_view();
            if(this->empty())
            {
                return this->make_c_iter(str, 0, 0);
            }
            else [[likely]]
            {
                char8_t ch = str.data()[0];
                std::uint8_t len =
                    is_leading_byte(ch) ?
                    byte_count(ch) : 1;
                return this->make_c_iter(str, 0, len);
            }
        }
        constexpr const_iterator cend() const noexcept
        {
            string_view_type str = this->get_view();
            return this->make_c_iter(str, str.size(), 0);
        }
    };

#undef PAPILIO_UTF_STRING_CONTAINER_COMPARE

    template <char_like T, char_like U>
    constexpr bool operator==(const basic_string_container<T>& lhs, const basic_string_container<U>& rhs) noexcept(std::is_same_v<T, U>)
    {
        return basic_string_ref<T>(lhs) == basic_string_ref<U>(rhs);
    }
    template <char_like T, char_like U>
    constexpr bool operator==(const basic_string_container<T>& lhs, basic_string_ref<U> rhs) noexcept(std::is_same_v<T, U>)
    {
        return basic_string_ref<T>(lhs) == rhs;
    }
    template <char_like T, char_like U>
    constexpr bool operator==(basic_string_ref<T> lhs, const basic_string_container<U>& rhs) noexcept(std::is_same_v<T, U>)
    {
        return lhs == basic_string_ref<U>(rhs);
    }
    template <char_like T, char_like U>
    constexpr bool operator==(const basic_string_container<T>& lhs, const U* rhs) noexcept(std::is_same_v<T, U>)
    {
        return lhs == basic_string_ref<U>(rhs);
    }
    template <char_like T, char_like U>
    constexpr bool operator==(const T* lhs, const basic_string_container<U>& rhs) noexcept(std::is_same_v<T, U>)
    {
        return basic_string_ref<T>(lhs) == rhs;
    }
    template <char_like T, char_like U>
    constexpr bool operator==(const basic_string_container<T>& lhs, std::basic_string_view<U> rhs) noexcept(std::is_same_v<T, U>)
    {
        return lhs == basic_string_ref<U>(rhs);
    }
    template <char_like T, char_like U>
    constexpr bool operator==(std::basic_string_view<T> lhs, const basic_string_container<U>& rhs) noexcept(std::is_same_v<T, U>)
    {
        return basic_string_ref<T>(lhs) == rhs;
    }
    template <char_like T, char_like U>
    constexpr bool operator==(const basic_string_container<T>& lhs, const std::basic_string<U>& rhs) noexcept(std::is_same_v<T, U>)
    {
        return lhs == basic_string_ref<U>(rhs);
    }
    template <char_like T, char_like U>
    constexpr bool operator==(const std::basic_string<T>& lhs, const basic_string_container<U>& rhs) noexcept(std::is_same_v<T, U>)
    {
        return basic_string_ref<T>(lhs) == rhs;
    }

    template <char_like T, char_like U>
    constexpr std::strong_ordering operator<=>(const basic_string_container<T>& lhs, const basic_string_container<U>& rhs) noexcept
    {
        return lhs.compare(rhs) <=> 0;
    }
    template <char_like T, char_like U>
    constexpr std::strong_ordering operator<=>(const basic_string_container<T>& lhs, const U* rhs) noexcept
    {
        return lhs.compare(rhs) <=> 0;
    }
    template <char_like T, char_like U>
    constexpr std::strong_ordering operator<=>(const T* lhs, const basic_string_container<U>& rhs) noexcept
    {
        return basic_string_ref(lhs).compare(rhs) <=> 0;
    }
    template <char_like T, char_like U>
    constexpr std::strong_ordering operator<=>(const basic_string_container<T>& lhs, std::basic_string_view<U> rhs) noexcept
    {
        return lhs.compare(rhs) <=> 0;
    }
    template <char_like T, char_like U>
    constexpr std::strong_ordering operator<=>(std::basic_string_view<T> lhs, const basic_string_container<U>& rhs) noexcept
    {
        return basic_string_ref(lhs).compare(rhs) <=> 0;
    }
    template <char_like T, char_like U>
    constexpr std::strong_ordering operator<=>(const basic_string_container<T>& lhs, const std::basic_string<U>& rhs) noexcept
    {
        return lhs.compare(rhs) <=> 0;
    }
    template <char_like T, char_like U>
    constexpr std::strong_ordering operator<=>(const std::basic_string<T>& lhs, const basic_string_container<U>& rhs) noexcept
    {
        return basic_string_ref(lhs).compare(rhs) <=> 0;
    }

    using string_container = basic_string_container<char>;
    using u8string_container = basic_string_container<char8_t>;

    inline namespace literals
    {
        constexpr string_container operator""_sc(const char* str, std::size_t size) noexcept
        {
            return string_container(str, size);
        }
        constexpr u8string_container operator""_sc(const char8_t* str, std::size_t size) noexcept
        {
            return u8string_container(str, size);
        }
    }
}
