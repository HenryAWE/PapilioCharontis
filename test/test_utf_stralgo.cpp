#include <gtest/gtest.h>
#include <papilio/utf/stralgo.hpp>
#include <papilio_test/setup.hpp>

#define PAPILIO_TEST_TMP "temp"

static_assert(PAPILIO_STRINGIZE(hello world) == std::string_view("hello world"));
static_assert(PAPILIO_STRINGIZE(PAPILIO_TEST_TMP) == std::string_view("PAPILIO_TEST_TMP"));
static_assert(PAPILIO_STRINGIZE_EX(PAPILIO_TEST_TMP) == std::string_view("\"temp\""));
static_assert(std::size(PAPILIO_TSTRING_ARRAY(wchar_t, "hello")) == 6);
static_assert(PAPILIO_TSTRING_VIEW(wchar_t, "hello") == L"hello");
static_assert(PAPILIO_TSTRING_VIEW(wchar_t, "hello") == PAPILIO_TSTRING_CSTR(wchar_t, "hello"));

TEST(Strlen, Char8)
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

    constexpr char8_t s[] = {'a', 0x80, 'b', 'c', '\0'};

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
        EXPECT_EQ(static_cast<int>(e.get()), 0x80);
    }
}

TEST(Strlen, InvalidBytes)
{
    using namespace papilio;
    using namespace utf;

    // 0xF8 - 0xFF are not valid UTF-8 lead bytes.
    // They are treated as single bytes instead of triggering undefined behavior.
    static_assert(utf::byte_count(0xF8u) == 1);
    static_assert(utf::byte_count(0xFBu) == 1);
    static_assert(utf::byte_count(0xFFu) == 1);

    constexpr char8_t s[] = {'a', static_cast<char8_t>(0xFF), 'b', '\0'};
    static_assert(utf::strlen(s) == 3);

    EXPECT_EQ(utf::strlen(std::u8string_view(s, 3)), 3);
}

TEST(IndexOffset, Char8)
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

TEST(IndexOffset, Char16)
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

TEST(IndexOffset, Char32)
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

    // Reverse indexing on an empty string or out-of-range indices
    EXPECT_EQ(utf::index_offset(reverse_index, 0, U""sv), utf::npos);
    EXPECT_EQ(utf::index_offset(reverse_index, 1, U""sv), utf::npos);
    EXPECT_EQ(utf::index_offset(reverse_index, 2, U"AB"sv), utf::npos);
    EXPECT_EQ(utf::index_offset(reverse_index, 1, U"AB"sv), 0);
}

TEST(Strlen, Char32Pointer)
{
    using namespace papilio;
    using namespace utf;

    // No NUL terminator within max_chars: returns max_chars
    EXPECT_EQ(utf::strlen(U"abc", 3), 3);
    EXPECT_EQ(utf::strlen(U"", 5), 0);
    // Stops at the NUL terminator
    EXPECT_EQ(utf::strlen(U"ab\0c", 4), 2);
}

TEST(Strlen, Char16Surrogates)
{
    using namespace papilio;
    using namespace utf;
    using enum strlen_behavior;

    // A valid surrogate pair is counted as one character
    constexpr char16_t ok[] = {u'A', static_cast<char16_t>(0xD800), static_cast<char16_t>(0xDC00), u'B', 0};
    EXPECT_EQ(utf::strlen<stop>(ok, 4), 3);
    EXPECT_EQ(utf::strlen<exception>(ok, 4), 3);

    // A high surrogate not followed by a low surrogate is invalid
    constexpr char16_t bad_high[] = {u'A', static_cast<char16_t>(0xD800), u'B', 0};
    EXPECT_EQ(utf::strlen<stop>(bad_high, 3), 1);
    EXPECT_THROW((void)utf::strlen<exception>(bad_high, 3), invalid_surrogate);

    // A lone low surrogate is invalid
    constexpr char16_t bad_low[] = {static_cast<char16_t>(0xDC00), 0};
    EXPECT_EQ(utf::strlen<stop>(bad_low, 1), 0);
    EXPECT_THROW((void)utf::strlen<exception>(bad_low, 1), invalid_surrogate);

    // A trailing high surrogate at the end of the string is invalid
    constexpr char16_t bad_trail[] = {u'A', static_cast<char16_t>(0xD800), 0};
    EXPECT_EQ(utf::strlen<stop>(bad_trail, 2), 1);
    EXPECT_THROW((void)utf::strlen<exception>(bad_trail, 2), invalid_surrogate);
}
