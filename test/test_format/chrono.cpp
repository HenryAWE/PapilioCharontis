#include <gtest/gtest.h>
#include <papilio/papilio.hpp>
#include <papilio/formatter/chrono.hpp>
#include <papilio_test/setup.hpp>

namespace test_format
{
static std::tm create_tm_epoch()
{
    // The Unix epoch (January 1, 1970)
    const std::time_t t = 0;
    std::tm result{};

    result = *std::gmtime(&t);

    return result;
}
} // namespace test_format

TEST(formatter, tm)
{
    using namespace papilio;

    std::tm epoch = test_format::create_tm_epoch();
    static_assert(formattable<std::tm>);
    EXPECT_EQ(
        PAPILIO_NS format("{}", epoch),
        "Thu Jan  1 00:00:00 1970"
    );
    EXPECT_EQ(
        PAPILIO_NS format("{:=^32}", epoch),
        "====Thu Jan  1 00:00:00 1970===="
    );
}
