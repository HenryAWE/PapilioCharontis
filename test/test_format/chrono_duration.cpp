// Split source files of chrono test to reduce size of the object file

#include <gtest/gtest.h>
#if __has_include(<format>)
#    include <format> // Test ADL-proof
#endif
#include <papilio/papilio.hpp>
#include <papilio/formatter/chrono.hpp>
#include <papilio_test/setup.hpp>


TEST(chrono_formatter, duration)
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
