#include <gtest/gtest.h>
#include <vector>
#include <papilio/utf/codepoint.hpp>


TEST(TestUTFCodepoint, Codepoint)
{
    using namespace papilio;
    using namespace utf;

    {
        static_assert(std::is_trivial_v<codepoint>);
        static_assert(std::is_standard_layout_v<codepoint>);
    }

    {
        static_assert(decoder<char32_t>::size_bytes(U'A') == 1);
        static_assert(decoder<char32_t>::size_bytes(U'Ä') == 2);
        static_assert(decoder<char32_t>::size_bytes(U'我') == 3);
        static_assert(decoder<char32_t>::size_bytes(U'🔊') == 4);
    }

    {
        utf::codepoint cp;

        cp = decoder<char32_t>::to_codepoint(U'A').first;
        EXPECT_EQ(cp, U'A');
        EXPECT_EQ(cp.size(), 1);

        cp = decoder<char32_t>::to_codepoint(U'Ä').first;
        EXPECT_EQ(cp, U'Ä');
        EXPECT_EQ(cp.size(), 2);

        cp = decoder<char32_t>::to_codepoint(U'我').first;
        EXPECT_EQ(cp, U'我');
        EXPECT_EQ(cp.size(), 3);

        cp = decoder<char32_t>::to_codepoint(U'🔊').first;
        EXPECT_EQ(cp, U'🔊');
        EXPECT_EQ(cp.size(), 4);
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
