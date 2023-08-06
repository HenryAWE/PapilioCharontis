// type traits and concepts

#pragma once

#include <cstddef>
#include <type_traits>
#include <concepts>
#include <string>
#include <string_view>


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
    concept pointer_like =
        (requires(T ptr) { *ptr; ptr.operator->(); } || requires(T ptr, std::size_t i) { ptr[i]; })
        && requires(T ptr) { static_cast<bool>(ptr); };

    // ^^^ concepts ^^^ / vvv tags vvv

    struct reverse_index_t {};
    constexpr reverse_index_t reverse_index = {};
}
