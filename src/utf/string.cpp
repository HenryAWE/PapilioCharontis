#include <papilio/utf/string.hpp>
#include <iostream>

#ifdef PAPILIO_COMPILER_CLANG_CL
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wpre-c++17-compat"
#endif

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

#ifdef PAPILIO_COMPILER_CLANG_CL
#    pragma clang diagnostic pop
#endif
