#include <gtest/gtest.h>
#include <papilio/script.hpp>


TEST(interpreter, access)
{
    using namespace papilio;
    using namespace script;

    {
        static_format_args<1, 0> args(182375);
        format_parse_context parse_ctx("{}", args);
        parse_ctx.advance_to(parse_ctx.begin() + 1);

        interpreter intp;

        auto [arg, it] = intp.access(parse_ctx);

        ASSERT_NE(it, parse_ctx.end());
        EXPECT_EQ(*it, U'}');

        ASSERT_TRUE(arg.holds<int>());
        EXPECT_EQ(get<int>(arg), 182375);
    }

    {
        static_format_args<1, 0> args(182375);
        format_parse_context parse_ctx("{:}", args);
        parse_ctx.advance_to(parse_ctx.begin() + 1);

        interpreter intp;

        auto [arg, it] = intp.access(parse_ctx);

        ASSERT_NE(it, parse_ctx.end());
        EXPECT_EQ(*it, U':');

        ASSERT_TRUE(arg.holds<int>());
        EXPECT_EQ(get<int>(arg), 182375);
    }

    {
        static_format_args<2, 0> args(182375, 182376);
        format_parse_context parse_ctx("{1}", args);
        parse_ctx.advance_to(parse_ctx.begin() + 1);

        interpreter intp;

        auto [arg, it] = intp.access(parse_ctx);

        ASSERT_NE(it, parse_ctx.end());
        EXPECT_EQ(*it, U'}');

        ASSERT_TRUE(arg.holds<int>());
        EXPECT_EQ(get<int>(arg), 182376);
    }

    {
        static_format_args<0, 1> args("scene"_a = 182375);
        format_parse_context parse_ctx("{scene}", args);
        parse_ctx.advance_to(parse_ctx.begin() + 1);

        interpreter intp;

        auto [arg, it] = intp.access(parse_ctx);

        ASSERT_NE(it, parse_ctx.end());
        EXPECT_EQ(*it, U'}');

        ASSERT_TRUE(arg.holds<int>());
        EXPECT_EQ(get<int>(arg), 182375);
    }

    {
        auto helper = [](std::string_view fmt) -> format_arg
        {
            static_format_args<0, 1> args("name"_a = "Hu Tao");
            format_parse_context parse_ctx(fmt, args);
            parse_ctx.advance_to(parse_ctx.begin() + 1);

            interpreter intp;

            auto [arg, it] = intp.access(parse_ctx);

            EXPECT_NE(it, parse_ctx.end());
            EXPECT_EQ(*it, U'}');

            return std::move(arg);
        };

        {
            format_arg arg = helper("{name.length}");
            EXPECT_EQ(arg.as_variable(), 6);
        }

        {
            format_arg arg = helper("{name[0]}");
            EXPECT_EQ(arg.as_variable(), "H");
        }

        {
            format_arg arg = helper("{name[-1]}");
            EXPECT_EQ(arg.as_variable(), "o");
        }

        {
            format_arg arg = helper("{name[3:]}");
            EXPECT_EQ(arg.as_variable(), "Tao");
        }

        {
            format_arg arg = helper("{name[-3:]}");
            EXPECT_EQ(arg.as_variable(), "Tao");
        }

        {
            format_arg arg = helper("{name[2:3]}");
            EXPECT_EQ(arg.as_variable(), " ");
        }

        {
            format_arg arg = helper("{name[:]}");
            EXPECT_EQ(arg.as_variable(), "Hu Tao");
        }

        {
            format_arg arg = helper("{name[:].length}");
            EXPECT_EQ(arg.as_variable(), 6);
        }
    }
}

namespace test_interpreter
{
    template <typename... Args>
    auto run_script(std::string_view fmt, Args&&... args)
    {
        using namespace papilio;

        auto fmt_args = PAPILIO_NS make_format_args(std::forward<Args>(args)...);

        format_parse_context parse_ctx(fmt, fmt_args);
        parse_ctx.advance_to(parse_ctx.begin() + 2); // skip '{$'

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
    using namespace test_interpreter;

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
