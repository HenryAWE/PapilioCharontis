#include <gtest/gtest.h>
#include <algorithm>
#include <ranges>
#include <papilio/script.hpp>


TEST(TestScript, Utilities)
{
    using namespace papilio;
    using namespace script;

    static_assert(is_lexeme<lexeme::argument>);
    static_assert(is_lexeme<lexeme::identifier>);
    static_assert(is_lexeme<lexeme::constant>);
    static_assert(is_lexeme<lexeme::keyword>);
    static_assert(is_lexeme<lexeme::operator_>);

    {
        using namespace std::literals;
        namespace stdr = std::ranges;

        EXPECT_TRUE(stdr::all_of(" \n\t\f\v"sv, detail::is_space));
        EXPECT_FALSE(stdr::all_of("abcABC_$.*/+-="sv, detail::is_space));

        EXPECT_TRUE(stdr::all_of("1234567890"sv, detail::is_digit));
        EXPECT_FALSE(stdr::all_of("a1234567890"sv, detail::is_digit));
        EXPECT_TRUE(stdr::all_of("1234567890abcdefABCDEF"sv, detail::is_xdigit));
        EXPECT_FALSE(stdr::all_of("1234567890abcdefABCDEFgG"sv, detail::is_xdigit));

        EXPECT_TRUE(stdr::all_of("name"sv, detail::is_identifier_helper()));
        EXPECT_TRUE(stdr::all_of("_name"sv, detail::is_identifier_helper()));
        EXPECT_TRUE(stdr::all_of("NAME"sv, detail::is_identifier_helper()));
        EXPECT_TRUE(stdr::all_of("name_1"sv, detail::is_identifier_helper()));
        EXPECT_FALSE(stdr::all_of("$name"sv, detail::is_identifier_helper()));
        EXPECT_FALSE(stdr::all_of("-name"sv, detail::is_identifier_helper()));
        EXPECT_FALSE(stdr::all_of("1_name"sv, detail::is_identifier_helper()));
        EXPECT_FALSE(stdr::all_of("name 1"sv, detail::is_identifier_helper()));
    }
}
TEST(TestScript, Lexer)
{
    using namespace papilio;
    using namespace script;

    lexer l;
    l.parse(R"(if $0: 'one\'s')");
    {
        auto lexemes = l.lexemes();
        
        EXPECT_EQ(lexemes[0].type(), lexeme_type::keyword);
        EXPECT_EQ(lexemes[0].as<lexeme::keyword>().get(), keyword_type::if_);

        EXPECT_EQ(lexemes[1].type(), lexeme_type::argument);
        EXPECT_EQ(lexemes[1].as<lexeme::argument>().get_index(), 0);

        EXPECT_EQ(lexemes[2].type(), lexeme_type::operator_);
        EXPECT_EQ(lexemes[2].as<lexeme::operator_>().get(), operator_type::colon);

        EXPECT_EQ(lexemes[3].type(), lexeme_type::constant);
        EXPECT_EQ(lexemes[3].as<lexeme::constant>().get_string(), "one's");
    }

    l.clear();
    l.parse(R"(if $name: {name} else: '(empty)')");
    {
        auto lexemes = l.lexemes();

        EXPECT_EQ(lexemes[0].type(), lexeme_type::keyword);
        EXPECT_EQ(lexemes[0].as<lexeme::keyword>().get(), keyword_type::if_);

        EXPECT_EQ(lexemes[1].type(), lexeme_type::argument);
        EXPECT_EQ(lexemes[1].as<lexeme::argument>().get_string(), "name");

        EXPECT_EQ(lexemes[2].type(), lexeme_type::operator_);
        EXPECT_EQ(lexemes[2].as<lexeme::operator_>().get(), operator_type::colon);

        EXPECT_EQ(lexemes[3].type(), lexeme_type::field);
        EXPECT_EQ(lexemes[3].as<lexeme::field>().get(), "name");

        EXPECT_EQ(lexemes[4].type(), lexeme_type::keyword);
        EXPECT_EQ(lexemes[4].as<lexeme::keyword>().get(), keyword_type::else_);

        EXPECT_EQ(lexemes[5].type(), lexeme_type::operator_);
        EXPECT_EQ(lexemes[5].as<lexeme::operator_>().get(), operator_type::colon);

        EXPECT_EQ(lexemes[6].type(), lexeme_type::constant);
        EXPECT_EQ(lexemes[6].as<lexeme::constant>().get_string(), "(empty)");
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
