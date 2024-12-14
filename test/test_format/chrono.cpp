#include <gtest/gtest.h>
#if __has_include(<format>)
#    include <format> // Test ADL-proof
#endif
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

        EXPECT_THROW((void)PAPILIO_NS format("{:%m}", 2024y), format_error);
    }

    // month
    {
        static_assert(formattable<decltype(std::chrono::January)>);
        EXPECT_EQ(PAPILIO_NS format("{:%m}", std::chrono::January), "01");
        EXPECT_EQ(PAPILIO_NS format("{:%m}", std::chrono::December), "12");
        EXPECT_EQ(PAPILIO_NS format("{:%b}", std::chrono::January), "Jan");
        EXPECT_EQ(PAPILIO_NS format("{:%B}", std::chrono::January), "January");
        EXPECT_EQ(PAPILIO_NS format("{:%b}", std::chrono::December), "Dec");
        EXPECT_EQ(PAPILIO_NS format("{:%B}", std::chrono::December), "December");
        EXPECT_EQ(PAPILIO_NS format("{:%b}", std::chrono::month(13)), "month(13)");
        EXPECT_EQ(PAPILIO_NS format("{:%B}", std::chrono::month(13)), "month(13)");
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
        {
            std::chrono::hh_mm_ss<std::chrono::seconds> hms{3600s + 2 * 60s + 5s};

            static_assert(formattable<decltype(hms)>);
            EXPECT_EQ(PAPILIO_NS format("{:%H}", hms), "01");
            EXPECT_EQ(PAPILIO_NS format("{:%I}", hms), "01");
            EXPECT_EQ(PAPILIO_NS format("{:%M}", hms), "02");
            EXPECT_EQ(PAPILIO_NS format("{:%S}", hms), "05");
            EXPECT_EQ(PAPILIO_NS format("{:%R}", hms), "01:02");
            EXPECT_EQ(PAPILIO_NS format("{:%R}", hms), PAPILIO_NS format("{:%H:%M}", hms));
            EXPECT_EQ(PAPILIO_NS format("{:%T}", hms), "01:02:05");
            EXPECT_EQ(PAPILIO_NS format("{:%T}", hms), PAPILIO_NS format("{:%H:%M:%S}", hms));
            EXPECT_EQ(PAPILIO_NS format("{}", hms), "01:02:05");
            EXPECT_EQ(PAPILIO_NS format("{}", hms), PAPILIO_NS format("{:%T}", hms));

            EXPECT_EQ(PAPILIO_NS format("{:%p}", hms), "AM");
            EXPECT_EQ(PAPILIO_NS format("{:%r}", hms), "01:02:05 AM");
            EXPECT_EQ(PAPILIO_NS format("{:%r}", hms), PAPILIO_NS format("{:%I:%M:%S %p}", hms));
        }

        {
            std::chrono::hh_mm_ss<std::chrono::seconds> hms{13h};

            static_assert(formattable<decltype(hms)>);
            EXPECT_EQ(PAPILIO_NS format("{:%H}", hms), "13");
            EXPECT_EQ(PAPILIO_NS format("{:%I}", hms), "01");
            EXPECT_EQ(PAPILIO_NS format("{:%R}", hms), "13:00");
            EXPECT_EQ(PAPILIO_NS format("{:%R}", hms), PAPILIO_NS format("{:%H:%M}", hms));
            EXPECT_EQ(PAPILIO_NS format("{:%T}", hms), "13:00:00");
            EXPECT_EQ(PAPILIO_NS format("{:%T}", hms), PAPILIO_NS format("{:%H:%M:%S}", hms));
            EXPECT_EQ(PAPILIO_NS format("{}", hms), "13:00:00");
            EXPECT_EQ(PAPILIO_NS format("{}", hms), PAPILIO_NS format("{:%T}", hms));

            EXPECT_EQ(PAPILIO_NS format("{:%p}", hms), "PM");
            EXPECT_EQ(PAPILIO_NS format("{:%r}", hms), "01:00:00 PM");
            EXPECT_EQ(PAPILIO_NS format("{:%r}", hms), PAPILIO_NS format("{:%I:%M:%S %p}", hms));
        }

        {
            std::chrono::hh_mm_ss<std::chrono::milliseconds> hms_ms{100ms};
            SCOPED_TRACE("fractional_width = " + std::to_string(hms_ms.fractional_width));

            EXPECT_EQ(PAPILIO_NS format("{:%S}", hms_ms), "00.100");
            EXPECT_EQ(PAPILIO_NS format("{:%T}", hms_ms), "00:00:00.100");
            EXPECT_EQ(PAPILIO_NS format("{:%T}", hms_ms), PAPILIO_NS format("{:%H:%M:%S}", hms_ms));
            EXPECT_EQ(PAPILIO_NS format("{:%r}", hms_ms), PAPILIO_NS format("{:%I:%M:%S %p}", hms_ms));
        }

        {
            std::chrono::hh_mm_ss<std::chrono::seconds> hms{};

            EXPECT_THROW((void)PAPILIO_NS format("{:%Y}", hms), format_error);
            EXPECT_THROW((void)PAPILIO_NS format("{:%m}", hms), format_error);
            EXPECT_THROW((void)PAPILIO_NS format("{:%d}", hms), format_error);
            EXPECT_THROW((void)PAPILIO_NS format("{:%u}", hms), format_error);
        }
    }

    // Day of the week
    {
        static_assert(formattable<decltype(std::chrono::Sunday)>);
        EXPECT_EQ(PAPILIO_NS format("{:%w}", std::chrono::Sunday), "0");
        EXPECT_EQ(PAPILIO_NS format("{:%u}", std::chrono::Sunday), "7");
        EXPECT_EQ(PAPILIO_NS format("{:%a}", std::chrono::Sunday), "Sun");
        EXPECT_EQ(PAPILIO_NS format("{:%A}", std::chrono::Sunday), "Sunday");
        EXPECT_EQ(PAPILIO_NS format("{}", std::chrono::Sunday), "Sun");
        EXPECT_EQ(PAPILIO_NS format("{:%w}", std::chrono::Monday), "1");
        EXPECT_EQ(PAPILIO_NS format("{:%u}", std::chrono::Monday), "1");
        EXPECT_EQ(PAPILIO_NS format("{:%a}", std::chrono::Monday), "Mon");
        EXPECT_EQ(PAPILIO_NS format("{:%A}", std::chrono::Monday), "Monday");
        EXPECT_EQ(PAPILIO_NS format("{}", std::chrono::Monday), "Mon");

        EXPECT_EQ(PAPILIO_NS format("{}", std::chrono::weekday(8)), "weekday(8)");
        EXPECT_EQ(PAPILIO_NS format("{:%a}", std::chrono::weekday(8)), "weekday(8)");
        EXPECT_EQ(PAPILIO_NS format("{:%A}", std::chrono::weekday(8)), "weekday(8)");

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
        EXPECT_EQ(PAPILIO_NS format("{}", 1ns), "1ns");
        EXPECT_EQ(PAPILIO_NS format("{}", 1us), "1us");
        EXPECT_EQ(PAPILIO_NS format("{}", 1ms), "1ms");
        EXPECT_EQ(PAPILIO_NS format("{}", 1s), "1s");
        EXPECT_EQ(PAPILIO_NS format("{}", 1min), "1min");
        EXPECT_EQ(PAPILIO_NS format("{}", 1h), "1h");

        EXPECT_EQ(PAPILIO_NS format(L"{}", 1ns), L"1ns");
        EXPECT_EQ(PAPILIO_NS format(L"{}", 1us), L"1us");
        EXPECT_EQ(PAPILIO_NS format(L"{}", 1ms), L"1ms");
        EXPECT_EQ(PAPILIO_NS format(L"{}", 1s), L"1s");
        EXPECT_EQ(PAPILIO_NS format(L"{}", 1min), L"1min");
        EXPECT_EQ(PAPILIO_NS format(L"{}", 1h), L"1h");

        auto d0 = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::sys_days(2023y / 11 / 8) - std::chrono::sys_days(2023y / 11 / 8)
        );

        static_assert(formattable<decltype(d0)>);
        EXPECT_EQ(PAPILIO_NS format("{:%Q}", d0), "0");
        EXPECT_EQ(PAPILIO_NS format("{:%q}", d0), "0s");
        EXPECT_EQ(PAPILIO_NS format("{}", d0), "0s");

        auto d7 = std::chrono::duration_cast<std::chrono::days>(
            std::chrono::sys_days(2023y / 11 / 8) - std::chrono::sys_days(2023y / 11 / 1)
        );

        static_assert(formattable<decltype(d7)>);
        EXPECT_EQ(PAPILIO_NS format("{:%Q}", d7), "7");
        EXPECT_EQ(PAPILIO_NS format("{:%q}", d7), "7d");
        EXPECT_EQ(PAPILIO_NS format("{}", d7), "7d");
    }

    // Date
    {
        using std::chrono::last;
        auto date = 2023y / 11 / 8;
        static_assert(formattable<decltype(date)>);
        static_assert(formattable<decltype(2023y / 11)>);
        static_assert(formattable<decltype(11 / last)>);
        static_assert(formattable<decltype(2023y / 11 / last)>);
        static_assert(formattable<decltype(11 / 8d)>);

        EXPECT_EQ(PAPILIO_NS format("{:%Y}", date), "2023");
        EXPECT_EQ(PAPILIO_NS format("{:%y}", date), "23");
        EXPECT_EQ(PAPILIO_NS format("{:%Y-%m}", 2023y / 11), "2023-11");
        EXPECT_EQ(PAPILIO_NS format("{}", 2023y / 11), "2023/Nov");
        EXPECT_EQ(PAPILIO_NS format("{:%m-%d}", 11 / 8d), "11-08");
        EXPECT_EQ(PAPILIO_NS format("{}", 11 / 8d), "Nov/08");
        EXPECT_EQ(PAPILIO_NS format("{}", 11 / last), "Nov/last");
        EXPECT_EQ(PAPILIO_NS format("{}", 2023y / 11 / last), "2023/Nov/last");

        EXPECT_EQ(PAPILIO_NS format("{:%D}", date), "11/08/23");
        EXPECT_EQ(PAPILIO_NS format("{:%D}", date), PAPILIO_NS format("{:%m/%d/%y}", date));
        EXPECT_EQ(PAPILIO_NS format("{:%F}", date), "2023-11-08");
        EXPECT_EQ(PAPILIO_NS format("{:%F}", date), PAPILIO_NS format("{:%Y-%m-%d}", date));
        EXPECT_EQ(PAPILIO_NS format("{}", date), PAPILIO_NS format("{:%F}", date));
    }

    // Day of the year (%j)
    {
        EXPECT_EQ(PAPILIO_NS format("{:%j}", 2023y / 1 / 1), "001");
        EXPECT_EQ(PAPILIO_NS format("{:%j}", 2023y / 12 / 31), "365");
        EXPECT_EQ(PAPILIO_NS format("{:%j}", 2024y / 1 / 1), "001");
        EXPECT_EQ(PAPILIO_NS format("{:%j}", 2024y / 12 / 31), "366");
    }

    {
        auto date = 2023y / 11 / 8;
        system_clock::time_point t = std::chrono::sys_days(date);
        static_assert(formattable<system_clock::time_point>);

        EXPECT_EQ(PAPILIO_NS format("{:%F}", t), "2023-11-08");
        EXPECT_EQ(PAPILIO_NS format("{:%c}", t), "Wed Nov  8 00:00:00 2023");
        EXPECT_EQ(PAPILIO_NS format("{:%Z}", t), "UTC");
        EXPECT_EQ(PAPILIO_NS format("{:%z}", t), "+0000");
        EXPECT_EQ(PAPILIO_NS format("{:%Ez}", t), "+00:00");
        EXPECT_EQ(PAPILIO_NS format("{:%Oz}", t), "+00:00");
        EXPECT_EQ(PAPILIO_NS format("{}", t), PAPILIO_NS format("{:%F %T}", t));

        // Print platform-dependent result for visual check
        auto sys_now = std::chrono::system_clock::now();
        PAPILIO_NS println(
            std::cerr,
            "now(): {}\n"
            "fractional_width = {}",
            sys_now,
            std::chrono::hh_mm_ss<system_clock::duration>::fractional_width
        );

#ifndef PAPILIO_CHRONO_NO_TIMEZONE

        static_assert(formattable<std::chrono::sys_info>);
        PAPILIO_NS println(
            std::cerr,
            "timezone: {0:%z %Z}\n"
            "sys_info: {0}",
            std::chrono::current_zone()->get_info(sys_now)
        );

#endif
    }

    // Plain text and special characters
    {
        EXPECT_EQ(PAPILIO_NS format("{:plain text}", 2024y), "plain text");
        EXPECT_EQ(PAPILIO_NS format("{:%%%t%n}", 2024y), "%\t\n");
    }

    // Error handling
    {
        EXPECT_THROW((void)PAPILIO_NS format("{:{{}", 2024y), format_error);
        EXPECT_THROW((void)PAPILIO_NS format("{:}}", 2024y), format_error);
    }
}
