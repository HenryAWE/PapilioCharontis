#include <papilio/utf8.hpp>
#include <cassert>


namespace papilio
{
    namespace utf8
    {
        std::pair<char32_t, std::uint8_t> decode(const char* src) noexcept
        {
            assert(src != nullptr);

            std::uint8_t first_byte = src[0];

            // ASCII (single byte)
            if(first_byte <= 0b0111'1111)
            {
                return std::make_pair(src[0], 1);
            }
            // 2 bytes
            else if(0b1100'0000 <= first_byte && first_byte <= 0b1101'1111)
            {
                char32_t result = U'\0';
                result |= src[0] & 0b0001'1111;
                result <<= 6;
                result |= src[1] & 0b0011'1111;

                return std::make_pair(result, 2);
            }
            // 3 bytes
            else if(0b1110'0000 <= first_byte && first_byte <= 0b1110'1111)
            {
                char32_t result = U'\0';
                result |= src[0] & 0b0000'1111;
                result <<= 6;
                result |= src[1] & 0b0011'1111;
                result <<= 6;
                result |= src[2] & 0b0011'1111;

                return std::make_pair(result, 3);
            }
            // 4 bytes
            else if(0b1111'0000 <= first_byte && first_byte <= 0b1111'0111)
            {
                char32_t result = U'\0';
                result |= src[0] & 0b0000'1111;
                result <<= 6;
                result |= src[1] & 0b0011'1111;
                result <<= 6;
                result |= src[2] & 0b0011'1111;
                result <<= 6;
                result |= src[3] & 0b0011'1111;

                return std::make_pair(result, 3);
            }

            return std::make_pair(U'\0', 0);
        }

        std::size_t strlen(std::string_view str) noexcept
        {
            std::size_t result = 0;
            for(std::size_t i = 0; i < str.size();)
            {
                ++result;
                i += decode(str.data() + i).second;
            }

            return result;
        }

        std::string_view index(std::string_view str, std::size_t idx) noexcept
        {
            std::size_t i = 0;
            std::size_t ch_count = 0;

            ch_count = decode(str.data()).second;
            assert(idx != -1);
            for(std::size_t n = 0; n < idx; ++n)
            {
                i += ch_count;
                if(i >= str.size())
                    return std::string_view();
                ch_count = decode(str.data() + i).second;
            }

            return std::string_view(str.data() + i, ch_count);
        }
        std::string index(const std::string& str, std::size_t idx)
        {
            return std::string(
                index(std::string_view(str), idx)
            );
        }

        std::string_view substr(
            std::string_view str,
            std::size_t off,
            std::size_t count
        ) noexcept {
            if(count == 0)
                return std::string_view();

            std::size_t i = 0;
            std::size_t off_count = 0;
            for(; off_count < off && i < str.size();)
            {
                ++off_count;
                i += decode(str.data() + i).second;
            }

            if(i >= str.size())
                return std::string_view();
            if(count == str.npos)
                return str.substr(i);

            std::size_t j = i;
            std::size_t char_count = 0;
            for(; char_count < count && j < str.size();)
            {
                ++char_count;
                j += decode(str.data() + j).second;
            }

            return str.substr(i, j - i);
        }
        std::string substr(
            const std::string& str,
            std::size_t off,
            std::size_t count
        ) {
            std::string_view sv = substr(
                std::string_view(str),
                off,
                count
            );
            return std::string(sv);
        }
    }
}
