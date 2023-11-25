#pragma once

#include "codepoint.hpp"
#include <tuple> // std::tie


namespace papilio::utf
{
    constexpr auto decoder<char32_t>::size_bytes(char32_t ch) noexcept->std::uint8_t
    {
        if(ch <= 0x7F)
            return 1;
        else if(ch <= 0x7FF)
            return 2;
        else if(ch <= 0xFFFF)
            return 3;
        else
            return 4;
    }

    constexpr auto decoder<char32_t>::to_codepoint(char32_t ch) -> std::pair<codepoint, std::uint8_t>
    {
        std::uint8_t len = size_bytes(ch);
        char8_t bytes[4] = { 0, 0, 0, 0 };
        switch(len)
        {
        case 1:
            bytes[0] = static_cast<char8_t>(ch);
            break;

        case 2:
            bytes[1] = static_cast<char8_t>((ch & 0b11'1111) | 0b1000'0000);
            bytes[0] = static_cast<char8_t>((ch >> 6) | 0b1100'0000);
            break;

        case 3:
            bytes[2] = static_cast<char8_t>((ch & 0b11'1111) | 0b1000'0000);
            bytes[1] = static_cast<char8_t>(((ch >> 6) & 0b11'1111) | 0b1000'0000);
            bytes[0] = static_cast<char8_t>((ch >> 12) | 0b1110'0000);
            break;

        case 4:
            bytes[3] = static_cast<char8_t>((ch & 0b11'1111) | 0b1000'0000);
            bytes[2] = static_cast<char8_t>(((ch >> 6) & 0b11'1111) | 0b1000'0000);
            bytes[1] = static_cast<char8_t>(((ch >> 12) & 0b11'1111) | 0b1000'0000);
            bytes[0] = static_cast<char8_t>((ch >> 18) | 0b1111'0000);
            break;

        default:
            PAPILIO_UNREACHABLE();
        }

        return std::make_pair(codepoint(bytes, len), 1);
    }

    constexpr auto decoder<char32_t>::from_codepoint(codepoint cp) noexcept -> std::pair<char32_t, std::uint8_t>
    {
        const char8_t* bytes = cp.u8data();

        // ASCII (single byte)
        if(bytes[0] <= 0b0111'1111)
        {
            return std::make_pair(static_cast<char32_t>(cp.data()[0]), 1);
        }
        // 2 bytes
        else if(0b1100'0000 <= bytes[0] && bytes[0] <= 0b1101'1111)
        {
            char32_t result = U'\0';
            result |= bytes[0] & 0b0001'1111;
            result <<= 6;
            result |= bytes[1] & 0b0011'1111;

            return std::make_pair(result, 2);
        }
        // 3 bytes
        else if(0b1110'0000 <= bytes[0] && bytes[0] <= 0b1110'1111)
        {
            char32_t result = U'\0';
            result |= bytes[0] & 0b0000'1111;
            result <<= 6;
            result |= bytes[1] & 0b0011'1111;
            result <<= 6;
            result |= bytes[2] & 0b0011'1111;

            return std::make_pair(result, 3);
        }
        // 4 bytes
        else if(0b1111'0000 <= bytes[0] && bytes[0] <= 0b1111'0111)
        {
            char32_t result = U'\0';
            result |= bytes[0] & 0b0000'1111;
            result <<= 6;
            result |= bytes[1] & 0b0011'1111;
            result <<= 6;
            result |= bytes[2] & 0b0011'1111;
            result <<= 6;
            result |= bytes[3] & 0b0011'1111;

            return std::make_pair(result, 3);
        }

        return std::make_pair(U'\0', 0);
    }

    constexpr std::uint8_t decoder<char8_t>::size_bytes(char8_t ch) noexcept
    {
        return byte_count(ch);
    }

    constexpr auto decoder<char8_t>::to_codepoint(std::u8string_view ch) -> std::pair<codepoint, std::uint8_t>
    {
        if(ch.empty()) [[unlikely]]
            return std::make_pair(codepoint(), 0);
        std::uint8_t len = size_bytes(ch[0]);
        return std::make_pair(codepoint(ch.data(), len), len);
    }

    constexpr auto decoder<char16_t>::to_char32_t(std::u16string_view ch) -> std::pair<char32_t, std::uint8_t>
    {
        if(ch.empty()) [[unlikely]]
            return std::make_pair(U'\0', 0);

        if(!is_high_surrogate(ch[0])) [[likely]]
        {
            char32_t result = ch[0];
            return std::make_pair(result, 1);
        }
        else
        {
            if(ch.size() < 2) [[unlikely]]
                throw invalid_surrogate(ch[0]);
            else if(!is_low_surrogate(ch[1])) [[unlikely]]
                throw invalid_surrogate(ch[1]);

            char32_t result =
                (ch[0] - 0xD800 << 10) +
                (ch[1] - 0xDC00) +
                0x10000;
            return std::make_pair(result, 2);
        }
    }
    constexpr auto decoder<char16_t>::to_char32_t(char16_t first, char16_t second) -> std::pair<char32_t, std::uint8_t>
    {
        char16_t tmp[2] = { first, second };

        return to_char32_t(std::u16string_view(tmp, 2));
    }

    constexpr auto decoder<char16_t>::to_codepoint(std::u16string_view ch) -> std::pair<codepoint, std::uint8_t>
    {
        auto [ch32, processed_size] = to_char32_t(ch);
        return std::make_pair(decoder<char32_t>::to_codepoint(ch32).first, processed_size);
    }
    constexpr auto decoder<char16_t>::to_codepoint(char16_t first, char16_t second) -> std::pair<codepoint, std::uint8_t>
    {
        return decoder<char32_t>::to_codepoint(to_char32_t(first, second).first);
    }

    constexpr auto decoder<char16_t>::from_codepoint(codepoint cp) -> from_codepoint_result
    {
        from_codepoint_result result;

        char32_t ch32;
        std::tie(ch32, result.processed_size) = decoder<char32_t>::from_codepoint(cp);
        if(ch32 <= 0xD7FF || (0xE000 <= ch32 && ch32 <= 0xFFFF))
        {
            result.chars[0] = static_cast<char16_t>(ch32);
            result.size = 1;
        }
        else if(0x10000 <= ch32 && ch32 <= 0x10FFFF)
        {
            std::uint32_t tmp = ch32 - 0x10000;
            result.chars[0] = 0xD800 + (tmp >> 10);
            result.chars[1] = 0xDC00 + (tmp & 0x3FF);
            result.size = 2;
        }
        else
        {
            throw std::invalid_argument("invalid codepoint"); // TODO: Better exception
        }

        return result;
    }
}
