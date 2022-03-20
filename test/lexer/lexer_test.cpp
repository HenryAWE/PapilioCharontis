#include <gtest/gtest.h>
#include <papilio/papilio.hpp>


using namespace papilio;

TEST(TestLexer, Lexemes)
{
    script::lexer lex;
    lex.parse(R"(if @0: "2\"\"@" else: "" end)");
    auto lexemes = lex.lexemes();
    EXPECT_EQ(lexemes.size(), 8);
    EXPECT_EQ(lexemes[0].type(), script::lexeme_type::keyword);
    EXPECT_EQ(lexemes[0].str(), "if");
    EXPECT_EQ(lexemes[1].type(), script::lexeme_type::identifier);
    EXPECT_EQ(lexemes[1].str(), "0");
    EXPECT_EQ(lexemes[2].type(), script::lexeme_type::operator_);
    EXPECT_EQ(lexemes[2].str(), ":");
    EXPECT_EQ(lexemes[3].type(), script::lexeme_type::literal);
    EXPECT_EQ(lexemes[3].str(), R"("2""@")");

    lex.clear();
    lex.parse(R"(if @0 == 1: "is" end)");
    lexemes = lex.lexemes();
    EXPECT_EQ(lexemes.size(), 7);
    EXPECT_EQ(lexemes[2].type(), script::lexeme_type::operator_);
    EXPECT_EQ(lexemes[2].str(), "==");
    EXPECT_EQ(lexemes[3].type(), script::lexeme_type::literal);
    EXPECT_EQ(lexemes[3].str(), "1");
    EXPECT_EQ(lexemes[6].type(), script::lexeme_type::keyword);
    EXPECT_EQ(lexemes[6].str(), "end");

    lex.clear();
    lex.parse(R"(1.1)");
    lexemes = lex.lexemes();
    EXPECT_EQ(lexemes.size(), 1);
    EXPECT_EQ(lexemes[0].type(), script::lexeme_type::literal);
    EXPECT_EQ(lexemes[0].str(), "1.1");

    lex.clear();
    lex.parse(R"(@number)");
    lexemes = lex.lexemes();
    EXPECT_EQ(lexemes.size(), 1);
    EXPECT_EQ(lexemes[0].type(), script::lexeme_type::identifier);
    EXPECT_EQ(lexemes[0].str(), "number");
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}