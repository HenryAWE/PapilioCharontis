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

    // reversely decode
    {
        auto [cp, len] = utf8::rdecode((const char*)u8"我们");
        EXPECT_EQ(cp, U'们');
        EXPECT_EQ(len, 3);
    }

    {
        std::string_view str = (const char*)u8"我们";
        auto [cp0, len0] = utf8::rdecode(str.data(), str.size());
        EXPECT_EQ(cp0, U'们');
        EXPECT_EQ(len0, 3);
    }
}
TEST(TestUTF8, Utilities)
{
    using namespace papilio;

    // length
    {
        EXPECT_EQ(utf8::strlen((const char*)u8"你好，世界"), 5);
    }
    
    // substring
    {
        std::string src = (const char*)u8"你好，世界！";
        EXPECT_EQ(utf8::substr(src, 1), (const char*)u8"好，世界！");
        EXPECT_EQ(utf8::substr(src, 3, 2), (const char*)u8"世界");
        EXPECT_EQ(utf8::substr(src, 0, 1), (const char*)u8"你");
    }

    // indexing
    {
        std::string src = (const char*)u8"你好，世界！";
        EXPECT_EQ(utf8::index(src, 0), (const char*)u8"你");
        EXPECT_EQ(utf8::index(src, 1), (const char*)u8"好");
        EXPECT_EQ(utf8::index(src, 2), (const char*)u8"，");
        EXPECT_EQ(utf8::index(src, 3), (const char*)u8"世");
        EXPECT_EQ(utf8::index(src, 4), (const char*)u8"界");
        EXPECT_EQ(utf8::index(src, 5), (const char*)u8"！");
        EXPECT_EQ(utf8::index(src, 6), std::string());
    }

    // reversely indexing
    {
        std::string src = (const char*)u8"你好，世界！";
        EXPECT_EQ(utf8::rindex(src, 0), (const char*)u8"！");
        EXPECT_EQ(utf8::rindex(src, 1), (const char*)u8"界");
        EXPECT_EQ(utf8::rindex(src, 2), (const char*)u8"世");
        EXPECT_EQ(utf8::rindex(src, 6), std::string());
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
