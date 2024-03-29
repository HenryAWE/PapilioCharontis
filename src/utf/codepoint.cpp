#include <papilio/utf/codepoint.hpp>
#include <iostream>
#include <papilio/detail/prefix.hpp>

namespace papilio::utf
{
std::uint8_t decoder<char>::size_bytes(char8_t ch) noexcept
{
    return PAPILIO_NS utf::byte_count(ch);
}

std::pair<codepoint, std::uint8_t> decoder<char>::to_codepoint(std::string_view ch)
{
    std::u8string_view u8_ch(
        reinterpret_cast<const char8_t*>(ch.data()),
        ch.size()
    );

    return decoder<char8_t>::to_codepoint(u8_ch);
}

auto decoder<wchar_t>::to_char32_t(std::wstring_view ch) -> std::pair<char32_t, std::uint8_t>
{
    if(ch.empty()) [[unlikely]]
        return std::make_pair(U'\0', std::uint8_t(0));

    if constexpr(sizeof(wchar_t) == sizeof(char32_t))
    {
        return std::make_pair(ch[0], std::uint8_t(1));
    }
    else
    {
        std::u16string_view sv(reinterpret_cast<const char16_t*>(ch.data()), ch.size());
        return decoder<char16_t>::to_char32_t(sv);
    }
}

std::pair<codepoint, std::uint8_t> decoder<wchar_t>::to_codepoint(std::wstring_view ch)
{
    auto [ch32, processed_size] = to_char32_t(ch);
    return std::make_pair(decoder<char32_t>::to_codepoint(ch32).first, processed_size);
}

#ifdef PAPILIO_COMPILER_CLANG
#    pragma clang diagnostic push
#    if __clang_major__ >= 16
#        pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#    endif
#endif

auto decoder<wchar_t>::from_codepoint(codepoint cp) -> from_codepoint_result
{
    from_codepoint_result result;

    if constexpr(sizeof(wchar_t) == 2)
    {
        auto ch16_result = decoder<char16_t>::from_codepoint(cp);
        result.chars[0] = ch16_result.chars[0];
        if(ch16_result.size == 2)
            result.chars[1] = ch16_result.chars[1];
        result.size = ch16_result.size;
        result.processed_size = ch16_result.processed_size;
    }
    else // sizeof(wchar_t) == 4
    {
        auto ch32_result = decoder<char32_t>::from_codepoint(cp);
        result.chars[0] = static_cast<wchar_t>(ch32_result.first);
        result.size = 1;
        result.processed_size = ch32_result.second;
    }

    return result;
}

#ifdef PAPILIO_COMPILER_CLANG
#    pragma clang diagnostic pop
#endif

std::ostream& operator<<(std::ostream& os, codepoint cp)
{
    os.write(cp.data(), cp.size_bytes());
    return os;
}

std::wostream& operator<<(std::wostream& os, codepoint cp)
{
    auto result = decoder<wchar_t>::from_codepoint(cp);
    os.write(result.chars, result.size);
    return os;
}

std::basic_ostream<char8_t>& operator<<(std::basic_ostream<char8_t>& os, codepoint cp)
{
    os.write(cp.u8data(), cp.size_bytes());
    return os;
}

std::basic_ostream<char16_t>& operator<<(std::basic_ostream<char16_t>& os, codepoint cp)
{
    auto result = decoder<char16_t>::from_codepoint(cp);
    os.write(result.chars, result.size);
    return os;
}

std::basic_ostream<char32_t>& operator<<(std::basic_ostream<char32_t>& os, codepoint cp)
{
    char32_t ch = decoder<char32_t>::from_codepoint(cp).first;
    os.write(&ch, 1);
    return os;
}
} // namespace papilio::utf

#include <papilio/detail/suffix.hpp>
