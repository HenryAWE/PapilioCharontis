// Split source files of chrono test to reduce size of the object file

#include <gtest/gtest.h>
#if __has_include(<format>)
#    include <format> // Test ADL-proof
#endif
#include <papilio/papilio.hpp>
#include <papilio/formatter/chrono.hpp>
#include <papilio_test/setup.hpp>

TEST(ChronoFormatter, Duration)
{
    using namespace std::chrono_literals;
    using namespace papilio;

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

TEST(ChronoFormatter, NegativeDuration)
{
    using namespace std::chrono_literals;
    using namespace papilio;

    // Negative durations produce sign-preserving time-of-day fields
    EXPECT_EQ(PAPILIO_NS format("{:%T}", -1s), "-00:00:01");
    EXPECT_EQ(PAPILIO_NS format("{:%T}", -90min), "-01:30:00");
    EXPECT_EQ(PAPILIO_NS format("{:%T}", -25h), "-01:00:00");
    EXPECT_EQ(PAPILIO_NS format("{:%R}", -1s), "-00:00");
    EXPECT_EQ(PAPILIO_NS format("{:%H}", -1s), "-00");
    EXPECT_EQ(PAPILIO_NS format("{:%M}", -1s), "-00");
    EXPECT_EQ(PAPILIO_NS format("{:%S}", -1s), "-01");
    EXPECT_EQ(PAPILIO_NS format("{:%S}", -1h), "-00");

    EXPECT_EQ(PAPILIO_NS format(L"{:%T}", -1s), L"-00:00:01");
    EXPECT_EQ(PAPILIO_NS format(L"{:%S}", -1s), L"-01");

    // Positive durations are unchanged
    EXPECT_EQ(PAPILIO_NS format("{:%T}", 3661s), "01:01:01");
    EXPECT_EQ(PAPILIO_NS format("{:%H}", 1s), "00");
    EXPECT_EQ(PAPILIO_NS format("{:%S}", 90s), "30");

    // The default format still shows the signed count
    EXPECT_EQ(PAPILIO_NS format("{}", -1s), "-1s");
    EXPECT_EQ(PAPILIO_NS format("{}", -90min), "-90min");
}
