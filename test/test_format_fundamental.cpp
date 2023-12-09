#include <gtest/gtest.h>
#include <papilio/format.hpp>

TEST(format_fundamental, int)
{
    using namespace papilio;

    static_assert(PAPILIO_NS formattable<int>);

    EXPECT_EQ(PAPILIO_NS format("{}", 182376), "182376");
    EXPECT_EQ(PAPILIO_NS format("{:<8}", 182376), "182376  ");
    EXPECT_EQ(PAPILIO_NS format("{:>8}", 182376), "  182376");
    EXPECT_EQ(PAPILIO_NS format("{:^8}", 182376), " 182376 ");

    EXPECT_EQ(PAPILIO_NS format("{:*<8}", 182376), "182376**");
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
