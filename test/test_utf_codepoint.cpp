#include <gtest/gtest.h>
#include <papilio/utf/codepoint.hpp>

// codepoint should be POD
static_assert(std::is_trivial_v<papilio::utf::codepoint>);
static_assert(std::is_standard_layout_v<papilio::utf::codepoint>);

TEST(decoder, char8_t)
{
    using namespace papilio;
    using namespace utf;

    // Test data

    // "Ä"
    constexpr char8_t capital_a_with_diaeresis[] = u8"\u00c4";
    // CJK Unified Ideographs 4E00
    // "一"
    constexpr char8_t cjk_4e00[] = u8"\u4e00";
    // Peach Emoji
    // "🍑"
    constexpr char8_t peach_emoji[] = u8"\U0001f351";

    {
        static_assert(decoder<char8_t>::size_bytes(u8"A"[0]) == 1);
        static_assert(decoder<char8_t>::size_bytes(capital_a_with_diaeresis[0]) == 2);
        static_assert(decoder<char8_t>::size_bytes(cjk_4e00[0]) == 3);
        static_assert(decoder<char8_t>::size_bytes(peach_emoji[0]) == 4);
    }

    {
        codepoint cp;
        std::uint8_t size;

        std::tie(cp, size) = decoder<char8_t>::to_codepoint(u8"A");
        EXPECT_EQ(cp, U'A');
        EXPECT_EQ(size, cp.size_bytes());
    }
}

TEST(decoder, char16_t)
{
    using namespace papilio;
    using namespace utf;

    // Test data

    // "Ä"
    constexpr char16_t capital_a_with_diaeresis = u'\u00c4';
    // CJK Unified Ideographs 4E00
    // "一"
    constexpr char16_t cjk_4e00 = u'\u4e00';
    // Peach Emoji
    // "🍑"
    constexpr char16_t peach_emoji[] = u"\U0001f351";

    {
        codepoint cp;
        std::uint8_t processed_size;

        std::tie(cp, processed_size) = decoder<char16_t>::to_codepoint(u"A");
        EXPECT_EQ(cp, U'A');
        EXPECT_EQ(processed_size, 1);

        std::tie(cp, processed_size) = decoder<char16_t>::to_codepoint(capital_a_with_diaeresis);
        EXPECT_EQ(cp, U'\u00c4');
        EXPECT_EQ(processed_size, 1);

        std::tie(cp, processed_size) = decoder<char16_t>::to_codepoint(cjk_4e00);
        EXPECT_EQ(cp, U'\u4e00');
        EXPECT_EQ(processed_size, 1);

        std::tie(cp, processed_size) = decoder<char16_t>::to_codepoint(peach_emoji);
        EXPECT_EQ(cp, U'\U0001f351');
        EXPECT_EQ(processed_size, 2);
    }
}

TEST(decoder, char32_t)
{
    using namespace papilio;
    using namespace utf;

    // Test data

    // "Ä"
    constexpr char32_t capital_a_with_diaeresis = U'\u00c4';
    // CJK Unified Ideographs 4E00
    // "一"
    constexpr char32_t cjk_4e00 = U'\u4e00';
    // Peach Emoji
    // "🍑"
    constexpr char32_t peach_emoji = U'\U0001f351';

    {
        static_assert(decoder<char32_t>::size_bytes(U'A') == 1);
        static_assert(decoder<char32_t>::size_bytes(capital_a_with_diaeresis) == 2);
        static_assert(decoder<char32_t>::size_bytes(cjk_4e00) == 3);
        static_assert(decoder<char32_t>::size_bytes(peach_emoji) == 4);
    }

    {
        utf::codepoint cp;

        cp = decoder<char32_t>::to_codepoint(U'A').first;
        EXPECT_EQ(cp, U'A');
        EXPECT_EQ(cp.size_bytes(), 1);

        cp = decoder<char32_t>::to_codepoint(capital_a_with_diaeresis).first;
        EXPECT_EQ(cp, capital_a_with_diaeresis);
        EXPECT_EQ(cp.size_bytes(), 2);

        cp = decoder<char32_t>::to_codepoint(cjk_4e00).first;
        EXPECT_EQ(cp, cjk_4e00);
        EXPECT_EQ(cp.size_bytes(), 3);

        cp = decoder<char32_t>::to_codepoint(peach_emoji).first;
        EXPECT_EQ(cp, peach_emoji);
        EXPECT_EQ(cp.size_bytes(), 4);
    }
}

TEST(decoder, wchar_t)
{
    using namespace papilio;
    using namespace utf;

    // Test data

    // "Ä"
    constexpr wchar_t capital_a_with_diaeresis[] = L"\u00c4";
    // CJK Unified Ideographs 4E00
    // "一"
    constexpr wchar_t cjk_4e00[] = L"\u4e00";
    // Peach Emoji
    // "🍑"
    constexpr wchar_t peach_emoji[] = L"\U0001f351";

    {
        codepoint cp;
        std::uint8_t processed_size;

        std::tie(cp, processed_size) = decoder<wchar_t>::to_codepoint(L"A");
        EXPECT_EQ(cp, U'A');
        EXPECT_EQ(processed_size, 1);
        EXPECT_EQ(cp.size_bytes(), 1);

        std::tie(cp, processed_size) = decoder<wchar_t>::to_codepoint(capital_a_with_diaeresis);
        EXPECT_EQ(cp, U'\u00c4');
        EXPECT_EQ(processed_size, 1);
        EXPECT_EQ(cp.size_bytes(), 2);

        std::tie(cp, processed_size) = decoder<wchar_t>::to_codepoint(cjk_4e00);
        EXPECT_EQ(cp, U'\u4e00');
        EXPECT_EQ(processed_size, 1);
        EXPECT_EQ(cp.size_bytes(), 3);

        std::tie(cp, processed_size) = decoder<wchar_t>::to_codepoint(peach_emoji);
        EXPECT_EQ(cp, U'\U0001f351');
        EXPECT_EQ(processed_size, (sizeof(wchar_t) == sizeof(char32_t) ? 1 : 2));
        EXPECT_EQ(cp.size_bytes(), 4);
    }
}

TEST(codepoint, estimate_width)
{
    using namespace papilio;
    using namespace utf;

    {
        codepoint a = U'a'_cp;
        EXPECT_EQ(a.estimate_width(), 1);
    }

    {
        // CJK Unified Ideographs 6587
        // "文"
        codepoint cjk_6587 = U'\u6587'_cp;
        EXPECT_EQ(cjk_6587.estimate_width(), 2);
    }
}

TEST(codepoint, append_to)
{
    using namespace papilio;
    using namespace utf;

    {
        codepoint a = U'a'_cp;

        std::string result;
        a.append_to(result);
        EXPECT_EQ(result, "a");
    }

    {
        // CJK Unified Ideographs 6587
        // "文"
        codepoint cjk_6587 = U'\u6587'_cp;

        std::string result;
        cjk_6587.append_to(result);
        EXPECT_EQ(result, "\u6587");
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
