#include <gtest/gtest.h>
#if __has_include(<format>)
#    include <format> // Test ADL-proof
#endif
#include <papilio/format.hpp>
#include <papilio_test/setup.hpp>

namespace test_format
{
struct test_combined
{
    int val = 0;
};
} // namespace test_format

template <typename CharT>
class papilio::formatter<test_format::test_combined, CharT>
{
public:
    using value_type = test_format::test_combined;

    template <typename ParseContext, typename FormatContext>
    auto format(const value_type& val, ParseContext& parse_ctx, FormatContext& fmt_ctx) const
        -> typename FormatContext::iterator
    {
        bool uppercase = false;

        auto parse_it = parse_ctx.begin();
        if(parse_it != parse_ctx.end())
        {
            char32_t ch = *parse_it;
            if(ch == U'U')
            {
                uppercase = true;
                ++parse_it;
            }
            else if(ch != U'}')
                throw format_error("bad spec");
            parse_ctx.advance_to(parse_it);
        }

        using context_t = format_context_traits<FormatContext>;
        if(uppercase)
            context_t::append(fmt_ctx, PAPILIO_TSTRING_VIEW(CharT, "COMBINED: "));
        else
            context_t::append(fmt_ctx, PAPILIO_TSTRING_VIEW(CharT, "combined: "));

        context_t::append_by_formatter(fmt_ctx, val.val);

        return fmt_ctx.out();
    }
};

TEST(format, combined_formatter)
{
    using namespace papilio;
    using test_format::test_combined;

    static_assert(formattable<test_combined>);
    static_assert(formattable_with<test_combined, wformat_context>);

    EXPECT_EQ(PAPILIO_NS format("{}", test_combined(0)), "combined: 0");
    EXPECT_EQ(PAPILIO_NS format("{:U}", test_combined(0)), "COMBINED: 0");

    EXPECT_EQ(PAPILIO_NS format(L"{}", test_combined(0)), L"combined: 0");
    EXPECT_EQ(PAPILIO_NS format(L"{:U}", test_combined(0)), L"COMBINED: 0");
}
