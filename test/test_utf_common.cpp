#include <gtest/gtest.h>
#include <vector>
#include <papilio/utf/common.hpp>


TEST(TestUTFCommon, Common)
{
    using namespace papilio;
    using namespace utf;

    {
        static_assert(utf::is_leading_byte('A'));
        static_assert(!utf::is_leading_byte(0x80));
        static_assert(!utf::is_trailing_byte('A'));
        static_assert(utf::is_trailing_byte(0x80));

        static_assert(utf::byte_count(u8"a"[0]) == 1);
        static_assert(utf::byte_count(u8"ä"[0]) == 2);
        static_assert(utf::byte_count(u8"我"[0]) == 3);
        static_assert(utf::byte_count(u8"🔊"[0]) == 4);
    }

    {
        static_assert(utf::strlen(u8"") == 0);
        static_assert(utf::strlen(u8"A") == 1);
        static_assert(utf::strlen(u8"Ä") == 1);
        static_assert(utf::strlen(u8"我") == 1);
        static_assert(utf::strlen(u8"🔊") == 1);
    }

    {
        using enum strlen_behavior;

        constexpr char8_t s[] = { 'a', 0x80, 'b', 'c', '\0' };

        static_assert(utf::strlen<replace>(s) == 4);
        EXPECT_EQ(utf::strlen<ignore>(s), 3);
        EXPECT_EQ(utf::strlen<stop>(s), 1);

        try
        {
            (void)utf::strlen<exception>(s);
            FAIL() << "unreachable";
        }
        catch(const utf::invalid_byte& e)
        {
            EXPECT_STREQ(e.what(), "invalid byte");
            EXPECT_EQ((int)e.get(), 0x80);
        }
    }

    {
        using namespace std::literals;

        EXPECT_EQ(utf::index_offset(0, u8""sv), utf::npos);
        EXPECT_EQ(utf::index_offset(0, u8"A"sv), 0);
        EXPECT_EQ(utf::index_offset(1, u8"A"sv), utf::npos);
        EXPECT_EQ(utf::index_offset(1, u8"ÄA"sv), 2);
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
