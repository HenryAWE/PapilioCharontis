#include <gtest/gtest.h>
#include <papilio/format.hpp>
#include <vector>
#include <iostream>

namespace test_format
{
template <std::integral T>
void test_int_formatter()
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS format("{}", T(0)), "0");
    EXPECT_EQ(PAPILIO_NS format(L"{}", T(0)), L"0");

    EXPECT_EQ(PAPILIO_NS format("{:6}", T(42)), "    42");
    EXPECT_EQ(PAPILIO_NS format(L"{:6}", T(42)), L"    42");

    EXPECT_EQ(PAPILIO_NS format("{0:},{0:+},{0:-},{0: }", T(1)), "1,+1,1, 1");
    EXPECT_EQ(PAPILIO_NS format(L"{0:},{0:+},{0:-},{0: }", T(1)), L"1,+1,1, 1");
    if constexpr(std::is_signed_v<T>)
    {
        EXPECT_EQ(PAPILIO_NS format("{0:},{0:+},{0:-},{0: }", T(-1)), "-1,-1,-1,-1");
        EXPECT_EQ(PAPILIO_NS format(L"{0:},{0:+},{0:-},{0: }", T(-1)), L"-1,-1,-1,-1");
    }

    EXPECT_EQ(PAPILIO_NS format("{:+06d}", T(42)), "+00042");
    EXPECT_EQ(PAPILIO_NS format(L"{:+06d}", T(42)), L"+00042");
    EXPECT_EQ(PAPILIO_NS format("{:#06x}", T(0xa)), "0x000a");
    EXPECT_EQ(PAPILIO_NS format(L"{:#06x}", T(0xa)), L"0x000a");
    if constexpr(std::is_signed_v<T>)
    {
        EXPECT_EQ(PAPILIO_NS format("{:<06}", T(-42)), "-42   ");
        EXPECT_EQ(PAPILIO_NS format(L"{:<06}", T(-42)), L"-42   ");
    }

    EXPECT_EQ(PAPILIO_NS format("{:{}d}", T(42), 4), "  42");
    EXPECT_EQ(PAPILIO_NS format(L"{:{}d}", T(42), 4), L"  42");

    EXPECT_EQ(PAPILIO_NS format("{:d>6}", T(42)), "dddd42");
    EXPECT_EQ(PAPILIO_NS format(L"{:d>6}", T(42)), L"dddd42");

    EXPECT_EQ(PAPILIO_NS format("{:^5c}", T(97)), "  a  ");
    EXPECT_EQ(PAPILIO_NS format(L"{:^5c}", T(97)), L"  a  ");
}

template <std::floating_point T>
void test_float_formatter()
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS format("{}", T(3.14L)), "3.14");
    EXPECT_EQ(PAPILIO_NS format(L"{}", T(3.14L)), L"3.14");

    {
        const T inf = std::numeric_limits<T>::infinity();

        EXPECT_EQ(PAPILIO_NS format("{0:},{0:+},{0:-},{0: }", inf), "inf,+inf,inf, inf");
        EXPECT_EQ(PAPILIO_NS format(L"{0:},{0:+},{0:-},{0: }", inf), L"inf,+inf,inf, inf");

        EXPECT_EQ(PAPILIO_NS format("{0:},{0:+},{0:-},{0: }", -inf), "-inf,-inf,-inf,-inf");
        EXPECT_EQ(PAPILIO_NS format(L"{0:},{0:+},{0:-},{0: }", -inf), L"-inf,-inf,-inf,-inf");
    }

    {
        const T nan = std::numeric_limits<T>::quiet_NaN();

        EXPECT_EQ(PAPILIO_NS format("{}", nan), "nan");
        EXPECT_EQ(PAPILIO_NS format(L"{}", nan), L"nan");
    }
}
} // namespace test_format

TEST(formatter, int)
{
    using test_format::test_int_formatter;

    test_int_formatter<int>();
    test_int_formatter<unsigned int>();
    test_int_formatter<long long int>();
    test_int_formatter<unsigned long long int>();
}

TEST(formatter, float)
{
    using test_format::test_float_formatter;

    test_float_formatter<float>();
    test_float_formatter<double>();
    test_float_formatter<long double>();

    using namespace papilio;

    const float pi = 3.14f;

    EXPECT_EQ(PAPILIO_NS format("{:10f}", pi), "  3.140000");
    EXPECT_EQ(PAPILIO_NS format(L"{:10f}", pi), L"  3.140000");

    EXPECT_EQ(PAPILIO_NS format("{:.5f}", pi), "3.14000");
    EXPECT_EQ(PAPILIO_NS format(L"{:.5f}", pi), L"3.14000");

    EXPECT_EQ(PAPILIO_NS format("{:10.5f}", pi), "   3.14000");
    EXPECT_EQ(PAPILIO_NS format(L"{:10.5f}", pi), L"   3.14000");
}

TEST(formatter, codepoint)
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS format("{}", 'a'), "a");
    EXPECT_EQ(PAPILIO_NS format(L"{}", L'a'), L"a");

    EXPECT_EQ(PAPILIO_NS format("{:d}", 'a'), "97");
    EXPECT_EQ(PAPILIO_NS format(L"{:d}", L'a'), L"97");
}

TEST(formatter, string)
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS format("{}", "hello"), "hello");
    EXPECT_EQ(PAPILIO_NS format(L"{}", L"hello"), L"hello");

    EXPECT_EQ(PAPILIO_NS format("{:s}", "hello"), "hello");
    EXPECT_EQ(PAPILIO_NS format(L"{:s}", L"hello"), L"hello");

    EXPECT_EQ(PAPILIO_NS format("{:.5}", "hello!"), "hello");
    EXPECT_EQ(PAPILIO_NS format(L"{:.5}", L"hello!"), L"hello");

    EXPECT_EQ(PAPILIO_NS format("{:^8.5}", "hello!"), " hello  ");
    EXPECT_EQ(PAPILIO_NS format(L"{:^8.5}", L"hello!"), L" hello  ");
}

namespace test_format
{
template <typename CharT>
class yes_no_numpunct : public std::numpunct<CharT>
{
    using base = std::numpunct<CharT>;

public:
    using string_type = base::string_type;

protected:
    string_type do_truename() const override
    {
        const CharT yes_str[] = {'y', 'e', 's'};
        return string_type(yes_str, std::size(yes_str));
    }

    string_type do_falsename() const override
    {
        const CharT no_str[] = {'n', 'o'};
        return string_type(no_str, std::size(no_str));
    }
};

template <typename CharT = char>
std::locale attach_yes_no(const std::locale& loc = std::locale::classic())
{
    return std::locale(loc, new yes_no_numpunct<CharT>());
}
} // namespace test_format

TEST(formatter, bool)
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS format("{}", true), "true");
    EXPECT_EQ(PAPILIO_NS format(L"{}", true), L"true");
    EXPECT_EQ(PAPILIO_NS format("{}", false), "false");
    EXPECT_EQ(PAPILIO_NS format(L"{}", false), L"false");

    EXPECT_EQ(PAPILIO_NS format("{:d}", true), "1");
    EXPECT_EQ(PAPILIO_NS format(L"{:d}", true), L"1");
    EXPECT_EQ(PAPILIO_NS format("{:#x}", true), "0x1");
    EXPECT_EQ(PAPILIO_NS format(L"{:#x}", true), L"0x1");

    {
        auto loc = test_format::attach_yes_no<char>();
        EXPECT_EQ(PAPILIO_NS format(loc, "{:L}", true), "yes");
        EXPECT_EQ(PAPILIO_NS format(loc, "{:L}", false), "no");
    }

    {
        auto loc = test_format::attach_yes_no<wchar_t>();
        EXPECT_EQ(PAPILIO_NS format(loc, L"{:L}", true), L"yes");
        EXPECT_EQ(PAPILIO_NS format(loc, L"{:L}", false), L"no");
    }
}

TEST(format, plain_text)
{
    using namespace papilio;

    EXPECT_EQ(PAPILIO_NS format(""), "");
    EXPECT_EQ(PAPILIO_NS format("plain text"), "plain text");
    EXPECT_EQ(PAPILIO_NS format("{{plain text}}"), "{plain text}");
}

namespace test_format
{
struct unformattable_type
{};

struct large_unformattable_type
{
    char dummy[1024]{};
};

struct custom_type
{
    int val = 0;
};

struct large_custom_type
{
    int val = 0;

    large_custom_type(int v)
        : val(v) {}

    large_custom_type(const large_custom_type&) = default;

private:
    char dummy[1024]{};
};
} // namespace test_format

namespace papilio
{
template <>
class formatter<test_format::custom_type>
{
public:
    template <typename ParseContext>
    auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const test_format::custom_type& v, FormatContext& ctx) const
    {
        return format_to(ctx.out(), "custom_type.val={}", v.val);
    }
};

template <>
class formatter<test_format::large_custom_type>
{
public:
    template <typename ParseContext>
    auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const test_format::large_custom_type& v, FormatContext& ctx) const
    {
        return format_to(ctx.out(), "large_custom_type.val={}", v.val);
    }
};
} // namespace papilio

TEST(format, custom_type)
{
    using namespace papilio;
    using namespace test_format;

    static_assert(!formattable<unformattable_type>);
    static_assert(!formattable<large_unformattable_type>);
    static_assert(formattable<custom_type>);
    static_assert(formattable<large_custom_type>);

    {
        format_arg fmt_arg(unformattable_type{});
        EXPECT_FALSE(fmt_arg.is_formattable());
    }

    {
        format_arg fmt_arg(large_unformattable_type{});
        EXPECT_FALSE(fmt_arg.is_formattable());
    }

    {
        format_arg fmt_arg(custom_type(182376));
        EXPECT_TRUE(fmt_arg.is_formattable());
    }

    {
        format_arg fmt_arg(large_custom_type(182376));
        EXPECT_TRUE(fmt_arg.is_formattable());
    }

    EXPECT_EQ(format("{}", custom_type(182376)), "custom_type.val=182376");
    EXPECT_EQ(format("{}", large_custom_type(182376)), "large_custom_type.val=182376");
}

TEST(format, format_to)
{
    using namespace papilio;

    {
        std::vector<char> result;
        auto it = format_to(std::back_inserter(result), "vec");
        *it = '\0';
        EXPECT_EQ(result.size(), 4);
        EXPECT_STREQ(result.data(), "vec");
    }

    {
        auto loc = test_format::attach_yes_no();

        std::vector<char> result;
        auto it = format_to(std::back_inserter(result), loc, "{:L}", true);
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
        auto result = format_to_n(str.begin(), str.size(), "hello world");

        EXPECT_EQ(result.out, str.begin() + str.size());
        EXPECT_EQ(result.size, str.size());
        EXPECT_EQ(result.size, 5);
        EXPECT_EQ(str, "hello");
    }

    {
        auto loc = test_format::attach_yes_no();

        std::string str;
        str.resize(4);
        auto result = format_to_n(str.begin(), str.size(), loc, "{:L}!!", true);

        EXPECT_EQ(result.out, str.begin() + str.size());
        EXPECT_EQ(result.size, str.size());
        EXPECT_EQ(result.size, 4);
        EXPECT_EQ(str, "yes!");
    }
}

TEST(format, exception)
{
    using namespace papilio;

    EXPECT_THROW((void)PAPILIO_NS format("{"), papilio::invalid_format);
    EXPECT_THROW((void)PAPILIO_NS format("}"), papilio::invalid_format);
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
        auto it = format_to(std::back_inserter(result), L"vec");
        *it = L'\0';
        EXPECT_EQ(result.size(), 4);
        EXPECT_STREQ(result.data(), L"vec");
    }

    {
        auto loc = test_format::attach_yes_no<wchar_t>();

        std::vector<wchar_t> result;
        auto it = format_to(std::back_inserter(result), loc, L"{:L}", true);
        *it = L'\0';
        EXPECT_EQ(result.size(), 4);
        EXPECT_STREQ(result.data(), L"yes");
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

    {
        auto loc = test_format::attach_yes_no<wchar_t>();

        std::wstring str;
        str.resize(4);
        auto result = format_to_n(str.begin(), str.size(), loc, L"{:L}!!", true);

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
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
