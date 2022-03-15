#include <gtest/gtest.h>
#include <papilio/papilio.hpp>


using namespace papilio;

TEST(TestLexer, Lexemes)
{
    script::lexer lex;
    const char src[] = R"(if @0: "2\"\"@" else: "")";
    lex.parse(src);
    auto lexemes = lex.lexemes();

    EXPECT_EQ(lexemes[0].type(), script::lexeme_type::keyword);
    EXPECT_EQ(lexemes[0].str(), "if");
    EXPECT_EQ(lexemes[1].type(), script::lexeme_type::identifier);
    EXPECT_EQ(lexemes[1].str(), "0");
    EXPECT_EQ(lexemes[2].type(), script::lexeme_type::operator_);
    EXPECT_EQ(lexemes[2].str(), ":");
    EXPECT_EQ(lexemes[3].type(), script::lexeme_type::literal);
    EXPECT_EQ(lexemes[3].str(), R"(2""@)");
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}