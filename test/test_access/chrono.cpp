#include <gtest/gtest.h>
#include <papilio/accessor/chrono.hpp>
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
}
