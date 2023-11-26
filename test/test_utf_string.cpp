#include <gtest/gtest.h>
#include <vector>
#include <papilio/utf/string.hpp>


TEST(basic_string_ref, u8string_ref)
{
    using namespace papilio;
    using namespace utf;

    {
        u8string_ref ref = u8"🔊我ÄA"_sr;
        EXPECT_EQ(ref.length(), 4);

        EXPECT_EQ(ref.index(0), U'🔊');
        EXPECT_EQ(ref.index(reverse_index, 3), U'🔊');

        EXPECT_THROW((void)ref.at(4), std::out_of_range);
        EXPECT_THROW((void)ref.at(reverse_index, 4), std::out_of_range);

        EXPECT_EQ(ref.to_u16string(), u"🔊我ÄA");
        EXPECT_EQ(ref.to_u32string(), U"🔊我ÄA");
        EXPECT_EQ(ref.to_wstring(), L"🔊我ÄA");

        EXPECT_EQ(ref, u"🔊我ÄA");
        EXPECT_EQ(u"🔊我ÄA", ref);
        EXPECT_EQ(ref, U"🔊我ÄA");
        EXPECT_EQ(U"🔊我ÄA", ref);

        EXPECT_EQ(ref.substr(0, 2), u8"🔊我");
        EXPECT_TRUE(ref.substr(4, 2).empty());
        EXPECT_THROW((void)ref.substr(5, 2), std::out_of_range);
        EXPECT_TRUE(ref.substr<substr_behavior::empty_string>(5, 2).empty());
    }

    {
        u8string_ref ref = u8"🔊我ÄA";
        ref.remove_prefix(2);
        EXPECT_EQ(ref, u8"ÄA");
    }

    {
        u8string_ref ref = u8"🔊我ÄA";
        ref.remove_suffix(2);
        EXPECT_EQ(ref, u8"🔊我");
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
        EXPECT_EQ(it, ref.begin());
        it = ref.find(u8"BCD");
        EXPECT_EQ(it, ref.begin() + 1);
        it = ref.find(u8"ABC", 1);
        EXPECT_EQ(it, ref.end() - 3);
    }

    {
        u8string_ref ref(string_ref(""));
    }
    {
        string_ref ref(u8string_ref(""));
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
        u16string_ref ref = u"🔊我"_sr;
        EXPECT_EQ(ref.length(), 2);
        EXPECT_EQ(ref.size(), 3);

        EXPECT_EQ(ref[0], U'🔊');
        EXPECT_EQ(ref[1], U'我');
        EXPECT_EQ(ref.index(reverse_index, 1), U'🔊');
        EXPECT_EQ(ref.index(reverse_index, 0), U'我');

        EXPECT_TRUE(ref.to_u8string() == u8"🔊我"); // GoogleTest doesn't support u8string
        EXPECT_EQ(ref.to_u16string(), u"🔊我");
        EXPECT_EQ(ref.to_u32string(), U"🔊我");
        EXPECT_EQ(ref.to_wstring(), L"🔊我");

        auto it = ref.find(U'我');
        EXPECT_EQ(it, ref.begin() + 1);
    }
}

TEST(basic_string_ref, u32string_ref)
{
    using namespace papilio;
    using namespace utf;

    {
        u32string_ref ref = U"🔊我"_sr;
        EXPECT_EQ(ref.length(), 2);
        EXPECT_EQ(ref.size(), 2);

        EXPECT_EQ(ref[0], U'🔊');
        EXPECT_EQ(ref[1], U'我');
        EXPECT_EQ(ref.index(reverse_index, 1), U'🔊');
        EXPECT_EQ(ref.index(reverse_index, 0), U'我');

        EXPECT_TRUE(ref.to_u8string() == u8"🔊我"); // GoogleTest doesn't support u8string
        EXPECT_EQ(ref.to_u16string(), u"🔊我");
        EXPECT_EQ(ref.to_u32string(), U"🔊我");
        EXPECT_EQ(ref.to_wstring(), L"🔊我");

        auto it = ref.find(U'我');
        EXPECT_EQ(it, ref.begin() + 1);
    }
}

TEST(basic_string_ref, wstring_ref)
{
    using namespace papilio;
    using namespace utf;

    {
        wstring_ref ref = L"🔊我"_sr;
        EXPECT_EQ(ref.length(), 2);
        if constexpr(sizeof(wchar_t) == 2)
            EXPECT_EQ(ref.size(), 3);
        else
            EXPECT_EQ(ref.size(), 2);

        EXPECT_EQ(ref[0], U'🔊');
        EXPECT_EQ(ref[1], U'我');
        EXPECT_EQ(ref.index(reverse_index, 1), U'🔊');
        EXPECT_EQ(ref.index(reverse_index, 0), U'我');

        EXPECT_TRUE(ref.to_u8string() == u8"🔊我"); // GoogleTest doesn't support u8string
        EXPECT_EQ(ref.to_u16string(), u"🔊我");
        EXPECT_EQ(ref.to_u32string(), U"🔊我");
        EXPECT_EQ(ref.to_wstring(), L"🔊我");

        auto it = ref.find(U'我');
        EXPECT_EQ(it, ref.begin() + 1);
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

#if __cpp_multidimensional_subscript >= 202110L

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
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
