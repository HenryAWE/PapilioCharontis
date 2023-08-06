#pragma once

#include "codepoint.hpp"


namespace papilio::utf
{
    constexpr auto decoder<char32_t>::size_bytes(char32_t ch) noexcept->size_type
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

    constexpr auto decoder<char32_t>::to_codepoint(char32_t ch)->std::pair<codepoint, size_type>
    {
        size_type len = size_bytes(ch);
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

        return std::make_pair(codepoint(bytes, len), len);
    }

    constexpr auto decoder<char32_t>::from_codepoint(codepoint cp) noexcept -> std::pair<char32_t, size_type>
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

    constexpr auto decoder<char8_t>::size_bytes(char8_t ch) noexcept -> size_type
    {
        return byte_count(ch);
    }

    constexpr auto decoder<char8_t>::to_codepoint(std::u8string_view ch) -> std::pair<codepoint, size_type>
    {
        if(ch.empty()) [[unlikely]]
            return std::make_pair(codepoint(), 0);
        size_type len = size_bytes(ch[0]);
        return std::make_pair(codepoint(ch.data(), len), len);
    }
}
