#include <iterator>
#include <gtest/gtest.h>
#include <papilio/papilio.hpp>


using namespace papilio;

TEST(TestFundamental, Utilities)
{
    EXPECT_EQ(format::detailed::ipow(10, 2), 100);
}
TEST(TestFundamental, VFormattedSize)
{
    EXPECT_EQ(format::vformatted_size(0, { .base = 16 }), 1);
    EXPECT_EQ(format::vformatted_size(0), 1);
    EXPECT_EQ(format::vformatted_size(0, { .base = 8 }), 1);
    EXPECT_EQ(format::vformatted_size(0, { .base = 2 }), 1);

    EXPECT_EQ(format::vformatted_size(1, { .base = 16 }), 1);
    EXPECT_EQ(format::vformatted_size(1), 1);
    EXPECT_EQ(format::vformatted_size(1, { .base = 8 }), 1);
    EXPECT_EQ(format::vformatted_size(1, { .base = 2 }), 1);

    EXPECT_EQ(format::vformatted_size(0x10, { .base = 16 }), 2);
    EXPECT_EQ(format::vformatted_size(10), 2);
    EXPECT_EQ(format::vformatted_size(010, { .base = 8 }), 2);
    EXPECT_EQ(format::vformatted_size(0b10, { .base = 2 }), 2);

    EXPECT_EQ(format::vformatted_size(-0x10, { .base = 16 }), 3);
    EXPECT_EQ(format::vformatted_size(-10), 3);
    EXPECT_EQ(format::vformatted_size(-010, { .base = 8 }), 3);
    EXPECT_EQ(format::vformatted_size(-0b10, { .base = 2 }), 3);

    EXPECT_EQ(format::vformatted_size("string"), std::size("string") - 1);
}
TEST(TestFundamental, VFormatTo)
{
    std::string str;
    format::vformat_to(std::back_inserter(str), 123);
    EXPECT_EQ(str, "123");
    str.clear();
    format::vformat_to(std::back_inserter(str), 0xfff, { .base = 16 });
    EXPECT_EQ(str, "fff");
    str.clear();
    format::vformat_to(std::back_inserter(str), 0b101, { .base = 2 });
    EXPECT_EQ(str, "101");
    EXPECT_EQ(str.size(), format::vformatted_size(0b101, { .base = 2 }));
    str.clear();
    format::vformat_to(std::back_inserter(str), -10);
    EXPECT_EQ(str, "-10");
    EXPECT_EQ(str.size(), format::vformatted_size(-10));

    str.clear();
    format::vformat_to(std::back_inserter(str), 1.23f);
    EXPECT_EQ(str, "1.23");
    str.clear();
    format::vformat_to(std::back_inserter(str), 1.003f);
    EXPECT_EQ(str, "1.003");

    std::u32string u32str;
    format::vformat_to(std::back_inserter(u32str), 123);
    EXPECT_EQ(u32str, U"123");
}


int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
