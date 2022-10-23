#include <gtest/gtest.h>
#include <papilio/papilio.hpp>
#include <papilio/util/chrono.hpp>


TEST(TestChrono, Tm)
{
    using namespace papilio;

    {
        std::tm t{};
        t.tm_year = 2022 - 1900;
        t.tm_mday = 1;
        t.tm_wday = 6;

        std::string result = papilio::format("{}", t);
        EXPECT_EQ(result, "Sat Jan  1 00:00:00 2022\n");
        result = papilio::format("{:%H:%M:%S}", t);
        EXPECT_EQ(result, "00:00:00");
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
