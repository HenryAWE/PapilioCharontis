#include <gtest/gtest.h>
#include <list>
#include <map>
#include <papilio/core.hpp>


TEST(TestCore, Utilities)
{
    using namespace std::literals;

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
        format_arg arg('a');
        EXPECT_TRUE(arg.holds<utf::codepoint>());
        EXPECT_EQ(get<utf::codepoint>(arg), U'a');
    }

    {
        format_arg arg(1);
        EXPECT_TRUE(arg.holds<int>());
        EXPECT_EQ(get<int>(arg), 1);
    }

    {
        format_arg arg(1.0);
        EXPECT_TRUE(arg.holds<double>());
        EXPECT_DOUBLE_EQ(get<double>(arg), 1.0);
    }

    {
        papilio::format_arg fmt_arg("test");
        EXPECT_TRUE(fmt_arg.holds<utf::string_container>());
        EXPECT_FALSE(get<utf::string_container>(fmt_arg).has_ownership());

        EXPECT_EQ(get<std::size_t>(fmt_arg.attribute("length")), std::string("test").length());
        EXPECT_EQ(get<utf::codepoint>(fmt_arg.index(0)), U't');
        EXPECT_EQ(get<utf::codepoint>(fmt_arg.index(1)), U'e');
        EXPECT_EQ(get<utf::codepoint>(fmt_arg.index(2)), U's');
        EXPECT_EQ(get<utf::codepoint>(fmt_arg.index(3)), U't');
        EXPECT_EQ(get<utf::codepoint>(fmt_arg.index(4)), utf::codepoint());
    }

    {
        using namespace std::literals;
        format_arg fmt_arg("test"s);
        EXPECT_TRUE(fmt_arg.holds<utf::string_container>());
        EXPECT_TRUE(get<utf::string_container>(fmt_arg).has_ownership());
    }

    {
        using namespace std::literals;
        std::string s = "test"s;
        format_arg fmt_arg(s);
        EXPECT_TRUE(fmt_arg.holds<utf::string_container>());
        EXPECT_FALSE(get<utf::string_container>(fmt_arg).has_ownership());
    }

    {
        papilio::format_arg fmt_arg((const char*)u8"测试");
        EXPECT_TRUE(fmt_arg.holds<utf::string_container>());

        EXPECT_EQ(get<std::size_t>(fmt_arg.attribute("length")), 2);
        EXPECT_EQ(get<utf::codepoint>(fmt_arg.index(0)), U'测');
        EXPECT_EQ(get<utf::codepoint>(fmt_arg.index(1)), U'试');
        EXPECT_EQ(get<utf::codepoint>(fmt_arg.index(2)), utf::codepoint());
    }

    {
        papilio::format_arg fmt_arg("test");

        auto var = fmt_arg.as_variable();
        EXPECT_EQ(var.as<utf::string_container>(), "test");
    }

    {
        papilio::format_arg fmt_arg("long sentence for testing slicing");

        EXPECT_EQ(get<utf::string_container>(fmt_arg.index(slice(0, 4))), "long");
        EXPECT_EQ(get<utf::string_container>(fmt_arg.index(slice(-7, slice::npos))), "slicing");
        EXPECT_EQ(get<utf::string_container>(fmt_arg.index(slice(14, -16))), "for");
        EXPECT_EQ(get<utf::string_container>(fmt_arg.index(slice(-slice::npos, -20))), "long sentence");

        EXPECT_EQ(get<std::string>(fmt_arg.index(slice(0, 4))), "long");
        EXPECT_EQ(get<std::string_view>(fmt_arg.index(slice(0, 4))), "long");
    }
}

using map_type = std::map<std::string, std::string, std::less<>>;
namespace papilio
{
    template <>
    struct accessor<map_type>
    {
        using has_key = void;

        static format_arg get(const map_type& m, std::string_view k)
        {
            auto it = m.find(k);
            if(it == m.end())
                return format_arg();
            return utf::string_container(independent, it->second);
        }

        static format_arg get_attr(const map_type& m, const attribute_name& attr)
        {
            using namespace std::literals;
            if(attr == "size"sv)
                return m.size();
            else
                throw invalid_attribute(attr);
        }
    };
}
TEST(TestCore, FormatArgHandle)
{
    using namespace papilio;

    {
        static_assert(!detail::use_handle<int>);
        static_assert(!detail::use_handle<char>);
        static_assert(detail::use_handle<map_type>);

        static_assert(accessor_traits<map_type>::has_key());

        map_type m;
        m["1"] = "one";
        m["2"] = "two";

        EXPECT_EQ(get<std::size_t>(accessor_traits<map_type>::get_attr(m, "size")), 2);
        EXPECT_EQ(get<utf::string_container>(accessor_traits<map_type>::get_arg(m, "1")), "one");
        EXPECT_EQ(get<utf::string_container>(accessor_traits<map_type>::get_arg(m, "2")), "two");
        EXPECT_TRUE(accessor_traits<map_type>::get_arg(m, "3").empty());

        format_arg::handle h = m;
        EXPECT_EQ(get<std::size_t>(h.attribute("size")), 2);
        EXPECT_EQ(get<utf::string_container>(h.index("1")), "one");
        EXPECT_EQ(get<utf::string_container>(h.index("2")), "two");

        format_arg arg = m;
        EXPECT_EQ(get<std::size_t>(arg.attribute("size")), 2);
        EXPECT_EQ(get<utf::string_container>(arg.index("1")), "one");
        EXPECT_EQ(get<utf::string_container>(arg.index("2")), "two");
    }
}

TEST(TestCore, StaticFormatArgStore)
{
    using namespace papilio;

    static_assert(format_arg_store<static_format_arg_store<0, 0>>);
    static_assert(format_arg_store<static_format_arg_store<1, 0>>);
    static_assert(format_arg_store<static_format_arg_store<0, 1>>);
    static_assert(format_arg_store<static_format_arg_store<1, 1>>);
}
TEST(TestCore, MutableFormatArgStore)
{
    using namespace papilio;

    static_assert(format_arg_store<mutable_format_args>);

    {
        mutable_format_args store(1, "three"_a = 3, 2);

        EXPECT_EQ(store.size(), 2);
        EXPECT_EQ(store.named_size(), 1);

        EXPECT_EQ(get<int>(store[0]), 1);
        EXPECT_EQ(get<int>(store[1]), 2);
        EXPECT_EQ(get<int>(store["three"]), 3);

        store.clear();

        EXPECT_EQ(store.size(), 0);
        EXPECT_EQ(store.named_size(), 0);

        store.push('a', 'b', "c"_a = 'c', "d"_a = 'd');

        EXPECT_EQ(store.size(), 2);
        EXPECT_EQ(store.named_size(), 2);

        EXPECT_EQ(get<utf::codepoint>(store[0]), U'a');
        EXPECT_EQ(get<utf::codepoint>(store[1]), U'b');
        EXPECT_EQ(get<utf::codepoint>(store["c"]), U'c');
        EXPECT_EQ(get<utf::codepoint>(store["d"]), U'd');
    }
}
TEST(TestCore, DynamicFormatArgStore)
{
    using namespace papilio;

    static_assert(format_arg_store<dynamic_format_args>);

    {
        mutable_format_args underlying_store;
        dynamic_format_args dyn_store(underlying_store);

        EXPECT_EQ(&dyn_store.to_underlying(), &underlying_store);
    }
}
TEST(TestCore, FormatContext)
{
    using namespace papilio;

    {
        std::string result;
        mutable_format_args store;
        basic_format_context ctx(
            std::back_inserter(result),
            store
        );

        using context_traits = format_context_traits<decltype(ctx)>;
        EXPECT_EQ(&context_traits::get_store(ctx).to_underlying(), &store);

        context_traits::append(ctx, "1234");
        EXPECT_EQ(result, "1234");

        result.clear();
        context_traits::append(ctx, '1', 4);
        EXPECT_EQ(result, "1111");

        result.clear();
        context_traits::append(ctx, U'ä', 2);
        EXPECT_EQ(result, (const char*)u8"ää");
    }

    {
        std::string result;
        mutable_format_args store;
        basic_format_context ctx(
            std::back_inserter(result),
            store
        );
        dynamic_format_context dyn_ctx(ctx);

        using context_traits = format_context_traits<decltype(dyn_ctx)>;
        EXPECT_EQ(&context_traits::get_store(dyn_ctx).to_underlying(), &store);

        context_traits::append(dyn_ctx, "1234");
        EXPECT_EQ(result, "1234");

        result.clear();
        context_traits::append(dyn_ctx, '1', 4);
        EXPECT_EQ(result, "1111");

        result.clear();
        context_traits::append(dyn_ctx, U'ä', 2);
        EXPECT_EQ(result, (const char*)u8"ää");
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
