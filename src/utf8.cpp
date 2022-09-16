#include <papilio/utf8.hpp>
#include <cassert>


namespace papilio
{
    namespace utf8
    {
        std::pair<char32_t, std::uint8_t> decode(const char* src)
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

        std::size_t strlen(std::string_view str)
        {
            std::size_t result = 0;
            for(std::size_t i = 0; i < str.size();)
            {
                ++result;
                i += decode(str.data() + i).second;
            }

            return result;
        }
    }
}
