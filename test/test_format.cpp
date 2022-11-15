#include <gtest/gtest.h>
#include <papilio/format.hpp>


TEST(TestFormat, Formatter)
{
    using namespace papilio;

    {
        static_assert(formatter_traits<int>::has_formatter());
        static_assert(formatter_traits<unsigned int>::has_formatter());
        static_assert(formatter_traits<long>::has_formatter());
        static_assert(formatter_traits<unsigned long>::has_formatter());
        static_assert(formatter_traits<long long>::has_formatter());
        static_assert(formatter_traits<unsigned long long>::has_formatter());

        EXPECT_EQ(format("{}", 0), "0");
        EXPECT_EQ(format("{}", 10), "10");
        EXPECT_EQ(format("{}", 0u), "0");
        EXPECT_EQ(format("{}", 10u), "10");

        EXPECT_EQ(format("{:x}", 0xF), "f");
        EXPECT_EQ(format("{:b}", 0b10), "10");
        EXPECT_EQ(format("{:#b}", 0b10), "0b10");
        EXPECT_EQ(format("{:x}", 0xFF), "ff");
        EXPECT_EQ(format("{:X}", 0xFF), "FF");
        EXPECT_EQ(format("{:o}", 017), "17");
        EXPECT_EQ(format("{:#o}", 017), "0o17");

        EXPECT_EQ(format("{:08x}", 0xff), "000000ff");

        EXPECT_EQ(format("{:#08x}", 0), "0x00000000");
        EXPECT_EQ(format("{0:},{0:+},{0:-},{0: }", 1), "1,+1,1, 1");
        EXPECT_EQ(format("{0:},{0:+},{0:-},{0: }", -1), "-1,-1,-1,-1");
        EXPECT_EQ(format("{:6}", 42), "    42");
        EXPECT_EQ(format("{:06}", 42), "000042");
        EXPECT_EQ(format("{:*<6}", 1), "1*****");
        EXPECT_EQ(format("{:*>6}", 1), "*****1");
        EXPECT_EQ(format("{:*^6}", 1), "**1***");
        EXPECT_EQ(format("{:<06}", -42), "-42   "); // ignore zero

        EXPECT_EQ(format("{:}", U'A'), "A");
        EXPECT_EQ(format("{:*<6}", U'A'), "A*****");
        EXPECT_EQ(format("{:*^6}", U'A'), "**A***");
        EXPECT_EQ(format("{:*>6}", U'A'), "*****A");
        EXPECT_EQ(format("{:c}", U'A'), "A");
        EXPECT_EQ(format("{:}", U'我'), (const char*)u8"我");
        EXPECT_EQ(format("{:*<6}", U'我'), (const char*)u8"我****");
        EXPECT_EQ(format("{:*^6}", U'我'), (const char*)u8"**我**");
        EXPECT_EQ(format("{:*>6}", U'我'), (const char*)u8"****我");
        EXPECT_EQ(format("{:c}", U'我'), (const char*)u8"我");
        EXPECT_EQ(format("{:#x}", U'我'), "0x6211");
        EXPECT_EQ(format("{:d}", U'A'), "65");

        EXPECT_EQ(format("{.length:06}", "ABCDEFGHIJKL"), "000012");
    }

    {
        static_assert(formatter_traits<const void*>::has_formatter());

        EXPECT_EQ(format("{}", nullptr), "0x0");
        EXPECT_EQ(format("{:p}", nullptr), "0x0");
        EXPECT_EQ(format("{:#08p}", nullptr), "0x00000000");
        EXPECT_EQ(format("{:#08p}", (const void*)0xFFFF), "0x0000ffff");
        // redirection to other type is not allowed for pointer
        EXPECT_THROW(format("{:08d}", nullptr), invalid_format);
    }

    {
        static_assert(formatter_traits<string_container>::has_formatter());

        EXPECT_EQ(format("{}", "hello"), "hello");
        EXPECT_EQ(format("{:6}", "hello"), "hello ");
        EXPECT_EQ(format("{:>6}", "hello"), " hello");
        EXPECT_EQ(format("{:^6}", "hello"), "hello ");
        EXPECT_EQ(format("{:.^5s}", (const char*)u8"我"), ".我..");
        EXPECT_EQ(format("{:.5s}", (const char*)u8"我我我"), "我我");
        EXPECT_EQ(format("{:.<5.5s}", (const char*)u8"我我我"), "我我.");
    }

    {
        static_assert(formatter_traits<bool>::has_formatter());

        EXPECT_EQ(format("{}", true), "true");
        EXPECT_EQ(format("{}", false), "false");
        EXPECT_EQ(format("{:s}", true), "true");
        EXPECT_EQ(format("{:s}", false), "false");
        EXPECT_EQ(format("{:d}", true), "1");
        EXPECT_EQ(format("{:d}", false), "0");
        EXPECT_EQ(format("{:#02b}", true), "0b01");
        EXPECT_EQ(format("{:#02b}", false), "0b00");
    }

    {
        static_assert(formatter_traits<float>::has_formatter());
        static_assert(formatter_traits<double>::has_formatter());
        static_assert(formatter_traits<long double>::has_formatter());

        EXPECT_EQ(format("{}", 1.0), "1");
        EXPECT_EQ(format("{}", 1.5f), "1.5");
        EXPECT_EQ(format("{}", 1.5), "1.5");
        EXPECT_EQ(format("{}", 1.5L), "1.5");

        const float pi = 3.14f;
        EXPECT_EQ(format("{:10f}", pi), "  3.140000");
        EXPECT_EQ(format("{:{}f}", pi, 10), "  3.140000");
        EXPECT_EQ(format("{:.5f}", pi), "3.14000");
        EXPECT_EQ(format("{:.{}f}", pi, 5), "3.14000");
        EXPECT_EQ(format("{:10.5f}", pi), "   3.14000");
        EXPECT_EQ(format("{:{}.{}f}", pi, 10, 5), "   3.14000");

        constexpr double inf = std::numeric_limits<double>::infinity();
        constexpr double nan = std::numeric_limits<double>::quiet_NaN();
        EXPECT_EQ(format("{0:},{0:+},{0:-},{0: }", inf), "inf,+inf,inf, inf");
        EXPECT_EQ(format("{0:},{0:+},{0:-},{0: }", nan), "nan,+nan,nan, nan");
    }
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

        result = vformat("length of \"{0}\" is {0.length}", make_format_args("hello"));
        EXPECT_EQ(result, "length of \"hello\" is 5");

        result = vformat("{[:5]:}", make_format_args("hello world"));
        EXPECT_EQ(result, "hello");
        result = vformat("{[-5:]:}", make_format_args("hello world"));
        EXPECT_EQ(result, "world");
        result = vformat("{[0]:}", make_format_args("hello world"));
        EXPECT_EQ(result, "h");
        result = vformat("{[-1]:}", make_format_args("hello world"));
        EXPECT_EQ(result, "d");
    }
}
TEST(TestFormat, FormatTo)
{
    using namespace papilio;

    {
        std::vector<char> buf;

        format_to(std::back_inserter(buf), "plain text");
        EXPECT_EQ(std::string_view(buf.data(), buf.size()), "plain text");

        buf.clear();
        buf.resize(4);
        EXPECT_EQ(format_to(buf.begin(), "{}", 1234), buf.end());
        EXPECT_EQ(std::string_view(buf.data(), 4), "1234");
    }
}
TEST(TestFormat, Format)
{
    using namespace papilio;

    EXPECT_EQ(format("plain text"), "plain text");
    EXPECT_EQ(format("{{}}"), "{}");
    EXPECT_EQ(format("[[]]"), "[]");

    EXPECT_EQ(format("{}", true), "true");
    EXPECT_EQ(format("{}", false), "false");
    EXPECT_EQ(format("{}", nullptr), "0x0");
    EXPECT_EQ(format("{}", (const void*)0xABCD), "0xabcd");
    EXPECT_EQ(format("{.length}", "hello"), "5");

    std::string_view fmt =
        "There "
        "[if $0 != 1: 'are' else: 'is']"
        " {} "
        "apple[if $0 != 1: 's']";
    EXPECT_EQ(format(fmt, 1), "There is 1 apple");
    EXPECT_EQ(format(fmt, 2), "There are 2 apples");

    struct tmp_type {};
    EXPECT_ANY_THROW(format("{}", tmp_type()));
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
TEST(TestFormat, CustomType)
{
    using namespace papilio;
    using test_format::my_value;

    {
        static_assert(formatter_traits<my_value>::has_formatter());

        my_value my_val_a('A', 4);

        EXPECT_EQ(format("{:s}", my_val_a), "AAAA");
        EXPECT_EQ(format("{}", my_val_a), "(A, 4)");

        my_value my_val_b('B', 3);
        EXPECT_EQ(format("{}", my_val_b), "(B, 3)");
        EXPECT_EQ(format("{:s}", my_val_b), "BBB");
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
