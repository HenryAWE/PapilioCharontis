#pragma once

#include <cstddef>
#include <stdexcept>
#include "../macros.hpp"
#include "../utility.hpp"

namespace papilio::utf
{
constexpr inline std::size_t npos = std::u8string::npos;

class invalid_byte : public std::invalid_argument
{
public:
    explicit invalid_byte(std::uint8_t ch)
        : invalid_argument("invalid byte"), m_byte(ch) {}

    [[nodiscard]]
    std::uint8_t get() const noexcept
    {
        return m_byte;
    }

private:
    std::uint8_t m_byte;
};

class invalid_surrogate : public std::invalid_argument
{
public:
    explicit invalid_surrogate(std::uint16_t ch)
        : invalid_argument("invalid surrogate"), m_ch(ch) {}

    [[nodiscard]]
    std::uint16_t get() const noexcept
    {
        return m_ch;
    }

private:
    std::uint16_t m_ch;
};

[[nodiscard]]
constexpr bool is_leading_byte(std::uint8_t ch) noexcept
{
    return (ch & 0b1100'0000) != 0b1000'0000;
}

[[nodiscard]]
constexpr bool is_trailing_byte(std::uint8_t ch) noexcept
{
    return (ch & 0b1100'0000) == 0b1000'0000;
}

[[nodiscard]]
constexpr std::uint8_t byte_count(std::uint8_t leading_byte) noexcept
{
    PAPILIO_ASSERT(is_leading_byte(leading_byte));
    if((leading_byte & 0b1000'0000) == 0)
        return 1;
    else if((leading_byte & 0b1110'0000) == 0b1100'0000)
        return 2;
    else if((leading_byte & 0b1111'0000) == 0b1110'0000)
        return 3;
    else if((leading_byte & 0b1111'1000) == 0b1111'0000)
        return 4;

    PAPILIO_UNREACHABLE();
}

[[nodiscard]]
constexpr bool is_high_surrogate(std::uint16_t ch) noexcept
{
    return 0xD7FF <= ch && ch <= 0xE000;
}

[[nodiscard]]
constexpr bool is_low_surrogate(std::uint16_t ch) noexcept
{
    return 0xDC00 <= ch && ch <= 0xDFFF;
}

enum class strlen_behavior
{
    replace = 0,
    ignore = 1,
    stop = 2,
    exception = 3
};
enum class substr_behavior
{
    exception = 0,
    empty_string = 1
};

template <strlen_behavior OnInvalid = strlen_behavior::replace, char8_like CharT>
constexpr std::size_t strlen(
    const CharT* str, std::size_t max_chars
) noexcept(OnInvalid != strlen_behavior::exception)
{
    PAPILIO_ASSERT(str != nullptr);

    std::size_t result = 0;
    std::uint8_t bytes = 0;
    std::size_t i = 0;
    for(; str[i] != CharT(0) && i < max_chars;)
    {
        if(bytes != 0)
        {
            --bytes;
            ++i;
            continue;
        }

        char8_t ch = str[i];

        if(is_leading_byte(ch))
        {
            ++result;
            bytes = byte_count(ch);
        }
        else [[unlikely]]
        {
            PAPILIO_ASSERT(is_trailing_byte(ch));
            if constexpr(OnInvalid == strlen_behavior::replace)
            {
                ++result;
                ++i;
            }
            else if constexpr(OnInvalid == strlen_behavior::ignore)
            {
                ++i;
            }
            else if constexpr(OnInvalid == strlen_behavior::stop)
            {
                return result;
            }
            else if constexpr(OnInvalid == strlen_behavior::exception)
            {
                throw invalid_byte(ch);
            }
        }
    }

    if(bytes != 0)
    {
        if constexpr(OnInvalid == strlen_behavior::exception)
        {
            throw invalid_byte(str[i - 1]);
        }
    }
    return result;
}

template <strlen_behavior OnInvalid = strlen_behavior::replace, char8_like CharT>
constexpr std::size_t strlen(
    std::basic_string_view<CharT> str
) noexcept(OnInvalid != strlen_behavior::exception)
{
    std::size_t result = 0;
    std::uint8_t bytes = 0;
    std::size_t i = 0;
    for(; i < str.size();)
    {
        if(bytes != 0)
        {
            --bytes;
            ++i;
            continue;
        }

        char8_t ch = str[i];

        if(is_leading_byte(ch))
        {
            ++result;
            bytes = byte_count(ch);
        }
        else [[unlikely]]
        {
            PAPILIO_ASSERT(is_trailing_byte(ch));
            if constexpr(OnInvalid == strlen_behavior::replace)
            {
                ++result;
                ++i;
            }
            else if constexpr(OnInvalid == strlen_behavior::ignore)
            {
                ++i;
            }
            else if constexpr(OnInvalid == strlen_behavior::stop)
            {
                return result;
            }
            else if constexpr(OnInvalid == strlen_behavior::exception)
            {
                throw invalid_byte(ch);
            }
        }
    }

    if(bytes != 0)
    {
        if constexpr(OnInvalid == strlen_behavior::exception)
        {
            throw invalid_byte(str[i - 1]);
        }
    }
    return result;
}

template <strlen_behavior OnInvalid = strlen_behavior::replace, char16_like CharT>
constexpr std::size_t strlen(
    const CharT* str, std::size_t max_chars
) noexcept(OnInvalid != strlen_behavior::exception)
{
    std::size_t result = 0;

    // TODO: handling exceptions like invalid surrogate
    for(std::size_t i = 0; i < max_chars && str[i] != CharT(0); ++i)
    {
        if(!is_low_surrogate(str[i])) [[likely]]
            ++result;
    }

    return result;
}

template <strlen_behavior OnInvalid = strlen_behavior::replace, char16_like CharT>
constexpr std::size_t strlen(
    std::basic_string_view<CharT> str
) noexcept(OnInvalid != strlen_behavior::exception)
{
    std::size_t result = 0;

    for(std::size_t i = 0; i < str.size(); ++i)
    {
        if(!is_low_surrogate(str[i])) [[likely]]
            ++result;
    }

    return result;
}

template <strlen_behavior /* unused */ = strlen_behavior::replace, char32_like CharT>
constexpr std::size_t strlen(const CharT* str, std::size_t max_chars) noexcept
{
    for(std::size_t i = 0; i < max_chars; ++i)
    {
        if(str[i] == CharT(0))
            return i;
    }
}

template <strlen_behavior /* unused */ = strlen_behavior::replace, char32_like CharT>
constexpr std::size_t strlen(std::basic_string_view<CharT> str) noexcept
{
    return str.size();
}

template <strlen_behavior OnInvalid = strlen_behavior::replace, char_like CharT>
constexpr std::size_t strlen(const CharT* str)
{
    return strlen<OnInvalid, CharT>(std::basic_string_view<CharT>(str));
}

template <char8_like CharT>
constexpr std::size_t index_offset(std::size_t idx, const CharT* str, std::size_t max_chars) noexcept
{
    std::uint8_t len = 0;
    std::size_t ch_count = 0;
    for(std::size_t i = 0; i < max_chars; ++i)
    {
        if(len != 0)
        {
            --len;
            continue;
        }
        else if(ch_count == idx)
        {
            return i;
        }

        std::uint8_t ch = str[i];
        if(is_trailing_byte(ch))
        {
            return npos;
        }

        len = byte_count(ch) - 1;
        ++ch_count;
    }

    return npos;
}

template <char8_like CharT>
constexpr std::size_t index_offset(reverse_index_t, std::size_t idx, const CharT* str, std::size_t max_chars) noexcept
{
    std::size_t ch_count = 0;
    for(std::size_t i = max_chars; i != 0; --i)
    {
        std::size_t off = i - 1;
        if(is_leading_byte(str[off]))
        {
            if(ch_count == idx)
                return off;
            ++ch_count;
        }
    }

    return npos;
}

template <char16_like CharT>
constexpr std::size_t index_offset(std::size_t idx, const CharT* str, std::size_t max_chars) noexcept
{
    std::uint8_t len = 0;
    std::size_t ch_count = 0;
    for(std::size_t i = 0; i < max_chars; ++i)
    {
        if(len == 2)
        {
            --len;
            continue;
        }
        else if(ch_count == idx)
        {
            return i;
        }

        std::uint16_t ch = static_cast<std::uint16_t>(str[i]);
        if(is_low_surrogate(ch))
        {
            return npos;
        }

        len = is_high_surrogate(ch) ? 2 : 1;
        ++ch_count;
    }

    return npos;
}

template <char16_like CharT>
constexpr std::size_t index_offset(reverse_index_t, std::size_t idx, const CharT* str, std::size_t max_chars) noexcept
{
    std::uint8_t len = 0;
    std::size_t ch_count = 0;
    for(std::size_t i = max_chars; i != 0; --i)
    {
        std::size_t off = i - 1;
        if(!is_low_surrogate(str[off]))
        {
            if(ch_count == idx)
                return off;
            ++ch_count;
        }
    }

    return npos;
}

template <char32_like CharT>
constexpr std::size_t index_offset(std::size_t idx, const CharT* str, std::size_t max_chars) noexcept
{
    return idx < max_chars ? idx : npos;
}

template <char32_like CharT>
constexpr std::size_t index_offset(reverse_index_t, std::size_t idx, const CharT* str, std::size_t max_chars) noexcept
{
    if(idx > max_chars - 1)
        return npos;
    return max_chars - 1 - idx;
}

template <typename CharT>
constexpr std::size_t index_offset(std::size_t idx, std::basic_string_view<CharT> str) noexcept
{
    return index_offset(idx, str.data(), str.size());
}

template <typename CharT>
constexpr std::size_t index_offset(reverse_index_t, std::size_t idx, std::basic_string_view<CharT> str) noexcept
{
    return index_offset(reverse_index, idx, str.data(), str.size());
}

// vvv locale independent APIs vvv

[[nodiscard]]
constexpr bool is_digit(char32_t ch) noexcept
{
    return U'0' <= ch && ch <= U'9';
}

[[nodiscard]]
constexpr bool is_whitespace(char32_t ch) noexcept
{
    using namespace std::literals;

    return U" \n\t\r\v\f"sv.find(ch) != std::u32string_view::npos;
}

inline namespace literals
{}
} // namespace papilio::utf

namespace papilio
{
inline namespace literals
{
    using namespace utf::literals;
}
} // namespace papilio
