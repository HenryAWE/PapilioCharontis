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

TEST(formatter, chrono)
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
        EXPECT_EQ(PAPILIO_NS format("{}", 2024y), "2024");
        EXPECT_EQ(PAPILIO_NS format("{}", 2024y), PAPILIO_NS format("{:%Y}", 2024y));
    }

    // month
    {
        static_assert(formattable<decltype(std::chrono::January)>);
        EXPECT_EQ(PAPILIO_NS format("{:%m}", std::chrono::January), "01");
        EXPECT_EQ(PAPILIO_NS format("{:%m}", std::chrono::December), "12");
        EXPECT_EQ(PAPILIO_NS format("{:%b}", std::chrono::January), "Jan");
        EXPECT_EQ(PAPILIO_NS format("{:%b}", std::chrono::December), "Dec");
        EXPECT_EQ(PAPILIO_NS format("{:%b}", std::chrono::month(13)), "month(13)");
        EXPECT_EQ(PAPILIO_NS format("{:%b}", std::chrono::month(13)), "month(13)");
        EXPECT_EQ(PAPILIO_NS format("{}", std::chrono::January), "Jan");
        EXPECT_EQ(PAPILIO_NS format("{}", std::chrono::December), "Dec");
        EXPECT_EQ(PAPILIO_NS format("{}", std::chrono::month(13)), "month(13)");
        EXPECT_EQ(PAPILIO_NS format("{}", std::chrono::month(13)), "month(13)");
    }

    // day
    {
        static_assert(formattable<decltype(1d)>);
        EXPECT_EQ(PAPILIO_NS format("{:%d}", 1d), "01");
        EXPECT_EQ(PAPILIO_NS format("{:%d}", 10d), "10");
        EXPECT_EQ(PAPILIO_NS format("{:%e}", 1d), " 1");
        EXPECT_EQ(PAPILIO_NS format("{:%e}", 10d), "10");
        EXPECT_EQ(PAPILIO_NS format("{}", 1d), "01");
        EXPECT_EQ(PAPILIO_NS format("{}", 10d), "10");
        EXPECT_EQ(PAPILIO_NS format("{}", 1d), PAPILIO_NS format("{:%d}", 1d));
        EXPECT_EQ(PAPILIO_NS format("{}", 10d), PAPILIO_NS format("{:%d}", 10d));
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
        EXPECT_EQ(PAPILIO_NS format("{:%R}", hms), PAPILIO_NS format("{:%H:%M}", hms));
        EXPECT_EQ(PAPILIO_NS format("{:%T}", hms), "00:01:05");
        EXPECT_EQ(PAPILIO_NS format("{:%T}", hms), PAPILIO_NS format("{:%H:%M:%S}", hms));
        EXPECT_EQ(PAPILIO_NS format("{}", hms), "00:01:05");
        EXPECT_EQ(PAPILIO_NS format("{}", hms), PAPILIO_NS format("{:%T}", hms));
    }

    // Day of the week
    {
        static_assert(formattable<decltype(std::chrono::Sunday)>);
        EXPECT_EQ(PAPILIO_NS format("{:%w}", std::chrono::Sunday), "0");
        EXPECT_EQ(PAPILIO_NS format("{:%u}", std::chrono::Sunday), "7");
        EXPECT_EQ(PAPILIO_NS format("{:%a}", std::chrono::Sunday), "Sun");
        EXPECT_EQ(PAPILIO_NS format("{}", std::chrono::Sunday), "Sun");
        EXPECT_EQ(PAPILIO_NS format("{:%w}", std::chrono::Monday), "1");
        EXPECT_EQ(PAPILIO_NS format("{:%u}", std::chrono::Monday), "1");
        EXPECT_EQ(PAPILIO_NS format("{:%a}", std::chrono::Monday), "Mon");
        EXPECT_EQ(PAPILIO_NS format("{}", std::chrono::Monday), "Mon");

        static_assert(formattable<decltype(std::chrono::Sunday[1])>);
        EXPECT_EQ(PAPILIO_NS format("{:%w}", std::chrono::Sunday[1]), "0");
        EXPECT_EQ(PAPILIO_NS format("{:%u}", std::chrono::Sunday[1]), "7");
        EXPECT_EQ(PAPILIO_NS format("{:%a}", std::chrono::Sunday[1]), "Sun");
        EXPECT_EQ(PAPILIO_NS format("{}", std::chrono::Sunday[1]), "Sun[1]");
        EXPECT_EQ(PAPILIO_NS format("{:%w}", std::chrono::Monday[1]), "1");
        EXPECT_EQ(PAPILIO_NS format("{:%u}", std::chrono::Monday[1]), "1");
        EXPECT_EQ(PAPILIO_NS format("{:%a}", std::chrono::Monday[1]), "Mon");
        EXPECT_EQ(PAPILIO_NS format("{}", std::chrono::Monday[1]), "Mon[1]");

        using std::chrono::last;
        static_assert(formattable<decltype(std::chrono::Sunday[last])>);
        EXPECT_EQ(PAPILIO_NS format("{:%w}", std::chrono::Sunday[last]), "0");
        EXPECT_EQ(PAPILIO_NS format("{:%u}", std::chrono::Sunday[last]), "7");
        EXPECT_EQ(PAPILIO_NS format("{:%a}", std::chrono::Sunday[last]), "Sun");
        EXPECT_EQ(PAPILIO_NS format("{}", std::chrono::Sunday[last]), "Sun[last]");
        EXPECT_EQ(PAPILIO_NS format("{:%w}", std::chrono::Monday[last]), "1");
        EXPECT_EQ(PAPILIO_NS format("{:%u}", std::chrono::Monday[last]), "1");
        EXPECT_EQ(PAPILIO_NS format("{:%a}", std::chrono::Monday[last]), "Mon");
        EXPECT_EQ(PAPILIO_NS format("{}", std::chrono::Monday[last]), "Mon[last]");
    }

    // %Q and %q
    {
        auto d0 = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::sys_days(2023y / 11 / 8) - std::chrono::sys_days(2023y / 11 / 8)
        );

        EXPECT_EQ(PAPILIO_NS format("{:%Q}", d0), "0");
        EXPECT_EQ(PAPILIO_NS format("{:%q}", d0), "0s");
        EXPECT_EQ(PAPILIO_NS format("{}", d0), "0s");

        auto d7 = std::chrono::duration_cast<std::chrono::days>(
            std::chrono::sys_days(2023y / 11 / 8) - std::chrono::sys_days(2023y / 11 / 1)
        );

        EXPECT_EQ(PAPILIO_NS format("{:%Q}", d7), "7");
        EXPECT_EQ(PAPILIO_NS format("{:%q}", d7), "7d");
        EXPECT_EQ(PAPILIO_NS format("{}", d7), "7d");
    }

    // Date
    {
        auto date = 2023y / 11 / 8;
        static_assert(formattable<decltype(date)>);

        EXPECT_EQ(PAPILIO_NS format("{:%Y}", date), "2023");
        EXPECT_EQ(PAPILIO_NS format("{:%y}", date), "23");

        EXPECT_EQ(PAPILIO_NS format("{:%D}", date), "11/08/23");
        EXPECT_EQ(PAPILIO_NS format("{:%D}", date), PAPILIO_NS format("{:%m/%d/%y}", date));
        EXPECT_EQ(PAPILIO_NS format("{:%F}", date), "2023-11-08");
        EXPECT_EQ(PAPILIO_NS format("{:%F}", date), PAPILIO_NS format("{:%Y-%m-%d}", date));
        EXPECT_EQ(PAPILIO_NS format("{}", date), PAPILIO_NS format("{:%F}", date));
    }

    {
        auto date = 2023y / 11 / 8;
        system_clock::time_point t = std::chrono::sys_days(date);
        static_assert(formattable<system_clock::time_point>);

        EXPECT_EQ(PAPILIO_NS format("{:%F %T}", t), "2023-11-08 00:00:00");
        EXPECT_EQ(PAPILIO_NS format("{}", t), "2023-11-08 00:00:00");
        EXPECT_EQ(PAPILIO_NS format("{}", t), PAPILIO_NS format("{:%F %T}", t));

        // Print platform-dependent result for visual check
        PAPILIO_NS println(std::cerr, "now(): {}", system_clock::now());
    }

    // Plain text and special characters
    {
        EXPECT_EQ(PAPILIO_NS format("{:plain text}", 2024y), "plain text");
        EXPECT_EQ(PAPILIO_NS format("{:%%%t%n}", 2024y), "%\t\n");
    }
}
