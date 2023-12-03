#pragma once

#include <cstddef>
#include <stdexcept>
#include <papilio/macros.hpp>
#include <papilio/detail/compat.hpp>
#include <papilio/utility.hpp>


namespace papilio::utf
{
    constexpr inline std::size_t npos = std::u8string::npos;

    class invalid_byte : public std::invalid_argument
    {
    public:
        invalid_byte(std::uint8_t ch)
            : invalid_argument("invalid byte"), m_byte(ch) {}

        [[nodiscard]]
        std::uint8_t get() const noexcept
        {
            return m_byte;
        }

    private:
        std::uint8_t m_byte;
    };
    class invalid_leading_byte : public invalid_byte {};

    class invalid_surrogate : public std::invalid_argument
    {
    public:
        invalid_surrogate(std::uint16_t ch)
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
        PAPILIO_ASSUME(is_leading_byte(leading_byte));
        if((leading_byte & 0b10000000) == 0)
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

    namespace detail
    {
        template <char8_like CharT, strlen_behavior OnInvalid>
        constexpr std::size_t strlen_impl(const CharT* str, std::size_t max_chars)
            noexcept(OnInvalid != strlen_behavior::exception)
        {
            PAPILIO_ASSUME(str != nullptr);

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
                    PAPILIO_ASSUME(is_trailing_byte(ch));
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
        template <char8_like CharT, strlen_behavior OnInvalid>
        constexpr std::size_t strlen_impl(std::basic_string_view<CharT> str)
            noexcept(OnInvalid != strlen_behavior::exception)
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
                    PAPILIO_ASSUME(is_trailing_byte(ch));
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

        template <char16_like CharT, strlen_behavior OnInvalid>
        constexpr std::size_t strlen_impl(const CharT* str, std::size_t max_chars)
            noexcept(OnInvalid != strlen_behavior::exception)
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
        template <char16_like CharT, strlen_behavior OnInvalid>
        constexpr std::size_t strlen_impl(std::basic_string_view<CharT> str)
            noexcept(OnInvalid != strlen_behavior::exception)
        {
            std::size_t result = 0;

            for(std::size_t i = 0; i < str.size(); ++i)
            {
                if(!is_low_surrogate(str[i])) [[likely]]
                    ++result;
            }

            return result;
        }

        template <char32_like CharT, strlen_behavior OnInvalid /* unused */>
        constexpr std::size_t strlen_impl(const CharT* str, std::size_t max_chars) noexcept
        {
            for(std::size_t i = 0; i < max_chars; ++i)
            {
                if(str[i] == CharT(0))
                    return i;
            }
        }
        template <char32_like CharT, strlen_behavior OnInvalid /* unused */>
        constexpr std::size_t strlen_impl(std::basic_string_view<CharT> str) noexcept
        {
            return str.size();
        }
    }

#define PAPILIO_UTF_STRLEN(char_t)\
    template <strlen_behavior OnInvalid = strlen_behavior::replace>\
    [[nodiscard]]\
    constexpr std::size_t strlen(const char_t* str, std::size_t max_chars = -1)\
        noexcept(OnInvalid != strlen_behavior::exception)\
    {\
        return detail::strlen_impl<char_t, OnInvalid>(str, max_chars);\
    }\
    template <strlen_behavior OnInvalid = strlen_behavior::replace>\
    [[nodiscard]]\
    constexpr std::size_t strlen(std::basic_string_view<char_t> str)\
        noexcept(OnInvalid != strlen_behavior::exception)\
    {\
        return detail::strlen_impl<char_t, OnInvalid>(str);\
    }

    PAPILIO_UTF_STRLEN(char);
    PAPILIO_UTF_STRLEN(char8_t);
    PAPILIO_UTF_STRLEN(char16_t);
    PAPILIO_UTF_STRLEN(char32_t);
    PAPILIO_UTF_STRLEN(wchar_t);

#undef PAPILIO_UTF_STRLEN

    namespace detail
    {
        template <char8_like CharT>
        constexpr std::size_t index_offset_impl(std::size_t idx, const CharT* str, std::size_t max_chars) noexcept
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
        constexpr std::size_t index_offset_impl_r(std::size_t idx, const CharT* str, std::size_t max_chars) noexcept
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
        constexpr std::size_t index_offset_impl(std::size_t idx, const CharT* str, std::size_t max_chars) noexcept
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
        constexpr std::size_t index_offset_impl_r(std::size_t idx, const CharT* str, std::size_t max_chars) noexcept
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
        constexpr std::size_t index_offset_impl(std::size_t idx, const CharT* str, std::size_t max_chars) noexcept
        {
            return idx < max_chars ? idx : npos;
        }
        template <char32_like CharT>
        constexpr std::size_t index_offset_impl_r(std::size_t idx, const CharT* str, std::size_t max_chars) noexcept
        {
            if(idx > max_chars - 1)
                return npos;
            return max_chars - 1 - idx;
        }
    }

#define PAPILIO_UTF_INDEX_OFFSET(char_t)\
    [[nodiscard]]\
    constexpr std::size_t index_offset(std::size_t idx, const char_t* str, std::size_t max_chars = -1) noexcept\
    {\
        return detail::index_offset_impl<char_t>(idx, str, max_chars);\
    }\
    [[nodiscard]]\
    constexpr std::size_t index_offset(std::size_t idx, std::basic_string_view<char_t> str) noexcept\
    {\
        return detail::index_offset_impl<char_t>(idx, str.data(), str.size());\
    }

#define PAPILIO_UTF_INDEX_OFFSET_R(char_t)\
    [[nodiscard]]\
    constexpr std::size_t index_offset(reverse_index_t, std::size_t idx, const char_t* str, std::size_t max_chars) noexcept\
    {\
        return detail::index_offset_impl_r<char_t>(idx, str, max_chars);\
    }\
    [[nodiscard]]\
    constexpr std::size_t index_offset(reverse_index_t, std::size_t idx, std::basic_string_view<char_t> str) noexcept\
    {\
        return detail::index_offset_impl_r<char_t>(idx, str.data(), str.size());\
    }

    PAPILIO_UTF_INDEX_OFFSET(char);
    PAPILIO_UTF_INDEX_OFFSET_R(char);
    PAPILIO_UTF_INDEX_OFFSET(char8_t);
    PAPILIO_UTF_INDEX_OFFSET_R(char8_t);

    PAPILIO_UTF_INDEX_OFFSET(char16_t);
    PAPILIO_UTF_INDEX_OFFSET_R(char16_t);
    PAPILIO_UTF_INDEX_OFFSET(char32_t);
    PAPILIO_UTF_INDEX_OFFSET_R(char32_t);

    PAPILIO_UTF_INDEX_OFFSET(wchar_t);
    PAPILIO_UTF_INDEX_OFFSET_R(wchar_t);

#undef PAPILIO_UTF_INDEX_OFFSET
#undef PAPILIO_UTF_INDEX_OFFSET_R

    // vvv locale independent APIs vvv

    [[nodiscard]]
    constexpr bool is_digit(char32_t ch) noexcept
    {
        return U'0' <= ch && ch <= U'9';
    }

    inline namespace literals {}
}

namespace papilio
{
    inline namespace literals
    {
        using namespace utf::literals;
    }
}
