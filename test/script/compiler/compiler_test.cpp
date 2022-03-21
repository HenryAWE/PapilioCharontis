#include <gtest/gtest.h>
#include <papilio/papilio.hpp>


using namespace papilio;

TEST(TestCompiler, Compile)
{
    using namespace script;

    lexer lex;
    compiler cl;
    exec ctx;
    std::unique_ptr<exec::script> script;

    lex.clear();
    script = cl.compile(lex.lexemes());
    EXPECT_TRUE(script->invoke(ctx).empty());

    lex.clear();
    lex.parse(R"("string")");
    script = cl.compile(lex.lexemes());
    EXPECT_EQ(script->invoke(ctx), "string");

    lex.clear();
    lex.parse(R"(1 > 2)");
    script = cl.compile(lex.lexemes());
    EXPECT_FALSE(script->invoke(ctx).as<bool>());

    lex.clear();
    lex.parse(R"(@0)");
    script = cl.compile(lex.lexemes());
    ctx.clear_arg();
    ctx.push_arg(exec::argument(1));
    EXPECT_EQ(script->invoke(ctx).as<int>(), 1);

    lex.clear();
    lex.parse(R"(@str)");
    script = cl.compile(lex.lexemes());
    ctx.clear_arg();
    ctx.set_named_arg("str", exec::argument("string value"));
    EXPECT_EQ(script->invoke(ctx).as<std::string>(), "string value");

    lex.clear();
    lex.parse(R"(@0 > 2)");
    script = cl.compile(lex.lexemes());
    ctx.clear_arg();
    ctx.push_arg(exec::argument(1));
    EXPECT_FALSE(script->invoke(ctx).as<bool>());
    ctx.clear_arg();
    ctx.push_arg(exec::argument(3));
    EXPECT_TRUE(script->invoke(ctx).as<bool>());

    lex.clear();
    lex.parse(R"(if @0 == 1: "is" else: "are" end)");
    script = cl.compile(lex.lexemes());
    ctx.clear_arg();
    ctx.push_arg(exec::argument(1));
    EXPECT_EQ(script->invoke(ctx).as<std::string>(), "is");
    ctx.clear_arg();
    ctx.push_arg(exec::argument(2));
    EXPECT_EQ(script->invoke(ctx).as<std::string>(), "are");

    lex.clear();
    lex.parse(
        R"(if @0 == 0: "zero")"
        R"(elif @0 > 1: "more than one")"
        R"(elif @0 == 1: "one")"
        R"(else: "other")"
        R"(end)"
    );
    script = cl.compile(lex.lexemes());
    ctx.clear_arg();
    ctx.push_arg(exec::argument(0));
    EXPECT_EQ(script->invoke(ctx).as<std::string>(), "zero");
    ctx.clear_arg();
    ctx.push_arg(exec::argument(2));
    EXPECT_EQ(script->invoke(ctx).as<std::string>(), "more than one");
    ctx.clear_arg();
    ctx.push_arg(exec::argument(1));
    EXPECT_EQ(script->invoke(ctx).as<std::string>(), "one");
    ctx.clear_arg();
    ctx.push_arg(exec::argument(-1));
    EXPECT_EQ(script->invoke(ctx).as<std::string>(), "other");
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
