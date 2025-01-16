#include <gtest/gtest.h>
#include <papilio/utf/codepoint.hpp>
#include <sstream>
#include <papilio_test/setup.hpp>

// codepoint should be POD
static_assert(std::is_trivial_v<papilio::utf::codepoint>);
static_assert(std::is_standard_layout_v<papilio::utf::codepoint>);

namespace test_utf_codepoint
{
// Test data for codepoint
//
// It includes following Unicode codepoints:
// 1. A           : captial a.
// 2. A_diaeresis : 'Ä', captial a with diaeresis.
// 3. cjk_4e00    : '一', CJK Unified Ideographs 4E00.
// 4. peach_emoji : '🍑', peach emoji.
template <typename CharT>
struct decoder_test_data;

#define TEST_UTF_CODEPOINT_DEFINE_TEST_DATA_CHAR8(char_t, prefix) \
    static constexpr char_t A = prefix##'A';                      \
    static constexpr char_t A_diaeresis[] = prefix##"\u00c4";     \
    static constexpr char_t cjk_4e00[] = prefix##"\u4e00";        \
    static constexpr char_t peach_emoji[] = prefix##"\U0001f351"

#define TEST_UTF_CODEPOINT_DEFINE_TEST_DATA_CHAR16(char_t, prefix) \
    static constexpr char_t A = prefix##'A';                       \
    static constexpr char_t A_diaeresis = prefix##'\u00c4';        \
    static constexpr char_t cjk_4e00 = prefix##'\u4e00';           \
    static constexpr char_t peach_emoji[] = prefix##"\U0001f351"

#define TEST_UTF_CODEPOINT_DEFINE_TEST_DATA_CHAR32(char_t, prefix) \
    static constexpr char_t A = prefix##'A';                       \
    static constexpr char_t A_diaeresis = prefix##'\u00c4';        \
    static constexpr char_t cjk_4e00 = prefix##'\u4e00';           \
    static constexpr char_t peach_emoji = prefix##'\U0001f351'

template <>
struct decoder_test_data<char>
{
    TEST_UTF_CODEPOINT_DEFINE_TEST_DATA_CHAR8(char, );
};

template <>
struct decoder_test_data<char8_t>
{
    TEST_UTF_CODEPOINT_DEFINE_TEST_DATA_CHAR8(char8_t, u8);
};

template <>
struct decoder_test_data<char16_t>
{
    TEST_UTF_CODEPOINT_DEFINE_TEST_DATA_CHAR16(char16_t, u);
};

template <>
struct decoder_test_data<char32_t>
{
    TEST_UTF_CODEPOINT_DEFINE_TEST_DATA_CHAR32(char32_t, U);
};

template <>
struct decoder_test_data<wchar_t>
{
#ifdef PAPILIO_PLATFORM_WINDOWS
    TEST_UTF_CODEPOINT_DEFINE_TEST_DATA_CHAR16(wchar_t, L);
#else
    TEST_UTF_CODEPOINT_DEFINE_TEST_DATA_CHAR32(wchar_t, L);
#endif
};

#undef TEST_UTF_CODEPOINT_DEFINE_TEST_DATA_CHAR8
#undef TEST_UTF_CODEPOINT_DEFINE_TEST_DATA_CHAR16
#undef TEST_UTF_CODEPOINT_DEFINE_TEST_DATA_CHAR32
} // namespace test_utf_codepoint

template <typename CharT>
class decoder_suite : public ::testing::Test
{
public:
    using test_data = test_utf_codepoint::decoder_test_data<CharT>;
    using decoder_t = papilio::utf::decoder<CharT>;
};

template <typename CharT>
class codepoint_suite : public ::testing::Test
{};

using char_types = ::testing::Types<char, wchar_t, char16_t, char32_t, char8_t>;
TYPED_TEST_SUITE(decoder_suite, char_types);
TYPED_TEST_SUITE(codepoint_suite, char_types);

TYPED_TEST(decoder_suite, size_bytes)
{
    using namespace papilio;
    using test_data = typename TestFixture::test_data;
    using decoder_t = typename TestFixture::decoder_t;

    if constexpr(char8_like<TypeParam>)
    {
        EXPECT_EQ(decoder_t::size_bytes(test_data::A), 1);
        EXPECT_EQ(decoder_t::size_bytes(test_data::A_diaeresis[0]), 2);
        EXPECT_EQ(decoder_t::size_bytes(test_data::cjk_4e00[0]), 3);
        EXPECT_EQ(decoder_t::size_bytes(test_data::peach_emoji[0]), 4);
    }
    else if constexpr(std::same_as<TypeParam, char32_t>)
    {
        EXPECT_EQ(decoder_t::size_bytes(test_data::A), 1);
        EXPECT_EQ(decoder_t::size_bytes(test_data::A_diaeresis), 2);
        EXPECT_EQ(decoder_t::size_bytes(test_data::cjk_4e00), 3);
        EXPECT_EQ(decoder_t::size_bytes(test_data::peach_emoji), 4);
    }
    else
    {
        // TODO
    }
}

TYPED_TEST(decoder_suite, to_codepoint)
{
    using namespace papilio;
    using decoder_t = typename TestFixture::decoder_t;

    utf::codepoint cp;
    std::uint8_t processed_size;

    std::tie(cp, processed_size) = decoder_t::to_codepoint(
        PAPILIO_TSTRING_CSTR(TypeParam, "A")
    );
    EXPECT_EQ(cp, U'A');
    EXPECT_EQ(cp.size_bytes(), 1);
    EXPECT_EQ(processed_size, 1);

    std::tie(cp, processed_size) = decoder_t::to_codepoint(
        PAPILIO_TSTRING_CSTR(TypeParam, "\u00c4")
    );
    EXPECT_EQ(cp, U'\u00c4');
    EXPECT_EQ(cp.size_bytes(), 2);
    if constexpr(char8_like<TypeParam>)
        EXPECT_EQ(processed_size, 2);
    else
        EXPECT_EQ(processed_size, 1);

    std::tie(cp, processed_size) = decoder_t::to_codepoint(
        PAPILIO_TSTRING_CSTR(TypeParam, "\u4e00")
    );
    EXPECT_EQ(cp, U'\u4e00');
    EXPECT_EQ(cp.size_bytes(), 3);
    if constexpr(char8_like<TypeParam>)
        EXPECT_EQ(processed_size, 3);
    else
        EXPECT_EQ(processed_size, 1);

    std::tie(cp, processed_size) = decoder_t::to_codepoint(
        PAPILIO_TSTRING_CSTR(TypeParam, "\U0001f351")
    );
    EXPECT_EQ(cp, U'\U0001f351');
    if constexpr(char8_like<TypeParam>)
        EXPECT_EQ(processed_size, 4);
    else if constexpr(char16_like<TypeParam>)
        EXPECT_EQ(processed_size, 2);
    else
        EXPECT_EQ(processed_size, 1);
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

TYPED_TEST(codepoint_suite, ostream)
{
    using namespace papilio;
    using namespace utf;

    const codepoint a = U'a'_cp;
    constexpr TypeParam expected_str[2]{TypeParam('a'), TypeParam('\0')};

    std::basic_stringstream<TypeParam> ss;
    ss << a;

    EXPECT_EQ(ss.str(), expected_str);
}

TYPED_TEST(codepoint_suite, append_to)
{
    using namespace papilio;
    using namespace utf;

    {
        codepoint a = U'a'_cp;
        constexpr TypeParam expected_str[2]{TypeParam('a'), TypeParam('\0')};

        std::basic_string<TypeParam> result;
        a.append_to(result);
        EXPECT_EQ(result, expected_str);

        std::basic_stringstream<TypeParam> ss;
        a.append_to(ss);
        EXPECT_EQ(ss.str(), expected_str);
    }

    {
        // CJK Unified Ideographs 6587
        // "文"
        codepoint cjk_6587 = U'\u6587'_cp;
        constexpr auto expected_str = PAPILIO_TSTRING_ARRAY(TypeParam, "\u6587");

        std::basic_string<TypeParam> result;
        cjk_6587.append_to(result);
        EXPECT_EQ(result, expected_str);
    }
}

TYPED_TEST(codepoint_suite, iterator)
{
    using namespace papilio;
    using namespace utf;

    std::basic_string_view<TypeParam> str = PAPILIO_TSTRING_VIEW(TypeParam, "hello");

    auto start = codepoint_begin(str);
    auto stop = codepoint_end(str);

    EXPECT_EQ(start.base(), str.data());
    EXPECT_EQ(stop.base(), std::to_address(str.end()));

    EXPECT_EQ(*start, U'h');

    {
        auto it = std::prev(stop);
        EXPECT_EQ(start + 4, it);
        EXPECT_EQ(4 + start, it);
        EXPECT_EQ(start, it - 4);

        EXPECT_EQ(start - it, -4);
        EXPECT_EQ(it - start, 4);

        EXPECT_EQ(*it, U'o');
    }

    EXPECT_LT(start, stop);
    EXPECT_GT(stop, start);
    EXPECT_EQ(std::distance(start, stop), 5);
    EXPECT_EQ(stop - start, 5);
}

TYPED_TEST(codepoint_suite, iterator_swap)
{
    using namespace papilio;

    auto str = PAPILIO_TSTRING_VIEW(TypeParam, "swap");

    auto a = utf::codepoint_begin(str);
    auto b = std::prev(utf::codepoint_end(str));

    EXPECT_EQ(*a, U's');
    EXPECT_EQ(*b, U'p');

    swap(a, b);

    EXPECT_EQ(*a, U'p');
    EXPECT_EQ(*b, U's');
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
