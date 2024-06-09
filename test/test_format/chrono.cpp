#include <gtest/gtest.h>
#include <papilio/papilio.hpp>
#include <papilio/formatter/chrono.hpp>
#include <papilio_test/setup.hpp>

TEST(formatter, tm)
{
    using namespace papilio;

    std::time_t t = 0;
    std::tm* tm = std::gmtime(&t);

    static_assert(formattable<std::tm>);
    EXPECT_EQ(
        PAPILIO_NS format("{}", *tm),
        "Thu Jan  1 00:00:00 1970\n"
    );
    EXPECT_EQ(
        PAPILIO_NS format("{:=^32%c}", *tm),
        "====Thu Jan  1 00:00:00 1970===="
    );
}
