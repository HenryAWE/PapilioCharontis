#include <gtest/gtest.h>
#include <papilio/utf8.hpp>


TEST(TestUTF8, Decode)
{
    using namespace papilio;

    {
        auto [cp, len] = utf8::decode("A");
        EXPECT_EQ(cp, U'A');
        EXPECT_EQ(len, 1);
    }

    {
        auto [cp, len] = utf8::decode((const char*)u8"ü");
        EXPECT_EQ(cp, U'ü');
        EXPECT_EQ(len, 2);
    }

    {
        auto [cp, len] = utf8::decode((const char*)u8"我");
        EXPECT_EQ(cp, U'我');
        EXPECT_EQ(len, 3);
    }
}
TEST(TestUTF8, Utilities)
{
    using namespace papilio;

    EXPECT_EQ(utf8::strlen((const char*)u8"你好，世界"), 5);
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
