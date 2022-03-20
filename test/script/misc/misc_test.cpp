#include <gtest/gtest.h>
#include <papilio/papilio.hpp>


using namespace papilio;

TEST(TestMsic, Keywords)
{
    EXPECT_EQ(script::keywords::if_(), "if");
    EXPECT_EQ(script::wkeywords::if_(), L"if");
    EXPECT_EQ(script::u16keywords::if_(), u"if");
    EXPECT_EQ(script::u32keywords::if_(), U"if");
    EXPECT_EQ(script::u8keywords::if_(), u8"if");

    EXPECT_TRUE(script::is_keyword("if"));
    EXPECT_TRUE(script::is_keyword("elif"));
    EXPECT_TRUE(script::is_keyword("else"));
    EXPECT_TRUE(script::is_keyword("end"));
    EXPECT_TRUE(script::is_keyword("and"));
    EXPECT_TRUE(script::is_keyword("or"));
}
TEST(TestMsic, Operators)
{
    EXPECT_EQ(script::operators::op_equal(), "==");
    EXPECT_EQ(script::woperators::op_equal(), L"==");
    EXPECT_EQ(script::u16operators::op_equal(), u"==");
    EXPECT_EQ(script::u32operators::op_equal(), U"==");
    EXPECT_EQ(script::u8operators::op_equal(), u8"==");

    EXPECT_TRUE(script::is_operator("=="));
    EXPECT_TRUE(script::is_operator("!="));
    EXPECT_TRUE(script::is_operator("<"));
    EXPECT_TRUE(script::is_operator("<="));
    EXPECT_TRUE(script::is_operator(">"));
    EXPECT_TRUE(script::is_operator(">="));
    EXPECT_TRUE(script::is_operator(":"));
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
