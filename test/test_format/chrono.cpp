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

TEST(formatter, sys_time)
{
    using std::chrono::system_clock;
    using namespace std::chrono_literals;

    using namespace papilio;

    // century and year
    {
        static_assert(formattable<decltype(2024y)>);
        EXPECT_EQ(PAPILIO_NS format("{:%C}", 2024y), "20");
        EXPECT_EQ(PAPILIO_NS format("{:%Y}", 2024y), "2024");
        EXPECT_EQ(PAPILIO_NS format("{:%y}", 2024y), "24");
    }

    // month
    {
        static_assert(formattable<decltype(std::chrono::January)>);
        EXPECT_EQ(PAPILIO_NS format("{:%m}", std::chrono::January), "01");
        EXPECT_EQ(PAPILIO_NS format("{:%m}", std::chrono::December), "12");
    }

    // day
    {
        static_assert(formattable<decltype(1d)>);
        EXPECT_EQ(PAPILIO_NS format("{:%d}", 1d), "01");
        EXPECT_EQ(PAPILIO_NS format("{:%d}", 10d), "10");
        EXPECT_EQ(PAPILIO_NS format("{:%e}", 1d), " 1");
        EXPECT_EQ(PAPILIO_NS format("{:%e}", 10d), "10");
    }

    // H:M:S
    {
        std::chrono::hh_mm_ss<std::chrono::seconds> hms{65s};

        static_assert(formattable<decltype(hms)>);
        EXPECT_EQ(PAPILIO_NS format("{:%H}", hms), "00");
        EXPECT_EQ(PAPILIO_NS format("{:%I}", hms), "00");
        EXPECT_EQ(PAPILIO_NS format("{:%M}", hms), "01");
        EXPECT_EQ(PAPILIO_NS format("{:%S}", hms), "05");
        EXPECT_EQ(PAPILIO_NS format("{:%R}", hms), "00:01");
        EXPECT_EQ(PAPILIO_NS format("{:%T}", hms), "00:01:05");
    }

    // %Q and %q
    {
        auto d0 = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::sys_days(2023y / 11 / 8) - std::chrono::sys_days(2023y / 11 / 8)
        );

        EXPECT_EQ(PAPILIO_NS format("{:%Q}", d0), "0");
        EXPECT_EQ(PAPILIO_NS format("{:%q}", d0), "0s");

        auto d7 = std::chrono::duration_cast<std::chrono::days>(
            std::chrono::sys_days(2023y / 11 / 8) - std::chrono::sys_days(2023y / 11 / 1)
        );

        EXPECT_EQ(PAPILIO_NS format("{:%Q}", d7), "7");
        EXPECT_EQ(PAPILIO_NS format("{:%q}", d7), "7d");
    }

    {
        auto date = 2023y / 11 / 8;
        system_clock::time_point t = std::chrono::sys_days(date);
        static_assert(formattable<system_clock::time_point>);

        EXPECT_EQ(PAPILIO_NS format("{:%Y}", t), "2023");
        EXPECT_EQ(PAPILIO_NS format("{:%y}", t), "23");
    }

    // Print platform-dependent result for visual check
    PAPILIO_NS println(std::cerr, "now(): {:%Y-%m-%d}", system_clock::now());
}
