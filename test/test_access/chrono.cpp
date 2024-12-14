#include <gtest/gtest.h>
#include <papilio/accessor/chrono.hpp>
#include <papilio/formatter/chrono.hpp>
#include <papilio/format.hpp>
#include <papilio_test/setup.hpp>

namespace test_access
{
static std::tm create_tm_epoch()
{
    // The Unix epoch (January 1, 1970)
    const std::time_t t = 0;
    std::tm result{};
#ifdef PAPILIO_STDLIB_MSVC_STL
    gmtime_s(&result, &t);
#else
    result = *std::gmtime(&t);
#endif

    return result;
}
} // namespace test_access

TEST(accessor, chrono)
{
    using namespace papilio;
    using namespace std::chrono_literals;

    std::tm val = test_access::create_tm_epoch();

    EXPECT_EQ(
        PAPILIO_NS format("{.year}", val),
        PAPILIO_NS format("{}", val.tm_year + 1900)
    );
    EXPECT_EQ(
        PAPILIO_NS format(L"{.year}", val),
        PAPILIO_NS format(L"{}", val.tm_year + 1900)
    );

    EXPECT_EQ(PAPILIO_NS format("{.is_dst}", val), "false");
    EXPECT_EQ(PAPILIO_NS format(L"{.is_dst}", val), L"false");

    EXPECT_EQ(PAPILIO_NS format("{.year}", 2023y / 11), "2023");
    EXPECT_EQ(PAPILIO_NS format("{.month}", 2023y / 11), "Nov");

    EXPECT_EQ(PAPILIO_NS format("{.month}", 11 / 8d), "Nov");
    EXPECT_EQ(PAPILIO_NS format("{.day}", 11 / 8d), "08");

    EXPECT_EQ(PAPILIO_NS format("{.ok}", 2023y / 11 / 8), "true");
    EXPECT_EQ(PAPILIO_NS format("{.year}", 2023y / 11 / 8), "2023");
    EXPECT_EQ(PAPILIO_NS format("{.month}", 2023y / 11 / 8), "Nov");
    EXPECT_EQ(PAPILIO_NS format("{.day}", 2023y / 11 / 8), "08");
    EXPECT_EQ(PAPILIO_NS format("{.weekday}", 2023y / 11 / 8), "Wed");

    EXPECT_EQ(PAPILIO_NS format("{.ok}", 2023y / 11 / 31), "false");

    {
        std::chrono::hh_mm_ss<std::chrono::seconds> hms{3600s + 60s + 1s};

        EXPECT_EQ(PAPILIO_NS format("{.ok}", hms), "true");
        EXPECT_EQ(PAPILIO_NS format("{.hour}", hms), "1h");
        EXPECT_EQ(PAPILIO_NS format("{.minute}", hms), "1min");
        EXPECT_EQ(PAPILIO_NS format("{.second}", hms), "1s");
    }
}
