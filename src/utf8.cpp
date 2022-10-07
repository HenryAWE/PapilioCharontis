#include <papilio/utf8.hpp>
#include <cassert>
#include <optional>
#include <papilio/core.hpp> // slice


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
        std::pair<char32_t, std::uint8_t> rdecode(const char* src, std::size_t size) noexcept
        {
            if(size == 0)
                size = std::string_view(src).size();
            
            const char* start = nullptr;
            for(const char* it = src + size; it != src; --it)
            {
                if((*(it - 1) & 0b1100'0000) != 0b1000'0000)
                {
                    start = it - 1;
                    break;
                }
            }
            if(!start)
                return std::pair(U'\0', 0);

            return decode(start);
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

        std::optional<std::pair<std::size_t, std::uint8_t>> byte_index(std::string_view str, std::size_t idx) noexcept
        {
            std::size_t i = 0;
            std::uint8_t ch_count = 0;

            ch_count = decode(str.data()).second;
            assert(idx != -1);
            for(std::size_t n = 0; n < idx; ++n)
            {
                i += ch_count;
                if(i >= str.size())
                    return std::nullopt;
                ch_count = decode(str.data() + i).second;
            }

            return std::make_pair(i, ch_count);
        }

        std::string_view index(std::string_view str, std::size_t idx) noexcept
        {
            auto result = byte_index(str, idx);
            if(!result.has_value())
                return std::string_view();

            auto& [i, ch_count] = *result;
            return std::string_view(str.data() + i, ch_count);
        }
        std::string index(const std::string& str, std::size_t idx)
        {
            return std::string(
                index(std::string_view(str), idx)
            );
        }

        std::optional<std::pair<std::size_t, std::uint8_t>> rbyte_index(std::string_view str, std::size_t idx) noexcept
        {
            std::size_t i = str.size();
            std::uint8_t ch_count = 0;

            ch_count = rdecode(str.data(), str.size()).second;
            i -= ch_count;
            assert(idx != -1);
            for(std::size_t n = 0; n < idx; ++n)
            {
                if(i < ch_count)
                    return std::nullopt;
                ch_count = rdecode(str.data(), str.size() - i).second;
                i -= ch_count;
                if(i == 0)
                    return std::nullopt;
            }

            return std::make_pair(i, ch_count);
        }

        std::string_view rindex(std::string_view str, std::size_t idx) noexcept
        {
            auto result = rbyte_index(str, idx);
            if(!result.has_value())
                return std::string_view();

            auto& [i, ch_count] = *result;
            return std::string_view(str.data() + i, ch_count);
        }
        std::string rindex(const std::string& str, std::size_t idx)
        {
            return std::string(
                rindex(std::string_view(str), idx)
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

        std::pair<std::size_t, std::size_t> get_slice_byte_range(
            std::string_view str,
            const slice& s
        ) noexcept {
            std::pair<std::size_t, std::size_t> result;

            if(s.begin() < 0)
            {
                auto begin_index = rbyte_index(str, -(s.begin() + 1));
                if(begin_index.has_value())
                    result.first = begin_index->first;
                else
                    result.first = 0;
            }
            else
            {
                auto begin_index = byte_index(str, s.begin());
                if(begin_index.has_value())
                    result.first = begin_index->first;
                else
                    return std::make_pair(0, 0);
            }

            if(s.end() < 0)
            {
                auto end_index = rbyte_index(str, -(s.end() + 1));
                if(end_index.has_value())
                    result.second = end_index->first;
                else
                    return std::make_pair(0, 0);
            }
            else
            {
                auto end_index = byte_index(str, s.end());
                if(end_index.has_value())
                    result.second = end_index->first;
                else
                    result.second = str.size();
            }

            if(result.first > result.second)
                return std::make_pair(0, 0);
            return result;
        }

        std::string_view substr(
            std::string_view str,
            const slice& s
        ) noexcept {
            auto r = get_slice_byte_range(str, s);

            if(r.first == r.second)
                return std::string_view();
            return std::string_view(str.data() + r.first, str.data() + r.second);
        }
        std::string substr(
            const std::string& str,
            const slice& s
        ) {
            return std::string(
                substr(std::string_view(str), s)
            );
        }
    }
}
