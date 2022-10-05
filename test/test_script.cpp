#include <gtest/gtest.h>
#include <algorithm>
#include <ranges>
#include <papilio/papilio.hpp>


TEST(TestScript, Utilities)
{
    using namespace papilio::script;

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
    l.parse((const char*)u8R"('非ASCII字符串')");
    {
        auto lexemes = l.lexemes();
        EXPECT_EQ(lexemes[0].type(), lexeme_type::constant);
        EXPECT_EQ(
            lexemes[0].as<lexeme::constant>().get_string(),
            (const char*)u8"非ASCII字符串"
        );
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
TEST(TestScript, Executor)
{
    using namespace papilio;
    using namespace script;

    {
        executor::context ctx;
        ctx.push(1);
        EXPECT_EQ(ctx.top().get<executor::int_type>(), 1);
    }
    {
        executor::context ctx;
        ctx.push(2.0L);
        EXPECT_DOUBLE_EQ(ctx.top().get<executor::float_type>(), 2.0L);
    }
    {
        executor::context ctx;
        ctx.push("string");
        EXPECT_EQ(ctx.top().get<executor::string_type>(), "string");
    }

    {
        executor::context ctx;

        executor ex(std::in_place_type<executor::constant<executor::int_type>>, 2);
        ex(ctx);

        auto i = ctx.copy_and_pop().get<executor::int_type>();
        EXPECT_EQ(i, 2);
    }

    {
        using namespace std::literals;

        int a1 = 1;
        float a2 = 2.0f;
        std::string a3 = "test";

        executor::context ctx(dynamic_format_arg_store(a1, a2, "string"_a = a3));

        executor ex1(std::in_place_type<executor::argument>, 0);
        ex1(ctx);

        EXPECT_EQ(ctx.copy_and_pop().get<executor::int_type>(), 1);

        executor ex2(std::in_place_type<executor::argument>, 1);
        ex2(ctx);

        EXPECT_DOUBLE_EQ(ctx.copy_and_pop().get<executor::float_type>(), 2.0);

        executor ex3(std::in_place_type<executor::argument>, "string"s);
        ex3(ctx);

        EXPECT_EQ(ctx.copy_and_pop().get<executor::string_type>(), "test");

        executor ex4(
            std::in_place_type<executor::argument>,
            "string"s,
            std::vector<executor::argument::member_type>{ attribute_name("length") }
        );
        ex4(ctx);

        EXPECT_EQ(ctx.copy_and_pop().get<executor::int_type>(), std::string("test").length());
    }
}
TEST(TestScript, Interpreter)
{
    using namespace papilio;
    using namespace script;

    // constant value
    {
        interpreter intp;
        std::string result = intp.run("'hello'", {});
        EXPECT_EQ(result, "hello");
    }

    // if
    {
        interpreter intp;

        std::string src = "if $0: 'hello'";
        std::string result = intp.run(src, true);
        EXPECT_EQ(result, "hello");

        result = intp.run(src, false);
        EXPECT_EQ(result, "");
    }

    // if-else
    {
        interpreter intp;

        std::string src = "if $0: 'a' else: 'b'";
        std::string result = intp.run(src, true);
        EXPECT_EQ(result, "a");

        result = intp.run(src, false);
        EXPECT_EQ(result, "b");
    }

    // if-elif-else
    {
        interpreter intp;

        std::string src = "if $0: 'a' elif $1: 'b' else: 'c'";
        std::string result = intp.run(src, true, false);
        EXPECT_EQ(result, "a");

        result = intp.run(src, false, true);
        EXPECT_EQ(result, "b");

        result = intp.run(src, false, false);
        EXPECT_EQ(result, "c");
    }

    // indexing
    {
        interpreter intp;

        std::string result = intp.run("$0[0]", "hello");
        EXPECT_EQ(result, "h");
        result = intp.run("$0[4]", "hello");
        EXPECT_EQ(result, "o");

        std::string str = "argument";
        result = intp.run("$0[0]", str);
        EXPECT_EQ(result, "a");
    }

    // indexing for non-ASCII characters
    {
        using namespace std::literals;
        interpreter intp;

        std::string result = intp.run("$0[0]", (const char*)u8"这是一个测试字符串");
        EXPECT_EQ(result, (const char*)u8"这");

        std::string str = (const char*)u8"参数";
        result = intp.run("$0[0]", str);
        EXPECT_EQ(result, (const char*)u8"参");
        result = intp.run("$0[1]", str);
        EXPECT_EQ(result, (const char*)u8"数");
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
