#include <gtest/gtest.h>
#include <papilio/core.hpp>
#include <papilio_test/setup.hpp>

TEST(format_args, dynamic)
{
    using namespace papilio;

    {
        dynamic_format_args args;
        EXPECT_EQ(args.indexed_size(), 0);
        EXPECT_EQ(args.named_size(), 0);
    }

    {
        dynamic_format_args args(1, "three"_a = 3, 2);

        EXPECT_EQ(args.indexed_size(), 2);
        EXPECT_EQ(args.named_size(), 1);

        EXPECT_EQ(get<int>(args[0]), 1);
        EXPECT_EQ(get<int>(args[1]), 2);
        EXPECT_EQ(get<int>(args["three"]), 3);

        args.clear();

        EXPECT_EQ(args.indexed_size(), 0);
        EXPECT_EQ(args.named_size(), 0);

        args.push_tuple('a', 'b', "c"_a = 'c', "d"_a = 'd');

        EXPECT_EQ(args.indexed_size(), 2);
        EXPECT_EQ(args.named_size(), 2);

        EXPECT_EQ(get<utf::codepoint>(args[0]), U'a');
        EXPECT_EQ(get<utf::codepoint>(args[1]), U'b');
        EXPECT_EQ(get<utf::codepoint>(args["c"]), U'c');
        EXPECT_EQ(get<utf::codepoint>(args["d"]), U'd');
    }
}

TEST(format_args, static)
{
    using namespace papilio;

    {
        static_format_args<0, 0> empty;
        EXPECT_EQ(empty.indexed_size(), 0);
        EXPECT_EQ(empty.named_size(), 0);
    }

    {
        static_format_args<1, 0> args{182375};
        EXPECT_EQ(args.indexed_size(), 1);
        EXPECT_EQ(args.named_size(), 0);
    }

    {
        auto args = make_format_args(182375, 182376);
        EXPECT_EQ(args.indexed_size(), 2);
    }
}

TEST(format_args, ref)
{
    using namespace papilio;

    {
        dynamic_format_args underlying_fmt_args;
        format_args_ref args_ref(underlying_fmt_args);

        EXPECT_EQ(args_ref.indexed_size(), 0);

        format_args_ref new_ref(args_ref);
    }

    {
        auto underlying_fmt_args = make_format_args(182375, 182376);
        format_args_ref args_ref(underlying_fmt_args);

        EXPECT_EQ(args_ref.indexed_size(), 2);
    }
}

TEST(format_parse_context, char)
{
    using namespace papilio;

    {
        dynamic_format_args args;
        args.push_tuple(0, 1, 2);
        args.push("value"_a = 0);

        utf::string_ref sr = "{}";

        format_parse_context ctx(sr, args);

        EXPECT_EQ(ctx.begin(), sr.begin());
        EXPECT_EQ(ctx.end(), sr.end());
        EXPECT_EQ(*ctx.begin(), U'{');

        ctx.advance_to(std::next(ctx.begin()));
        EXPECT_EQ(ctx.begin(), std::next(sr.begin()));
        EXPECT_EQ(*ctx.begin(), U'}');

        ctx.check_arg_id(0);
        ctx.check_arg_id(1);
        ctx.check_arg_id(2);

        ctx.check_arg_id("value");
        EXPECT_THROW(ctx.check_arg_id("error"), format_error);
    }

    {
        dynamic_format_args args;
        args.push_tuple(0, 1, 2);
        args.push("value"_a = 0);

        format_parse_context ctx("{0} {}", args);

        EXPECT_EQ(ctx.current_arg_id(), 0);
        EXPECT_EQ(ctx.next_arg_id(), 1);
        ctx.check_arg_id(0);
        EXPECT_THROW((void)ctx.current_arg_id(), format_error);
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
