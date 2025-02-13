#include <gtest/gtest.h>
#if __has_include(<format>)
#    include <format> // Test ADL-proof
#endif
#include <papilio/format.hpp>
#include <papilio/xchar.hpp>
#include <vector>
#include <iostream>
#include "test_format.hpp"
#include <papilio_test/setup.hpp>

TEST(xchar, char8_t)
{
    using namespace papilio;

    {
        using context_t = format_context_traits<u8format_context>;
        static_assert(!context_t::use_locale());
    }

    EXPECT_EQ(PAPILIO_NS format(u8"{}", true), u8"true");
    EXPECT_EQ(PAPILIO_NS formatted_size(u8"{}", true), 4);

    {
        std::u8string str;
        str.resize(2);

        auto [it, n] = PAPILIO_NS format_to_n(str.begin(), 2, u8"{}", 182376);
        EXPECT_EQ(n, 2);
        EXPECT_EQ(it, str.end());
        EXPECT_EQ(str, u8"18");
    }
}

TEST(xchar, char16_t)
{
    using namespace papilio;

    {
        using context_t = format_context_traits<u16format_context>;
        static_assert(!context_t::use_locale());
    }

    EXPECT_EQ(PAPILIO_NS format(u"{}", true), u"true");
    EXPECT_EQ(PAPILIO_NS formatted_size(u"{}", true), 4);

    {
        std::u16string str;
        str.resize(2);

        auto [it, n] = PAPILIO_NS format_to_n(str.begin(), 2, u"{}", 182376);
        EXPECT_EQ(n, 2);
        EXPECT_EQ(it, str.end());
        EXPECT_EQ(str, u"18");
    }
}

TEST(xchar, char32_t)
{
    using namespace papilio;

    {
        using context_t = format_context_traits<u32format_context>;
        static_assert(!context_t::use_locale());
    }

    EXPECT_EQ(PAPILIO_NS format(U"{}", true), U"true");
    EXPECT_EQ(PAPILIO_NS formatted_size(U"{}", true), 4);

    {
        std::u32string str;
        str.resize(2);

        auto [it, n] = PAPILIO_NS format_to_n(str.begin(), 2, U"{}", 182376);
        EXPECT_EQ(n, 2);
        EXPECT_EQ(it, str.end());
        EXPECT_EQ(str, U"18");
    }
}
