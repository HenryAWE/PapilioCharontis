#include <gtest/gtest.h>
#include <papilio/script.hpp>


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
}

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
        auto helper = [](std::string_view fmt) { return test_access(fmt, "name"_a = "Hu Tao"); };

        EXPECT_EQ(helper("{name.length}").as_variable(), 6);

        EXPECT_EQ(helper("{name[0]}").as_variable(), "H");

        EXPECT_EQ(helper("{name[-1]}").as_variable(), "o");

        EXPECT_EQ(helper("{name[3:]}").as_variable(), "Tao");

        EXPECT_EQ(helper("{name[-3:]}").as_variable(), "Tao");

        EXPECT_EQ(helper("{name[2:3]}").as_variable(), " ");

        EXPECT_EQ(helper("{name[:]}").as_variable(), "Hu Tao");

        EXPECT_EQ(helper("{name[:].length}").as_variable(), 6);
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
}

TEST(interpreter, run)
{
    using namespace papilio;
    using namespace test_script_interpreter;

    {
        auto arg = run_script("{$ {val}: 'true'}", "val"_a = true);

        EXPECT_EQ(arg.as_variable(), "true");
    }

    {
        auto arg = run_script("{$ !{val}: 'false'}", "val"_a = false);

        EXPECT_EQ(arg.as_variable(), "false");
    }

    {
        auto arg = run_script("{$ {val}: 'true' : 'false'}", "val"_a = true);

        EXPECT_EQ(arg.as_variable(), "true");
    }
    
    {
        auto arg = run_script("{$ {val}: 'true' : 'false'}", "val"_a = false);

        EXPECT_EQ(arg.as_variable(), "false");
    }

    {
        auto arg = run_script("{$ {val} == 0: 'zero'}", "val"_a = 0);

        EXPECT_EQ(arg.as_variable(), "zero");
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
