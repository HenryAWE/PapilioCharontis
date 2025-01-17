#ifndef PAPILIO_TEST_FORMAT_TEST_FORMAT_HPP
#define PAPILIO_TEST_FORMAT_TEST_FORMAT_HPP

#pragma once

#include <locale>
#include <papilio/format.hpp>
#include <gtest/gtest.h>
#include <papilio_test/locale_helper.hpp>
#include <papilio_test/prefix.hpp>

namespace test_format
{
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
    using char_type = CharT;
    using string_type = std::basic_string<CharT>;
    using string_view_type = std::basic_string_view<CharT>;
};

using char_types = ::testing::Types<char, wchar_t>;
TYPED_TEST_SUITE(format_suite, char_types);

#include <papilio_test/suffix.hpp>

#endif
