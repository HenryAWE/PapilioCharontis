#ifndef PAPILIO_TEST_FORMAT_TEST_CORE_HPP
#define PAPILIO_TEST_FORMAT_TEST_CORE_HPP

#pragma once

#include <papilio/core.hpp>
#include <gtest/gtest.h>
#include <papilio_test/prefix.hpp>

namespace test_core
{
template <typename CharT>
class format_context_suite : public ::testing::Test
{
public:
    using char_type = CharT;
    using string_type = std::basic_string<CharT>;
    using context_type = papilio::basic_format_context<
        std::back_insert_iterator<string_type>,
        CharT>;

    static context_type create_context(string_type& output)
    {
        return context_type{
            std::back_inserter(output),
            papilio::empty_format_args_for<context_type>()
        };
    }
};

using format_context_char_types = ::testing::Types<char, wchar_t, char8_t, char16_t, char32_t>;
TYPED_TEST_SUITE(format_context_suite, format_context_char_types);
} // namespace test_core

#include <papilio_test/suffix.hpp>

#endif
