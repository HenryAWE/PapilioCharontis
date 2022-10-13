#include <gtest/gtest.h>
#include <papilio/core.hpp>


TEST(TestCore, Variable)
{
    using namespace papilio;
    using namespace script;

    static_assert(is_variable_type<variable::int_type>);
    static_assert(is_variable_type<variable::float_type>);
    static_assert(is_variable_type<variable::string_type>);

    {
        variable var = 10;
        EXPECT_TRUE(var.holds<variable::int_type>());
        EXPECT_EQ(var.get<variable::int_type>(), 10);
    }

    {
        variable var = "test";
        EXPECT_TRUE(var.holds<std::string>());
        EXPECT_EQ(var.get<std::string>(), "test");
        EXPECT_TRUE(var.as<bool>());
    }

    {
        variable var1 = 2;
        variable var2 = 2.1f;
        EXPECT_LT(var1, var2);
    }

    {
        variable var1 = 1.0f;
        variable var2 = 1.0f;
        EXPECT_EQ(var1, var2);
    }

    {
        variable var1 = 1.0f;
        variable var2 = 1;
        EXPECT_EQ(var1, var2);
    }

    {
        variable var1 = "1";
        variable var2 = 1;
        EXPECT_NE(var1, var2);
    }
}
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
        EXPECT_EQ(get<std::string_view>(fmt_arg.index(0)), "t");
        EXPECT_EQ(get<std::string_view>(fmt_arg.index(1)), "e");
        EXPECT_EQ(get<std::string_view>(fmt_arg.index(2)), "s");
        EXPECT_EQ(get<std::string_view>(fmt_arg.index(3)), "t");
        EXPECT_EQ(get<std::string_view>(fmt_arg.index(4)), std::string_view());
    }

    {
        papilio::format_arg fmt_arg((const char*)u8"测试");

        EXPECT_EQ(get<std::size_t>(fmt_arg.attribute("length")), 2);
        EXPECT_EQ(get<std::string_view>(fmt_arg.index(0)), (const char*)u8"测");
        EXPECT_EQ(get<std::string_view>(fmt_arg.index(1)), (const char*)u8"试");
        EXPECT_EQ(get<std::string_view>(fmt_arg.index(2)), std::string_view());
    }

    {
        papilio::format_arg fmt_arg("test");

        auto var = fmt_arg.as_variable();
        EXPECT_EQ(var.as<std::string>(), "test");
    }

    {
        papilio::format_arg fmt_arg("long sentence for testing slicing");

        EXPECT_EQ(get<std::string_view>(fmt_arg.index(slice(0, 4))), "long");
        EXPECT_EQ(get<std::string_view>(fmt_arg.index(slice(-7, slice::npos))), "slicing");
        EXPECT_EQ(get<std::string_view>(fmt_arg.index(slice(14, -16))), "for");
        EXPECT_EQ(get<std::string_view>(fmt_arg.index(slice(-slice::npos, -20))), "long sentence");
    }
}
TEST(TestCore, DynamicFormatArgStore)
{
    using namespace papilio;

    {
        dynamic_format_arg_store store(1, "three"_a = 3, 2);

        EXPECT_EQ(store.size(), 2);
        EXPECT_EQ(store.named_size(), 1);

        EXPECT_EQ(get<int>(store[0]), 1);
        EXPECT_EQ(get<int>(store[1]), 2);
        EXPECT_EQ(get<int>(store["three"]), 3);

        store.clear();

        EXPECT_EQ(store.size(), 0);
        EXPECT_EQ(store.named_size(), 0);

        store.emplace('a', 'b', "c"_a = 'c', "d"_a = 'd');

        EXPECT_EQ(store.size(), 2);
        EXPECT_EQ(store.named_size(), 2);

        EXPECT_EQ(get<char>(store[0]), 'a');
        EXPECT_EQ(get<char>(store[1]), 'b');
        EXPECT_EQ(get<char>(store["c"]), 'c');
        EXPECT_EQ(get<char>(store["d"]), 'd');
    }

    {
        dynamic_format_arg_store store(1, 2.0f, "string", "named"_a = "named");

        EXPECT_EQ(get<std::size_t>(store[2].attribute("length")), 6);

        EXPECT_EQ(get<int>(store[indexing_value(0)]), 1);
        EXPECT_FLOAT_EQ(get<float>(store[indexing_value(1)]), 2.0f);
        EXPECT_STREQ(get<const char*>(store[indexing_value("named")]), "named");
    }

    {
        dynamic_format_arg_store store(0, 1, 2, "test1"_a = "test 1", "test2"_a = "test 2");
        EXPECT_EQ(store.size(), 3);
        EXPECT_EQ(store.named_size(), 2);

        for(std::size_t i : { 0, 1, 2 })
            EXPECT_TRUE(store.check(i));
        EXPECT_FALSE(store.check(3));

        EXPECT_TRUE(store.check("test1"));
        EXPECT_TRUE(store.check("test2"));
        EXPECT_FALSE(store.check("test3"));
    }
}
TEST(TestCore, FormatContext)
{
    using namespace papilio;

    {
        format_context ctx;
        ctx.append("1234");
        EXPECT_EQ(ctx.str(), "1234");
    }
}
TEST(TestCore, Formatter)
{
    using namespace papilio;

    auto integer_formatter = []<std::integral T>(T val, std::string_view fmt)->std::string
    {
        using namespace papilio;

        dynamic_format_arg_store store(val);
        format_spec_parse_context parse_ctx(fmt, store);

        formatter<T> f;
        f.parse(parse_ctx);

        format_context f_ctx;
        f.format(val, f_ctx);
        return std::move(f_ctx).str();
    };

    EXPECT_EQ(integer_formatter(0, ""), "0");
    EXPECT_EQ(integer_formatter(10, ""), "10");
    EXPECT_EQ(integer_formatter(0u, ""), "0");
    EXPECT_EQ(integer_formatter(10u, ""), "10");

    EXPECT_EQ(integer_formatter(0xF, "x"), "f");
    EXPECT_EQ(integer_formatter(0b10, "b"), "10");
    EXPECT_EQ(integer_formatter(017, "o"), "17");
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
