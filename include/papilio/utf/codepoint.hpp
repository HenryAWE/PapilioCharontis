#pragma once

#include <cstdint>
#include <cstddef>
#include <iosfwd>
#include <bit> // std::bit_cast
#include <span>
#include <string>
#include <array>
#include "common.hpp"

#ifdef PAPILIO_COMPILER_MSVC
#   pragma warning(push)
#   pragma warning(disable:26495)
#endif


namespace papilio::utf
{
    template <>
    class decoder<char32_t>
    {
    public:
        using size_type = std::uint8_t;

        static constexpr auto size_bytes(char32_t ch) noexcept -> size_type;

        static constexpr auto to_codepoint(char32_t ch) -> std::pair<codepoint, size_type>;

        static constexpr auto from_codepoint(codepoint cp) noexcept -> std::pair<char32_t, size_type>;
    };
    template <>
    class decoder<char8_t>
    {
    public:
        using size_type = std::uint8_t;

        static constexpr auto size_bytes(char8_t ch) noexcept -> size_type;

        static constexpr auto to_codepoint(std::u8string_view ch) -> std::pair<codepoint, size_type>;
    };
    template <>
    class decoder<char16_t>
    {
    public:
        using size_type = std::uint8_t;

        static auto to_char32_t(std::u16string_view ch) -> std::pair<char32_t, size_type>;
        static auto to_char32_t(char16_t first, char16_t second = u'\0') -> std::pair<char32_t, size_type>;

        static auto size_bytes(char16_t ch) noexcept -> size_type;

        static auto to_codepoint(std::u16string_view ch) -> std::pair<codepoint, size_type>;
        static auto to_codepoint(char16_t first, char16_t second = u'\0') -> std::pair<codepoint, size_type>;

        struct from_codepoint_result
        {
            using size_type = size_type;

            constexpr from_codepoint_result() noexcept = default;
            constexpr from_codepoint_result(const from_codepoint_result&) noexcept = default;

            constexpr from_codepoint_result& operator=(from_codepoint_result&) noexcept = default;

            char16_t chars[2] = {};
            size_type size = 0;
            size_type processed_size = 0;

            [[nodiscard]]
            std::u16string_view get() const noexcept
            {
                return std::u16string_view(chars, size);
            }
            operator std::u16string_view() const noexcept
            {
                return get();
            }
        };

        static auto from_codepoint(codepoint cp) -> from_codepoint_result;
    };
    template <>
    class decoder<wchar_t>
    {
    public:
        using size_type = std::uint8_t;

        static auto to_char32_t(std::wstring_view ch) -> std::pair<char32_t, size_type>;

        struct from_codepoint_result
        {
            using size_type = size_type;

            constexpr from_codepoint_result() noexcept = default;
            constexpr from_codepoint_result(const from_codepoint_result&) noexcept = default;

            constexpr from_codepoint_result& operator=(from_codepoint_result&) noexcept = default;

            wchar_t chars[sizeof(wchar_t) == sizeof(char16_t) ? 2 : 1] = {};
            size_type size = 0;
            size_type processed_size = 0;

            [[nodiscard]]
            std::wstring_view get() const noexcept
            {
                return std::wstring_view(chars, size);
            }
            operator std::wstring_view() const noexcept
            {
                return get();
            }
        };

        static auto to_codepoint(std::wstring_view ch) -> std::pair<codepoint, size_type>;

        static auto from_codepoint(codepoint cp) -> from_codepoint_result;
    };

    class codepoint
    {
    public:
        using size_type = std::uint8_t;
        using value_type = char8_t;

        constexpr codepoint() noexcept = default;
        constexpr codepoint(const codepoint&) noexcept = default;
        template <std::integral T>
            requires(sizeof(T) == 1)
        constexpr codepoint(std::span<T, 4> bytes) noexcept
        {
            assign(bytes);
        }
        constexpr codepoint(const value_type* ptr, size_type len) noexcept
        {
            assign(ptr, len);
        }
        constexpr codepoint(const char* ptr, size_type len) noexcept
        {
            assign(ptr, len);
        }

        constexpr codepoint& assign(value_type b0) noexcept
        {
            m_data[0] = b0;

            return *this;
        }
        constexpr codepoint& assign(value_type b0, value_type b1) noexcept
        {
            m_data[0] = b0;
            m_data[1] = b1;

            return *this;
        }
        constexpr codepoint& assign(value_type b0, value_type b1, value_type b2) noexcept
        {
            m_data[0] = b0;
            m_data[1] = b1;
            m_data[2] = b2;

            return *this;
        }
        constexpr codepoint& assign(value_type b0, value_type b1, value_type b2, value_type b3) noexcept
        {
            m_data[0] = b0;
            m_data[1] = b1;
            m_data[2] = b2;
            m_data[3] = b3;

            return *this;
        }
        template <std::integral T>
            requires(sizeof(T) == 1)
        constexpr codepoint& assign(std::span<T, 4> bytes) noexcept
        {
            return assign(
                static_cast<value_type>(bytes[0]),
                static_cast<value_type>(bytes[1]),
                static_cast<value_type>(bytes[2]),
                static_cast<value_type>(bytes[3])
            );
        }
        constexpr codepoint& assign(const char8_t* ptr, size_type len) noexcept
        {
            if(len == 0)
            {
                clear();
                return *this;
            }
            PAPILIO_ASSUME(len <= 4);
            for(size_type i = 0; i < len; ++i)
                m_data[i] = ptr[i];

            return *this;
        }
        constexpr codepoint& assign(const char* ptr, size_type len) noexcept
        {
            if(len == 0)
            {
                clear();
                return *this;
            }
            PAPILIO_ASSUME(len <= 4);
            for(size_type i = 0; i < len; ++i)
                m_data[i] = static_cast<char8_t>(ptr[i]);

            return *this;
        }

        constexpr void clear() noexcept
        {
            m_data[0] = 0;
        }

        [[nodiscard]]
        constexpr const char8_t* u8data() const noexcept
        {
            return m_data;
        }
        [[nodiscard]]
        constexpr const char* data() const noexcept
        {
            return std::bit_cast<const char*>(u8data());
        }
        [[nodiscard]]
        size_type size() const noexcept
        {
            return byte_count(m_data[0]);
        }

        constexpr operator char32_t() const noexcept
        {
            return decoder<char32_t>::from_codepoint(*this).first;
        }
        explicit constexpr operator std::u8string_view() const noexcept
        {
            return std::u8string_view(m_data, size());
        }
        explicit constexpr operator std::string_view() const noexcept
        {
            return std::string_view(data(), size());
        }

        template <std::integral To = char8_t>
            requires(sizeof(To) == 1)
        constexpr std::pair<std::array<To, 4>, size_type> as_array() const noexcept
        {
            std::array<To, 4> arr{
                static_cast<To>(m_data[0]),
                static_cast<To>(m_data[1]),
                static_cast<To>(m_data[2]),
                static_cast<To>(m_data[3])
            };

            return std::make_pair(arr, size());
        }

        constexpr bool operator==(codepoint rhs) const noexcept
        {
            return static_cast<char32_t>(*this) == static_cast<char32_t>(rhs);
        }
        friend constexpr bool operator==(codepoint lhs, char32_t rhs) noexcept
        {
            return static_cast<char32_t>(lhs) == rhs;
        }
        friend constexpr bool operator==(char32_t rhs, codepoint lhs) noexcept
        {
            return lhs == static_cast<char32_t>(rhs);
        }

        constexpr std::strong_ordering operator<=>(codepoint rhs) const noexcept
        {
            return static_cast<char32_t>(*this) <=> static_cast<char32_t>(rhs);
        }
        friend constexpr std::strong_ordering operator<=>(codepoint lhs, char32_t rhs) noexcept
        {
            return static_cast<char32_t>(lhs) <=> rhs;
        }
        friend constexpr std::strong_ordering operator<=>(char32_t rhs, codepoint lhs) noexcept
        {
            return lhs <=> static_cast<char32_t>(rhs);
        }

        explicit constexpr operator bool() const noexcept
        {
            return *this != U'\0';
        }

        friend std::ostream& operator<<(std::ostream& os, codepoint cp);
        friend std::wostream& operator<<(std::wostream& os, codepoint cp);
        friend std::basic_ostream<char8_t>& operator<<(std::basic_ostream<char8_t>& os, codepoint cp);
        friend std::basic_ostream<char16_t>& operator<<(std::basic_ostream<char16_t>& os, codepoint cp);
        friend std::basic_ostream<char32_t>& operator<<(std::basic_ostream<char32_t>& os, codepoint cp);

    private:
        char8_t m_data[4];
    };
}

#ifdef PAPILIO_COMPILER_MSVC
#   pragma warning(pop)
#endif

#include "codepoint.inl"
