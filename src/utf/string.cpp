#include <papilio/utf/string.hpp>
#include <iostream>
#include <papilio/detail/prefix.hpp>

namespace papilio::utf
{
namespace detail
{
    template <typename CharT>
    static void read_stream_impl(std::basic_istream<CharT>& is, basic_string_container<CharT>& str)
    {
        std::basic_string<CharT> tmp;
        is >> tmp;

        str.clear();
        if(is)
            str.assign(std::move(tmp));
    }
} // namespace detail

std::istream& operator>>(std::istream& is, string_container& str)
{
    detail::read_stream_impl<char>(is, str);
    return is;
}

std::wistream& operator>>(std::wistream& is, wstring_container& str)
{
    detail::read_stream_impl<wchar_t>(is, str);
    return is;
}
} // namespace papilio::utf

#include <papilio/detail/suffix.hpp>
