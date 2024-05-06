#include <gtest/gtest.h>
#include <papilio/utf/string.hpp>
#include <sstream>

// Peach Emoji, CJK Unified Ideographs 4E00, Capital A with Diaeresis, A
// "🍑一ÄA"
#define PAPILIO_TEST_UTF_STRING_TEST_DATA(prefix, suffix) prefix##"\U0001f351\u4e00\u00c4A"##suffix

namespace test_utf_string
{
static_assert(papilio::string_like<papilio::utf::string_ref>);
static_assert(papilio::string_like<papilio::utf::string_container>);

template <typename CharT>
void test_string_ref_index(papilio::utf::basic_string_ref<CharT> ref)
{
    using namespace papilio;

    EXPECT_EQ(ref[0], U'\U0001f351');
    EXPECT_EQ(ref[1], U'\u4e00');
    EXPECT_EQ(ref[2], U'\u00c4');
    EXPECT_EQ(ref[3], U'A');

    EXPECT_EQ(ref.index(0), U'\U0001f351');
    EXPECT_EQ(ref.index(1), U'\u4e00');
    EXPECT_EQ(ref.index(2), U'\u00c4');
    EXPECT_EQ(ref.index(3), U'A');

    EXPECT_EQ(ref.index(reverse_index, 3), U'\U0001f351');
    EXPECT_EQ(ref.index(reverse_index, 2), U'\u4e00');
    EXPECT_EQ(ref.index(reverse_index, 1), U'\u00c4');
    EXPECT_EQ(ref.index(reverse_index, 0), U'A');

    EXPECT_EQ(ref.at(0), U'\U0001f351');
    EXPECT_EQ(ref.at(1), U'\u4e00');
    EXPECT_EQ(ref.at(2), U'\u00c4');
    EXPECT_EQ(ref.at(3), U'A');

    EXPECT_EQ(ref.at(reverse_index, 3), U'\U0001f351');
    EXPECT_EQ(ref.at(reverse_index, 2), U'\u4e00');
    EXPECT_EQ(ref.at(reverse_index, 1), U'\u00c4');
    EXPECT_EQ(ref.at(reverse_index, 0), U'A');

    EXPECT_EQ(ref.index_or(0, utf::codepoint()), U'\U0001f351');
    EXPECT_EQ(ref.index_or(1, utf::codepoint()), U'\u4e00');
    EXPECT_EQ(ref.index_or(2, utf::codepoint()), U'\u00c4');
    EXPECT_EQ(ref.index_or(3, utf::codepoint()), U'A');

    EXPECT_THROW((void)ref.at(4), std::out_of_range);
    EXPECT_THROW((void)ref.at(reverse_index, 4), std::out_of_range);
    EXPECT_THROW((void)ref.at(std::size_t(-1)), std::out_of_range);
    EXPECT_THROW((void)ref.at(reverse_index, std::size_t(-1)), std::out_of_range);

    EXPECT_EQ(ref.index_or(4, utf::codepoint(U'?')), U'?');
    EXPECT_EQ(ref.index_or(reverse_index, 4, utf::codepoint(U'?')), U'?');
    EXPECT_EQ(ref.index_or(std::size_t(-1), utf::codepoint(U'?')), U'?');
    EXPECT_EQ(ref.index_or(reverse_index, std::size_t(-1), utf::codepoint(U'?')), U'?');
}

template <typename CharT>
void test_string_ref_interoperability(papilio::utf::basic_string_ref<CharT> ref)
{
    // GoogleTest doesn't support char8_t
    EXPECT_TRUE(ref.to_u8string() == PAPILIO_TEST_UTF_STRING_TEST_DATA(u8, ));
    EXPECT_EQ(ref.to_u16string(), PAPILIO_TEST_UTF_STRING_TEST_DATA(u, ));
    EXPECT_EQ(ref.to_u32string(), PAPILIO_TEST_UTF_STRING_TEST_DATA(U, ));
    EXPECT_EQ(ref.to_wstring(), PAPILIO_TEST_UTF_STRING_TEST_DATA(L, ));

    EXPECT_EQ(ref, PAPILIO_TEST_UTF_STRING_TEST_DATA(, ));
    EXPECT_EQ(PAPILIO_TEST_UTF_STRING_TEST_DATA(, ), ref);
    EXPECT_EQ(ref, PAPILIO_TEST_UTF_STRING_TEST_DATA(u, ));
    EXPECT_EQ(PAPILIO_TEST_UTF_STRING_TEST_DATA(u, ), ref);
    EXPECT_EQ(ref, PAPILIO_TEST_UTF_STRING_TEST_DATA(U, ));
    EXPECT_EQ(PAPILIO_TEST_UTF_STRING_TEST_DATA(U, ), ref);
    EXPECT_EQ(ref, PAPILIO_TEST_UTF_STRING_TEST_DATA(L, ));
    EXPECT_EQ(PAPILIO_TEST_UTF_STRING_TEST_DATA(L, ), ref);
}
} // namespace test_utf_string

TEST(basic_string_ref, u8string_ref)
{
    using namespace papilio;
    using namespace utf;

    {
        u8string_ref ref = PAPILIO_TEST_UTF_STRING_TEST_DATA(u8, _sr);
        EXPECT_EQ(ref.size(), 10);
        EXPECT_EQ(ref.length(), 4);

        test_utf_string::test_string_ref_index(ref);

        test_utf_string::test_string_ref_interoperability(ref);

        EXPECT_EQ(ref.substr(0, 2), u8"\U0001f351\u4e00");
        EXPECT_TRUE(ref.substr(4, 2).empty());
        EXPECT_THROW((void)ref.substr(5, 2), std::out_of_range);
        EXPECT_TRUE(ref.substr<substr_behavior::empty_string>(5, 2).empty());
    }

    {
        u8string_ref ref = u8"\U0001f351\u4e00\u00c4A";
        ref.remove_prefix(2);
        EXPECT_EQ(ref, u8"\u00c4A");
    }

    {
        u8string_ref ref = u8"\U0001f351\u4e00\u00c4A";
        ref.remove_suffix(2);
        EXPECT_EQ(ref, u8"\U0001f351\u4e00");
    }

    {
        u8string_ref ref = u8"BCD";
        EXPECT_EQ(ref.compare(u8"BCD"), 0);

        EXPECT_GT(ref.compare(u8"BC"), 0);
        EXPECT_LT(ref.compare(u8"BCDE"), 0);

        EXPECT_GT(ref.compare(u8"ABC"), 0);
        EXPECT_LT(ref.compare(u8"EFG"), 0);

        EXPECT_GT(ref, u8"BC");
        EXPECT_LT(ref, u8"BCDE");

        EXPECT_GT(ref, u8"ABC");
        EXPECT_LT(ref, u8"EFG");
    }

    {
        using namespace utf::literals;

        u8string_ref ref = u8"ABCDEFG ABC";

        EXPECT_TRUE(ref.starts_with(u8"ABC"));
        EXPECT_TRUE(ref.starts_with(u8"ABCDEFG ABC"));
        EXPECT_FALSE(ref.starts_with(u8"BCD"));
        EXPECT_FALSE(ref.starts_with(u8"ABCDEFG ABCD"));
        EXPECT_TRUE(ref.starts_with(U'A'));
        EXPECT_FALSE(ref.starts_with(U'B'));

        EXPECT_TRUE(ref.ends_with(u8"ABC"));
        EXPECT_TRUE(ref.ends_with(u8"ABCDEFG ABC"));
        EXPECT_FALSE(ref.ends_with(u8"BCD"));
        EXPECT_FALSE(ref.ends_with(u8"ABCDEFG ABCD"));
        EXPECT_TRUE(ref.ends_with(U'C'));
        EXPECT_FALSE(ref.ends_with(U'D'));

        auto it = ref.find(u8"ABC");

        static_assert(std::bidirectional_iterator<decltype(it)>);
        static_assert(std::is_convertible_v<
                      std::iterator_traits<decltype(it)>::iterator_category,
                      std::bidirectional_iterator_tag>);

        EXPECT_EQ(it, ref.begin());
        it = ref.find(u8"BCD");
        EXPECT_EQ(it, ref.begin() + 1);
        EXPECT_EQ(it, std::next(ref.begin(), 1));
        it = ref.find(u8"ABC", 1);
        EXPECT_EQ(it, ref.end() - 3);
        EXPECT_EQ(it, std::prev(ref.end(), 3));
    }

    {
        string_ref ref = "123456";

        ref.remove_prefix(2);
        EXPECT_EQ(ref, "3456");
        ref.remove_suffix(2);
        EXPECT_EQ(ref, "34");
    }
}

TEST(basic_string_ref, u16string_ref)
{
    using namespace papilio;
    using namespace utf;

    {
        u16string_ref ref = PAPILIO_TEST_UTF_STRING_TEST_DATA(u, _sr);
        EXPECT_EQ(ref.size(), 5);
        EXPECT_EQ(ref.length(), 4);

        test_utf_string::test_string_ref_index(ref);

        test_utf_string::test_string_ref_interoperability(ref);

        auto it = ref.find(U'\u4e00');
        EXPECT_EQ(it, ref.begin() + 1);
        EXPECT_EQ(it, std::next(ref.begin(), 1));
        EXPECT_EQ(it, ref.end() - 3);
        EXPECT_EQ(it, std::prev(ref.end(), 3));
    }
}

TEST(basic_string_ref, u32string_ref)
{
    using namespace papilio;
    using namespace utf;

    {
        u32string_ref ref = PAPILIO_TEST_UTF_STRING_TEST_DATA(U, _sr);
        EXPECT_EQ(ref.size(), 4);
        EXPECT_EQ(ref.length(), 4);

        test_utf_string::test_string_ref_index(ref);

        test_utf_string::test_string_ref_interoperability(ref);

        auto it = ref.find(U'\u4e00');
        EXPECT_EQ(it, ref.begin() + 1);
        EXPECT_EQ(it, std::next(ref.begin(), 1));
        EXPECT_EQ(it, ref.end() - 3);
        EXPECT_EQ(it, std::prev(ref.end(), 3));
    }
}

TEST(basic_string_ref, wstring_ref)
{
    using namespace papilio;
    using namespace utf;

    {
        wstring_ref ref = PAPILIO_TEST_UTF_STRING_TEST_DATA(L, _sr);
        if constexpr(sizeof(wchar_t) == 2)
            EXPECT_EQ(ref.size(), 5);
        else
            EXPECT_EQ(ref.size(), 4);
        EXPECT_EQ(ref.length(), 4);

        test_utf_string::test_string_ref_index(ref);

        test_utf_string::test_string_ref_interoperability(ref);

        auto it = ref.find(U'\u4e00');
        EXPECT_EQ(it, ref.begin() + 1);
        EXPECT_EQ(it, std::next(ref.begin(), 1));
        EXPECT_EQ(it, ref.end() - 3);
        EXPECT_EQ(it, std::prev(ref.end(), 3));
    }
}

TEST(basic_string_ref, substr_slice)
{
    using namespace papilio;
    using namespace utf;

    {
        string_ref src = "hello world!";

        EXPECT_EQ(src.substr(slice(0)), "hello world!");
        EXPECT_EQ(src.substr(slice(0, 1)), "h");
        EXPECT_EQ(src.substr(slice(-1)), "!");
        EXPECT_EQ(src.substr(slice(0, 5)), "hello");
        EXPECT_EQ(src.substr(slice(-6)), "world!");
        EXPECT_EQ(src.substr(slice(-6, -1)), "world");
        EXPECT_EQ(src.substr(slice(6, -1)), "world");

        EXPECT_THROW((void)src.substr(slice(13)), std::out_of_range);
        EXPECT_THROW((void)src.substr(slice(-14)), std::out_of_range);

        EXPECT_EQ(src.substr<substr_behavior::empty_string>(slice(6, 15)), "world!");
        EXPECT_EQ(src.substr<substr_behavior::empty_string>(slice(-15, 5)), "hello");
        EXPECT_TRUE(src.substr<substr_behavior::empty_string>(slice(13)).empty());

        EXPECT_TRUE(src.substr(slice(0, 0)).empty());
        EXPECT_TRUE(src.substr(slice(2, 1)).empty());
        EXPECT_TRUE(src.substr(slice(-1, -1)).empty());
        EXPECT_TRUE(src.substr(slice(-1, -2)).empty());
        EXPECT_TRUE(src.substr(slice(-5, 5)).empty());
    }
}

TEST(basic_string_container, string_container)
{
    using namespace papilio;
    using namespace utf;

    {
        string_container sc = "test";

        EXPECT_FALSE(sc.has_ownership());
        EXPECT_EQ(sc.size(), 4);
        EXPECT_EQ(sc.length(), 4);
        EXPECT_TRUE(sc.null_terminated());

        EXPECT_EQ(sc[0], U't');
        EXPECT_EQ(sc[1], U'e');
        EXPECT_EQ(sc[2], U's');
        EXPECT_EQ(sc[3], U't');

#ifdef PAPILIO_HAS_MULTIDIMENSIONAL_SUBSCRIPT

        EXPECT_EQ((sc[reverse_index, 0]), U't');
        EXPECT_EQ((sc[reverse_index, 1]), U's');
        EXPECT_EQ((sc[reverse_index, 2]), U'e');
        EXPECT_EQ((sc[reverse_index, 3]), U't');

#endif

        EXPECT_EQ(sc.front(), U't');
        EXPECT_EQ(sc.back(), U't');

        EXPECT_EQ(sc.index_or(4, U'\0'), U'\0');
        EXPECT_EQ(sc.index_or(reverse_index, 4, U'\0'), U'\0');
        EXPECT_THROW((void)sc.at(4), std::out_of_range);

        string_container sc_2 = sc;
        EXPECT_FALSE(sc_2.has_ownership());

        sc_2.obtain_ownership();
        EXPECT_TRUE(sc_2.has_ownership());
    }

    {
        using namespace std::literals;

        string_container sc = "hello world"s;
        EXPECT_TRUE(sc.has_ownership());
        EXPECT_EQ(sc.find("hello"), sc.begin());
        EXPECT_EQ(sc.find("world"), sc.begin() + 6);
        EXPECT_TRUE(sc.contains("hello"));
        EXPECT_TRUE(sc.contains("world"));

        auto sc_1 = sc.substr(0, 5);
        EXPECT_FALSE(sc_1.has_ownership());
        EXPECT_EQ(sc_1.length(), 5);
        EXPECT_TRUE(sc_1.contains("hello"));
        EXPECT_FALSE(sc_1.contains("world"));
    }

    {
        using namespace utf::literals;

        EXPECT_EQ("test"_sc.size(), 4);
        EXPECT_EQ(u8"test"_sc.size(), 4);
    }

    {
        using namespace utf::literals;

        string_ref ref = "test"_sr;
        string_container sc(ref.begin(), ref.end());

        EXPECT_EQ(sc, "test");
        EXPECT_EQ(sc, ref);
    }

    {
        using namespace utf::literals;

        string_container sc = "test"_sc;
        string_ref ref(sc.begin(), sc.end());

        EXPECT_EQ(ref, "test");
        EXPECT_EQ(ref, sc);
    }
}

TEST(basic_string_container, wstring_container)
{
    using namespace papilio;
    using namespace utf;

    {
        wstring_container sc = L"test";

        EXPECT_EQ(sc[0], U't');
        EXPECT_EQ(sc[1], U'e');
        EXPECT_EQ(sc[2], U's');
        EXPECT_EQ(sc[3], U't');
    }
}

TEST(basic_string_container, push_back)
{
    using namespace papilio;
    using namespace utf;

    // Fullwidth Exclamation Mark
    // '！'
    const codepoint fullwidth_exclamation = U'\uff01'_cp;

    {
        string_container sc = "hello";
        EXPECT_FALSE(sc.has_ownership());

        sc.push_back('!');
        ASSERT_TRUE(sc.has_ownership());
        EXPECT_EQ(sc, "hello!");

        sc.push_back('!');
        EXPECT_EQ(sc, "hello!!");
    }

    {
        string_container sc = "hello";

        sc.push_back(fullwidth_exclamation);
        EXPECT_EQ(sc, "hello\uff01");
    }

    {
        wstring_container sc = L"hello";

        sc.push_back(L'!');
        ASSERT_TRUE(sc.has_ownership());
        EXPECT_EQ(sc, L"hello!");
    }

    {
        u32string_container sc = U"hello";

        sc.push_back(U'!');
        ASSERT_TRUE(sc.has_ownership());
        EXPECT_EQ(sc, U"hello!");
    }
}

TEST(basic_string_container, istream)
{
    using namespace papilio;
    using namespace utf;

    {
        std::stringstream ss("test");

        string_container sc;
        ss >> sc;
        EXPECT_EQ(sc, "test");
    }

    {
        std::wstringstream ss(L"test");

        wstring_container sc;
        ss >> sc;
        EXPECT_EQ(sc, L"test");
    }
}

TEST(basic_string_container, ostream)
{
    using namespace papilio;
    using namespace utf;

    {
        std::stringstream ss;
        ss << "test"_sc;
        EXPECT_EQ(ss.str(), "test");
    }

    {
        std::wstringstream ss;
        ss << L"test"_sc;
        EXPECT_EQ(ss.str(), L"test");
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
