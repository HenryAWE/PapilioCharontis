#include <gtest/gtest.h>
#include <papilio/papilio.hpp>


using namespace papilio;

TEST(TestUtility, Utility)
{
    EXPECT_TRUE(detailed::is_whitespace(' '));
    EXPECT_TRUE(detailed::is_whitespace(L' '));
    EXPECT_TRUE(detailed::is_whitespace(u' '));
    EXPECT_TRUE(detailed::is_whitespace(U' '));
    EXPECT_TRUE(detailed::is_whitespace(u8' '));

    EXPECT_TRUE(detailed::is_whitespace('\n'));
    EXPECT_TRUE(detailed::is_whitespace('\t'));
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
