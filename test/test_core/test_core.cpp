#include <gtest/gtest.h>
#include <papilio/papilio.hpp>
#include <papilio_test/setup.hpp>

TEST(format_args, empty)
{
    using namespace papilio;

    {
        std::string buf;
        format_context ctx(
            std::back_inserter(buf),
            empty_format_args_for<format_context>
        );

        EXPECT_EQ(ctx.get_args().indexed_size(), 0);
        EXPECT_EQ(ctx.get_args().named_size(), 0);
        EXPECT_FALSE(ctx.get_args().contains("test"));
    }

    {
        std::wstring buf;
        wformat_context ctx(
            std::back_inserter(buf),
            empty_format_args_for<wformat_context>
        );

        EXPECT_EQ(ctx.get_args().indexed_size(), 0);
        EXPECT_EQ(ctx.get_args().named_size(), 0);
        EXPECT_FALSE(ctx.get_args().contains(L"test"));
    }
}

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

        args.append('a', 'b', "c"_a = 'c', "d"_a = 'd');

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

    [](auto args)
    {
        EXPECT_EQ(args.indexed_size(), 2);
        EXPECT_EQ(args.named_size(), 1);
        EXPECT_TRUE(args.contains("name"));

        EXPECT_EQ(get<std::string>(args.get("name")), "scene");
    }(make_format_args(182375, 182376, "name"_a = "scene"));
}

namespace test_core
{
template <typename Context>
class custom_format_args final : public papilio::basic_dynamic_format_args<Context>
{
    using my_base = papilio::basic_dynamic_format_args<Context>;

public:
    using char_type = typename Context::char_type;
    using string_type = typename my_base::string_type;
    using string_view_type = typename my_base::string_view_type;
    using format_arg_type = typename my_base::format_arg_type;

    using my_base::my_base;

    bool contains(string_view_type key) const noexcept override
    {
        if(key == PAPILIO_TSTRING_VIEW(char_type, "argc"))
            return true;

        return my_base::contains(key);
    }

    const format_arg_type& get(string_view_type key) const override
    {
        if(key == PAPILIO_TSTRING_VIEW(char_type, "argc"))
        {
            update_argc();
            return m_argc;
        }

        return my_base::get(key);
    }

private:
    mutable format_arg_type m_argc;

    void update_argc() const
    {
        m_argc = format_arg_type(this->indexed_size() + this->named_size());
    }
};
} // namespace test_core

TEST(format_args, custom)
{
    using namespace papilio;

    test_core::custom_format_args<format_context> args(1, "three"_a = 3, 2);

    EXPECT_EQ(get<int>(args[0]), 1);
    EXPECT_EQ(get<int>(args[1]), 2);
    EXPECT_EQ(get<int>(args["three"]), 3);

    EXPECT_TRUE(args.contains("argc"));
    EXPECT_EQ(get<std::size_t>(args["argc"]), 3);

    format_args_ref args_ref(args);
    EXPECT_TRUE(args_ref.contains("argc"));
    EXPECT_EQ(get<std::size_t>(args_ref["argc"]), 3);
}

TEST(format_args, ref)
{
    using namespace papilio;
    using namespace std::literals;

    {
        dynamic_format_args underlying_fmt_args;
        underlying_fmt_args.emplace("named"_a = "value"s);

        format_args_ref args_ref(underlying_fmt_args);

        EXPECT_EQ(args_ref.indexed_size(), 0);
        EXPECT_EQ(args_ref.named_size(), 1);
        EXPECT_TRUE(args_ref.contains("named"));

        format_args_ref new_ref(args_ref);

        EXPECT_EQ(new_ref.indexed_size(), 0);
        EXPECT_EQ(new_ref.named_size(), 1);
        EXPECT_TRUE(new_ref.contains("named"));

        EXPECT_EQ(get<std::string>(new_ref.get("named")), "value");
    }

    [](auto underlying_fmt_args)
    {
        format_args_ref args_ref(underlying_fmt_args);

        EXPECT_EQ(args_ref.indexed_size(), 2);
        EXPECT_EQ(args_ref.named_size(), 1);
        EXPECT_TRUE(args_ref.contains("name"));

        EXPECT_EQ(get<std::string>(args_ref.get("name")), "scene");
    }(make_format_args(182375, 182376, "name"_a = "scene"s));
}

TEST(format_parse_context, char)
{
    using namespace papilio;

    {
        dynamic_format_args args;
        args.append(0, 1, 2);
        args.emplace("value"_a = 0);

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
        args.append(0, 1, 2);
        args.emplace("value"_a = 0);

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

    papilio::println(
        std::cerr,
        "Papilio Version = {}\n"
        "PAPILIO_CPLUSPLUS = {}",
        papilio::get_version(),
        PAPILIO_CPLUSPLUS
    );

    return RUN_ALL_TESTS();
}
