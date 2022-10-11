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

        script::executor::context ctx(dynamic_format_arg_store(true));
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

        script::executor::context ctx(dynamic_format_arg_store(0));
        EXPECT_TRUE(std::holds_alternative<format_parser::script_block>(seg[2]));
        EXPECT_EQ(
            std::get<format_parser::script_block>(seg[2])(ctx),
            "s"
        );
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
