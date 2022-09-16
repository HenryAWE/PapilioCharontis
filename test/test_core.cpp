#include <gtest/gtest.h>
#include <papilio/core.hpp>


TEST(TestCore, Utilities)
{
    using namespace std::literals;

    {
        using namespace papilio;

        static_assert(string_like<char*>);
        static_assert(string_like<const char*>);
        static_assert(string_like<char[10]>);
        static_assert(string_like<const char[10]>);
        static_assert(string_like<std::string>);
        static_assert(string_like<std::string_view>);
    }

    {
        using namespace papilio;

        const std::string str_val = "hello world";
        const auto a_0 = arg("string", str_val);
        EXPECT_STREQ(a_0.name, "string");
        EXPECT_EQ(a_0.value, "hello world");

        const int int_val = 1;
        const auto a_1 = "integer"_a = int_val;
        EXPECT_STREQ(a_1.name, "integer");
        EXPECT_EQ(a_1.value, int_val);
    }
}
TEST(TestCore, AttributeName)
{
    using namespace std::literals;
    using namespace papilio;
    
    auto attr = attribute_name("name");

    EXPECT_EQ("name"sv, attr);
    EXPECT_EQ(attr, "name"sv);

    EXPECT_TRUE(attribute_name::validate("name"));
    EXPECT_TRUE(attribute_name::validate("_name"));
    EXPECT_TRUE(attribute_name::validate("NAME"));
    EXPECT_TRUE(attribute_name::validate("NAME_123"));

    EXPECT_FALSE(attribute_name::validate("123name"));
    EXPECT_FALSE(attribute_name::validate("NAME name"));
    EXPECT_FALSE(attribute_name::validate("$name"));
    EXPECT_FALSE(attribute_name::validate("!name"));
}
TEST(TestCore, FormatArg)
{
    using namespace std::literals;
    using namespace papilio;

    {
        papilio::format_arg fmt_arg("test");

        EXPECT_EQ(get<std::size_t>(fmt_arg.attribute("length")), std::string("test").length());
        EXPECT_EQ(get<char>(fmt_arg.index(0)), 't');
        EXPECT_EQ(get<char>(fmt_arg.index(1)), 'e');
        EXPECT_EQ(get<char>(fmt_arg.index(2)), 's');
        EXPECT_EQ(get<char>(fmt_arg.index(3)), 't');
        EXPECT_THROW(get<char>(fmt_arg.index(4)), std::out_of_range);
    }
}
TEST(TestCore, FormatContext)
{

}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
