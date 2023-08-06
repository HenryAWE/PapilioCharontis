#include <papilio/utf/codepoint.hpp>
#include <papilio/format.hpp>


namespace papilio::utf
{
    auto decoder<char16_t>::to_char32_t(std::u16string_view ch) -> std::pair<char32_t, std::uint8_t>
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
    auto decoder<char16_t>::to_char32_t(char16_t first, char16_t second) -> std::pair<char32_t, std::uint8_t>
    {
        char16_t tmp[2] = { first, second };

        return to_char32_t(std::u16string_view(tmp, 2));
    }

    std::uint8_t decoder<char16_t>::size_bytes(char16_t ch) noexcept
    {
        if(!is_high_surrogate(ch))
        {
            return 1;
        }
        else
        {
            return 2;
        }
    }

    auto decoder<char16_t>::to_codepoint(std::u16string_view ch) -> std::pair<codepoint, std::uint8_t>
    {
        return decoder<char32_t>::to_codepoint(to_char32_t(ch).first);
    }
    auto decoder<char16_t>::to_codepoint(char16_t first, char16_t second) -> std::pair<codepoint, std::uint8_t>
    {
        return decoder<char32_t>::to_codepoint(to_char32_t(first, second).first);
    }

    auto decoder<char16_t>::from_codepoint(codepoint cp) -> from_codepoint_result
    {
        from_codepoint_result result;

        char32_t ch32;
        std::tie(ch32, result.processed_size) = decoder<char32_t>::from_codepoint(cp);
        if(ch32 <= 0xD7FF || (0xE000 <= ch32 && ch32 <=0xFFFF))
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

    auto decoder<wchar_t>::to_char32_t(std::wstring_view ch)->std::pair<char32_t, std::uint8_t>
    {
        if(ch.empty()) [[unlikely]]
            return std::make_pair(U'\0', 0);

        if constexpr(sizeof(wchar_t) == sizeof(char32_t))
        {
            return std::make_pair(ch[0], 1);
        }
        else
        {
            std::u16string_view sv(reinterpret_cast<const char16_t*>(ch.data()), ch.size());
            return decoder<char16_t>::to_char32_t(sv);
        }
    }

    auto decoder<wchar_t>::to_codepoint(std::wstring_view ch) -> std::pair<codepoint, std::uint8_t>
    {
        return decoder<char32_t>::to_codepoint(to_char32_t(ch).first);
    }

    auto decoder<wchar_t>::from_codepoint(codepoint cp) -> from_codepoint_result
    {
        from_codepoint_result result;

        if constexpr(sizeof(wchar_t) == sizeof(char16_t))
        {
            auto ch16_result = decoder<char16_t>::from_codepoint(cp);
            result.chars[0] = ch16_result.chars[0];
            if(ch16_result.size == 2)
                result.chars[1] = ch16_result.chars[1];
            result.size = ch16_result.size;
            result.processed_size = ch16_result.processed_size;
        }
        else
        {
            auto ch32_result = decoder<char32_t>::from_codepoint(cp);
            result.chars[0] = static_cast<wchar_t>(ch32_result.first);
            result.size = 1;
            result.processed_size = ch32_result.second;
        }

        return result;
    }

    std::ostream& operator<<(std::ostream& os, codepoint cp)
    {
        os << std::string_view(cp);
        return os;
    }
    std::wostream& operator<<(std::wostream& os, codepoint cp)
    {
        os << decoder<wchar_t>::from_codepoint(cp).get();
        return os;
    }
    std::basic_ostream<char8_t>& operator<<(std::basic_ostream<char8_t>& os, codepoint cp)
    {
        os << std::u8string_view(cp);
        return os;
    }
    std::basic_ostream<char16_t>& operator<<(std::basic_ostream<char16_t>& os, codepoint cp)
    {
        os << decoder<char16_t>::from_codepoint(cp).get();
        return os;
    }
    std::basic_ostream<char32_t>& operator<<(std::basic_ostream<char32_t>& os, codepoint cp)
    {
        os << decoder<char32_t>::from_codepoint(cp).first;
        return os;
    }
}
