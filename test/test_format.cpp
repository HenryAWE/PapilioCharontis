#include <gtest/gtest.h>
#include <papilio/format.hpp>


TEST(TestFormat, FormatParser)
{
    using namespace papilio;

    {
        format_parser p;
        dynamic_format_arg_store store;
        p.parse("plain text", store);

        auto seg = p.segments();
        EXPECT_EQ(seg.size(), 1);

        EXPECT_TRUE(std::holds_alternative<format_parser::plain_text>(seg[0]));
        EXPECT_EQ(
            std::get<format_parser::plain_text>(seg[0]).get(),
            "plain text"
        );
    }

    {
        format_parser p;
        dynamic_format_arg_store store;
        p.parse("{{plain text 2}}", store);

        auto seg = p.segments();
        EXPECT_EQ(seg.size(), 1);

        EXPECT_TRUE(std::holds_alternative<format_parser::plain_text>(seg[0]));
        EXPECT_EQ(
            std::get<format_parser::plain_text>(seg[0]).get(),
            "{plain text 2}"
        );
    }

    {
        format_parser p;
        dynamic_format_arg_store store;
        p.parse("[[plain text 3]]", store);

        auto seg = p.segments();
        EXPECT_EQ(seg.size(), 1);

        EXPECT_TRUE(std::holds_alternative<format_parser::plain_text>(seg[0]));
        EXPECT_EQ(
            std::get<format_parser::plain_text>(seg[0]).get(),
            "[plain text 3]"
        );
    }

    {
        format_parser p;
        dynamic_format_arg_store store(true);
        p.parse("formatting {}", store);

        auto seg = p.segments();
        EXPECT_EQ(seg.size(), 2);

        EXPECT_TRUE(std::holds_alternative<format_parser::plain_text>(seg[0]));
        EXPECT_EQ(
            std::get<format_parser::plain_text>(seg[0]).get(),
            "formatting "
        );

        EXPECT_TRUE(std::holds_alternative<format_parser::replacement_field>(seg[1]));
        auto& field = std::get<format_parser::replacement_field>(seg[1]);
        EXPECT_TRUE(field.get_arg().is_index());
        EXPECT_EQ(field.get_arg().as_index(), 0);
        EXPECT_TRUE(field.get_access().empty());
        EXPECT_EQ(field.get_fmt(), "");
    }

    {
        format_parser p;
        dynamic_format_arg_store store(true);
        p.parse("{name.length:02d}", store);

        auto seg = p.segments();
        EXPECT_EQ(seg.size(), 1);

        EXPECT_TRUE(std::holds_alternative<format_parser::replacement_field>(seg[0]));
        auto& field = std::get<format_parser::replacement_field>(seg[0]);
        EXPECT_TRUE(field.get_arg().is_key());
        EXPECT_EQ(field.get_arg().as_key(), "name");
        EXPECT_FALSE(field.get_access().empty());
        EXPECT_EQ(field.get_fmt(), "02d");

        std::string test = "test";
        EXPECT_EQ(
            get<std::size_t>(field.get_access().access(format_arg(test))),
            4
        );
    }

    {
        format_parser p;
        dynamic_format_arg_store store(true);
        p.parse("test script [if $0: 'true']", store);

        auto seg = p.segments();
        EXPECT_EQ(seg.size(), 2);

        EXPECT_TRUE(std::holds_alternative<format_parser::plain_text>(seg[0]));
        EXPECT_EQ(
            std::get<format_parser::plain_text>(seg[0]).get(),
            "test script "
        );

        dynamic_format_arg_store store_2(true);
        script::executor::context ctx(store_2);
        EXPECT_TRUE(std::holds_alternative<format_parser::script_block>(seg[1]));
        EXPECT_EQ(
            std::get<format_parser::script_block>(seg[1])(ctx),
            "true"
        );
    }

    {
        format_parser p;
        dynamic_format_arg_store store(true);
        p.parse("{} apple[if $0 != 1: 's']", store);

        auto seg = p.segments();
        EXPECT_EQ(seg.size(), 3);

        EXPECT_TRUE(std::holds_alternative<format_parser::replacement_field>(seg[0]));

        EXPECT_TRUE(std::holds_alternative<format_parser::plain_text>(seg[1]));
        EXPECT_EQ(
            std::get<format_parser::plain_text>(seg[1]).get(),
            " apple"
        );

        dynamic_format_arg_store store_2(0);
        script::executor::context ctx(store_2);
        EXPECT_TRUE(std::holds_alternative<format_parser::script_block>(seg[2]));
        EXPECT_EQ(
            std::get<format_parser::script_block>(seg[2])(ctx),
            "s"
        );
    }
}
TEST(TestFormat, Formatter)
{
    using namespace papilio;

    {
        static_assert(formatter_traits<int>::has_formatter());

        auto integer_formatter = []<std::integral T>(T val, std::string_view fmt)->std::string
        {
            using namespace papilio;

            dynamic_format_arg_store store(val);
            format_spec_parse_context parse_ctx(fmt, store);

            formatter<T> f;
            f.parse(parse_ctx);

            std::string result;
            basic_format_context fmt_ctx(std::back_inserter(result), store);
            f.format(val, fmt_ctx);
            return result;
        };

        EXPECT_EQ(integer_formatter(0, ""), "0");
        EXPECT_EQ(integer_formatter(10, ""), "10");
        EXPECT_EQ(integer_formatter(0u, ""), "0");
        EXPECT_EQ(integer_formatter(10u, ""), "10");

        EXPECT_EQ(integer_formatter(0xF, "x"), "f");
        EXPECT_EQ(integer_formatter(0b10, "b"), "10");
        EXPECT_EQ(integer_formatter(017, "o"), "17");
    }

    {
        auto dyn_formatter = [](format_arg fmt_arg, std::string_view fmt)
        {
            dynamic_format_arg_store store(fmt_arg);
            format_spec_parse_context parse_ctx(fmt, store);

            std::string result;
            basic_format_context fmt_ctx(std::back_inserter(result), store);
            fmt_arg.format(parse_ctx, fmt_ctx);
            return result;
        };

        format_arg a1 = 1;
        EXPECT_EQ(dyn_formatter(a1, ""), "1");
        format_arg a2 = 0xF;
        EXPECT_EQ(dyn_formatter(a2, "x"), "f");
    }
}
namespace test_format
{
    class my_value
    {
    public:
        char ch;
        int count;
    };
}
namespace papilio
{
    template <>
    class formatter<test_format::my_value>
    {
    public:
        void parse(format_spec_parse_context& spec)
        {
            std::string_view view(spec.begin(), spec.end());
            if(view == "s")
                m_as_str = true;
        }
        template <typename Context>
        void format(const test_format::my_value& val, Context& ctx)
        {
            format_context_traits traits(ctx);
            if(m_as_str)
                traits.append(val.ch, val.count);
            else
            {
                std::string result;
                result += '(';
                result += val.ch;
                result += ", ";
                result += std::to_string(val.count);
                result += ')';
                traits.append(result);
            }
        }

    private:
        bool m_as_str = false;
    };
}
TEST(TestFormat, VFormat)
{
    using namespace papilio;

    {
        std::string result = vformat("plain text", make_format_args());
        EXPECT_EQ(result, "plain text");

        result = vformat("Count: {}", make_format_args(10));
        EXPECT_EQ(result, "Count: 10");

        result = vformat("Bin {0:b}, Dec {0}, Hex {0:x}", make_format_args(0xF));
        EXPECT_EQ(result, "Bin 1111, Dec 15, Hex f");

        std::string_view apple_fmt = "{} apple[if $0 != 1: 's']";
        result = vformat(apple_fmt, make_format_args(1));
        EXPECT_EQ(result, "1 apple");
        result = vformat(apple_fmt, make_format_args(2));
        EXPECT_EQ(result, "2 apples");
    }
}
TEST(TestFormat, CustomType)
{
    using namespace papilio;
    using test_format::my_value;

    {
        static_assert(formatter_traits<my_value>::has_formatter());

        my_value my_val('A', 4);
        format_arg fmt_arg = my_val;

        auto dyn_formatter = [](format_arg fmt_arg, std::string_view fmt)
        {
            dynamic_format_arg_store store(fmt_arg);
            format_spec_parse_context parse_ctx(fmt, store);

            std::string result;
            basic_format_context fmt_ctx(std::back_inserter(result), store);
            fmt_arg.format(parse_ctx, fmt_ctx);
            return result;
        };

        EXPECT_EQ(dyn_formatter(fmt_arg, "s"), "AAAA");
        EXPECT_EQ(dyn_formatter(fmt_arg, ""), "(A, 4)");
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
