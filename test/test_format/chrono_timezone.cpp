// Split source files of chrono test to reduce size of the object file

#include <gtest/gtest.h>
#if __has_include(<format>)
#    include <format> // Test ADL-proof
#endif
#include <papilio/papilio.hpp>
#include <papilio/formatter/chrono.hpp>
#include <papilio_test/setup.hpp>

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
