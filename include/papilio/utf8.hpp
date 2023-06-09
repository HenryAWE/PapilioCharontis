#pragma once

#include <string>
#include <utility>
#include <span>
#include <memory>
#include <concepts>
#include <type_traits>
#include <variant>
#include "memory.hpp"
#include "type.hpp"


namespace papilio
{
    class slice;

    namespace utf8
    {
        // return the number of total bytes if it is a leading byte
        [[nodiscard]]
        constexpr std::uint8_t is_leading_byte(char ch) noexcept
        {
            if((ch & 0b10000000) == 0)
                return 1;
            else if((ch & 0b11100000) == 0b11000000)
                return 2;
            else if((ch & 0b11110000) == 0b11100000)
                return 3;
            else if((ch & 0b11111000) == 0b11110000)
                return 4;

            return 0;
        }

        class codepoint
        {
        public:
            using size_type = std::uint8_t;
            using int_type = std::uint32_t;

            static std::pair<codepoint, size_type> decode(std::u8string_view src)
            {
                if(src.empty())
                    return std::make_pair(codepoint(), 0);
                size_type count = is_leading_byte(src[0]);
                if(count == 0)
                    return std::make_pair(codepoint(), 0);
                return std::make_pair(
                    codepoint(src.data(), count),
                    count
                );
            }
            static std::pair<codepoint, size_type> decode_u16be(std::u16string_view src)
            {
                if(src.empty())
                    return std::make_pair(codepoint(), 0);

                if(src[0] <= 0xD7FF || 0xE000 <= src[0])
                {
                    return std::make_pair(
                        codepoint(static_cast<int_type>(src[0])),
                        1
                    );
                }
                else
                {
                    if(src.size() < 2)
                        return std::make_pair(codepoint('?'), 1);
                    char32_t ch32 =
                        (src[0] - 0xD800) * 0x400 +
                        (src[1] - 0xDC00) +
                        0x10000;
                    return std::make_pair(
                        codepoint(ch32), 2
                    );
                }
            }
            static std::pair<codepoint, size_type> decode(std::string_view src)
            {
                return decode(
                    std::u8string_view(reinterpret_cast<const char8_t*>(src.data()), src.size())
                );
            }
            static std::pair<codepoint, size_type> rdecode(std::u8string_view src)
            {
                std::u8string_view::size_type idx = src.size();
                size_type count = 0;
                while(count == 0)
                {
                    if(idx == 0)
                        return std::make_pair(codepoint(), 0);
                    --idx;
                    count = is_leading_byte(src[idx]);
                }

                return std::make_pair(
                    codepoint(src.data() + idx, count),
                    count
                );
            }
            static std::pair<codepoint, size_type> rdecode(std::string_view src)
            {
                return rdecode(
                    std::u8string_view(reinterpret_cast<const char8_t*>(src.data()), src.size())
                );
            }

            constexpr codepoint() noexcept = default;
            constexpr codepoint(const codepoint&) noexcept = default;
            constexpr codepoint(const char8_t* ch, size_type length) noexcept
            {
                assign(ch, length);
            }
            constexpr codepoint(char32_t ch) noexcept
            {
                assign(ch);
            }

            codepoint& operator=(const codepoint& rhs) noexcept = default;
            codepoint& operator=(char32_t ch) noexcept
            {
                assign(ch);
                return *this;
            }

            constexpr void clear() noexcept
            {
                for(auto c : m_bytes)
                    c = 0;
            }

            constexpr void assign(const char8_t* data, size_type length) noexcept
            {
                for(size_type i = 0; i < length; ++i)
                    m_bytes[i] = data[i];
            }
            constexpr void assign(int_type val) noexcept
            {
                if(val <= 0x7F)
                {
                    m_bytes[0] = static_cast<char8_t>(val);
                }
                else if(val <= 0x7FF)
                {
                    m_bytes[1] = static_cast<char8_t>((val & 0b11'1111) | 0b1000'0000);
                    m_bytes[0] = static_cast<char8_t>((val >> 6) | 0b1100'0000);
                }
                else if(val <= 0xFFFF)
                {
                    m_bytes[2] = static_cast<char8_t>((val & 0b11'1111) | 0b1000'0000);
                    m_bytes[1] = static_cast<char8_t>(((val >> 6) & 0b11'1111) | 0b1000'0000);
                    m_bytes[0] = static_cast<char8_t>((val >> 12) | 0b1110'0000);
                }
                else
                {
                    m_bytes[3] = static_cast<char8_t>((val & 0b11'1111) | 0b1000'0000);
                    m_bytes[2] = static_cast<char8_t>(((val >> 6) & 0b11'1111) | 0b1000'0000);
                    m_bytes[1] = static_cast<char8_t>(((val >> 12) & 0b11'1111) | 0b1000'0000);
                    m_bytes[0] = static_cast<char8_t>((val >> 18) | 0b1111'0000);
                }
            }
            void assign(char16_t high, char16_t low) noexcept
            {
                char16_t src[2] = { high, low };
                *this = decode_u16be(std::u16string_view(src, 2)).first;
            }

            [[nodiscard]]
            size_type size() const noexcept
            {
                return is_leading_byte(m_bytes[0]);
            }

            [[nodiscard]]
            constexpr std::pair<int_type, size_type> to_int() const noexcept
            {
                // ASCII (single byte)
                if(m_bytes[0] <= 0b0111'1111)
                {
                    return std::make_pair(m_bytes[0], 1);
                }
                // 2 bytes
                else if(0b1100'0000 <= m_bytes[0] && m_bytes[0] <= 0b1101'1111)
                {
                    char32_t result = U'\0';
                    result |= m_bytes[0] & 0b0001'1111;
                    result <<= 6;
                    result |= m_bytes[1] & 0b0011'1111;

                    return std::make_pair(result, 2);
                }
                // 3 bytes
                else if(0b1110'0000 <= m_bytes[0] && m_bytes[0] <= 0b1110'1111)
                {
                    char32_t result = U'\0';
                    result |= m_bytes[0] & 0b0000'1111;
                    result <<= 6;
                    result |= m_bytes[1] & 0b0011'1111;
                    result <<= 6;
                    result |= m_bytes[2] & 0b0011'1111;

                    return std::make_pair(result, 3);
                }
                // 4 bytes
                else if(0b1111'0000 <= m_bytes[0] && m_bytes[0] <= 0b1111'0111)
                {
                    char32_t result = U'\0';
                    result |= m_bytes[0] & 0b0000'1111;
                    result <<= 6;
                    result |= m_bytes[1] & 0b0011'1111;
                    result <<= 6;
                    result |= m_bytes[2] & 0b0011'1111;
                    result <<= 6;
                    result |= m_bytes[3] & 0b0011'1111;

                    return std::make_pair(result, 3);
                }

                return std::make_pair(U'\0', 0);
            }

            [[nodiscard]]
            constexpr bool operator==(const codepoint& rhs) const noexcept = default;
            [[nodiscard]]
            friend bool operator==(char32_t lhs, const codepoint& rhs) noexcept
            {
                return lhs == rhs.to_int().first;
            }
            [[nodiscard]]
            friend bool operator==(const codepoint& lhs, char32_t rhs) noexcept
            {
                return lhs.to_int().first == rhs;
            }
            [[nodiscard]]
            friend bool operator==(char lhs, const codepoint& rhs) noexcept
            {
                return lhs == rhs.to_int().first;
            }
            [[nodiscard]]
            friend bool operator==(const codepoint& lhs, char rhs) noexcept
            {
                return lhs.to_int().first == rhs;
            }

            [[nodiscard]]
            constexpr std::strong_ordering operator<=>(const codepoint& rhs) const noexcept
            {
                return to_int().first <=> rhs.to_int().first;
            }
            [[nodiscard]]
            friend std::strong_ordering operator<=>(char32_t lhs, const codepoint& rhs) noexcept
            {
                return lhs <=> rhs.to_int().first;
            }
            [[nodiscard]]
            friend std::strong_ordering operator<=>(const codepoint& lhs, char32_t rhs) noexcept
            {
                return lhs.to_int().first <=> rhs;
            }

            [[nodiscard]]
            const char* data() const noexcept
            {
                return reinterpret_cast<const char*>(m_bytes);
            }
            [[nodiscard]]
            const char8_t* u8data() const noexcept
            {
                return m_bytes;
            }

            operator char32_t() const noexcept
            {
                return to_int().first;
            }
            operator std::string_view() const noexcept
            {
                return std::string_view(
                    reinterpret_cast<const char*>(m_bytes),
                    size()
                );
            }
            operator std::u8string_view() const noexcept
            {
                return std::u8string_view(m_bytes, size());
            }
            operator std::span<const char8_t, 4>() const noexcept
            {
                return m_bytes;
            }

            [[nodiscard]]
            std::size_t estimate_width() const noexcept
            {
                // [begin, end) intervals
                constexpr std::pair<char32_t, char32_t> estimate_intervals[] =
                {
                    { 0x1100u, 0x1160u },
                    { 0x2329u, 0x232Bu },
                    { 0x2E80u, 0x303Fu },
                    { 0x3040u, 0xA4D0u },
                    { 0xAC00u, 0xD7A4u },
                    { 0xF900u, 0xFB00u },
                    { 0xFE10u, 0xFE1Au },
                    { 0xFE30u, 0xFE70u },
                    { 0xFF00u, 0xFF61u },
                    { 0xFFE0u, 0xFFE7u },
                    { 0x1F300u, 0x1F650u },
                    { 0x1F900u, 0x1FA00u },
                    { 0x20000u, 0x2FFFEu },
                    { 0x30000u, 0x3FFFEu }
                };

                char32_t ch = *this;
                for(const auto& i : estimate_intervals)
                {
                    if(i.first <= ch && ch < i.second)
                        return 2;
                }

                return 1;
            }

        private:
            char8_t m_bytes[4] = { '\0', '\0', '\0', '\0' };
        };

        std::size_t strlen(std::string_view str) noexcept;

        std::string_view index(std::string_view str, std::size_t idx) noexcept;
        std::string index(const std::string& str, std::size_t idx);
        std::string_view index(const char* str, std::size_t idx) noexcept;

        std::string_view rindex(std::string_view str, std::size_t idx) noexcept;
        std::string rindex(const std::string& str, std::size_t idx);
        std::string_view rindex(const char* str, std::size_t idx) noexcept;

        std::string_view substr(
            std::string_view str,
            std::size_t off,
            std::size_t count = std::string_view::npos
        ) noexcept;
        std::string substr(
            const std::string& str,
            std::size_t off,
            std::size_t count = std::string_view::npos
        );
        std::string_view substr(
            const char* str,
            std::size_t off,
            std::size_t count = std::string_view::npos
        );

        std::string_view substr(
            std::string_view str,
            const slice& s
        ) noexcept;
        std::string substr(
            const std::string& str,
            const slice& s
        );
        std::string_view substr(
            const char* str,
            const slice& s
        ) noexcept;

        inline namespace literals
        {
            [[nodiscard]]
            constexpr codepoint operator""_cp(const char8_t* data, std::size_t length) noexcept
            {
                return codepoint(data, static_cast<codepoint::size_type>(length));
            }
        }
    }

    inline namespace literals
    {
        using namespace utf8::literals;
    }

    // copy-on-write and UTF-8 encoded string class
    class string_container
    {
    public:
        using char_type = char;
        using string_type = std::basic_string<char_type>;
        using string_view_type = std::basic_string_view<char_type>;
        using value_type = char;
        using size_type = std::size_t;
        using index_type = std::make_signed_t<size_type>;
        using pointer = char_type*;
        using const_pointer = const char_type*;
        using const_reference = utf8::codepoint;

        class const_iterator
        {
        public:
            using underlying_type = const_pointer;
            using value_type = utf8::codepoint;
            using difference_type = std::size_t;

            const_iterator() noexcept
                : m_it(nullptr), m_len(0) {}
            const_iterator(const const_iterator&) noexcept = default;
            const_iterator(underlying_type it, utf8::codepoint::size_type len) noexcept
                : m_it(it), m_len(len) {}

            const_iterator& operator++() noexcept
            {
                m_it = std::next(m_it, utf8::is_leading_byte(*m_it));
                return *this;
            }
            const_iterator& operator--() noexcept
            {
                do
                {
                    --m_it;
                    m_len = utf8::is_leading_byte(*m_it);
                } while(m_len == 0);
                m_len = m_len;

                return *this;
            }
            const_iterator operator++(int) noexcept
            {
                const_iterator tmp = *this;
                ++(*this);
                return tmp;
            }
            const_iterator operator--(int) noexcept
            {
                const_iterator tmp = *this;
                --(*this);
                return tmp;
            }

            [[nodiscard]]
            value_type operator*() const noexcept
            {
                if(m_len == 0)
                    return utf8::codepoint();
                return utf8::codepoint::decode(
                    string_view_type(std::to_address(m_it), m_len)
                ).first;
            }

            [[nodiscard]]
            constexpr bool operator==(const const_iterator& rhs) const noexcept
            {
                return m_it == rhs.m_it;
            }

            [[nodiscard]]
            friend underlying_type to_address(const const_iterator& it) noexcept
            {
                return it.m_it;
            }

        private:
            underlying_type m_it;
            utf8::codepoint::size_type m_len = 0;
        };
        using iterator = const_iterator;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        using reverse_iterator = const_reverse_iterator;

        [[nodiscard]]
        friend auto to_address(const const_reverse_iterator& it) noexcept
        {
            auto base_it = it.base();
            return to_address(--base_it);
        }

        static constexpr size_type npos = -1;

    private:
        using string_store = std::variant<
            std::shared_ptr<string_type>,
            string_view_type
        >;

    public:
        string_container() noexcept
            : m_str(std::in_place_type<string_view_type>) {}
        string_container(const string_container& other) noexcept
        {
            link(other);
        }
        string_container(string_container&& other) noexcept
            : string_container()
        {
            swap(other);
        }
        string_container(const string_type& str) noexcept
            : m_str(std::in_place_type<string_view_type>, str) {}
        string_container(string_view_type str) noexcept
            : m_str(std::in_place_type<string_view_type>, str) {}
        string_container(const char_type* str) noexcept
            : m_str(std::in_place_type<string_view_type>, str) {}
        template <std::contiguous_iterator Iterator>
        string_container(Iterator begin, Iterator end) noexcept
            : m_str(std::in_place_type<string_view_type>, begin, end) {}
        template <typename T>
        string_container(independent_t::proxy<T> str)
            : m_str(std::make_shared<string_type>(str.get())) {}
        string_container(string_type&& str)
            : m_str(std::make_shared<string_type>(std::move(str))) {}
        string_container(independent_t, string_type str)
            : m_str(std::make_shared<string_type>(std::move(str))) {}
        string_container(utf8::codepoint cp, size_type count)
        {
            auto result = std::make_shared<string_type>();
            result->reserve(cp.size() * count);
            for(size_type i = 0; i < count; ++i)
                result->append(string_view_type(cp));
            m_str = std::move(result);
        }
        string_container(independent_t, string_view_type str)
            : m_str(std::make_shared<string_type>(str)) {}
        string_container(independent_t, const char* str)
            : m_str(std::make_shared<string_type>(str)) {}
        template <typename Iterator>
        string_container(independent_t, Iterator begin, Iterator end)
            : m_str(std::make_shared<string_type>(begin, end)) {}

        string_container& operator=(string_container&) = default;
        string_container& operator=(string_container&& other) noexcept
        {
            m_str = std::move(other.m_str);
            return *this;
        }
        string_container& operator=(string_type& str)
        {
            m_str.emplace<string_view_type>(str);
            return *this;
        }
        string_container& operator=(string_view_type str)
        {
            m_str.emplace<string_view_type>(str);
            return *this;
        }
        string_container& operator=(const char_type* str)
        {
            m_str.emplace<string_view_type>(str);
            return *this;
        }
        template <typename T>
        string_container& operator=(independent_t::proxy<T> str)
        {
            m_str = std::make_shared<string_type>(str.get());
            return *this;
        }

        [[nodiscard]]
        bool is_borrowed() const noexcept
        {
            return std::holds_alternative<string_view_type>(m_str);
        }
        void make_independent()
        {
            get_string();
        }

        void link(const string_container& other) const noexcept
        {
            m_str = other.m_str;
        }
        void link(const string_container& other) noexcept
        {
            m_str = other.m_str;
        }
        void link(string_view_type str) noexcept
        {
            m_str.emplace<1>(str);
        }
        void copy(string_container& other)
        {
            m_str.emplace<0>(
                std::make_shared<string_type>(other.get_string_view())
            );
        }
        void swap(string_container& other) noexcept
        {
            m_str.swap(other.m_str);
        }

        [[nodiscard]]
        bool operator==(const string_container& other) const noexcept
        {
            return get_string_view() == other.get_string_view();
        }
        template <std::convertible_to<string_view_type> String>
        [[nodiscard]]
        friend bool operator==(const string_container& lhs, String&& rhs)  noexcept
        {
            return lhs.get_string_view() == string_view_type(std::forward<String>(rhs));
        }
        template <std::convertible_to<string_view_type> String>
        [[nodiscard]]
        friend bool operator==(String&& lhs, const string_container& rhs) noexcept
        {
            return string_view_type(std::forward<String>(lhs)) == rhs.get_string_view();
        }

        [[nodiscard]]
        std::strong_ordering operator<=>(const string_container& other) const noexcept
        {
            return get_string_view() <=> other.get_string_view();
        }
        template <std::convertible_to<string_view_type> String>
        [[nodiscard]]
        friend std::strong_ordering operator<=>(const string_container& lhs, String&& rhs)  noexcept
        {
            return lhs.get_string_view() <=> string_view_type(std::forward<String>(rhs));
        }
        template <std::convertible_to<string_view_type> String>
        [[nodiscard]]
        friend std::strong_ordering operator<=>(String&& lhs, const string_container& rhs) noexcept
        {
            return string_view_type(std::forward<String>(lhs)) <=> rhs.get_string_view();
        }

        [[nodiscard]]
        size_type length() const noexcept
        {
            return utf8::strlen(get_string_view());
        }
        [[nodiscard]]
        size_type size() const noexcept
        {
            return get_string_view().size();
        }
        [[nodiscard]]
        bool empty() const noexcept
        {
            return get_string_view().empty();
        }
        [[nodiscard]]
        constexpr size_type capacity() const noexcept
        {
            if(m_str.index() == 0)
                return std::get_if<0>(&m_str)->get()->capacity();
            else
                return 0;
        }

        void clear() noexcept
        {
            m_str.emplace<1>();
        }

        void push_back(utf8::codepoint cp)
        {
            get_string().append(string_view_type(cp));
        }
        // WARNING: call this function when string is empty will cause undefined behavior
        void pop_back() noexcept;

        [[nodiscard]]
        const_iterator find(utf8::codepoint cp, size_type pos = 0) const noexcept;
        [[nodiscard]]
        const_iterator find(const string_container& str, size_type pos = 0) const noexcept;
        [[nodiscard]]
        const_iterator find(string_view_type str, size_type pos = 0) const noexcept;
        [[nodiscard]]
        const_iterator find(const string_type& str, size_type pos = 0) const noexcept
        {
            return find(string_view_type(str), pos);
        }
        [[nodiscard]]
        const_iterator find(const char* str, size_type pos = 0) const noexcept
        {
            return find(string_view_type(str), pos);
        }
        [[nodiscard]]
        const_iterator find(const char* str, size_type pos, size_type count) const noexcept
        {
            return find(string_view_type(str, count), pos);
        }

        [[nodiscard]]
        bool contains(utf8::codepoint cp) const noexcept
        {
            return find(cp) != cend();
        }
        [[nodiscard]]
        bool contains(const string_container& str) const noexcept
        {
            return find(str) != cend();
        }
        [[nodiscard]]
        bool contains(string_view_type str) const noexcept
        {
            return find(str) != cend();
        }
        [[nodiscard]]
        bool contains(const string_type& str) const noexcept
        {
            return find(str) != cend();
        }
        [[nodiscard]]
        bool contains(const char* str) const noexcept
        {
            return find(str) != cend();
        }

        [[nodiscard]]
        const_reference at(size_type i) const noexcept
        {
            string_view_type view = utf8::index(get_string_view(), i);
            return utf8::codepoint(
                reinterpret_cast<const char8_t*>(view.data()),
                view.size()
            );
        }
        // reverse at
        [[nodiscard]]
        const_reference rat(size_type i) const noexcept
        {
            string_view_type view = utf8::rindex(get_string_view(), i);
            return utf8::codepoint(
                reinterpret_cast<const char8_t*>(view.data()),
                view.size()
            );
        }

        [[nodiscard]]
        const_reference operator[](index_type i) const noexcept
        {
            if(i < 0)
                return rat(-(i + 1));
            else
                return at(i);
        }
        // equivalent to this->at(0)
        [[nodiscard]]
        const_reference front() const noexcept;
        // equivalent to this->rat(0)
        [[nodiscard]]
        const_reference back() const noexcept;

        operator string_view_type() const noexcept
        {
            return get_string_view();
        }

        [[nodiscard]]
        string_container substr(size_type off, size_type count = npos) const noexcept
        {
            return utf8::substr(get_string_view(), off, count);
        }
        [[nodiscard]]
        string_container substr(independent_t, size_type off, size_type count = npos) const
        {
            auto view =  utf8::substr(get_string_view(), off, count);
            return string_container(independent, view);
        }
        [[nodiscard]]
        string_container substr(const slice& s) const noexcept
        {
            return utf8::substr(get_string_view(), s);
        }
        [[nodiscard]]
        string_container substr(independent_t, const slice& s) const
        {
            auto view = utf8::substr(get_string_view(), s);
            return string_container(independent, view);
        }

        [[nodiscard]]
        const_pointer c_str() noexcept
        {
            return get_string().data();
        }
        [[nodiscard]]
        const_pointer data() const noexcept
        {
            return get_string_view().data();
        }

        [[nodiscard]]
        const_iterator cbegin() const noexcept
        {
            const_pointer ptr = data();
            return const_iterator(
                ptr, utf8::is_leading_byte(ptr[0])
            );
        }
        [[nodiscard]]
        const_iterator cend() const noexcept
        {
            string_view_type view = get_string_view();
            return const_iterator(view.data() + view.size(), 0);
        }
        [[nodiscard]]
        const_reverse_iterator crbegin() const
        {
            return const_reverse_iterator(cend());
        }
        [[nodiscard]]
        const_reverse_iterator crend() const
        {
            return const_reverse_iterator(cbegin());
        }
        [[nodiscard]]
        iterator begin() const noexcept
        {
            return cbegin();
        }
        [[nodiscard]]
        iterator end() const noexcept
        {
            return cend();
        }
        [[nodiscard]]
        reverse_iterator rbegin() const noexcept
        {
            return reverse_iterator(end());
        }
        [[nodiscard]]
        reverse_iterator rend() const noexcept
        {
            return reverse_iterator(begin());
        }

        [[nodiscard]]
        std::u8string to_u8string() const;
        [[nodiscard]]
        std::u32string to_u32string() const;

        std::size_t estimate_width() const noexcept;

    private:
        mutable string_store m_str;

        string_type& get_string();
        string_view_type get_string_view() const noexcept;
    };
}
