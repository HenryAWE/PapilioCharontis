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
// Variable 'variable' is uninitialized. Always initialize a member variable (type.6).
#   pragma warning(disable:26495)
#endif


namespace papilio::utf
{
    template <>
    class decoder<char32_t>
    {
    public:
        static constexpr std::uint8_t size_bytes(char32_t ch) noexcept;

        static constexpr auto to_codepoint(char32_t ch) -> std::pair<codepoint, std::uint8_t>;

        static constexpr auto from_codepoint(codepoint cp) noexcept -> std::pair<char32_t, std::uint8_t>;
    };
    template <>
    class decoder<char8_t>
    {
    public:
        static constexpr std::uint8_t size_bytes(char8_t ch) noexcept;

        static constexpr auto to_codepoint(std::u8string_view ch) -> std::pair<codepoint, std::uint8_t>;
    };
    template <>
    class decoder<char16_t>
    {
    public:
        static constexpr auto to_char32_t(std::u16string_view ch) -> std::pair<char32_t, std::uint8_t>;
        static constexpr auto to_char32_t(char16_t first, char16_t second = u'\0') -> std::pair<char32_t, std::uint8_t>;

        static constexpr auto to_codepoint(std::u16string_view ch) -> std::pair<codepoint, std::uint8_t>;
        static constexpr auto to_codepoint(char16_t first, char16_t second = u'\0') -> std::pair<codepoint, std::uint8_t>;

        struct from_codepoint_result
        {
            constexpr from_codepoint_result() noexcept = default;
            constexpr from_codepoint_result(const from_codepoint_result&) noexcept = default;

            constexpr from_codepoint_result& operator=(from_codepoint_result&) noexcept = default;

            char16_t chars[2] = {};
            std::uint8_t size = 0;
            std::uint8_t processed_size = 0;

            [[nodiscard]]
            constexpr std::u16string_view get() const noexcept
            {
                return std::u16string_view(chars, size);
            }
            constexpr operator std::u16string_view() const noexcept
            {
                return get();
            }
        };

        static constexpr auto from_codepoint(codepoint cp) -> from_codepoint_result;
    };
    template <>
    class decoder<wchar_t>
    {
    public:
        static auto to_char32_t(std::wstring_view ch) -> std::pair<char32_t, std::uint8_t>;

        struct from_codepoint_result
        {
            constexpr from_codepoint_result() noexcept = default;
            constexpr from_codepoint_result(const from_codepoint_result&) noexcept = default;

            constexpr from_codepoint_result& operator=(from_codepoint_result&) noexcept = default;

            wchar_t chars[sizeof(wchar_t) == sizeof(char16_t) ? 2 : 1] = {};
            std::uint8_t size = 0;
            std::uint8_t processed_size = 0;

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

        static auto to_codepoint(std::wstring_view ch) -> std::pair<codepoint, std::uint8_t>;

        static auto from_codepoint(codepoint cp) -> from_codepoint_result;
    };

    class codepoint
    {
    public:
        using value_type = char8_t;

        constexpr codepoint() noexcept = default;
        constexpr codepoint(const codepoint&) noexcept = default;
        template <std::integral T>
            requires(sizeof(T) == 1)
        constexpr codepoint(std::span<T, 4> bytes) noexcept
        {
            assign(bytes);
        }
        constexpr codepoint(char32_t ch) noexcept
        {
            assign(ch);
        }
        constexpr codepoint(const value_type* ptr, std::uint8_t len) noexcept
        {
            assign(ptr, len);
        }
        constexpr codepoint(const char* ptr, std::uint8_t len) noexcept
        {
            assign(ptr, len);
        }

        constexpr codepoint& assign(char32_t ch) noexcept
        {
            *this = decoder<char32_t>::to_codepoint(ch).first;

            return *this;
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
        constexpr codepoint& assign(const char8_t* ptr, std::uint8_t len) noexcept
        {
            if(len == 0)
            {
                clear();
                return *this;
            }
            PAPILIO_ASSUME(len <= 4);
            for(std::uint8_t i = 0; i < len; ++i)
                m_data[i] = ptr[i];

            return *this;
        }
        constexpr codepoint& assign(const char* ptr, std::uint8_t len) noexcept
        {
            if(len == 0)
            {
                clear();
                return *this;
            }
            PAPILIO_ASSUME(len <= 4);
            for(std::uint8_t i = 0; i < len; ++i)
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
        constexpr std::uint8_t size_bytes() const noexcept
        {
            return byte_count(m_data[0]);
        }

        constexpr operator char32_t() const noexcept
        {
            return decoder<char32_t>::from_codepoint(*this).first;
        }
        explicit constexpr operator std::u8string_view() const noexcept
        {
            return std::u8string_view(m_data, size_bytes());
        }
        explicit constexpr operator std::string_view() const noexcept
        {
            return std::string_view(data(), size_bytes());
        }

        template <std::integral To = char8_t>
            requires(sizeof(To) == 1)
        constexpr std::pair<std::array<To, 4>, std::uint8_t> as_array() const noexcept
        {
            std::array<To, 4> arr{
                static_cast<To>(m_data[0]),
                static_cast<To>(m_data[1]),
                static_cast<To>(m_data[2]),
                static_cast<To>(m_data[3])
            };

            return std::make_pair(arr, size_bytes());
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

        friend constexpr std::strong_ordering operator<=>(codepoint lhs, codepoint rhs) noexcept
        {
            return static_cast<char32_t>(lhs) <=> static_cast<char32_t>(rhs);
        }
        friend constexpr std::strong_ordering operator<=>(codepoint lhs, char32_t rhs) noexcept
        {
            return static_cast<char32_t>(lhs) <=> rhs;
        }
        friend constexpr std::strong_ordering operator<=>(char32_t lhs, codepoint rhs) noexcept
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

        constexpr std::size_t estimate_width() const
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

            char32_t ch = static_cast<char32_t>(*this);
            for(const auto& i : estimate_intervals)
            {
                if(i.first <= ch && ch < i.second)
                    return 2;
            }

            return 1;
        }

    private:
        char8_t m_data[4];
    };

    inline namespace literals
    {
        constexpr codepoint operator""_cp(char32_t ch) noexcept
        {
            return decoder<char32_t>::to_codepoint(ch).first;
        }
    }
}

#ifdef PAPILIO_COMPILER_MSVC
#   pragma warning(pop)
#endif

#include "codepoint.inl"
