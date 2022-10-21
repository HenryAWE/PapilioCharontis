#include <papilio/utf8.hpp>
#include <cassert>
#include <optional>
#include <papilio/core.hpp> // slice


namespace papilio
{
    namespace utf8
    {
        std::size_t strlen(std::string_view str) noexcept
        {
            std::size_t result = 0;
            for(std::size_t i = 0; i < str.size();)
            {
                ++result;
                std::uint8_t len = is_leading_byte(str[i]);
                if(len == 0) // corrupted string
                    break;
                i += len;
            }

            return result;
        }

        std::optional<std::pair<std::size_t, std::uint8_t>> byte_index(std::string_view str, std::size_t idx) noexcept
        {
            std::size_t byte_idx = 0;
            std::uint8_t count = 0;

            for(std::size_t i = 0; i < idx; ++i)
            {
                count = is_leading_byte(str[byte_idx]);
                if(count == 0)
                    return std::nullopt;
                byte_idx += count;
                if(byte_idx >= str.size())
                    return std::nullopt;
            }

            count = is_leading_byte(str[byte_idx]);

            return std::make_pair(byte_idx, count);
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
        std::string_view index(const char* str, std::size_t idx) noexcept
        {
            return index(std::string_view(str), idx);
        }

        std::optional<std::pair<std::size_t, std::uint8_t>> rbyte_index(std::string_view str, std::size_t idx) noexcept
        {
            if(idx == -1)
                return std::nullopt;
            std::size_t byte_idx = str.size();
            std::uint8_t count = 0;
            for(std::size_t i = 0; i < idx + 1; ++i)
            {
                count = 0;
                while(count == 0)
                {
                    if(byte_idx == 0)
                        return std::nullopt;
                    --byte_idx;
                    count = is_leading_byte(str[byte_idx]);
                }
            }

            return std::make_pair(byte_idx, count);
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
        std::string_view rindex(const char* str, std::size_t idx) noexcept
        {
            return rindex(std::string_view(str), idx);
        }

        std::string_view substr(
            std::string_view str,
            std::size_t off,
            std::size_t count
        ) noexcept {
            if(count == 0)
                return std::string_view();

            auto begin = byte_index(str, off);
            if(!begin.has_value())
                return std::string_view();

            str = str.substr(begin->first);
            if(count == std::string_view::npos)
                return str;
            auto byte_count = byte_index(str, count);
            if(byte_count.has_value())
                return str.substr(0, byte_count->first);
            else
                return str;
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
        std::string_view substr(
            const char* str,
            std::size_t off,
            std::size_t count
        ) {
            return substr(
                std::string_view(str),
                off,
                count
            );
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
        std::string_view substr(
            const char* str,
            const slice& s
        ) noexcept {
            return substr(
                std::string_view(str),
                s
            );
        }
    }

    void string_container::pop_back() noexcept
    {
        string_view_type view = get_string_view();
        assert(!view.empty());
        size_type idx = view.size() - 1;
        while(utf8::is_leading_byte(view[idx]) == 0)
        {
            if(idx == 0)
            {
                clear();
                return;
            }
            --idx;
        }
        view = string_view_type(view.data(), idx);
        if(is_borrowed())
            m_str = view;
        else
            m_str = std::make_shared<string_type>(view);
    }

    string_container::const_iterator string_container::find(utf8::codepoint cp, size_type pos) const noexcept
    {
        const_iterator it = cbegin();
        const_iterator cend_it = cend();
        for(size_type i = 0; i < pos; ++i)
        {
            if(it == cend_it)
                return cend_it;
            ++it;
        }
        for(; it != cend_it; ++it)
        {
            if(*it == cp)
                break;
        }
        return it;
    }
    string_container::const_iterator string_container::find(const string_container& str, size_type pos) const noexcept
    {
        return find(str.get_string_view(), pos);
    }
    string_container::const_iterator string_container::find(string_view_type str, size_type pos) const noexcept
    {
        string_view_type view = get_string_view();
        size_type byte_off = 0;
        for(size_type i = 0; i < pos; ++i)
        {
            if(byte_off >= view.size())
                return cend();
            byte_off += utf8::is_leading_byte(view[byte_off]);
        }
        size_type byte_idx = view.find(str, byte_off);
        if(byte_idx == view.npos)
            return cend();
        const_pointer ptr = view.data() + byte_idx;
        return const_iterator(
            ptr, utf8::is_leading_byte(ptr[0])
        );
    }

    string_container::const_reference string_container::front() const noexcept
    {
        assert(!empty());
        return utf8::codepoint::decode(get_string_view()).first;
    }
    string_container::const_reference string_container::back() const noexcept
    {
        assert(!empty());
        return utf8::codepoint::rdecode(get_string_view()).first;
    }

    std::u8string string_container::to_u8string() const
    {
        auto view = get_string_view();
        return std::u8string(
            reinterpret_cast<const char8_t*>(view.data()),
            view.size()
        );
    }
    std::u32string string_container::to_u32string() const
    {
        std::u32string result;
        std::copy(begin(), end(), std::back_inserter(result));
        return result;
    }

    string_container::string_type& string_container::get_string()
    {
        assert(!m_str.valueless_by_exception());
        if(m_str.index() != 0)
        {
            string_view_type sv = *std::get_if<1>(&m_str);
            m_str.emplace<0>(std::make_shared<string_type>(sv));
        }

        return **std::get_if<0>(&m_str);
    }
    string_container::string_view_type string_container::get_string_view() const noexcept
    {
        assert(!m_str.valueless_by_exception());
        if(m_str.index() == 0)
        {
            return **std::get_if<0>(&m_str);
        }
        else
        {
            return *std::get_if<1>(&m_str);
        }
    }
}
