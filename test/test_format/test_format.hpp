#ifndef PAPILIO_TEST_FORMAT_TEST_FORMAT_HPP
#define PAPILIO_TEST_FORMAT_TEST_FORMAT_HPP

#pragma once

#include <locale>
#include <papilio/format.hpp>
#include <gtest/gtest.h>

namespace test_format
{
template <typename CharT>
class yes_no_numpunct : public std::numpunct<CharT>
{
    using my_base = std::numpunct<CharT>;

public:
    using string_type = typename my_base::string_type;

    static constexpr CharT yes_string[] = {'y', 'e', 's', '\0'};

    static constexpr CharT no_string[] = {'n', 'o', '\0'};

protected:
    string_type do_truename() const override
    {
        return yes_string;
    }

    string_type do_falsename() const override
    {
        return no_string;
    }
};

class stream_only
{
public:
    friend std::ostream& operator<<(std::ostream& os, const stream_only&);

    friend std::wostream& operator<<(std::wostream& os, const stream_only&);
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

template <typename CharT>
class format_suite : public ::testing::Test
{
public:
    using string_type = std::basic_string<CharT>;
    using string_view_type = std::basic_string_view<CharT>;
};

using char_types = ::testing::Types<char, wchar_t>;
TYPED_TEST_SUITE(format_suite, char_types);

#endif
