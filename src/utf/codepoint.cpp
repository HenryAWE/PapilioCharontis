#include <papilio/utf/codepoint.hpp>
#include <iostream>


namespace papilio::utf
{
    auto decoder<wchar_t>::to_char32_t(std::wstring_view ch) -> std::pair<char32_t, std::uint8_t>
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
        auto [ch32, processed_size] = to_char32_t(ch);
        return std::make_pair(decoder<char32_t>::to_codepoint(ch32).first, processed_size);
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
