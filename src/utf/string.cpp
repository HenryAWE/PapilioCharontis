#include <papilio/utf/string.hpp>
#include <iostream>
#include <papilio/detail/prefix.hpp>

namespace papilio::utf
{
std::istream& operator>>(std::istream& is, string_container& str)
{
    std::string tmp = std::move(str).str();
    is >> tmp;
    str.assign(std::move(tmp));

    return is;
}

std::wistream& operator>>(std::wistream& is, wstring_container& str)
{
    std::wstring tmp = std::move(str).str();
    is >> tmp;
    str.assign(std::move(tmp));

    return is;
}
} // namespace papilio::utf

#include <papilio/detail/suffix.hpp>
