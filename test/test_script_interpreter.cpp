#include <gtest/gtest.h>
#include <papilio/script/interpreter.hpp>

namespace test_script_interpreter
{
template <typename... Args>
auto test_access(std::string_view fmt, Args&&... args)
{
    using namespace papilio;
    auto fmt_args = PAPILIO_NS make_format_args(std::forward<Args>(args)...);

    format_parse_context parse_ctx(fmt, fmt_args);
    parse_ctx.advance_to(parse_ctx.begin() + 1); // skip '{'

    script::interpreter intp;
    auto [result, it] = intp.access(parse_ctx);

    EXPECT_NE(it, parse_ctx.end());
    EXPECT_TRUE(*it == U'}' || *it == U':');

    return std::move(result);
}
} // namespace test_script_interpreter

TEST(interpreter, access)
{
    using namespace papilio;
    using namespace script;
    using namespace test_script_interpreter;

    {
        format_arg arg = test_access("{}", 182375);

        ASSERT_TRUE(arg.holds<int>());
        EXPECT_EQ(get<int>(arg), 182375);
    }

    {
        format_arg arg = test_access("{:}", 182375);

        ASSERT_TRUE(arg.holds<int>());
        EXPECT_EQ(get<int>(arg), 182375);
    }

    {
        format_arg arg = test_access("{1}", 182375, 182376);

        ASSERT_TRUE(arg.holds<int>());
        EXPECT_EQ(get<int>(arg), 182376);
    }

    {
        format_arg arg = test_access("{scene}", "scene"_a = 182375);

        ASSERT_TRUE(arg.holds<int>());
        EXPECT_EQ(get<int>(arg), 182375);
    }

    {
        format_arg arg = test_access("{}", "hello");

        ASSERT_TRUE(arg.holds<utf::string_container>());
        EXPECT_EQ(get<utf::string_container>(arg), "hello");
    }

    {
        auto helper = [](std::string_view fmt) -> variable
        {
            return variable(test_access(fmt, "name"_a = "Hu Tao").to_variant());
        };

        EXPECT_EQ(helper("{name.length}"), 6);

        EXPECT_EQ(helper("{name[0]}"), "H");

        EXPECT_EQ(helper("{name[-1]}"), "o");

        EXPECT_EQ(helper("{name[3:]}"), "Tao");

        EXPECT_EQ(helper("{name[-3:]}"), "Tao");

        EXPECT_EQ(helper("{name[2:3]}"), " ");

        EXPECT_EQ(helper("{name[:]}"), "Hu Tao");

        EXPECT_EQ(helper("{name[:].length}"), 6);
    }
}

namespace test_script_interpreter
{
template <typename... Args>
auto run_script(std::string_view fmt, Args&&... args)
{
    using namespace papilio;

    auto fmt_args = PAPILIO_NS make_format_args(std::forward<Args>(args)...);

    format_parse_context parse_ctx(fmt, fmt_args);
    parse_ctx.advance_to(parse_ctx.begin() + 2); // skip "{$"

    script::interpreter intp;

    auto [arg, it] = intp.run(parse_ctx);
    EXPECT_NE(it, parse_ctx.end());
    EXPECT_EQ(*it, U'}');

    return std::move(arg);
}
} // namespace test_script_interpreter

TEST(interpreter, run)
{
    using namespace papilio;
    using namespace script;
    using namespace test_script_interpreter;

    {
        auto arg = run_script("{$ {val}: 'true'}", "val"_a = true);

        EXPECT_EQ(variable(arg.to_variant()), "true");
    }

    {
        auto arg = run_script("{$ !{val}: 'false'}", "val"_a = false);

        EXPECT_EQ(variable(arg.to_variant()), "false");
    }

    {
        auto arg = run_script("{$ {val}: 'true' : 'false'}", "val"_a = true);

        EXPECT_EQ(variable(arg.to_variant()), "true");
    }

    {
        auto arg = run_script("{$ {val}: 'true' : 'false'}", "val"_a = false);

        EXPECT_EQ(variable(arg.to_variant()), "false");
    }

    {
        auto arg = run_script("{$ {val} == 0: 'zero'}", "val"_a = 0);

        EXPECT_EQ(variable(arg.to_variant()), "zero");
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
