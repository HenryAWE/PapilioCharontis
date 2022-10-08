#include <gtest/gtest.h>
#include <papilio/format.hpp>


TEST(TestFormat, FormatParser)
{
    using namespace papilio;

    {
        format_parser p;
        p.parse("plain text");

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
        p.parse("{{plain text 2}}");

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
        p.parse("[[plain text 3]]");

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
        p.parse("formatting {}");

        auto seg = p.segments();
        EXPECT_EQ(seg.size(), 2);

        EXPECT_TRUE(std::holds_alternative<format_parser::plain_text>(seg[0]));
        EXPECT_EQ(
            std::get<format_parser::plain_text>(seg[0]).get(),
            "formatting "
        );

        EXPECT_TRUE(std::holds_alternative<format_parser::replacement_field>(seg[1]));
        EXPECT_EQ(
            std::get<format_parser::replacement_field>(seg[1]).get(),
            ""
        );
    }

    {
        format_parser p;
        p.parse("test script [if $0: 'true']");

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
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
