#pragma once

#include <string>


namespace papilio
{
    class slice;

    namespace utf8
    {
        std::pair<char32_t, std::uint8_t> decode(const char* src) noexcept;
        std::pair<char32_t, std::uint8_t> rdecode(const char* src, std::size_t size = 0) noexcept;

        std::size_t strlen(std::string_view str) noexcept;

        std::string_view index(std::string_view str, std::size_t idx) noexcept;
        std::string index(const std::string& str, std::size_t idx);
        std::string_view index(const char* str, std::size_t idx) noexcept;

        std::string_view rindex(std::string_view str, std::size_t idx) noexcept;
        std::string rindex(const std::string& str, std::size_t idx);
        std::string_view rindex(const char* str, std::size_t idx) noexcept;

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
        std::string_view substr(
            const char* str,
            std::size_t off,
            std::size_t count = std::string_view::npos
        );

        std::string_view substr(
            std::string_view str,
            const slice& s
        ) noexcept;
        std::string substr(
            const std::string& str,
            const slice& s
        );
        std::string_view substr(
            const char* str,
            const slice& s
        ) noexcept;
    }
}
