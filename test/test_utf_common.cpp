#include <gtest/gtest.h>
#include <papilio/utf/common.hpp>


TEST(strlen, char8_t)
{
    using namespace papilio;
    using namespace utf;

    static_assert(utf::is_leading_byte('A'));
    static_assert(!utf::is_leading_byte(0x80));
    static_assert(!utf::is_trailing_byte('A'));
    static_assert(utf::is_trailing_byte(0x80));

    static_assert(utf::byte_count(u8"A"[0]) == 1);
    static_assert(utf::byte_count(u8"\u00c4"[0]) == 2);
    static_assert(utf::byte_count(u8"\u4e00"[0]) == 3);
    static_assert(utf::byte_count(u8"\U0001f351"[0]) == 4);

    static_assert(utf::strlen(u8"") == 0);
    static_assert(utf::strlen(u8"A") == 1);
    static_assert(utf::strlen(u8"\u00c4") == 1);
    static_assert(utf::strlen(u8"\u4e00") == 1);
    static_assert(utf::strlen(u8"\U0001f351") == 1);

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

TEST(index_offset, char8_t)
{
    using namespace papilio;
    using namespace std::literals;

    EXPECT_EQ(utf::index_offset(0, u8""sv), utf::npos);
    EXPECT_EQ(utf::index_offset(0, u8"A"sv), 0);
    EXPECT_EQ(utf::index_offset(1, u8"A"sv), utf::npos);
    EXPECT_EQ(utf::index_offset(1, u8"\u00c4A"sv), 2);
    EXPECT_EQ(utf::index_offset(1, u8"\U0001f351A"sv), 4);

    EXPECT_EQ(utf::index_offset(reverse_index, 0, u8""sv), utf::npos);
    EXPECT_EQ(utf::index_offset(reverse_index, 0, u8"A"sv), 0);
    EXPECT_EQ(utf::index_offset(reverse_index, 1, u8"A"sv), utf::npos);
    EXPECT_EQ(utf::index_offset(reverse_index, 1, u8"\u00c4A"sv), 0);
    EXPECT_EQ(utf::index_offset(reverse_index, 1, u8"\U0001f351A"sv), 0);
}

TEST(index_offset, char16_t)
{
    using namespace papilio;
    using namespace std::literals;

    EXPECT_EQ(utf::index_offset(0, u""sv), utf::npos);
    EXPECT_EQ(utf::index_offset(0, u"A"sv), 0);
    EXPECT_EQ(utf::index_offset(1, u"A"sv), utf::npos);
    EXPECT_EQ(utf::index_offset(1, u"\u00c4A"sv), 1);
    EXPECT_EQ(utf::index_offset(1, u"\U0001f351A"sv), 2);

    EXPECT_EQ(utf::index_offset(reverse_index, 0, u""sv), utf::npos);
    EXPECT_EQ(utf::index_offset(reverse_index, 0, u"A"sv), 0);
    EXPECT_EQ(utf::index_offset(reverse_index, 1, u"A"sv), utf::npos);
    EXPECT_EQ(utf::index_offset(reverse_index, 1, u"\u00c4A"sv), 0);
    EXPECT_EQ(utf::index_offset(reverse_index, 1, u"\U0001f351A"sv), 0);
}

TEST(index_offset, char32_t)
{
    using namespace papilio;
    using namespace std::literals;

    EXPECT_EQ(utf::index_offset(0, U""sv), utf::npos);
    EXPECT_EQ(utf::index_offset(0, U"A"sv), 0);
    EXPECT_EQ(utf::index_offset(1, U"A"sv), utf::npos);
    EXPECT_EQ(utf::index_offset(1, U"\u00c4A"sv), 1);
    EXPECT_EQ(utf::index_offset(1, U"\U0001f351A"sv), 1);

    EXPECT_EQ(utf::index_offset(reverse_index, 0, U""sv), utf::npos);
    EXPECT_EQ(utf::index_offset(reverse_index, 0, U"A"sv), 0);
    EXPECT_EQ(utf::index_offset(reverse_index, 1, U"A"sv), utf::npos);
    EXPECT_EQ(utf::index_offset(reverse_index, 1, U"\u00c4A"sv), 0);
    EXPECT_EQ(utf::index_offset(reverse_index, 1, U"\U0001f351A"sv), 0);
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
