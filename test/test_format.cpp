#include <gtest/gtest.h>
#include <papilio/format.hpp>
#include <vector>

TEST(format, plain_text)
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS format(""), "");
    EXPECT_EQ(PAPILIO_NS format("plain text"), "plain text");
    EXPECT_EQ(PAPILIO_NS format("{{plain text}}"), "{plain text}");
}

TEST(format, format_to)
{
    using namespace papilio;

    std::vector<char> result;
    auto it = format_to(std::back_inserter(result), "vec");
    *it = '\0';
    EXPECT_EQ(result.size(), 4);
    EXPECT_STREQ(result.data(), "vec");
}

TEST(format, formatted_size)
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS formatted_size(""), 0);
    EXPECT_EQ(PAPILIO_NS formatted_size("hello"), 5);
    EXPECT_EQ(PAPILIO_NS formatted_size("{{hello}}"), 7);
}

TEST(format, format_to_n)
{
    using namespace papilio;

    {
        std::string str;
        str.resize(5);
        auto result = format_to_n(str.begin(), str.size(), "hello world");

        EXPECT_EQ(result.out, str.begin() + str.size());
        EXPECT_EQ(result.size, str.size());
        EXPECT_EQ(result.size, 5);
        EXPECT_EQ(str, "hello");
    }
}

TEST(format, exception)
{
    using namespace papilio;

    EXPECT_THROW(PAPILIO_NS format("{"), papilio::invalid_format);
    EXPECT_THROW(PAPILIO_NS format("}"), papilio::invalid_format);
}

TEST(format, script)
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS format("{$ {}: 'true'}", 1), "true");
    EXPECT_EQ(PAPILIO_NS format("{$ !{}: 'false'}", 0), "false");
    EXPECT_EQ(PAPILIO_NS format("{$ {val}: 'true'}", "val"_a = 1), "true");

    EXPECT_EQ(PAPILIO_NS format("{$ {} == {}: 'eq'}", 1, 1), "eq");
    EXPECT_EQ(PAPILIO_NS format("{$ {} != {}: 'ne'}", 1, 2), "ne");

    EXPECT_EQ(PAPILIO_NS format("{$ {} > {}: 'gt'}", 2, 1), "gt");
    EXPECT_EQ(PAPILIO_NS format("{$ {} < {}: 'lt'}", 1, 2), "lt");

    EXPECT_EQ(PAPILIO_NS format("{$ {} >= {}: 'ge'}", 2, 1), "ge");
    EXPECT_EQ(PAPILIO_NS format("{$ {} <= {}: 'le'}", 1, 2), "le");
    EXPECT_EQ(PAPILIO_NS format("{$ {} >= {}: 'ge'}", 1, 1), "ge");
    EXPECT_EQ(PAPILIO_NS format("{$ {} <= {}: 'le'}", 1, 1), "le");
}

TEST(format, composite)
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS format("{} {}", 182375, 182376), "182375 182376");

    EXPECT_EQ(PAPILIO_NS format("{.length:*>4}", "hello"), "***5");
    EXPECT_EQ(PAPILIO_NS format("length is {.length}", "hello"), "length is 5");

    {
        std::string_view fmt = "{0} warning{${0}>1:'s'}";

        EXPECT_EQ(PAPILIO_NS format(fmt, 1), "1 warning");
        EXPECT_EQ(PAPILIO_NS format(fmt, 2), "2 warnings");
    }

    {
        std::string_view fmt =
            "There"
            " {${0} != 1: 'are' : 'is'} "
            "{0}"
            " apple{${0} != 1: 's'}";

        EXPECT_EQ(PAPILIO_NS format(fmt, 1), "There is 1 apple");
        EXPECT_EQ(PAPILIO_NS format(fmt, 2), "There are 2 apples");
    }
}

TEST(format, wchar_t)
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS format(L""), L"");
    EXPECT_EQ(PAPILIO_NS format(L"plain text"), L"plain text");
    EXPECT_EQ(PAPILIO_NS format(L"{{plain text}}"), L"{plain text}");

    EXPECT_EQ(PAPILIO_NS format(L"{.length:*>4}", L"hello"), L"***5");
    EXPECT_EQ(PAPILIO_NS format(L"length is {.length}", L"hello"), L"length is 5");

    {
        std::vector<wchar_t> result;
        auto it = format_to(std::back_inserter(result), L"vec");
        *it = L'\0';
        EXPECT_EQ(result.size(), 4);
        EXPECT_STREQ(result.data(), L"vec");
    }

    {
        std::wstring str;
        str.resize(5);
        auto result = format_to_n(str.begin(), str.size(), L"hello world");

        EXPECT_EQ(result.out, str.begin() + str.size());
        EXPECT_EQ(result.size, str.size());
        EXPECT_EQ(result.size, 5);
        EXPECT_EQ(str, L"hello");
    }

    EXPECT_EQ(PAPILIO_NS formatted_size(L""), 0);
    EXPECT_EQ(PAPILIO_NS formatted_size(L"hello"), 5);
    EXPECT_EQ(PAPILIO_NS formatted_size(L"{{hello}}"), 7);

    {
        std::wstring_view fmt = L"{0} warning{${0}>1:'s'}";

        EXPECT_EQ(PAPILIO_NS format(fmt, 1), L"1 warning");
        EXPECT_EQ(PAPILIO_NS format(fmt, 2), L"2 warnings");
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
