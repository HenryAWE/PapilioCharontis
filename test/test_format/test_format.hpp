#pragma once

#include <locale>
#include <papilio/format.hpp>

namespace test_format
{
template <typename CharT>
class yes_no_numpunct : public std::numpunct<CharT>
{
    using my_base = std::numpunct<CharT>;

public:
    using string_type = typename my_base::string_type;

protected:
    string_type do_truename() const override
    {
        const CharT yes_str[] = {'y', 'e', 's'};
        return string_type(yes_str, std::size(yes_str));
    }

    string_type do_falsename() const override
    {
        const CharT no_str[] = {'n', 'o'};
        return string_type(no_str, std::size(no_str));
    }
};

class format_disabled
{
public:
    template <typename CharT>
    friend std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, format_disabled)
    {
        os << PAPILIO_TSTRING_VIEW(CharT, "format disabled");
        return os;
    }
};

template <typename CharT = char>
std::locale attach_yes_no(const std::locale& loc = std::locale::classic())
{
    return std::locale(loc, new yes_no_numpunct<CharT>());
}
} // namespace test_format

namespace papilio
{
template <typename CharT>
struct formatter<test_format::format_disabled, CharT> : public disabled_formatter
{};
} // namespace papilio
