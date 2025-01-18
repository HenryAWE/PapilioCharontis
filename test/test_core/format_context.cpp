#include <gtest/gtest.h>
#include <papilio/core.hpp>
#include <papilio/format.hpp>
#include "test_core.hpp"
#include <papilio_test/setup.hpp>

using namespace test_core;

TYPED_TEST(format_context_suite, append)
{
    using namespace papilio;

    using context_type = typename TestFixture::context_type;

    typename TestFixture::string_type result{};
    context_type ctx = TestFixture::create_context(result);

    using context_t = format_context_traits<context_type>;

    {
        const auto str = PAPILIO_TSTRING_ARRAY(TypeParam, "1234");

        context_t::append(ctx, str);
        EXPECT_EQ(result, str);
    }

    result.clear();
    {
        context_t::append(ctx, '1', 4);

        const auto expected_str = PAPILIO_TSTRING_ARRAY(TypeParam, "1111");
        EXPECT_EQ(result, expected_str);
    }

    result.clear();
    {
        context_t::append(ctx, U'\u00c4', 2);

        const auto expected_str = PAPILIO_TSTRING_ARRAY(TypeParam, "\u00c4\u00c4");
        EXPECT_EQ(result, expected_str);
    }
}

TYPED_TEST(format_context_suite, format_to)
{
    using namespace papilio;

    using context_type = typename TestFixture::context_type;

    typename TestFixture::string_type result{};
    context_type ctx = TestFixture::create_context(result);

    using context_t = format_context_traits<context_type>;

    {
        context_t::append_by_format(ctx, true);

        const auto expected_str = PAPILIO_TSTRING_ARRAY(TypeParam, "true");
        EXPECT_EQ(result, expected_str);
    }

    result.clear();
    {
        context_t::append_by_format(ctx, false);

        const auto expected_str = PAPILIO_TSTRING_ARRAY(TypeParam, "false");
        EXPECT_EQ(result, expected_str);
    }

    result.clear();
    {
        context_t::append_by_formatter(ctx, TypeParam('\''), true);

        const auto expected_str = PAPILIO_TSTRING_ARRAY(TypeParam, "'\\''");
        EXPECT_EQ(result, expected_str);
    }

    result.clear();
    {
        context_t::format_to(ctx, PAPILIO_TSTRING_VIEW(TypeParam, "({:+})"), 1);

        const auto expected_str = PAPILIO_TSTRING_ARRAY(TypeParam, "(+1)");
        EXPECT_EQ(result, expected_str);
    }
}

TYPED_TEST(format_context_suite, append_escaped)
{
    using namespace papilio;

    using context_type = typename TestFixture::context_type;

    typename TestFixture::string_type result{};
    context_type ctx = TestFixture::create_context(result);

    using context_t = format_context_traits<context_type>;

    {
        using namespace utf::literals;
        context_t::append_escaped(ctx, U'\''_cp);
        context_t::append_escaped(ctx, U' '_cp);
        context_t::append_escaped(ctx, U'"'_cp);

        const auto expected_str = PAPILIO_TSTRING_ARRAY(TypeParam, "\\' \"");
        EXPECT_EQ(result, expected_str);
    }

    result.clear();
    {
        context_t::append_escaped(ctx, PAPILIO_TSTRING_VIEW(TypeParam, "hello\t"));

        const auto expected_str = PAPILIO_TSTRING_ARRAY(TypeParam, "hello\\t");
        EXPECT_EQ(result, expected_str);
    }

    if constexpr(char8_like<TypeParam>)
    {
        result.clear();
        {
            context_t::append_escaped(ctx, reinterpret_cast<const TypeParam*>("\xc3\x28"));

            const auto expected_str = PAPILIO_TSTRING_ARRAY(TypeParam, "\\x{c3}(");
            EXPECT_EQ(result, expected_str);
        }
    }
}
