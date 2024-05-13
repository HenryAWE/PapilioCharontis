#include <tuple> // std::tie

namespace papilio::utf
{
constexpr std::uint8_t decoder<char32_t>::size_bytes(char32_t ch) noexcept
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

constexpr std::pair<codepoint, std::uint8_t> decoder<char32_t>::to_codepoint(char32_t ch) noexcept
{
    std::uint8_t len = size_bytes(ch);
    char8_t bytes[4] = {0, 0, 0, 0};
    switch(len)
    {
    case 1:
        bytes[0] = static_cast<char8_t>(ch);
        break;

    case 2:
        bytes[1] = static_cast<char8_t>((ch & 0b0011'1111) | 0b1000'0000);
        bytes[0] = static_cast<char8_t>((ch >> 6) | 0b1100'0000);
        break;

    case 3:
        bytes[2] = static_cast<char8_t>((ch & 0b0011'1111) | 0b1000'0000);
        bytes[1] = static_cast<char8_t>(((ch >> 6) & 0b0011'1111) | 0b1000'0000);
        bytes[0] = static_cast<char8_t>((ch >> 12) | 0b1110'0000);
        break;

    case 4:
        bytes[3] = static_cast<char8_t>((ch & 0b0011'1111) | 0b1000'0000);
        bytes[2] = static_cast<char8_t>(((ch >> 6) & 0b0011'1111) | 0b1000'0000);
        bytes[1] = static_cast<char8_t>(((ch >> 12) & 0b0011'1111) | 0b1000'0000);
        bytes[0] = static_cast<char8_t>((ch >> 18) | 0b1111'0000);
        break;

    default:
        PAPILIO_UNREACHABLE();
    }

    return std::make_pair(codepoint(bytes, len), std::uint8_t(1));
}

constexpr std::pair<codepoint, std::uint8_t> decoder<char32_t>::to_codepoint(std::u32string_view ch) noexcept
{
    if(ch.empty()) [[unlikely]]
        return std::make_pair(codepoint(), std::uint8_t(0));
    return to_codepoint(ch[0]);
}

constexpr std::pair<char32_t, std::uint8_t> decoder<char32_t>::from_codepoint(codepoint cp) noexcept
{
    const char8_t* bytes = cp.u8data();

    // ASCII (single byte)
    if(bytes[0] <= 0b0111'1111)
    {
        return std::make_pair(static_cast<char32_t>(cp.data()[0]), std::uint8_t(1));
    }
    // 2 bytes
    else if(0b1100'0000 <= bytes[0] && bytes[0] <= 0b1101'1111)
    {
        char32_t result = U'\0';
        result |= bytes[0] & 0b0001'1111;
        result <<= 6;
        result |= bytes[1] & 0b0011'1111;

        return std::make_pair(result, std::uint8_t(2));
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

        return std::make_pair(result, std::uint8_t(3));
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

        return std::make_pair(result, std::uint8_t(3));
    }

    return std::make_pair(U'\0', std::uint8_t(0));
}

constexpr std::uint8_t decoder<char8_t>::size_bytes(char8_t ch) noexcept
{
    return PAPILIO_NS utf::byte_count(ch);
}

constexpr std::pair<codepoint, std::uint8_t> decoder<char8_t>::to_codepoint(std::u8string_view ch)
{
    if(ch.empty()) [[unlikely]]
        return std::make_pair(codepoint(), std::uint8_t(0));
    std::uint8_t len = size_bytes(ch[0]);
    return std::make_pair(codepoint(ch.data(), len), len);
}

constexpr std::pair<char32_t, std::uint8_t> decoder<char16_t>::to_char32_t(std::u16string_view ch)
{
    if(ch.empty()) [[unlikely]]
        return std::make_pair(U'\0', std::uint8_t(0));

    if(!PAPILIO_NS utf::is_high_surrogate(ch[0])) [[likely]]
    {
        char32_t result = ch[0];
        return std::make_pair(result, std::uint8_t(1));
    }
    else
    {
        if(ch.size() < 2) [[unlikely]]
            throw invalid_surrogate(ch[0]);
        else if(!PAPILIO_NS utf::is_low_surrogate(ch[1])) [[unlikely]]
            throw invalid_surrogate(ch[1]);

        std::uint32_t result =
            ((ch[0] - 0xD800u) << 10) +
            (ch[1] - 0xDC00u) +
            0x10000u;
        return std::make_pair(static_cast<char32_t>(result), std::uint8_t(2));
    }
}

constexpr std::pair<char32_t, std::uint8_t> decoder<char16_t>::to_char32_t(char16_t first, char16_t second)
{
    char16_t tmp[2] = {first, second};

    return to_char32_t(std::u16string_view(tmp, 2));
}

constexpr std::pair<codepoint, std::uint8_t> decoder<char16_t>::to_codepoint(std::u16string_view ch)
{
    auto [ch32, processed_size] = to_char32_t(ch);
    return std::make_pair(decoder<char32_t>::to_codepoint(ch32).first, processed_size);
}

constexpr std::pair<codepoint, std::uint8_t> decoder<char16_t>::to_codepoint(char16_t first, char16_t second)
{
    return decoder<char32_t>::to_codepoint(to_char32_t(first, second).first);
}

constexpr auto decoder<char16_t>::from_codepoint(codepoint cp) -> from_codepoint_result
{
    from_codepoint_result result;

    char32_t ch32;
    std::tie(ch32, result.processed_size) = decoder<char32_t>::from_codepoint(cp);
    if(ch32 <= 0xD7FF || (0xE000 <= ch32 && ch32 <= 0xFFFF)) [[likely]]
    {
        result.chars[0] = static_cast<char16_t>(ch32);
        result.size = 1;
    }
    else if(0x10000 <= ch32 && ch32 <= 0x10FFFF)
    {
        std::uint32_t tmp = ch32 - 0x10000;
        result.chars[0] = static_cast<char16_t>(0xD800 + (tmp >> 10));
        result.chars[1] = 0xDC00 + (tmp & 0x3FF);
        result.size = 2;
    }
    else [[unlikely]]
    {
        throw std::invalid_argument("invalid codepoint"); // TODO: Better exception
    }

    return result;
}

template <typename CharU>
constexpr codepoint_iterator<CharU> codepoint_begin(std::basic_string_view<CharU> str) noexcept
{
    using result_t = codepoint_iterator<CharU>;

    if constexpr(!char32_like<CharU>) // char8_like and char16_like
    {
        if(str.empty())
            return result_t(str, 0, std::uint8_t(0));
    }

    if constexpr(char8_like<CharU>)
    {
        std::uint8_t ch = static_cast<std::uint8_t>(str[0]);
        std::uint8_t ch_size = PAPILIO_NS utf::is_leading_byte(ch) ?
                                   PAPILIO_NS utf::byte_count(ch) :
                                   1;
        return result_t(str, 0, ch_size);
    }
    else if constexpr(char16_like<CharU>)
    {
        std::uint16_t ch = str[0];
        std::uint8_t ch_size = PAPILIO_NS utf::is_high_surrogate(ch) ?
                                   2 :
                                   1;
        return result_t(str, 0, ch_size);
    }
    else // char32_like
    {
        return result_t(str.begin());
    }
}

template <typename CharU>
constexpr codepoint_iterator<CharU> codepoint_end(std::basic_string_view<CharU> str) noexcept
{
    using result_t = codepoint_iterator<CharU>;

    if constexpr(!char32_like<CharU>) // char8_like and char16_like
    {
        return result_t(str, str.size(), std::uint8_t(0));
    }
    else
    {
        return result_t(str.end());
    }
}
} // namespace papilio::utf
