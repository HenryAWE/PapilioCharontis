#pragma once

#include <string>


namespace papilio
{
    namespace utf8
    {
        std::pair<char32_t, std::uint8_t> decode(const char* src) noexcept;

        std::size_t strlen(std::string_view str) noexcept;

        std::string_view index(std::string_view str, std::size_t idx) noexcept;
        std::string index(const std::string& str, std::size_t idx);

        std::string_view substr(
            std::string_view str,
            std::size_t off,
            std::size_t count = std::string_view::npos
        ) noexcept;
        std::string substr(
            const std::string& str,
            std::size_t off,
            std::size_t count = std::string_view::npos
        );
    }
}
