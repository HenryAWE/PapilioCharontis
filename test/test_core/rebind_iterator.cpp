#include <gtest/gtest.h>
#include <papilio/core.hpp>
#include <papilio/format.hpp>
#include <papilio_test/locale_helper.hpp>
#include "test_core.hpp"
#include <papilio_test/setup.hpp>

using namespace test_core;

TYPED_TEST(format_context_suite, rebind)
{
    using namespace papilio;

    using context_type = typename TestFixture::context_type;
    using char_type = typename TestFixture::char_type;

    typename TestFixture::string_type result{};
    context_type ctx = TestFixture::create_context(result);

    using context_t = format_context_traits<context_type>;

    {
        std::vector<char_type> buf;
        using buf_iter = decltype(std::back_inserter(buf));
        static_assert(context_t::template has_rebind<buf_iter>());

        using buf_context = typename context_t::template rebind<buf_iter>::type;
        using buf_ctx_t = format_context_traits<buf_context>;

        buf_context buf_ctx = context_t::template rebind_context<buf_iter>(
            ctx, std::back_inserter(buf)
        );

        buf_ctx_t::append(buf_ctx, char_type('A'), 3);

        constexpr auto expected_str = PAPILIO_TSTRING_VIEW(char_type, "AAA");
        EXPECT_EQ(
            std::basic_string_view<char_type>(buf.data(), 3),
            expected_str
        );
    }
}

TEST(format_context, rebind_locale)
{
    using namespace papilio;

    std::locale loc = papilio_test::attach_yes_no();

    std::string result;
    dynamic_format_args args;
    format_context ctx(loc, std::back_inserter(result), args);

    using context_t = format_context_traits<format_context>;

    {
        std::vector<char> buf;
        using buf_iter = decltype(std::back_inserter(buf));
        static_assert(context_t::has_rebind<buf_iter>());

        using buf_context = context_t::rebind<buf_iter>::type;
        using buf_ctx_t = format_context_traits<buf_context>;

        buf_context buf_ctx = context_t::rebind_context<buf_iter>(
            ctx, std::back_inserter(buf)
        );

        buf_ctx_t::format_to(buf_ctx, "{:L} {:L}", true, false);

        EXPECT_TRUE(result.empty());
        EXPECT_EQ(
            std::string_view(buf.data(), 3 + 1 + 2),
            "yes no"
        );
    }
}
