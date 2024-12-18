// Split source files of chrono test to reduce size of the object file

#include <gtest/gtest.h>
#if __has_include(<format>)
#    include <format> // Test ADL-proof
#endif
#include <papilio/papilio.hpp>
#include <papilio/formatter/chrono.hpp>
#include <papilio_test/chrono_helper.hpp>
#include <papilio_test/setup.hpp>

TEST(chrono_formatter, duration)
{
    using namespace std::chrono_literals;
    using namespace papilio;

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
    }

    // %Q and %q
    {
        auto d0 = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::sys_days(2023y / 11 / 8) - std::chrono::sys_days(2023y / 11 / 8)
        );

        static_assert(formattable<decltype(d0)>);
        EXPECT_EQ(PAPILIO_NS format("{:%Q}", d0), "0");
        EXPECT_EQ(PAPILIO_NS format("{:%q}", d0), "s");
        EXPECT_EQ(PAPILIO_NS format("{}", d0), "0s");

        auto d7 = std::chrono::duration_cast<std::chrono::days>(
            std::chrono::sys_days(2023y / 11 / 8) - std::chrono::sys_days(2023y / 11 / 1)
        );

        static_assert(formattable<decltype(d7)>);
        EXPECT_EQ(PAPILIO_NS format("{:%Q}", d7), "7");
        EXPECT_EQ(PAPILIO_NS format("{:%q}", d7), "d");
        EXPECT_EQ(PAPILIO_NS format("{}", d7), "7d");

        using my_ratio_1 = std::ratio<64>;
        using my_ratio_2 = std::ratio<7, 3>;

        using unit_list = std::tuple<
            std::atto,
            std::femto,
            std::pico,
            std::nano,
            std::micro,
            std::milli,
            std::centi,
            std::deci,
            std::ratio<1>,
            std::deca,
            std::hecto,
            std::kilo,
            std::mega,
            std::giga,
            std::tera,
            std::peta,
            std::exa,
            std::ratio<60>,
            std::ratio<3600>,
            std::ratio<86400>,
            my_ratio_1,
            my_ratio_2>;

        [&]<std::size_t... Is>(std::index_sequence<Is...>)
        {
            using std::chrono::duration;

            auto op_eq = []<typename Rep, typename Period>(const duration<Rep, Period>& d)
            {
                EXPECT_EQ(
                    PAPILIO_NS format("{}", d),
                    PAPILIO_NS format("{:%Q%q}", d)
                );
            };

            (op_eq(duration<long long, std::tuple_element_t<Is, unit_list>>{1}), ...);
            (op_eq(duration<long double, std::tuple_element_t<Is, unit_list>>{1.5L}), ...);
        }(std::make_index_sequence<std::tuple_size_v<unit_list>>());

        EXPECT_EQ(
            PAPILIO_NS format("{}", std::chrono::duration<int, my_ratio_1>(1)),
            "1[64]s"
        );
        EXPECT_EQ(
            PAPILIO_NS format("{}", std::chrono::duration<int, my_ratio_2>(1)),
            "1[7/3]s"
        );
    }
}

#ifndef PAPILIO_CHRONO_NO_TIMEZONE

TEST(chrono_formatter, time_zone)
{
    using namespace papilio;

    std::string_view tz_names[] = {
        "America/New_York",
        "UTC",
        "Europe/Paris",
        "Asia/Shanghai",
        "Australia/Sydney"
    };

    for(std::string_view tz_name : tz_names)
    {
        const std::chrono::time_zone* tz = nullptr;
        try
        {
            tz = std::chrono::locate_zone(tz_name);
        }
        catch(const std::runtime_error& e)
        {
            GTEST_SKIP()
                << "locate_zone(\"" << tz_name << "\") failed: "
                << e.what();
        }

        std::chrono::zoned_time zt(tz, std::chrono::system_clock::now());
        if(tz_name == "UTC")
        {
            EXPECT_EQ(PAPILIO_NS format("{:%z}", zt), "+0000");
            EXPECT_EQ(PAPILIO_NS format("{:%Z}", zt), "UTC");
        }

        EXPECT_EQ(PAPILIO_NS format("{}", zt), PAPILIO_NS format("{:%F %T %Z}", zt));

        // Print platform-dependent result for visual check
        PAPILIO_NS println(
            std::cout,
            "Time zone: {0}, offset: {1:%z}\n"
            "sys_info of zoned time: {1}\n"
            "direct output: {2}",
            tz_name,
            zt.get_info(),
            zt
        );
    }

    {
        auto sys_now = std::chrono::system_clock::now();

        static_assert(formattable<std::chrono::sys_info>);
        PAPILIO_NS println(
            std::cout,
            "Current zone: {0:%z %Z}\n"
            "sys_info: {0}",
            std::chrono::current_zone()->get_info(sys_now)
        );
    }
}

#endif

TEST(chrono_formatter, misc)
{
    using namespace std::chrono_literals;
    using namespace papilio;

    // Plain text and special characters
    {
        EXPECT_EQ(PAPILIO_NS format("{:plain text}", 2024y), "plain text");
        EXPECT_EQ(PAPILIO_NS format("{:%%%t%n}", 2024y), "%\t\n");
    }

    // Fill and align
    {
        EXPECT_EQ(PAPILIO_NS format("{:*^14plain text}", 2024y), "**plain text**");
        EXPECT_EQ(PAPILIO_NS format("{:%^6==}", 2024y), "%%==%%");
    }

    // Error handling
    {
        EXPECT_THROW((void)PAPILIO_NS format("{:{{}", 2024y), format_error);
        EXPECT_THROW((void)PAPILIO_NS format("{:}}", 2024y), format_error);
    }
}
