#include <gtest/gtest.h>
#include <vector>
#include <papilio/utf/string.hpp>


TEST(TestUTFString, StringRef)
{
    using namespace papilio;
    using namespace utf;

    {
        u8string_ref ref = u8"🔊我ÄA";
        EXPECT_EQ(ref.length(), 4);

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
        u8string_ref ref = u8"ABCDEFG ABC";
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
        u16string_ref ref = u"🔊我";
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
    }

    {
        u32string_ref ref = U"🔊我";
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
    }

    {
        wstring_ref ref = L"🔊我";
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
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
