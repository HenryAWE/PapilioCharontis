#include <gtest/gtest.h>
#include <papilio/format/integral.hpp>


TEST(format_integral, int)
{
    using namespace papilio;

    static_assert(PAPILIO_NS formattable<int>);

    {
        const int val = 182376;

        PAPILIO_NS formatter<int> fmt;

        static_format_args<1, 0> args(val);
        std::string result;
        format_context fmt_ctx(std::back_inserter(result), args);

        format_parse_context parse_ctx("{}", args);
        parse_ctx.advance_to(parse_ctx.begin() + 1);

        fmt.format(val, fmt_ctx);

        EXPECT_EQ(result, "182376");
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
