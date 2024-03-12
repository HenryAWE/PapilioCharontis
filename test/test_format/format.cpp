#include <gtest/gtest.h>
#if __has_include(<format>)
#    include <format> // Test ADL-proof
#endif
#include <papilio/format.hpp>
#include <vector>
#include <iostream>
#include "test_format.hpp"

TEST(format, plain_text)
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS format(""), "");
    EXPECT_EQ(PAPILIO_NS format("plain text"), "plain text");
    EXPECT_EQ(PAPILIO_NS format("{{plain text}}"), "{plain text}");

    EXPECT_EQ(PAPILIO_NS format(L""), L"");
    EXPECT_EQ(PAPILIO_NS format(L"plain text"), L"plain text");
    EXPECT_EQ(PAPILIO_NS format(L"{{plain text}}"), L"{plain text}");
}

TEST(format, format_to)
{
    using namespace papilio;

    {
        std::vector<char> result;
        auto it = PAPILIO_NS format_to(std::back_inserter(result), "vec");
        *it = '\0';
        EXPECT_EQ(result.size(), 4);
        EXPECT_STREQ(result.data(), "vec");
    }

    {
        auto loc = test_format::attach_yes_no();

        std::vector<char> result;
        auto it = PAPILIO_NS format_to(std::back_inserter(result), loc, "{:L}", true);
        *it = '\0';
        EXPECT_EQ(result.size(), 4);
        EXPECT_STREQ(result.data(), "yes");
    }
}

TEST(format, formatted_size)
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS formatted_size(""), 0);
    EXPECT_EQ(PAPILIO_NS formatted_size("hello"), 5);
    EXPECT_EQ(PAPILIO_NS formatted_size("{{hello}}"), 7);

    {
        auto loc = test_format::attach_yes_no();
        EXPECT_EQ(PAPILIO_NS formatted_size(loc, "{:L}", true), 3);
    }
}

TEST(format, format_to_n)
{
    using namespace papilio;

    {
        std::string str;
        str.resize(5);
        auto result = PAPILIO_NS format_to_n(str.begin(), str.size(), "hello world");

        EXPECT_EQ(result.out, str.begin() + str.size());
        EXPECT_EQ(result.size, str.size());
        EXPECT_EQ(result.size, 5);
        EXPECT_EQ(str, "hello");
    }

    {
        auto loc = test_format::attach_yes_no();

        std::string str;
        str.resize(4);
        auto result = PAPILIO_NS format_to_n(str.begin(), str.size(), loc, "{:L}!!", true);

        EXPECT_EQ(result.out, str.begin() + str.size());
        EXPECT_EQ(result.size, str.size());
        EXPECT_EQ(result.size, 4);
        EXPECT_EQ(str, "yes!");
    }
}

TEST(format, exception)
{
    using namespace papilio;

    EXPECT_THROW((void)PAPILIO_NS format("{"), papilio::format_error);
    EXPECT_THROW((void)PAPILIO_NS format("}"), papilio::format_error);
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

    {
        std::string_view script = "{$ {}: 'a': ${}: 'b' : 'c'}";

        EXPECT_EQ(PAPILIO_NS format(script, true, true), "a");
        EXPECT_EQ(PAPILIO_NS format(script, true, false), "a");
        EXPECT_THROW((void)PAPILIO_NS format(script, true), std::out_of_range);

        EXPECT_EQ(PAPILIO_NS format(script, false, true), "b");
        EXPECT_EQ(PAPILIO_NS format(script, false, false), "c");
    }

    {
        std::string_view script = "{$ {}: 'a': ${}: 'b' : ${} : 'c'}";

        EXPECT_EQ(PAPILIO_NS format(script, true, true, false), "a");
        EXPECT_EQ(PAPILIO_NS format(script, true, false, false), "a");

        EXPECT_EQ(PAPILIO_NS format(script, false, true, true), "b");
        EXPECT_EQ(PAPILIO_NS format(script, false, false, true), "c");
        EXPECT_EQ(PAPILIO_NS format(script, false, false, false), "");
    }
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

    {
        std::string_view fmt = "{${0}==0: 'zero' : {0}}";

        EXPECT_EQ(PAPILIO_NS format(fmt, 0), "zero");
        EXPECT_EQ(PAPILIO_NS format(fmt, 1), "1");
        EXPECT_EQ(PAPILIO_NS format(fmt, 2), "2");
    }
}

namespace test_format
{
class stream_only
{
public:
    friend std::ostream& operator<<(std::ostream& os, const stream_only&)
    {
        os << "stream only";
        return os;
    }

    friend std::wostream& operator<<(std::wostream& os, const stream_only&)
    {
        os << L"stream only";
        return os;
    }
};
} // namespace test_format

TEST(format, ostream_compat)
{
    using namespace papilio;

    {
        test_format::stream_only val;

        EXPECT_EQ(PAPILIO_NS format("{}", val), "stream only");
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
        auto it = PAPILIO_NS format_to(std::back_inserter(result), L"vec");
        *it = L'\0';
        EXPECT_EQ(result.size(), 4);
        EXPECT_STREQ(result.data(), L"vec");
    }

    {
        auto loc = test_format::attach_yes_no<wchar_t>();

        std::vector<wchar_t> result;
        auto it = PAPILIO_NS format_to(std::back_inserter(result), loc, L"{:L}", true);
        *it = L'\0';
        EXPECT_EQ(result.size(), 4);
        EXPECT_STREQ(result.data(), L"yes");
    }

    {
        std::wstring str;
        str.resize(5);
        auto result = PAPILIO_NS format_to_n(str.begin(), str.size(), L"hello world");

        EXPECT_EQ(result.out, str.begin() + str.size());
        EXPECT_EQ(result.size, str.size());
        EXPECT_EQ(result.size, 5);
        EXPECT_EQ(str, L"hello");
    }

    {
        auto loc = test_format::attach_yes_no<wchar_t>();

        std::wstring str;
        str.resize(4);
        auto result = PAPILIO_NS format_to_n(str.begin(), str.size(), loc, L"{:L}!!", true);

        EXPECT_EQ(result.out, str.begin() + str.size());
        EXPECT_EQ(result.size, str.size());
        EXPECT_EQ(result.size, 4);
        EXPECT_EQ(str, L"yes!");
    }

    EXPECT_EQ(PAPILIO_NS formatted_size(L""), 0);
    EXPECT_EQ(PAPILIO_NS formatted_size(L"hello"), 5);
    EXPECT_EQ(PAPILIO_NS formatted_size(L"{{hello}}"), 7);

    {
        test_format::stream_only val;

        EXPECT_EQ(PAPILIO_NS format(L"{}", val), L"stream only");
    }

    {
        std::wstring_view fmt = L"{0} warning{${0}>1:'s'}";

        EXPECT_EQ(PAPILIO_NS format(fmt, 1), L"1 warning");
        EXPECT_EQ(PAPILIO_NS format(fmt, 2), L"2 warnings");
    }

    {
        std::wstring_view fmt = L"{${0}==0: 'zero' : {0}}";

        EXPECT_EQ(PAPILIO_NS format(fmt, 0), L"zero");
        EXPECT_EQ(PAPILIO_NS format(fmt, 1), L"1");
        EXPECT_EQ(PAPILIO_NS format(fmt, 2), L"2");
    }

    {
        std::wstring_view script = L"{$ {}: 'a': ${}: 'b' : 'c'}";

        EXPECT_EQ(PAPILIO_NS format(script, true, true), L"a");
        EXPECT_EQ(PAPILIO_NS format(script, true, false), L"a");
        EXPECT_THROW((void)PAPILIO_NS format(script, true), std::out_of_range);

        EXPECT_EQ(PAPILIO_NS format(script, false, true), L"b");
        EXPECT_EQ(PAPILIO_NS format(script, false, false), L"c");
    }
}
