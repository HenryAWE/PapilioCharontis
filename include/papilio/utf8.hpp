#pragma once

#include <string>


namespace papilio
{
    namespace utf8
    {
        std::pair<char32_t, std::uint8_t> decode(const char* src);

        std::size_t strlen(std::string_view str);
    }
}
