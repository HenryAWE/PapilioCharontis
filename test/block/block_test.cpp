#include <gtest/gtest.h>
#include <papilio/papilio.hpp>


using namespace papilio;

TEST(TestBlock, Parsing)
{
    auto sample = R"(Text {0} ["script"])";
    format_parser parser;
    parser.parse(sample);

    auto blocks = parser.blocks();
    EXPECT_EQ(blocks[0].type(), block_type::text);
    EXPECT_EQ(blocks[0].str(), "Text ");
    EXPECT_EQ(blocks[1].type(), block_type::relpacement_field);
    EXPECT_EQ(blocks[1].str(), "0");
    EXPECT_EQ(blocks[2].type(), block_type::text);
    EXPECT_EQ(blocks[2].str(), " ");
    EXPECT_EQ(blocks[3].type(), block_type::script);
    EXPECT_EQ(blocks[3].str(), R"("script")");
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
