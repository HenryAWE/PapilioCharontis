#include <gtest/gtest.h>
#include <papilio/core.hpp>

TEST(format_arg, constructor)
{
    using namespace papilio;

    {
        PAPILIO_NS format_arg arg('a');
        EXPECT_TRUE(arg.holds<utf::codepoint>());
        EXPECT_EQ(get<utf::codepoint>(arg), U'a');
    }

    {
        PAPILIO_NS format_arg arg(1);
        EXPECT_TRUE(arg.holds<int>());
        EXPECT_EQ(get<int>(arg), 1);
    }

    {
        PAPILIO_NS format_arg arg(1.0);
        EXPECT_TRUE(arg.holds<double>());
        EXPECT_DOUBLE_EQ(get<double>(arg), 1.0);
    }

    {
        using namespace std::literals;

        PAPILIO_NS format_arg fmt_arg("test");
        EXPECT_TRUE(fmt_arg.holds<utf::string_container>());
        EXPECT_FALSE(get<utf::string_container>(fmt_arg).has_ownership());
    }

    {
        using namespace std::literals;

        PAPILIO_NS format_arg fmt_arg("test"s);
        EXPECT_TRUE(fmt_arg.holds<utf::string_container>());
        EXPECT_TRUE(get<utf::string_container>(fmt_arg).has_ownership());
    }

    {
        using namespace std::literals;

        std::string s = "test"s;
        format_arg fmt_arg(s);
        EXPECT_TRUE(fmt_arg.holds<utf::string_container>());
        EXPECT_FALSE(get<utf::string_container>(fmt_arg).has_ownership());
    }
}

TEST(format_arg, access)
{
    using namespace papilio;

    {
        PAPILIO_NS format_arg fmt_arg("test");
        EXPECT_TRUE(fmt_arg.holds<utf::string_container>());
        EXPECT_FALSE(get<utf::string_container>(fmt_arg).has_ownership());

        EXPECT_EQ(get<std::size_t>(fmt_arg.attribute("length")), std::string("test").length());
        EXPECT_EQ(get<utf::codepoint>(fmt_arg.index(0)), U't');
        EXPECT_EQ(get<utf::codepoint>(fmt_arg.index(1)), U'e');
        EXPECT_EQ(get<utf::codepoint>(fmt_arg.index(2)), U's');
        EXPECT_EQ(get<utf::codepoint>(fmt_arg.index(3)), U't');
        EXPECT_EQ(get<utf::codepoint>(fmt_arg.index(4)), utf::codepoint());
    }

    {
        // "测试", test in Chinese
        PAPILIO_NS format_arg fmt_arg("\u6d4b\u8bd5");
        EXPECT_TRUE(fmt_arg.holds<utf::string_container>());

        EXPECT_EQ(get<std::size_t>(fmt_arg.attribute("length")), 2);
        EXPECT_EQ(get<utf::codepoint>(fmt_arg.index(0)), U'\u6d4b');
        EXPECT_EQ(get<utf::codepoint>(fmt_arg.index(1)), U'\u8bd5');
        EXPECT_EQ(get<utf::codepoint>(fmt_arg.index(2)), utf::codepoint());
    }

    {
        PAPILIO_NS format_arg fmt_arg("test");

        auto var = script::variable(fmt_arg.to_variant());
        EXPECT_EQ(var.as<utf::string_container>(), "test");
    }

    {
        PAPILIO_NS format_arg fmt_arg("long sentence for testing slicing");

        EXPECT_EQ(get<utf::string_container>(fmt_arg.index(slice(0, 4))), "long");
        EXPECT_EQ(get<utf::string_container>(fmt_arg.index(slice(-7, slice::npos))), "slicing");
        EXPECT_EQ(get<utf::string_container>(fmt_arg.index(slice(14, -16))), "for");
        EXPECT_EQ(get<utf::string_container>(fmt_arg.index(slice(-slice::npos, -20))), "long sentence");

        EXPECT_EQ(get<std::string>(fmt_arg.index(slice(0, 4))), "long");
        EXPECT_EQ(get<std::string_view>(fmt_arg.index(slice(0, 4))), "long");
    }
}

TEST(format_args, mutable)
{
    using namespace papilio;

    {
        mutable_format_args args;
        EXPECT_EQ(args.indexed_size(), 0);
        EXPECT_EQ(args.named_size(), 0);
    }

    {
        mutable_format_args args(1, "three"_a = 3, 2);

        EXPECT_EQ(args.indexed_size(), 2);
        EXPECT_EQ(args.named_size(), 1);

        EXPECT_EQ(get<int>(args[0]), 1);
        EXPECT_EQ(get<int>(args[1]), 2);
        EXPECT_EQ(get<int>(args["three"]), 3);

        args.clear();

        EXPECT_EQ(args.indexed_size(), 0);
        EXPECT_EQ(args.named_size(), 0);

        args.push('a', 'b', "c"_a = 'c', "d"_a = 'd');

        EXPECT_EQ(args.indexed_size(), 2);
        EXPECT_EQ(args.named_size(), 2);

        EXPECT_EQ(get<utf::codepoint>(args[0]), U'a');
        EXPECT_EQ(get<utf::codepoint>(args[1]), U'b');
        EXPECT_EQ(get<utf::codepoint>(args["c"]), U'c');
        EXPECT_EQ(get<utf::codepoint>(args["d"]), U'd');
    }
}

TEST(format_args, dynamic)
{
    using namespace papilio;

    mutable_format_args underlying_fmt_args;
    dynamic_format_args dyn_fmt_args(underlying_fmt_args);

    EXPECT_EQ(&dyn_fmt_args.cast_to<mutable_format_args>(), &underlying_fmt_args);

    dynamic_format_args new_dyn_fmt_args(dyn_fmt_args);

    EXPECT_EQ(&new_dyn_fmt_args.cast_to<mutable_format_args>(), &underlying_fmt_args);
}

TEST(format_context, basic)
{
    using namespace papilio;

    std::string result;
    mutable_format_args store;
    basic_format_context ctx(
        std::back_inserter(result),
        store
    );

    using context_traits = format_context_traits<decltype(ctx)>;
    EXPECT_EQ(&context_traits::get_args(ctx).cast_to<mutable_format_args>(), &store);

    context_traits::append(ctx, "1234");
    EXPECT_EQ(result, "1234");

    result.clear();
    context_traits::append(ctx, '1', 4);
    EXPECT_EQ(result, "1111");

    result.clear();
    context_traits::append(ctx, U'\u00c4', 2);
    EXPECT_EQ(result, "\u00c4\u00c4");
}

TEST(format_context, dynamic_small)
{
    using namespace papilio;

    std::string result;
    mutable_format_args store;
    basic_format_context ctx(
        std::back_inserter(result),
        store
    );
    dynamic_format_context dyn_ctx(ctx);

    using context_traits = format_context_traits<decltype(dyn_ctx)>;
    EXPECT_EQ(&context_traits::get_args(dyn_ctx).cast_to<mutable_format_args>(), &store);

    context_traits::append(dyn_ctx, "1234");
    EXPECT_EQ(result, "1234");

    result.clear();
    context_traits::append(dyn_ctx, '1', 4);
    EXPECT_EQ(result, "1111");

    result.clear();
    context_traits::append(dyn_ctx, U'\u00c4', 2);
    EXPECT_EQ(result, "\u00c4\u00c4");

    dynamic_format_context new_dyn_ctx(dyn_ctx);
    result.clear();
    context_traits::append(new_dyn_ctx, "new");
    EXPECT_EQ(result, "new");
}

namespace test_core
{
class big_insert_iterator : public std::back_insert_iterator<std::string>
{
    using base = std::back_insert_iterator<std::string>;

public:
    big_insert_iterator(std::string& str) noexcept
        : base(std::back_inserter(str)) {}

private:
    std::byte m_dummy[1024]{};
};
} // namespace test_core

TEST(format_context, dynamic_big)
{
    using namespace papilio;

    std::string result;
    mutable_format_args store;
    basic_format_context ctx(
        test_core::big_insert_iterator(result),
        store
    );
    dynamic_format_context dyn_ctx(ctx);

    using context_traits = format_context_traits<decltype(dyn_ctx)>;
    EXPECT_EQ(&context_traits::get_args(dyn_ctx).cast_to<mutable_format_args>(), &store);

    context_traits::append(dyn_ctx, "1234");
    EXPECT_EQ(result, "1234");

    result.clear();
    context_traits::append(dyn_ctx, '1', 4);
    EXPECT_EQ(result, "1111");

    result.clear();
    context_traits::append(dyn_ctx, U'\u00c4', 2);
    EXPECT_EQ(result, "\u00c4\u00c4");

    dynamic_format_context new_dyn_ctx(dyn_ctx);
    result.clear();
    context_traits::append(new_dyn_ctx, "new");
    EXPECT_EQ(result, "new");
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
