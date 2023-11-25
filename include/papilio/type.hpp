// concepts, type traits, tags, auxiliary types

#pragma once

#include <cstddef>
#include <limits>
#include <type_traits>
#include <concepts>
#include <string>
#include <string_view>
#include "macros.hpp"


namespace papilio
{
    template <typename T>
    concept char_like =
        std::is_same_v<T, char> ||
        std::is_same_v<T, wchar_t> ||
        std::is_same_v<T, char16_t> ||
        std::is_same_v<T, char32_t> ||
        std::is_same_v<T, char8_t>;

    template <typename CharT>
    concept char_8b = std::is_same_v<CharT, char> || std::is_same_v<CharT, char8_t>;
    template <typename CharT>
    concept char_16b =
        std::is_same_v<CharT, char16_t> ||
        (sizeof(wchar_t) == 2 && std::is_same_v<CharT, wchar_t>);
    template <typename CharT>
    concept char_32b =
        std::is_same_v<CharT, char32_t> ||
        (sizeof(wchar_t) == 4 && std::is_same_v<CharT, wchar_t>);

    template <typename T, typename CharT>
    concept basic_string_like =
        std::is_same_v<std::decay_t<T>, CharT*> ||
        std::is_same_v<std::decay_t<T>, const CharT*> ||
        std::is_same_v<T, std::basic_string<CharT>> ||
        std::is_same_v<T, std::basic_string_view<CharT>>;

    template <typename T>
    concept string_like = basic_string_like<T, char>;
    template <typename T>
    concept u8string_like = basic_string_like<T, char8_t>;
    template <typename T>
    concept u16string_like = basic_string_like<T, char16_t>;
    template <typename T>
    concept u32string_like = basic_string_like<T, char32_t>;
    template <typename T>
    concept wstring_like = basic_string_like<T, wchar_t>;

    template <typename T>
    concept pointer_like =
        (requires(T ptr) { *ptr; ptr.operator->(); } || requires(T ptr, std::size_t i) { ptr[i]; })
        && requires(T ptr) { static_cast<bool>(ptr); };

    // ^^^ concepts ^^^ / vvv tags vvv

    struct reverse_index_t {};
    constexpr reverse_index_t reverse_index = {};

    // ^^^ tags ^^^ / vvv auxiliary types vvv

    // Signed size type
    using ssize_t = std::make_signed_t<std::size_t>;

    // [begin, end) range
    // Negative value means reverse index like Python.
    // For example, -1 refers to the last element, and -2 refers to the second to last element.
    class slice : public std::pair<ssize_t, ssize_t>
    {
    public:
        using size_type = std::size_t;
        using index_type = ssize_t;

        static constexpr index_type npos = std::numeric_limits<index_type>::max();

        constexpr slice() noexcept
            : slice(0, npos) {}
        constexpr slice(const slice&) noexcept = default;
        constexpr explicit slice(index_type start, index_type stop = npos) noexcept
            : pair(start, stop) {}

        constexpr slice& operator=(const slice&) noexcept = default;

        constexpr bool operator==(const slice&) const noexcept = default;

        constexpr void normalize(std::in_place_t, size_type len) noexcept
        {
            PAPILIO_ASSERT(len <= std::numeric_limits<index_type>::max());

            if(first < 0)
                first = len + first;

            if(second < 0)
                second = len + second;
            else if(second == npos)
                second = len;
        }
        [[nodiscard]]
        constexpr slice normalize(size_type len) const noexcept
        {
            slice result = *this;
            result.normalize(std::in_place, len);
            return result;
        }

        [[nodiscard]]
        constexpr index_type begin() const noexcept
        {
            return first;
        }
        [[nodiscard]]
        constexpr index_type end() const noexcept
        {
            return second;
        }

        [[nodiscard]]
        constexpr size_type length() const noexcept
        {
            PAPILIO_ASSERT(first > 0);
            PAPILIO_ASSERT(second > 0);
            PAPILIO_ASSERT(second != npos);
            return second - first;
        }
    };
}
